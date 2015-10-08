#include "rgb.h"

#include "esp_common.h"
#include "coap.h"
#include "uart.h"
#include "wifi.h"
//#include "ws2812.h"
static char light = '0';

const uint16_t rsplen = 1500;
static char rsp[1500] = "";
void build_rsp(void);

//#include <stdio.h>
void ICACHE_FLASH_ATTR endpoint_setup(void) { build_rsp(); }
static ICACHE_FLASH_ATTR uint8_t red(int color) {
  uint8_t red = (color & 0x00ff0000) >> 16;
  return red;
}

static ICACHE_FLASH_ATTR uint8_t green(int color) {
  uint8_t green = (color & 0x0000ff00) >> 8;
  return green;
}

static ICACHE_FLASH_ATTR uint8_t blue(int color) {
  uint8_t blue = (color & 0x000000ff);
  return blue;
}
static const coap_endpoint_path_t path_setup = {1, {"setup"}};
static const coap_endpoint_path_t path_q = {1, {"q"}};

static const coap_endpoint_path_t path_ping = {1, {"ping"}};

static const coap_endpoint_path_t path_rgb = {1, {"rgb"}};
static const coap_endpoint_path_t path_rgbcolor = {1, {"rgbcolor"}};

static ICACHE_FLASH_ATTR int handle_get_ping(coap_rw_buffer_t *scratch,
                                             const coap_packet_t *inpkt,
                                             coap_packet_t *outpkt,
                                             uint8_t id_hi, uint8_t id_lo) {
  int ret = 0;
  ret = coap_make_response(scratch, outpkt, (const uint8_t *)"ack",
                           strlen("ack"), id_hi, id_lo, &inpkt->tok,
                           COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);

  return ret;
}

static ICACHE_FLASH_ATTR int handle_get_rgb(coap_rw_buffer_t *scratch,
                                            const coap_packet_t *inpkt,
                                            coap_packet_t *outpkt,
                                            uint8_t id_hi, uint8_t id_lo) {
  int ret = 0;
  // paint buffer
  WS2812OutBuffer(inpkt->payload.p, inpkt->payload.len);
  ret = coap_make_response(scratch, outpkt, (const uint8_t *)"LEDup",
                           strlen("LEDup"), id_hi, id_lo, &inpkt->tok,
                           COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
  return ret;
}
#define MAX_LEDS 144
static ICACHE_FLASH_ATTR int handle_get_rgbcolor(coap_rw_buffer_t *scratch,
                                                 const coap_packet_t *inpkt,
                                                 coap_packet_t *outpkt,
                                                 uint8_t id_hi, uint8_t id_lo) {
  static uint8_t *buffer[MAX_LEDS * sizeof(rgb)];
  static rgb *rgb_buffer = (rgb *)buffer;
  int ret;
  uint8_t i;
  unsigned char hex[3];

  int nleds = inpkt->payload.len / 6;

  hex[2] = '\0';
  memset(rgb_buffer, 0, MAX_LEDS * sizeof(rgb));

  for (i = 0; i < nleds; i++) {
    uint8_t *p = (uint8_t *)(inpkt->payload.p + i * 6);

    memcpy(hex, p, 2);
    rgb_buffer[i].r = strtol(hex, NULL, 16);

    memcpy(hex, p + 2, 2);
    rgb_buffer[i].g = strtol(hex, NULL, 16);

    memcpy(hex, p + 4, 2);
    rgb_buffer[i].b = strtol(hex, NULL, 16);
  }

  WS2812OutBuffer(rgb_buffer, nleds);

  sprintf((char *)buffer, "rgbcolor::nleds=%d", nleds);

  ret = coap_make_response(scratch, outpkt, (const uint8_t *)buffer,
                           strlen((const char *)buffer), id_hi, id_lo,
                           &inpkt->tok, COAP_RSPCODE_CONTENT,
                           COAP_CONTENTTYPE_TEXT_PLAIN);
  return ret;
}
static ICACHE_FLASH_ATTR int handle_get_setup(coap_rw_buffer_t *scratch,
                                              const coap_packet_t *inpkt,
                                              coap_packet_t *outpkt,
                                              uint8_t id_hi, uint8_t id_lo) {
  const coap_option_t *opt;
  uint8_t count;
  int ret = 0;

  char *pay = malloc(inpkt->payload.len);
  memcpy(pay, inpkt->payload.p, inpkt->payload.len);

  printf("pay: %s\r\n", pay);
  char *ssid = strtok(pay, "=");
  char *pwd = strtok(NULL, "=");
  printf("ssid---->%s\n", ssid);
  printf("pwd---->%s\n", pwd);

  // configure(ssid,pwd);

  ret = coap_make_response(scratch, outpkt, (const uint8_t *)"task_exec",
                           strlen("task_exec"), id_hi, id_lo, &inpkt->tok,
                           COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
  return ret;
}
static ICACHE_FLASH_ATTR int handle_get_q(coap_rw_buffer_t *scratch,
                                          const coap_packet_t *inpkt,
                                          coap_packet_t *outpkt, uint8_t id_hi,
                                          uint8_t id_lo) {
  const coap_option_t *opt;
  uint8_t count;
  int ret = 0;

  char *query = malloc(32);

  if (NULL != (opt = coap_findOptions(inpkt, COAP_OPTION_URI_QUERY, &count))) {
    int i;
    memcpy(query, opt[0].buf.p, opt[0].buf.len);
    query[opt[0].buf.len] = '\0';

    printf("pay: %s\r\n", query);
    char *ssid = strtok(query, "=");
    char *pwd = strtok(NULL, "=");
    printf("ssid---->%s\n", ssid);
    printf("pwd---->%s\n", pwd);
    // configure(ssid,pwd);
  }
  ret = coap_make_response(scratch, outpkt, (const uint8_t *)"task_exec",
                           strlen("task_exec"), id_hi, id_lo, &inpkt->tok,
                           COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
  return ret;
}
const coap_endpoint_t endpoints[] = {

    {COAP_METHOD_GET, handle_get_q, &path_q, "ct=40"},
    {COAP_METHOD_GET, handle_get_setup, &path_setup, "ct=40"},
    {COAP_METHOD_GET, handle_get_rgb, &path_rgb, "ct=40"},
    {COAP_METHOD_GET, handle_get_rgbcolor, &path_rgbcolor, "ct=40"},
    {COAP_METHOD_GET, handle_get_ping, &path_ping, "ct=40"},
    {(coap_method_t)0, NULL, NULL, NULL}};

void ICACHE_FLASH_ATTR build_rsp(void) {
  uint16_t len = rsplen;
  const coap_endpoint_t *ep = endpoints;
  int i;

  len--;  // Null-terminated string

  while (NULL != ep->handler) {
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
