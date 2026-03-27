#ifndef _NTP_CONNECT_H_
#define _NTP_CONNECT_H_

#include "esp_check.h"

#define NTP_CONNECT_SRV_URL "tempus1.gum.gov.pl"
#define NTP_CONNECT_RETRY_MAX 15

esp_err_t ntp_connect(void);

#endif