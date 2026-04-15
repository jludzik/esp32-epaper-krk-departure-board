#ifndef _NTP_CONNECT_H_
#define _NTP_CONNECT_H_

#include "esp_check.h"

// The address of the NTP time server
#define NTP_CONNECT_SRV_URL "tempus1.gum.gov.pl"

// Maximum number of attempts to check if the time was downloaded.
#define NTP_CONNECT_RETRY_MAX 15


/**
 * @brief Downloads the current time from the NTP server.
 * * @return ESP_OK if the time was successfully synchronized.
 * @return ESP_FAIL if it failed after the maximum number of retries.
 */
esp_err_t ntp_connect(void);

#endif