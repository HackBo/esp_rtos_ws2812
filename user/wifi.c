/*
 * wifi.c
 *
 *  Created on: Dec 14, 2014
 *      Author: Baoshi
 */

#include "esp_common.h"
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



#define WIFI_APSSID	"TTYCAST"
#define WIFI_APPASSWORD	"12345678"
#define PLATFORM_DEBUG	true

static char macaddr[6];

void configure(char * ssid, char * pwd)
{
        char ssid_name[33];
	char password[33];
        struct station_config st_config;

	sprintf(ssid_name, "%s", ssid);
	sprintf(password,"%s",pwd);
	memset(st_config.ssid, 0, sizeof(st_config.ssid));
	memcpy(st_config.ssid, ssid_name, strlen(ssid_name));
	memset(st_config.password, 0, sizeof(st_config.password));
	memcpy(st_config.password, password, strlen(password));
	printf("STA config: SSID: %s, PASSWORD: %s\r\n",		st_config.ssid,st_config.password );
	printf("info: SSID: %s, PASSWORD: %s\r\n",ssid_name,password );
int ch;
	int ret = wifi_station_set_auto_connect(0);

	vTaskDelay( 100 );
        ret = wifi_set_opmode(NULL_MODE);
	

	vTaskDelay( 100 );
       //save_user_config(&user_config);
       wifi_set_opmode(STATION_MODE);
	
	vTaskDelay( 100 );
       wifi_station_set_config_current(&st_config);
	vTaskDelay( 100 );

       wifi_station_connect();
       wifi_station_dhcpc_start();
        struct station_config st;

	if(wifi_station_get_config(&st)) {
		printf("STA2 config: SSID: %s, PASSWORD: %s\r\n",		st.ssid,st.password );
	    struct ip_info ipinfo;
	    wifi_get_ip_info(STATION_IF, &ipinfo);
            printf("current ip%s\r\n",ipinfo.ip); 
	}
	
}
/*
static ICACHE_FLASH_ATTR int
save_user_config(user_config_t *config)
{
	config->valid = CONFIG_VALID;
    spi_flash_erase_sector(PRIV_PARAM_START_SEC);
    spi_flash_write((PRIV_PARAM_START_SEC) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)config, sizeof(user_config_t));
	return 0;
}


static ICACHE_FLASH_ATTR int
read_user_config(user_config_t *config)
{
	DBG ("size to read is %d\n\r", sizeof(user_config_t));
    spi_flash_read((PRIV_PARAM_START_SEC) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)config, sizeof(user_config_t));
	DBG ("valid flag is 0x%x\r\n", config->valid);
	if (config->valid != CONFIG_VALID)
	{
		config->thingspeak_key[0] = '\0';
		config->talkback_key[0] = '\0';
		return -1;
	}
	return 0;
}
*/
void wifi_event_cb(System_Event_t *evt) {
   static int serverinit=0;
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

void mode_info(){
	int CUR_MODE=wifi_get_opmode();
	switch(CUR_MODE){
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



void wifi_softap(void)
//void wifi(void *pvParameters)
{

	char debug[256];
   //sprintf(debug,"\n\nSDK version:%s\n\n", system_get_sdk_version());
	//uart1_puts(debug);
    
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
	    int set_result=wifi_set_ip_info(SOFTAP_IF, &ipinfo);
            //shell_printf("Set returned:%d", set_result );
	}


	if(wifi_get_opmode() != SOFTAP_MODE)
	{
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
	
	{
		wifi_softap_get_config(&apConfig);
//		sprintf(macaddress, MACSTR, MAC2STR(macaddr));

	/*	shell_printf("\n OPMODE: %u, SSID: %s, PASSWORD: %s, CHANNEL: %d, AUTHMODE: %d, MACADDRESS: %s\r\n",
					wifi_get_opmode(),
					apConfig.ssid,
					apConfig.password,
					apConfig.channel,
					apConfig.authmode,
					macaddress);*/

	}
		wifi_softap_dhcps_start();
//vTaskDelete(NULL);

}

void wifi_init(void)
{
 //xTaskCreate(wifi, "ap", 512, NULL, tskIDLE_PRIORITY+2, NULL);
    wifi_softap();
}
