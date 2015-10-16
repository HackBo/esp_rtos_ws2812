/*
 * wifi.c
 *
 */

#include "esp_common.h"
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define WIFI_APSSID "LEDS"
#define WIFI_APPASSWORD "12345678"
#define PLATFORM_DEBUG true

static char macaddr[6];

void wifi_event_cb(System_Event_t *evt) {
  static int serverinit = 0;
  switch (evt->event_id) {
    case EVENT_SOFTAPMODE_STACONNECTED:
      printf("EVENT_SOFTAPMODE_STACONNECTED:\n");
      break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
      printf("EVENT_SOFTAPMODE_STADISCONNECTED:\n");
      break;
    case EVENT_STAMODE_CONNECTED:
      printf("EVENT_STAMODE_CONNECTED:\n");
      break;
    case EVENT_STAMODE_DISCONNECTED:
      printf("EVENT_STAMODE_DISCONNECTED:\n");
      break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
      printf("EVENT_STAMODE_AUTHMODE_CHANGE:\n");
      break;
    case EVENT_STAMODE_GOT_IP:
      printf("EVENT_STAMODE_GOT_IP:\n");
      break;
    case EVENT_MAX:
      printf("EVENT_MAX:\n");
      break;
    default:
      printf("Unknown event\n");
      break;
  }
}

void mode_info() {
  int CUR_MODE = wifi_get_opmode();
  switch (CUR_MODE) {
    case NULL_MODE:
      printf("NULL\n");
      break;
    case STATION_MODE:
      printf("mode selected: STATION\n");
      break;
    case SOFTAP_MODE:
      printf("mode selected SOFTAP\n");
      break;
    case STATIONAP_MODE:
      printf("mode selected STATIONAP\n");
      break;
    case MAX_MODE:
      printf("mode selected MAX\n");
      break;
  }
}

void wifi_softap(void *pvParameters) {
  char debug[256];

  struct softap_config apConfig;
  struct ip_info ipinfo;
  struct ip_info getinfo;
  char ssid[33];
  char password[33];
  char macaddress[17];
  char info[150];
  wifi_softap_dhcps_stop();
  {
    IP4_ADDR(&ipinfo.ip, 192, 168, 50, 1);
    IP4_ADDR(&ipinfo.gw, 192, 168, 50, 1);
    IP4_ADDR(&ipinfo.netmask, 255, 255, 255, 0);
    int set_result = wifi_set_ip_info(SOFTAP_IF, &ipinfo);
    // shell_printf("Set returned:%d", set_result );
  }

  if (wifi_get_opmode() != SOFTAP_MODE) {
    wifi_set_opmode(SOFTAP_MODE);
  }
  mode_info();
  wifi_get_macaddr(SOFTAP_IF, macaddr);
  wifi_softap_get_config(&apConfig);
  memset(apConfig.ssid, 0, sizeof(apConfig.ssid));
  sprintf(ssid, "%s", WIFI_APSSID);
  memcpy(apConfig.ssid, ssid, strlen(ssid));
  wifi_set_event_handler_cb(wifi_event_cb);
  {
    memset(apConfig.password, 0, sizeof(apConfig.password));
    sprintf(password, "%s", WIFI_APPASSWORD);
    memcpy(apConfig.password, password, strlen(password));
    apConfig.authmode = AUTH_WPA_WPA2_PSK;
    apConfig.channel = 7;
    apConfig.max_connection = 255;
    apConfig.ssid_hidden = 0;
    wifi_softap_set_config(&apConfig);
  }

  wifi_softap_get_config(&apConfig);
  wifi_softap_dhcps_start();
}

void wifi_init(void) { wifi_softap(NULL); }
