//#include <stdbool.h>

//#include <string.h>
#include "esp_common.h"
#include "coap.h"
#include "uart.h"
#include "wifi.h"

static char light = '0';

const uint16_t rsplen = 1500;
static char rsp[1500] = "";
void build_rsp(void);

//#include <stdio.h>
void ICACHE_FLASH_ATTR endpoint_setup(void)
{
    build_rsp();
}

static const coap_endpoint_path_t path_q = {1, {"q"}};
static const coap_endpoint_path_t path_cen = {1, {"cen"}};
static const coap_endpoint_path_t path_ping = {1, {"ping"}};

static ICACHE_FLASH_ATTR int handle_get_ping(coap_rw_buffer_t *scratch, 
                          const coap_packet_t *inpkt, 
                          coap_packet_t *outpkt, 
                          uint8_t id_hi, uint8_t id_lo)
{
    int ret = 0;
    ret = coap_make_response(scratch, outpkt,
	        (const uint8_t *)"disco!", strlen("disco!"),
                             id_hi, id_lo, 
                             &inpkt->tok, 
                             COAP_RSPCODE_CONTENT, 
                             COAP_CONTENTTYPE_TEXT_PLAIN);
    //rt_free(query);
    return ret;
}



static ICACHE_FLASH_ATTR uint8_t red(int color) {

   uint8_t red = (color & 0x00ff0000) >> 16;

   //return (color >> 16) & 0xFF;
   return red;
}

static ICACHE_FLASH_ATTR uint8_t green(int color) {
    uint8_t green = (color & 0x0000ff00) >> 8; 
return green;
//    return (color >> 8) & 0xFF;
}

static ICACHE_FLASH_ATTR uint8_t blue(int color) {
uint8_t blue = (color & 0x000000ff);
//  return color & 0xFF;
 return blue;
}
static ICACHE_FLASH_ATTR int handle_get_cen(coap_rw_buffer_t *scratch, 
                          const coap_packet_t *inpkt, 
                          coap_packet_t *outpkt, 
                          uint8_t id_hi, uint8_t id_lo)
{
    int ret = 0;
    uint32_t frame = 0;
    int r=0;
    int g=0;
    int b=0;
    char *pay=malloc(inpkt->payload.len);
    memcpy(pay, inpkt->payload.p,inpkt->payload.len);
    uint32_t color=atoi(pay);
    r=red(color);
    g=green(color);
    b=blue(color);
       
    uint8_t lights=8;
    uint8_t buffer[lights*3];
    int i;
    //ws2812 likes GRB order :-o
    for( i = 0; i < lights; i++ ){
        buffer[0+i*3] = g;
	buffer[1+i*3] = r;
	buffer[2+i*3] = b;
    }
    WS2812OutBuffer(buffer, lights*3); 
    ret = coap_make_response(scratch, outpkt,
			    (const uint8_t*) "leds", strlen("leds"),
                             id_hi, id_lo, 
                             &inpkt->tok, 
                             COAP_RSPCODE_CONTENT, 
                             COAP_CONTENTTYPE_TEXT_PLAIN);
    return ret;
}
static ICACHE_FLASH_ATTR int handle_get_q(coap_rw_buffer_t *scratch, 
                          const coap_packet_t *inpkt, 
                          coap_packet_t *outpkt, 
                          uint8_t id_hi, uint8_t id_lo)
{
    const coap_option_t *opt;
    uint8_t count;
    int ret = 0;

    char* query = malloc(32);
    
    if (NULL != (opt = coap_findOptions(inpkt, COAP_OPTION_URI_QUERY, &count))) 
    {
       int i; 
            memcpy(query, opt[0].buf.p, opt[0].buf.len);
            query[opt[0].buf.len] = '\0';
	    
	    printf("pay: %s\r\n", query);
	    char *ssid= strtok(query,"=");
	    char * pwd=strtok(NULL,"=");
	    printf("ssid---->%s\n",ssid );
	    printf("pwd---->%s\n",pwd );
	    configure(ssid,pwd);
    }
    ret = coap_make_response(scratch, outpkt,
                             (const uint8_t *)"task_exec", strlen("task_exec"), 
                             id_hi, id_lo, 
                             &inpkt->tok, 
                             COAP_RSPCODE_CONTENT, 
                             COAP_CONTENTTYPE_TEXT_PLAIN);
    return ret;
}
const coap_endpoint_t endpoints[] =
{
    
    {COAP_METHOD_GET, handle_get_q, &path_q, "ct=40"},
    {COAP_METHOD_GET, handle_get_cen, &path_cen, "ct=40"},
    {COAP_METHOD_GET, handle_get_ping, &path_ping, "ct=40"},
    {(coap_method_t)0, NULL, NULL, NULL}
};

void ICACHE_FLASH_ATTR build_rsp(void)
{
    uint16_t len = rsplen;
    const coap_endpoint_t *ep = endpoints;
    int i;

    len--; // Null-terminated string

    while (NULL != ep->handler)
    {
        if (NULL == ep->core_attr) {
            ep++;
            continue;
        }

        if (0 < strlen(rsp)) {
            strncat(rsp, ",", len);
            len--;
        }

        strncat(rsp, "<", len);
        len--;

        for (i = 0; i < ep->path->count; i++) {
            strncat(rsp, "/", len);
            len--;

            strncat(rsp, ep->path->elems[i], len);
            len -= strlen(ep->path->elems[i]);
        }

        strncat(rsp, ">;", len);
        len -= 2;

        strncat(rsp, ep->core_attr, len);
        len -= strlen(ep->core_attr);

        ep++;
    }
}

