#include "esp_common.h"
//#include "ws2812.h"
//#include "ets_sys.h"
#include "gpio.h"
#include "c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint32_t WSGPIO = 0;


//#define GPIO_OUTPUT_SET(gpio_no, bit_value) \
	gpio_output_set(bit_value<<gpio_no, ((~bit_value)&0x01)<<gpio_no, 1<<gpio_no,0)


//I just used a scope to figure out the right time periods.
/*unsigned long HSVtoHEX( float hue, float sat, float value )
{

	float pr = 0;
	float pg = 0;
	float pb = 0;

	short ora = 0;
	short og = 0;
	short ob = 0;

	float ro = fmod( hue * 6, 6. );

	float avg = 0;

	ro = fmod( ro + 6 + 1, 6 ); //Hue was 60* off...

	if( ro < 1 ) //yellow->red
	{
		pr = 1;
		pg = 1. - ro;
	} else if( ro < 2 )
	{
		pr = 1;
		pb = ro - 1.;
	} else if( ro < 3 )
	{
		pr = 3. - ro;
		pb = 1;
	} else if( ro < 4 )
	{
		pb = 1;
		pg = ro - 3;
	} else if( ro < 5 )
	{
		pb = 5 - ro;
		pg = 1;
	} else
	{
		pg = 1;
		pr = ro - 5;
	}

	//Actually, above math is backwards, oops!
	pr *= value;
	pg *= value;
	pb *= value;

	avg += pr;
	avg += pg;
	avg += pb;

	pr = pr * sat + avg * (1.-sat);
	pg = pg * sat + avg * (1.-sat);
	pb = pb * sat + avg * (1.-sat);

	ora = pr * 255;
	og = pb * 255;
	ob = pg * 255;

	if( ora < 0 ) ora = 0;
	if( ora > 255 ) ora = 255;
	if( og < 0 ) og = 0;
	if( og > 255 ) og = 255;
	if( ob < 0 ) ob = 0;
	if( ob > 255 ) ob = 255;

	return (ob<<16) | (og<<8) | ora;
}

*/

static void  ICACHE_FLASH_ATTR SEND_WS_0()
{
	uint8_t time;
	time = 4; while(time--) GPIO_REG_WRITE( GPIO_ID_PIN(WSGPIO), 1 );
	time = 9; while(time--) GPIO_REG_WRITE( GPIO_ID_PIN(WSGPIO), 0 );

}

static void ICACHE_FLASH_ATTR SEND_WS_1()
{
	uint8_t time; 
        time = 8; while(time--) GPIO_REG_WRITE( GPIO_ID_PIN(WSGPIO), 1 );
	time = 6; while(time--) GPIO_REG_WRITE( GPIO_ID_PIN(WSGPIO), 0 );

}

static void ICACHE_FLASH_ATTR send_ws_0(uint8_t gpio){
     uint8_t i;
     i = 4; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
     i = 9; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}
static void ICACHE_FLASH_ATTR send_ws_1(uint8_t gpio){
uint8_t i;
    i = 8; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
    i = 6; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}

 ICACHE_FLASH_ATTR void WS2812OutBuffer( uint8_t * buffer, uint16_t length )
{
	GPIO_OUTPUT_SET(GPIO_ID_PIN(WSGPIO), 0);
	vTaskDelay(10);
        taskENTER_CRITICAL();
        GPIO_REG_WRITE(8, 1<<WSGPIO );
	const uint8_t * const end= buffer + length;
	while( buffer!=end )
	{
		uint8_t mask = 0x80;
	        char c= *buffer;
		uint8_t byte= c-'0';
		while (mask) 
		{
			if( byte & mask ) send_ws_1(WSGPIO); else send_ws_0(WSGPIO);
			mask >>= 1;
                }
                ++buffer; 
	}
        taskEXIT_CRITICAL();

}


void  ICACHE_FLASH_ATTR WS2812OutBuffer2( uint8_t * buffer, uint16_t length )
{
	uint16_t i;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(WSGPIO), 0);
	//vTaskDelay(1);
taskENTER_CRITICAL();
	for( i = 0; i < length; i++ )
	{
		uint8_t mask = 0x80;
		uint8_t byte = buffer[i];
                //char c= *buffer;
	        //uint8_t byte= c-'0';
                //buffer++; 
		while (mask) 
		{
			if( byte & mask ) SEND_WS_1(); else SEND_WS_0();
			mask >>= 1;
        }
	}
taskEXIT_CRITICAL();

}


