#pragma once

// Callback invoked during init to report status for on-screen display.
// msg is '\n'-delimited: first segment is a short title (rendered large),
// subsequent segments are detail lines (rendered small).
typedef void (*TimeManagerStatusCb)(const char* msg);

// Initialises the CYW43 WiFi chip, connects to the network defined in wifi.h,
// and starts the lwIP SNTP client.  Cycles through 0–3.pool.ntp.org up to
// NTP_CYCLES times (20 attempts total), calling status_cb on each attempt.
// After the loop SNTP continues running in the background with all 4 servers
// and resyncs automatically every hour.
void time_manager_init(TimeManagerStatusCb status_cb = nullptr);

// Returns the current UTC+offset hour (0-23), minute, and second.
// Values are 0 until the first NTP sync completes.
void time_manager_get(int &hour, int &minute, int &second);

// Returns true once at least one NTP sync has been received.
bool time_manager_is_synced();

