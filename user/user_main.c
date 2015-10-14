#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*
 * This is entry point for user code
 */
void ICACHE_FLASH_ATTR user_init(void) {
ledControllerTask(0);
  wifi_init();
  coap_init();
  leds_init();
}
