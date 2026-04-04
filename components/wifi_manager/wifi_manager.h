#ifndef _WIFI_MANAGER_H_
#define _WIFI_MANAGER_H_

#include <stdbool.h>

#define WIFI_RESET_KEY 12

#define WIFI_CONNECTION_PIN "1234567"
#define WIFI_CONNECTION_SSID "krk-departure-board"

void wifi_reset_key_config(void);

bool wifi_manager_check_reset(void);

void wifi_manager_init_sta(void);

#endif