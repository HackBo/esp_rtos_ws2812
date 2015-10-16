#include "esp_common.h"
#include "rgb.h"
#include "gpio.h"
#include "c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint32_t WSGPIO = 0;

static uint32_t _getCycleCount(void) __attribute__((always_inline));
static inline uint32_t _getCycleCount(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount" : "=a"(ccount));
  return ccount;
}

#define F_CPU CPU_CLK_FREQ
#define CYCLES_800_T0H (F_CPU / 2500000)  // 0.4us
#define CYCLES_800_T1H (F_CPU / 1250000)  // 0.8us
#define CYCLES_ERROR (F_CPU / 5000000)    // 0.2us
#define CYCLES_800 (F_CPU / 800000)       // 1.25us per bit

#define MAX_LEDS 144

static int nleds = MAX_LEDS;
static uint8_t buffer_1[MAX_LEDS * sizeof(rgb)];
static uint8_t buffer_2[MAX_LEDS * sizeof(rgb)];
static bool tainted = false;

static rgb *rgb_buffer = (rgb *)buffer_1;

static inline rgb *swap_buffer() {
  if (rgb_buffer == (rgb *)buffer_1)
    rgb_buffer = (rgb *)buffer_2;
  else
    rgb_buffer = (rgb *)buffer_1;
  tainted = true;
  return rgb_buffer;
}

static bool ICACHE_FLASH_ATTR
send_pixels_bit_banging(uint8_t *pixels, uint32_t numBytes, uint8_t pin) {
  const uint32_t pinRegister = 1 << pin;
  uint8_t mask;
  uint8_t subpix;
  uint32_t cyclesStart;
  uint8_t *end = pixels + numBytes;

  //  ets_intr_lock();

  cyclesStart = _getCycleCount() - CYCLES_800;
  do {
    subpix = *pixels++;
    for (mask = 0x80; mask != 0; mask >>= 1) {
      uint32_t cyclesBit = ((subpix & mask)) ? CYCLES_800_T1H : CYCLES_800_T0H;
      uint32_t cyclesNext = cyclesStart;
      uint32_t delta;
      uint32_t elapsed;

      do {
        cyclesStart = _getCycleCount();
        elapsed = cyclesStart - cyclesNext;

        if (elapsed >= (CYCLES_800 + CYCLES_ERROR))
          goto end_send_pixels;
        else if (elapsed >= CYCLES_800)
          break;
      } while (true);

      GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinRegister);

      do {
        cyclesNext = _getCycleCount();
        elapsed = cyclesNext - cyclesStart;

        if (elapsed >= (cyclesBit + CYCLES_ERROR))
          goto end_send_pixels;
        else if (elapsed >= cyclesBit)
          break;
      } while (true);

      GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinRegister);
    }
  } while (pixels < end);

  return true;

end_send_pixels:
  //  ets_intr_unlock();
  return false;
}

ICACHE_FLASH_ATTR bool WS2812OutBuffer(rgb *buffer, uint16_t length) {
  bool res;
  taskENTER_CRITICAL();

  res = send_pixels_bit_banging((uint8_t *)buffer, 3 * length, WSGPIO);

  taskEXIT_CRITICAL();
  return res;
}

void ledControllerTask(void *pvParameters) {
  int i;

  memset(rgb_buffer, 0, MAX_LEDS * sizeof(rgb));
  //  for (i = 0; i < MAX_LEDS; i++) rgb_buffer[i].b = rgb_buffer[i].g = i;

  GPIO_OUTPUT_SET(GPIO_ID_PIN(WSGPIO), 0);

  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << WSGPIO);

  while (true) {
    if (tainted) tainted = !WS2812OutBuffer(rgb_buffer, nleds);
    vTaskDelay(1);
  }
}

int leds_write_hex(uint8_t *bytes, int len) {
  uint8_t i;
  static unsigned char hex[3];
  int leds = len / 6;

  hex[2] = '\0';
  memset(rgb_buffer, 0, MAX_LEDS * sizeof(rgb));

  for (i = 0; i < leds; i++) {
    uint8_t *p = (uint8_t *)(bytes + i * 6);

    memcpy(hex, p, 2);
    rgb_buffer[i].r = strtol(hex, NULL, 16);

    memcpy(hex, p + 2, 2);
    rgb_buffer[i].g = strtol(hex, NULL, 16);

    memcpy(hex, p + 4, 2);
    rgb_buffer[i].b = strtol(hex, NULL, 16);
  }
  tainted = true;
}

int leds_set_nleds(int n) {
  nleds = n < MAX_LEDS ? n : MAX_LEDS;
  tainted = true;
  return nleds;
}

int leds_set_shift(int shift) {
  /*
  static uint8_t buffer[MAX_LEDS * sizeof(rgb)];
  static rgb *rgb_buffer = (rgb *)buffer;

  0000123000 1
  0000012300 -2
  0001230000
  */
  tainted = true;
}

void leds_init(void) {
  xTaskCreate(ledControllerTask, "ledControllerTask", 512, NULL, 1000, NULL);
}
