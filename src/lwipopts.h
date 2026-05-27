// lwIP configuration for the Word Clock.
// Enables only the stack components required for DHCP + DNS + SNTP.

#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// ── Threading ─────────────────────────────────────────────────────────────────
#define NO_SYS                      1   // bare-metal, no RTOS
#define SYS_LIGHTWEIGHT_PROT        1

// ── Disabled socket / netconn APIs ────────────────────────────────────────────
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0

// ── Transport protocols ───────────────────────────────────────────────────────
#define LWIP_TCP                    0   // not needed
#define LWIP_UDP                    1   // SNTP, DHCP, DNS all use UDP
#define LWIP_RAW                    0
#define LWIP_ICMP                   0

// ── Network layer ─────────────────────────────────────────────────────────────
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0
#define LWIP_ARP                    1
#define LWIP_DHCP                   1
#define LWIP_DNS                    1   // resolve NTP server hostname
#define DNS_TABLE_SIZE              4
#define DNS_MAX_SERVERS             2

// ── SNTP ──────────────────────────────────────────────────────────────────────
#define LWIP_SNTP                   1
#define SNTP_SERVER_DNS             1   // allow hostname (not just IP)
#define SNTP_MAX_SERVERS            4   // pool servers 0–3.pool.ntp.org
#define SNTP_UPDATE_DELAY           3600000  // resync every hour (ms)
#define SNTP_STARTUP_DELAY          0        // first sync immediately

// The default SNTP_SET_SYSTEM_TIME is a no-op (LWIP_UNUSED_ARG).
// Override it here to call our callback.  The value passed is already
// Unix time (seconds since 1970), converted inside lwIP via DIFF_SEC_1970_2036.
// The forward declaration is only injected for C translation units (sntp.c);
// in C++ the extern "C" definition in time_manager.cpp is its own declaration.
#ifndef __cplusplus
void sntp_set_system_time(unsigned int sec);
#endif
#define SNTP_SET_SYSTEM_TIME(sec)   sntp_set_system_time(sec)

// ── Memory ────────────────────────────────────────────────────────────────────
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4096
#define PBUF_POOL_SIZE              6
#define MEMP_NUM_UDP_PCB            4
#define MEMP_NUM_NETBUF             0
#define MEMP_NUM_NETCONN            0
#define MEMP_NUM_TCP_PCB            0
#define MEMP_NUM_TCP_PCB_LISTEN     0
// CYW43 driver + DHCP + DNS + SNTP each register timers at startup;
// the default pool (typically 4-6) is too small — 17 is the pico-w recommendation.
#define MEMP_NUM_SYS_TIMEOUT        17

// ── Misc ──────────────────────────────────────────────────────────────────────
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_STATS                  0
#define LWIP_STATS_DISPLAY          0
#define LWIP_CHKSUM_ALGORITHM       3

// ── Debug (SNTP and DNS only; everything else suppressed) ─────────────────────
#define LWIP_DEBUG                  1
#define SNTP_DEBUG                  LWIP_DBG_ON
#define DNS_DEBUG                   LWIP_DBG_ON
#define DHCP_DEBUG                  LWIP_DBG_OFF
#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define IGMP_DEBUG                  LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TIMERS_DEBUG                LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF

#endif // _LWIPOPTS_H
