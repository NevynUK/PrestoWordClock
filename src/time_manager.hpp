/**
 * @file time_manager.hpp
 * @brief WiFi connection and NTP time synchronisation API.
 */
#pragma once

/**
 * @brief Callback invoked during initialisation to report status for
 *        on-screen display.
 *
 * @p msg is @c '\\n'-delimited: the first segment is a short title (rendered
 * large); each subsequent segment is a detail line (rendered small).
 */
typedef void (*TimeManagerStatusCb)(const char *msg);

/**
 * @brief Initialises the CYW43 WiFi chip, connects to the network defined in
 *        wifi.h, and starts the lwIP SNTP client.
 *
 * Cycles through 0–3.pool.ntp.org up to NTP_CYCLES times (20 attempts total),
 * calling @p status_cb on each attempt.  After the loop SNTP continues running
 * in the background with all 4 servers and resyncs automatically every hour.
 *
 * @param status_cb  Optional callback for on-screen status updates; may be
 *                   @c nullptr.
 */
void time_manager_init(TimeManagerStatusCb status_cb = nullptr);

/**
 * @brief Returns the current local time, adjusted by UTC_OFFSET_SECONDS.
 *
 * All values are 0 until the first NTP sync completes.
 *
 * @param[out] hour    Local hour (0–23).
 * @param[out] minute  Minute (0–59).
 * @param[out] second  Second (0–59).
 */
void time_manager_get(int &hour, int &minute, int &second);

/**
 * @brief Returns @c true once at least one NTP sync has been received.
 */
bool time_manager_is_synced();
