#pragma once

/* ===================== LOG ===================== */
#define LOG_CONF_LEVEL_APP LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_RPL LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_IPV6 LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_MAC LOG_LEVEL_INFO

/* ===================== IEEE 802.15.4 ===================== */
#define IEEE802154_CONF_DEFAULT_CHANNEL 26
#define IEEE802154_CONF_PANID 0xABCD

/* ===================== IPv6 / RPL ===================== */
#define NETSTACK_CONF_WITH_IPV6 1

/* Nó normal (NÃO é root) */
#define RPL_CONF_ROOT 0

/* Endereçamento estável */
#define UIP_CONF_IPV6_AUTOCONF 1

/* ===================== CoAP ===================== */
#define COAP_MAX_OPEN_TRANSACTIONS 4
