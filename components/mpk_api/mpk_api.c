#include "mpk_api.h"
#include "mpk_api_credentials.h"

#include <string.h>
#include <stdlib.h>

/*
	Modified and adapted for ESP32 (ESP-IDF) by Jakub Łudzik
	https://github.com/jludzik
    Date: 21.03.2026
*/

// ---------------- ESP32 CONFIG -------------
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

// ------------------- DEBUG ----------------
#include "debug.h"

static char* recv_buffer = NULL;
static mpk_api_departure_t dep_internal_buffer[MAX_SAVE_DEPARTURES];
static int dep_download_count = 0;
static uint8_t dep_save_count = 0;

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

mpk_api_status_t mpk_api_update_departure_buffer(void)
{
    if(recv_buffer == NULL)
    {
        Debug("recv_buff not exsist");
        return MPK_API_ERR_UNINITIALIZED;
    }

    memset(dep_internal_buffer,0,sizeof(dep_internal_buffer));
    memset(recv_buffer,0,MAX_HTTP_RECV_BUFFER);

    esp_http_client_config_t http_config = {
        .url = MPK_API_URL,
        .method = HTTP_METHOD_GET,
        //.crt_bundle_attach = esp_crt_bundle_attach,
        //.cert_pem = mpk_cert_pem_start,
        .timeout_ms = 10000
    };

    esp_http_client_handle_t http_client_handle = esp_http_client_init(&http_config);
    if(http_client_handle == NULL)
    {
        Debug("HTTP Client init error");
        return MPK_API_ERR_HTTP;
    }

    esp_err_t esp_http_status = esp_http_client_open(http_client_handle, 0);
    if(esp_http_status != ESP_OK)
    {
        esp_http_client_cleanup(http_client_handle);
        Debug("HTTP connection failed");
        return MPK_API_ERR_HTTP;
    }

    Debug("HTTP connected");
    esp_http_client_fetch_headers(http_client_handle);

    int32_t recv_buffer_total_read_len = 0;
    int32_t recv_buffer_read_len = 0;
    while(1)
    {
        recv_buffer_read_len = esp_http_client_read(http_client_handle,recv_buffer + recv_buffer_total_read_len, MAX_HTTP_RECV_BUFFER - recv_buffer_total_read_len - 1);

        if(recv_buffer_read_len <= 0) break;
        recv_buffer_total_read_len += recv_buffer_read_len;
    }
    recv_buffer[recv_buffer_total_read_len] = '\0';

    if(recv_buffer_total_read_len > 0)
    {
        cJSON* json_root = cJSON_Parse(recv_buffer);
        if(json_root == NULL)
        {
            esp_http_client_cleanup(http_client_handle);
            return MPK_API_ERR_JSON_PARSE;
        }
        
        cJSON* actual_array = cJSON_GetObjectItem(json_root, "actual");
        if(!cJSON_IsArray(actual_array))
        {
            esp_http_client_cleanup(http_client_handle);
            cJSON_Delete(json_root);
            return MPK_API_ERR_JSON_ARRAY;
        }

        dep_download_count = cJSON_GetArraySize(actual_array);
        if((dep_download_count == 0) || (dep_download_count >= MAX_DOWNLOADED_DEPARTURES))
        {
            esp_http_client_cleanup(http_client_handle);
            cJSON_Delete(json_root);
            return MPK_API_ERR_JSON_ARRAY;
        }

        Debug("Pobrano: %d busow",dep_download_count);

        dep_save_count = 0;
        if(dep_download_count <= MAX_SAVE_DEPARTURES) dep_save_count = dep_download_count;
        else dep_save_count = MAX_SAVE_DEPARTURES;

        for(uint8_t i=0;i < dep_save_count;i++)
        {
            cJSON* dep_new = cJSON_GetArrayItem(actual_array,i);

            cJSON* dep_new_line = cJSON_GetObjectItem(dep_new,"patternText");
            cJSON* dep_new_direction = cJSON_GetObjectItem(dep_new, "direction");
            cJSON* dep_new_time_live = cJSON_GetObjectItem(dep_new, "actualTime");
            cJSON* dep_new_time_scheduled = cJSON_GetObjectItem(dep_new, "plannedTime");
            cJSON* dep_new_status = cJSON_GetObjectItem(dep_new,"status");
            cJSON* dep_new_sec_left_live = cJSON_GetObjectItem(dep_new, "actualRelativeTime");

            mpk_api_status_t dep_conv_status = MPK_API_OK;
            if(!cJSON_IsString(dep_new_line)) dep_conv_status = MPK_API_ERR_CONV_TYPE;
            if(!cJSON_IsString(dep_new_direction)) dep_conv_status = MPK_API_ERR_CONV_TYPE;
            if(!cJSON_IsString(dep_new_time_live)) dep_conv_status = MPK_API_ERR_CONV_TYPE;
            if(!cJSON_IsString(dep_new_time_scheduled)) dep_conv_status = MPK_API_ERR_CONV_TYPE;
            if(!cJSON_IsString(dep_new_status)) dep_conv_status = MPK_API_ERR_CONV_TYPE;
            if(!cJSON_IsNumber(dep_new_sec_left_live)) dep_conv_status = MPK_API_ERR_CONV_TYPE;

            if(dep_conv_status != MPK_API_OK)
            {
                esp_http_client_cleanup(http_client_handle);
                cJSON_Delete(json_root);
                return dep_conv_status;
            }

            strncpy(dep_internal_buffer[i].line, dep_new_line->valuestring, sizeof(dep_internal_buffer[i].line)-1);
            strncpy(dep_internal_buffer[i].direction, dep_new_direction->valuestring, sizeof(dep_internal_buffer[i].direction)-1);
            strncpy(dep_internal_buffer[i].time_live, dep_new_time_live->valuestring, sizeof(dep_internal_buffer[i].time_live)-1);
            strncpy(dep_internal_buffer[i].time_scheduled, dep_new_time_scheduled->valuestring, sizeof(dep_internal_buffer[i].time_scheduled)-1);
            strncpy(dep_internal_buffer[i].status, dep_new_status->valuestring, sizeof(dep_internal_buffer[i].status)-1);
            dep_internal_buffer[i].sec_left_live = dep_new_sec_left_live->valueint;
        }

        cJSON_Delete(json_root);
        esp_http_client_cleanup(http_client_handle);
        
        return MPK_API_OK;
    }
    else
    {
        Debug("Nothing read");
        esp_http_client_cleanup(http_client_handle);
        
        return MPK_API_ERR_NOTHING_READ;
    }
}

mpk_api_status_t mpk_api_get_departure(mpk_api_departure_t* dep_out, uint8_t dep_number)
{
    if(dep_out == NULL) return MPK_API_ERR_INPUT;
    if(recv_buffer == NULL) return MPK_API_ERR_UNINITIALIZED;
    if(dep_number >= dep_save_count) return MPK_API_ERR_INPUT;

    *dep_out = dep_internal_buffer[dep_number];
    return MPK_API_OK;
}

mpk_api_status_t mpk_api_get_departure_save_count(uint8_t* dep_out_count)
{
    if(recv_buffer == NULL) return MPK_API_ERR_UNINITIALIZED;
    if(dep_out_count == NULL) return MPK_API_ERR_INPUT;

    *dep_out_count = dep_download_count;
    return MPK_API_OK;
}