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
#define CYCLES_ERROR (F_CPU / 5000000)   // 0.2us
#define CYCLES_800 (F_CPU / 800000)       // 1.25us per bit

static void ICACHE_FLASH_ATTR
send_pixels_800(uint8_t *pixels, uint32_t numBytes, uint8_t pin) {
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

end_send_pixels:
  //  ets_intr_unlock();
  return;
}

ICACHE_FLASH_ATTR void WS2812OutBuffer(rgb *buffer, uint16_t length) {
  taskENTER_CRITICAL();

  send_pixels_800((uint8_t *)buffer, 3 * length, WSGPIO);

  taskEXIT_CRITICAL();
}

#define MAX_LEDS 144
static uint8_t buffer[MAX_LEDS * sizeof(rgb)];
static rgb *rgb_buffer = (rgb *)buffer;

void ledControllerTask(void *pvParameters) {
  int i, j;
  int nleds = 35;

  memset(rgb_buffer, 0, MAX_LEDS * sizeof(rgb));
  //  for (i = 0; i < MAX_LEDS; i++) rgb_buffer[i].b = rgb_buffer[i].g = i;

  GPIO_OUTPUT_SET(GPIO_ID_PIN(WSGPIO), 0);

  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << WSGPIO);

  j = 0;
  while (true) {
    WS2812OutBuffer(rgb_buffer, nleds);
    vTaskDelay(1);
  }
}

int leds_write_hex(uint8_t *bytes, int len) {
  uint8_t i;
  static unsigned char hex[3];
  int nleds = len / 6;

  hex[2] = '\0';
  memset(rgb_buffer, 0, MAX_LEDS * sizeof(rgb));

  for (i = 0; i < nleds; i++) {
    uint8_t *p = (uint8_t *)(bytes + i * 6);

    memcpy(hex, p, 2);
    rgb_buffer[i].r = strtol(hex, NULL, 16);

    memcpy(hex, p + 2, 2);
    rgb_buffer[i].g = strtol(hex, NULL, 16);

    memcpy(hex, p + 4, 2);
    rgb_buffer[i].b = strtol(hex, NULL, 16);
  }
}

void leds_init(void) {
  xTaskCreate(ledControllerTask, "ledControllerTask", 512, NULL, 1000, NULL);
}
