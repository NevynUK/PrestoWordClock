#include "time_manager.hpp"
#include "wifi.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/sntp.h"
#include "lwip/netif.h"
#include "lwip/dns.h"

#include <stdio.h>
#include <string.h>

// ── NTP server pool ───────────────────────────────────────────────────────────

static const char* const NTP_SERVERS[] = {
    "0.pool.ntp.org",
    "1.pool.ntp.org",
    "2.pool.ntp.org",
    "3.pool.ntp.org",
};
static constexpr int NTP_SERVER_COUNT = 4;
static constexpr int NTP_CYCLES       = 5;               // 4 servers × 5 = 20 attempts
static constexpr int NTP_ATTEMPT_MS   = 4000;            // wait per attempt

// ── Shared state (written by SNTP callback, read by main core) ────────────────

// lwIP passes Unix time (seconds since 1970) to SNTP_SET_SYSTEM_TIME — it
// already applies the NTP→Unix conversion (DIFF_SEC_1970_2036) internally.
static volatile uint32_t s_unix_epoch_secs = 0;
static absolute_time_t   s_ntp_sync_time   = {0};

// Used only for the one-shot DNS diagnostic before the SNTP loop
static volatile bool s_dns_done = false;
static volatile bool s_dns_ok   = false;
static ip_addr_t     s_dns_addr;

// ── Callbacks ─────────────────────────────────────────────────────────────────

extern "C" void sntp_set_system_time(u32_t sec) {
    s_unix_epoch_secs = sec;
    s_ntp_sync_time   = get_absolute_time();
    printf("NTP: sync OK (Unix %lu)\n", (unsigned long)sec);
}

static void dns_diag_callback(const char* name, const ip_addr_t* addr, void* /*arg*/) {
    s_dns_ok   = (addr != nullptr);
    if (addr) s_dns_addr = *addr;
    s_dns_done = true;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static void print_network_config() {
    cyw43_arch_lwip_begin();
    const struct netif* iface = netif_list;
    if (iface) {
        printf("  IP:   %s\n", ip4addr_ntoa(netif_ip4_addr(iface)));
        printf("  GW:   %s\n", ip4addr_ntoa(netif_ip4_gw(iface)));
        printf("  Mask: %s\n", ip4addr_ntoa(netif_ip4_netmask(iface)));
    } else {
        printf("  (no netif found)\n");
    }
    for (int i = 0; i < DNS_MAX_SERVERS; ++i) {
        const ip_addr_t* a = dns_getserver(i);
        if (a && !ip_addr_isany(a))
            printf("  DNS[%d]: %s\n", i, ipaddr_ntoa(a));
    }
    cyw43_arch_lwip_end();
}

static void test_dns(const char* hostname) {
    printf("DNS: resolving '%s'...\n", hostname);
    s_dns_done = false;
    s_dns_ok   = false;

    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname(hostname, &s_dns_addr, dns_diag_callback, nullptr);
    cyw43_arch_lwip_end();

    if (err == ERR_OK) {
        printf("DNS: '%s' -> %s (cached)\n", hostname, ipaddr_ntoa(&s_dns_addr));
    } else if (err == ERR_INPROGRESS) {
        absolute_time_t dl = make_timeout_time_ms(8000);
        while (!s_dns_done && !time_reached(dl)) sleep_ms(50);
        if (s_dns_ok)
            printf("DNS: '%s' -> %s\n", hostname, ipaddr_ntoa(&s_dns_addr));
        else
            printf("DNS: '%s' FAILED — check DNS server / network connectivity\n", hostname);
    } else {
        printf("DNS: error %d for '%s'\n", (int)err, hostname);
    }
}

// ── Public API ────────────────────────────────────────────────────────────────

void time_manager_init(TimeManagerStatusCb status_cb) {
    if (status_cb) status_cb("WIFI\nConnecting...");
    printf("WiFi: initialising...\n");

    if (cyw43_arch_init()) {
        printf("WiFi: init failed\n");
        return;
    }
    cyw43_arch_enable_sta_mode();

    printf("WiFi: connecting to '%s'...\n", WIFI_SSID);
    int rc = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (rc) {
        printf("WiFi: connection failed (%d)\n", rc);
        return;
    }
    printf("WiFi: connected\n");
    print_network_config();

    // Verify DNS before entering the SNTP loop
    test_dns(NTP_SERVERS[0]);

    // ── Retry loop: cycle through servers NTP_CYCLES times ───────────────────
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    char status_buf[64];
    const int max_attempts = NTP_SERVER_COUNT * NTP_CYCLES;
    int attempt = 0;

    for (int cycle = 0; cycle < NTP_CYCLES && s_unix_epoch_secs == 0; ++cycle) {
        for (int i = 0; i < NTP_SERVER_COUNT && s_unix_epoch_secs == 0; ++i) {
            ++attempt;
            printf("NTP: trying %s (%d/%d)\n", NTP_SERVERS[i], attempt, max_attempts);

            snprintf(status_buf, sizeof(status_buf),
                     "NTP\n%s\nAttempt %d of %d",
                     NTP_SERVERS[i], attempt, max_attempts);
            if (status_cb) status_cb(status_buf);

            cyw43_arch_lwip_begin();
            sntp_stop();
            sntp_setservername(0, NTP_SERVERS[i]);
            sntp_init();
            cyw43_arch_lwip_end();

            absolute_time_t deadline = make_timeout_time_ms(NTP_ATTEMPT_MS);
            while (s_unix_epoch_secs == 0 && !time_reached(deadline)) {
                sleep_ms(50);
            }

            if (s_unix_epoch_secs != 0) {
                printf("NTP: synced from %s\n", NTP_SERVERS[i]);
            } else {
                printf("NTP: no response from %s\n", NTP_SERVERS[i]);
            }
        }
    }

    // ── Set up all 4 servers for ongoing hourly resyncs ───────────────────────
    cyw43_arch_lwip_begin();
    sntp_stop();
    for (int i = 0; i < NTP_SERVER_COUNT; ++i) {
        sntp_setservername(i, NTP_SERVERS[i]);
    }
    sntp_init();
    cyw43_arch_lwip_end();

    if (s_unix_epoch_secs == 0) {
        printf("NTP: all %d attempts failed — will retry in background\n", max_attempts);
    }
}

bool time_manager_is_synced() {
    return s_unix_epoch_secs != 0;
}

void time_manager_get(int &hour, int &minute, int &second) {
    if (s_unix_epoch_secs == 0) {
        hour = 0; minute = 0; second = 0;
        return;
    }

    // s_unix_epoch_secs is seconds since 1970 (already converted by lwIP).
    uint64_t elapsed_s =
        absolute_time_diff_us(s_ntp_sync_time, get_absolute_time()) / 1000000ULL;

    uint64_t unix_secs =
        static_cast<uint64_t>(s_unix_epoch_secs)
        + elapsed_s
        + static_cast<int64_t>(UTC_OFFSET_SECONDS);

    uint64_t day_s = unix_secs % (24ULL * 3600ULL);
    hour   = static_cast<int>(day_s / 3600ULL);
    minute = static_cast<int>((day_s % 3600ULL) / 60ULL);
    second = static_cast<int>(day_s % 60ULL);
}

