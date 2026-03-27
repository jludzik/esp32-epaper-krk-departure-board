#include "wifi_manager.h"
#include "wifi_credentials.h" 

#include <string.h>

#include "debug.h"

// ---------------- ESP32 CONFIG -------------
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

volatile static uint8_t wifi_connected = false;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        Debug("Wi-Fi started, connecting: %s...", WIFI_SSID);
        //SP_LOGI(TAG, "Wi-Fi started, connecting: %s...", WIFI_SSID);
        esp_wifi_connect();
    }
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        Debug("Wi-Fi disconnected, retrying...");
        //ESP_LOGW(TAG, "Wi-Fi disconnected, retrying...");
        esp_wifi_connect();
    }
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        wifi_connected = true;
#ifdef DEBUG      
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        Debug("Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        //ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
#endif
    }
}

void wifi_manager_init_sta(void)
{
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    ESP_ERROR_CHECK(esp_wifi_start());

    while(!wifi_connected)
    {
        Debug(".");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}