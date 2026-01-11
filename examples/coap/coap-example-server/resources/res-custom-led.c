/*
 * Custom LED Control Resource for CoAP
 */

#include "coap-engine.h"
#include "dev/leds.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "CustomLED"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Timer para piscar */
#include "sys/etimer.h"

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_handler(coap_message_t *request, coap_message_t *response,
                             uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* Recurso CoAP */
RESOURCE(res_custom_led,
         "title=\"LED Control\";rt=\"Actuator\"",
         res_get_handler,
         res_post_handler,
         res_post_handler,
         NULL);

/* Estado lógico (controle da aplicação) */
static uint8_t red_on = 0;
static uint8_t green_on = 0;

/* Estados de piscar */
#define BLINK_OFF 0
#define BLINK_ON 1

static uint8_t red_blink_state = BLINK_OFF;
static uint8_t green_blink_state = BLINK_OFF;

/* Frequência de piscar em ms (padrão 500ms) */
static uint16_t red_blink_freq = 500;
static uint16_t green_blink_freq = 500;

/* Process para controlar o piscar */
PROCESS(blink_process, "LED Blink Process");

PROCESS_THREAD(blink_process, ev, data)
{
  static struct etimer blink_timer;

  PROCESS_BEGIN();

  etimer_set(&blink_timer, CLOCK_SECOND / 2); /* Inicia com 500ms */

  while (1)
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&blink_timer));

    /* Controla piscar do LED vermelho */
    if (red_blink_state == BLINK_ON)
    {
      leds_toggle(LEDS_RED);
      etimer_set(&blink_timer, (red_blink_freq * CLOCK_SECOND) / 1000);
    }
    /* Controla piscar do LED verde */
    else if (green_blink_state == BLINK_ON)
    {
      leds_toggle(LEDS_GREEN);
      etimer_set(&blink_timer, (green_blink_freq * CLOCK_SECOND) / 1000);
    }
    else
    {
      etimer_set(&blink_timer, CLOCK_SECOND / 2);
    }
  }

  PROCESS_END();
}

static void
res_get_handler(coap_message_t *request, coap_message_t *response,
                uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int len = snprintf((char *)buffer, preferred_size,
                     "red=%s(%dms)\ngreen=%s(%dms)\n",
                     red_on ? "on" : (red_blink_state ? "blink" : "off"), red_blink_freq,
                     green_on ? "on" : (green_blink_state ? "blink" : "off"), green_blink_freq);

  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_payload(response, buffer, len);

  LOG_INFO("GET actuators/led\n");
}

static void
res_post_handler(coap_message_t *request, coap_message_t *response,
                 uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *payload;
  int len = coap_get_payload(request, &payload);

  if (len <= 0)
  {
    coap_set_status_code(response, BAD_REQUEST_4_00);
    return;
  }

  /* Copia segura + null-terminate */
  char cmd[32];
  len = len > 31 ? 31 : len;
  memcpy(cmd, payload, len);
  cmd[len] = '\0';

  LOG_INFO("CMD: %s\n", cmd);

  /* Parser para comandos com frequência: "red-blink:500" */
  char *colon = strchr(cmd, ':');
  uint16_t freq = 500; /* frequência padrão */

  if (colon != NULL)
  {
    *colon = '\0'; /* Separa comando da frequência */
    freq = atoi(colon + 1);
    if (freq < 100 || freq > 5000)
    { /* Limita entre 100ms e 5000ms */
      freq = 500;
    }
  }

  if (strcmp(cmd, "red-on") == 0)
  {
    leds_on(LEDS_RED);
    red_on = 1;
    red_blink_state = BLINK_OFF;
  }
  else if (strcmp(cmd, "red-off") == 0)
  {
    leds_off(LEDS_RED);
    red_on = 0;
    red_blink_state = BLINK_OFF;
  }
  else if (strcmp(cmd, "red-blink") == 0)
  {
    red_on = 0;
    red_blink_freq = freq;
    red_blink_state = BLINK_ON;
    leds_off(LEDS_RED);
    LOG_INFO("Red LED blink freq: %d ms\n", freq);
  }
  else if (strcmp(cmd, "green-on") == 0)
  {
    leds_on(LEDS_GREEN);
    green_on = 1;
    green_blink_state = BLINK_OFF;
  }
  else if (strcmp(cmd, "green-off") == 0)
  {
    leds_off(LEDS_GREEN);
    green_on = 0;
    green_blink_state = BLINK_OFF;
  }
  else if (strcmp(cmd, "green-blink") == 0)
  {
    green_on = 0;
    green_blink_freq = freq;
    green_blink_state = BLINK_ON;
    leds_off(LEDS_GREEN);
    LOG_INFO("Green LED blink freq: %d ms\n", freq);
  }
  else if (strcmp(cmd, "toggle") == 0)
  {
    leds_toggle(LEDS_RED);
    red_on = !red_on;
    red_blink_state = BLINK_OFF;
  }
  else
  {
    coap_set_status_code(response, BAD_REQUEST_4_00);
    const char *msg = "use: red-on | red-off | red-blink[:freq] | green-on | green-off | green-blink[:freq] | toggle";
    coap_set_payload(response, msg, strlen(msg));
    return;
  }

  coap_set_status_code(response, CHANGED_2_04);
}