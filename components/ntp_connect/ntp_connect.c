#include "ntp_connect.h"

#include <time.h>

#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "debug.h"

esp_err_t ntp_connect(void)
{
    Debug("SNTP init");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0,NTP_CONNECT_SRV_URL);
    esp_sntp_init();

    time_t now = 0;
    uint8_t retry = 0;
    struct tm timeinfo;
    memset(&timeinfo,0,sizeof(timeinfo));
    
    while(timeinfo.tm_year <= (2000 - 1900))
    {
        Debug(".");
        vTaskDelay(pdMS_TO_TICKS(2000));

        time(&now);
        localtime_r(&now, &timeinfo);
        
        retry++;
        if(retry >= NTP_CONNECT_RETRY_MAX)
        {
            Debug("ERROR with updating time");
            return ESP_FAIL;
        }
    }
#if DEBUG
    char time_buf[64];
    strftime(time_buf,sizeof(time_buf),"%c",&timeinfo);
    Debug("SNTP success! Time: %s",time_buf);
#endif
    return ESP_OK;
}