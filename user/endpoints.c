#include "rgb.h"
#include "esp_common.h"
#include "coap.h"
#include "uart.h"
#include "wifi.h"

const uint16_t rsplen = 1500;
static char rsp[1500] = "";
void build_rsp(void);

void ICACHE_FLASH_ATTR endpoint_setup(void) { build_rsp(); }

static const coap_endpoint_path_t path_ping = {1, {"ping"}};
static const coap_endpoint_path_t path_rgb = {1, {"rgb"}};
static const coap_endpoint_path_t path_nleds = {1, {"nleds"}};

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

static ICACHE_FLASH_ATTR int handle_set_rgb(coap_rw_buffer_t *scratch,
                                            const coap_packet_t *inpkt,
                                            coap_packet_t *outpkt,
                                            uint8_t id_hi, uint8_t id_lo) {
  static uint8_t buffer[32];
  int nleds = inpkt->payload.len / 6;

  leds_write_hex(inpkt->payload.p, inpkt->payload.len);

  sprintf((char *)buffer, "rgb::nleds=%d", nleds);

  return coap_make_response(scratch, outpkt, (const uint8_t *)buffer,
                            strlen((const char *)buffer), id_hi, id_lo,
                            &inpkt->tok, COAP_RSPCODE_CONTENT,
                            COAP_CONTENTTYPE_TEXT_PLAIN);
}

static ICACHE_FLASH_ATTR int handle_set_nleds(coap_rw_buffer_t *scratch,
                                              const coap_packet_t *inpkt,
                                              coap_packet_t *outpkt,
                                              uint8_t id_hi, uint8_t id_lo) {
  static uint8_t buffer[32];

  int nleds = strtol(inpkt->payload.p, NULL, 10);

  leds_set_nleds(nleds);

  sprintf((char *)buffer, "rgb::nleds=%d", nleds);

  return coap_make_response(scratch, outpkt, (const uint8_t *)buffer,
                            strlen((const char *)buffer), id_hi, id_lo,
                            &inpkt->tok, COAP_RSPCODE_CONTENT,
                            COAP_CONTENTTYPE_TEXT_PLAIN);
}

const coap_endpoint_t endpoints[] = {
    {COAP_METHOD_GET, handle_get_ping, &path_ping, "ct=40"},
    {COAP_METHOD_GET, handle_set_rgb, &path_rgb, "ct=40"},
    {COAP_METHOD_GET, handle_set_nleds, &path_nleds, "ct=40"},
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
