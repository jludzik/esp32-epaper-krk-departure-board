#include "wifi_manager.h"
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
#include "driver/gpio.h"

// ---------------- PROVISIONING -------------
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"

volatile static uint8_t wifi_connected = false;

void wifi_reset_key_config(void)
{
    gpio_reset_pin(WIFI_RESET_KEY);
    gpio_set_direction(WIFI_RESET_KEY, GPIO_MODE_INPUT);
    gpio_set_pull_mode(WIFI_RESET_KEY,GPIO_PULLUP_ONLY);
}

void wifi_manager_check_reset(void)
{
    if(gpio_get_level(WIFI_RESET_KEY) == 0)
    {
        Debug("Erasing NVS and Wi-Fi settings...");
        nvs_flash_erase();
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        Debug("Wi-Fi started, connecting...");
        esp_wifi_connect();
    }
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        Debug("Wi-Fi disconnected, retrying...");
        wifi_connected = false;
        esp_wifi_connect();
    }
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        wifi_connected = true;
#ifdef DEBUG      
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        Debug("Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
#endif
    }
}

static void prov_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_PROV_EVENT) 
    {
        switch (event_id)
        {
            case WIFI_PROV_START:
                Debug("Provisioning started. Waiting for SoftAP app...");
            break;
            case WIFI_PROV_CRED_RECV: 
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                Debug("Received credentials from app! Trying to connect to SSID: %s", (const char *) wifi_sta_cfg->ssid);
            break;
            case WIFI_PROV_CRED_FAIL:
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                Debug("Invalid credentials provided in app! Reason: %s", (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wrong password" : "Network not found");
            break;
            case WIFI_PROV_CRED_SUCCESS:
                Debug("Credentials correct! Connected successfully.");
            break;
            case WIFI_PROV_END:
                wifi_prov_mgr_deinit();
                Debug("Provisioning finished and closed.");
            break;
            default:
            break;
        }
    }
}

void wifi_manager_init_sta(void)
{
    // -------------------------- NVS INIT ------------------------------
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // -------------------------- NETIF INIT -----------------------------
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

     // -------------------------- REGISTER NETWORK HANDLERS ------------
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL, NULL));

    // -------------------------- WIFI PROV ------------------------------
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool wifi_provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&wifi_provisioned));

    if(!wifi_provisioned) 
    {
        Debug("No credentials in memory! Starting own Wi-Fi for configuration.");
        
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA)); 
        
        const char *pop = WIFI_CONNECTION_PIN; 
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(WIFI_PROV_SECURITY_1, pop, WIFI_CONNECTION_SSID, NULL));
    } 
    else 
    {
       Debug("Found saved credentials. Starting normal connection.");
        
        wifi_prov_mgr_deinit();
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    while(!wifi_connected)
    {
        Debug(".");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}