#include "mpk_api.h"
#include "mpk_api_credentials.h"

#include <string.h>
#include <stdlib.h>

// ---------------- ESP32 CONFIG -------------
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

// ------------------- DEBUG ----------------
#include "debug.h"

//extern const char mpk_cert_pem_start[] asm("_binary_mpk_cert_pem_start");
//extern const char mpk_cert_pem_end[]   asm("_binary_mpk_cert_pem_end");

static char* recv_buffer = NULL;

mpk_api_status_t mpk_api_init(void)
{
    if(recv_buffer != NULL)
    {
        Debug("recv_buffer memory already alocated");
        return MPK_API_ERR_MEMORY_ALREADY_ALOCATED;
    }

    recv_buffer = (char*)malloc(MAX_HTTP_RECV_BUFFER);
    if(recv_buffer == NULL)
    {
        Debug("No enough memory on heap for recv_buffer");
        return MPK_API_ERR_NO_MEMORY;
    }

    Debug("Succesfully alocated %d of memory on heap for recv_buf", MAX_HTTP_RECV_BUFFER);
    return MPK_API_OK;
}

mpk_api_status_t mpk_api_get_departure_data(mpk_api_departure_t* dep_out)
{
    if(recv_buffer == NULL)
    {
        Debug("recv_buff not exzist");
        return MPK_API_ERR_UNINITIALIZED;
    }

    memset(recv_buffer,0,MAX_HTTP_RECV_BUFFER);

    Debug("KROK 2: Konfiguracja klienta HTTP...");
    esp_http_client_config_t http_config = {
        .url = MPK_API_URL,
        .method = HTTP_METHOD_GET,
        //.crt_bundle_attach = esp_crt_bundle_attach,
        //.cert_pem = mpk_cert_pem_start,
        .timeout_ms = 10000
    };

    Debug("KROK 3: Inicjalizacja klienta...");
    esp_http_client_handle_t http_client_handle = esp_http_client_init(&http_config);
    if(http_client_handle == NULL)
    {
        Debug("HTTP Client init error");
        return MPK_API_ERR_HTTP;
    }

    Debug("KROK 4: Otwieranie polaczenia (Handshake SSL) - to moze chwile potrwac...");
    mpk_api_status_t http_status = MPK_API_ERR_HTTP;
    esp_err_t esp_http_status = esp_http_client_open(http_client_handle, 0);

    if(esp_http_status != ESP_OK)
    {
        esp_http_client_cleanup(http_client_handle);
        return MPK_API_ERR_HTTP;
    }

    Debug("KROK 5: Polaczono! Pobieranie naglowkow...");
    esp_http_client_fetch_headers(http_client_handle);
    
    int32_t total_read_len = 0;
    int32_t read_len = 0;

    while(1)
    {
        read_len = esp_http_client_read(http_client_handle,recv_buffer + total_read_len, MAX_HTTP_RECV_BUFFER - total_read_len - 1);

        if(read_len <= 0) break;
        total_read_len += read_len;
    }
    recv_buffer[total_read_len] = '\0';

    mpk_api_status_t api_status = MPK_API_OK;

    if(total_read_len > 0)
    {
        cJSON* json_root = cJSON_Parse(recv_buffer);
        if(json_root == NULL)
        {
            esp_http_client_cleanup(http_client_handle);
            return MPK_API_ERR_JSON_PARSE;
        }
        
        cJSON* actual_array = cJSON_GetObjectItem(json_root, "actual");
        if(!cJSON_IsArray(actual_array) || (cJSON_GetArraySize(actual_array) == 0))
        {
            esp_http_client_cleanup(http_client_handle);
            cJSON_Delete(json_root);
            return MPK_API_ERR_JSON_ARRAY_EMPTY;
        }

        cJSON* first_bus = cJSON_GetArrayItem(actual_array,0);

        cJSON* bus_line = cJSON_GetObjectItem(first_bus, "patternText");
        cJSON* bus_direction = cJSON_GetObjectItem(first_bus, "direction");
        cJSON* bus_dep_time_live = cJSON_GetObjectItem(first_bus, "actualTime");
        cJSON* bus_dep_time_planned = cJSON_GetObjectItem(first_bus, "plannedTime");
        cJSON* bus_actualRelativeTime = cJSON_GetObjectItem(first_bus, "actualRelativeTime");
        cJSON* bus_status = cJSON_GetObjectItem(first_bus, "status");

        if(cJSON_IsString(bus_line)) strncpy(dep_out->line,bus_line->valuestring,sizeof(dep_out->line)-1);
        else api_status = MPK_API_ERR_JSON_PARSE;
        if(cJSON_IsString(bus_direction)) strncpy(dep_out->direction,bus_direction->valuestring,sizeof(dep_out->direction)-1);
        else api_status = MPK_API_ERR_JSON_PARSE;
        if(cJSON_IsString(bus_dep_time_live)) strncpy(dep_out->dep_time_live,bus_dep_time_live->valuestring,sizeof(dep_out->dep_time_live)-1);
        else api_status = MPK_API_ERR_JSON_PARSE;
        if(cJSON_IsString(bus_dep_time_planned)) strncpy(dep_out->dep_time_planned,bus_dep_time_planned->valuestring,sizeof(dep_out->dep_time_planned)-1);
        else api_status = MPK_API_ERR_JSON_PARSE;
        if(cJSON_IsNumber(bus_actualRelativeTime)) dep_out->sec_left_live = bus_actualRelativeTime->valueint;
        else api_status = MPK_API_ERR_JSON_PARSE;
        if(cJSON_IsString(bus_status)) strncpy(dep_out->status,bus_status->valuestring,sizeof(dep_out->status)-1);
        else api_status = MPK_API_ERR_JSON_PARSE;

        cJSON_Delete(json_root);
    }
    else
    {
        Debug("Nothing read");
        api_status = MPK_API_ERR_NOTHING_READ;
    }

    esp_http_client_cleanup(http_client_handle);
    return api_status;
}
