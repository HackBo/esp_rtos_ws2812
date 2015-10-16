#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void ICACHE_FLASH_ATTR user_init(void) {
  wifi_init();
  coap_init();
  leds_init();
}
