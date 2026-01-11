#include "contiki.h"
#include "rpl-border-router.h"
#include "rpl.h"
#include "rpl-dag-root.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-nbr.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "RPL BR"
#define LOG_LEVEL LOG_LEVEL_INFO

PROCESS(contiki_ng_br, "Contiki-NG Border Router");
AUTOSTART_PROCESSES(&contiki_ng_br);

PROCESS_THREAD(contiki_ng_br, ev, data)
{
  static struct etimer et;
  static uip_ipaddr_t known_neighbors[UIP_DS6_ADDR_NB];
  static int known_count = 0;

  PROCESS_BEGIN();

#if BORDER_ROUTER_CONF_WEBSERVER
  PROCESS_NAME(webserver_nogui_process);
  process_start(&webserver_nogui_process, NULL);
#endif

  LOG_INFO("Contiki-NG Border Router started\n");

  etimer_set(&et, CLOCK_SECOND * 3);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  rpl_border_router_init();
  LOG_INFO("RPL Border Router initialized\n");

  etimer_set(&et, CLOCK_SECOND * 2);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  if (!rpl_dag_root_is_root())
  {
    if (rpl_dag_root_start() == 0)
    {
      LOG_INFO("RPL DAG root started successfully\n");
    }
    else
    {
      LOG_ERR("Failed to start RPL DAG root\n");
    }
  }
  else
  {
    LOG_INFO("Already RPL root\n");
  }

  while (1)
  {
    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Itera sobre vizinhos usando a API correta */
    uip_ds6_nbr_t *nbr;
    for (nbr = uip_ds6_nbr_head(); nbr != NULL; nbr = uip_ds6_nbr_next(nbr))
    {
      int found = 0;
      for (int i = 0; i < known_count; i++)
      {
        if (uip_ipaddr_cmp(&nbr->ipaddr, &known_neighbors[i]))
        {
          found = 1;
          break;
        }
      }
      if (!found)
      {
        LOG_INFO("New mote connected: ");
        LOG_INFO_6ADDR(&nbr->ipaddr);
        LOG_INFO_("\n");
        if (known_count < UIP_DS6_ADDR_NB)
        {
          uip_ipaddr_copy(&known_neighbors[known_count++], &nbr->ipaddr);
        }
      }
    }

    /* Remove vizinhos que saíram */
    for (int i = 0; i < known_count; i++)
    {
      int still_exists = 0;
      for (nbr = uip_ds6_nbr_head(); nbr != NULL; nbr = uip_ds6_nbr_next(nbr))
      {
        if (uip_ipaddr_cmp(&known_neighbors[i], &nbr->ipaddr))
        {
          still_exists = 1;
          break;
        }
      }
      if (!still_exists)
      {
        LOG_INFO("Mote disconnected: ");
        LOG_INFO_6ADDR(&known_neighbors[i]);
        LOG_INFO_("\n");
        for (int j = i; j < known_count - 1; j++)
        {
          uip_ipaddr_copy(&known_neighbors[j], &known_neighbors[j + 1]);
        }
        known_count--;
        i--;
      }
    }
  }

  PROCESS_END();
}
