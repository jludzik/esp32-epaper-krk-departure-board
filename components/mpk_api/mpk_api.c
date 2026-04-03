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

    dep_download_count = 0;
    dep_save_count = 0;
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

    if(esp_http_client_open(http_client_handle, 0) != ESP_OK)
    {
        esp_http_client_cleanup(http_client_handle);
        Debug("HTTP connection failed");
        return MPK_API_ERR_HTTP;
    }
    Debug("HTTP connected");

    if(esp_http_client_fetch_headers(http_client_handle) == ESP_FAIL)
    {
        esp_http_client_cleanup(http_client_handle);
        Debug("HTTP fetch failed");
        return MPK_API_ERR_FETCH_HEADERS;
    }

    if(esp_http_client_get_status_code(http_client_handle) != 200)
    {
        Debug("HTTP status != 200");
        esp_http_client_cleanup(http_client_handle);
        return MPK_API_ERR_HTTP;
    }

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

        if(dep_download_count <= MAX_SAVE_DEPARTURES) dep_save_count = dep_download_count;
        else dep_save_count = MAX_SAVE_DEPARTURES;

        for(uint8_t i=0;i < dep_save_count;i++)
        {
            cJSON* dep_new = cJSON_GetArrayItem(actual_array,i);

            cJSON* dep_new_line = cJSON_GetObjectItem(dep_new,"patternText");
            cJSON* dep_new_direction = cJSON_GetObjectItem(dep_new, "direction");
            cJSON* dep_new_sec_left_live = cJSON_GetObjectItem(dep_new, "actualRelativeTime");
            cJSON* dep_new_status = cJSON_GetObjectItem(dep_new,"status");

            mpk_api_status_t dep_conv_info = MPK_API_OK;
            if(!cJSON_IsString(dep_new_line)) dep_conv_info = MPK_API_ERR_CONV_TYPE;
            if(!cJSON_IsString(dep_new_direction)) dep_conv_info = MPK_API_ERR_CONV_TYPE;
            if(!cJSON_IsNumber(dep_new_sec_left_live)) dep_conv_info = MPK_API_ERR_CONV_TYPE;
            if(!cJSON_IsString(dep_new_status)) dep_conv_info = MPK_API_ERR_CONV_TYPE;

            if(dep_conv_info != MPK_API_OK)
            {
                esp_http_client_cleanup(http_client_handle);
                cJSON_Delete(json_root);
                return dep_conv_info;
            }

            mpk_api_parse_line(dep_new_line->valuestring,dep_internal_buffer[i].line);
            mpk_api_parse_direction(dep_new_direction->valuestring, dep_internal_buffer[i].direction);
            mpk_api_parse_actualRelativeTime(dep_new_sec_left_live->valueint,dep_internal_buffer[i].sec_left_live);
            mpk_api_parse_state(dep_new_status->valuestring, &dep_internal_buffer[i].status);
            
            Debug("STATUS: %d",dep_internal_buffer[i].status);
        }
#ifdef DEBUG
            Debug("Zapisano %d busow",dep_save_count);
#endif

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

    *dep_out_count = dep_save_count;
    return MPK_API_OK;
}

static void remove_polish_diacritics(char* text)
{
    uint8_t read_i = 0;
    uint8_t write_i = 0;

    while(text[read_i] != '\0')
    {
        switch((unsigned char)text[read_i])
        {
            case (unsigned char)0xC3:
                if(((unsigned char)text[read_i+1]) == (unsigned char)0x93)         //Ó
                {
                    text[write_i] = 'O';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0xB3)    //ó
                {
                    text[write_i] = 'o';
                    write_i++;
                    read_i += 2;
                }
                else
                {
                    text[write_i] = text[read_i];
                    write_i++;
                    read_i++;
                }
            break;
            case (unsigned char)0xC4:
                if(((unsigned char)text[read_i+1]) == (unsigned char)0x84)         //Ą
                {
                    text[write_i] = 'A';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x85)    //ą
                {
                    text[write_i] = 'a';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x86)    //Ć
                {
                    text[write_i] = 'C';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x87)    //ć
                {
                    text[write_i] = 'c';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x98)    //Ę
                {
                    text[write_i] = 'E';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x99)    //ę
                {
                    text[write_i] = 'e';
                    write_i++;
                    read_i += 2;
                }
                else
                {
                    text[write_i] = text[read_i];
                    write_i++;
                    read_i++;
                }
            break;
            case (unsigned char)0xC5:
                if(((unsigned char)text[read_i+1]) == (unsigned char)0x81)         //Ł
                {
                    text[write_i] = 'L';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x82)         //ł
                {
                    text[write_i] = 'l';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x83)         //Ń
                {
                    text[write_i] = 'N';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x84)         //ń
                {
                    text[write_i] = 'n';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x9A)         //Ś
                {
                    text[write_i] = 'S';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0x9B)         //ś
                {
                    text[write_i] = 's';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0xB9)         //Ź
                {
                    text[write_i] = 'Z';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0xBA)         //ź
                {
                    text[write_i] = 'z';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0xBB)         //Ż
                {
                    text[write_i] = 'Z';
                    write_i++;
                    read_i += 2;
                }
                else if(((unsigned char)text[read_i+1]) == (unsigned char)0xBC)         //ż
                {
                    text[write_i] = 'z';
                    write_i++;
                    read_i += 2;
                }
                else
                {
                    text[write_i] = text[read_i];
                    write_i++;
                    read_i++;
                }
            break;
            default:
                text[write_i] = text[read_i];
                write_i++;
                read_i++;
            break;
        }
    }

    text[write_i] = '\0';
}

mpk_api_status_t mpk_api_parse_state(char* dep_status, mpk_api_state_t* dst_status)
{
    if(dep_status == NULL) return MPK_API_ERR_INPUT;

    if(strcmp(dep_status,"PLANNED") == 0)
    {
        Debug("PLANNED OK");
        *dst_status = MPK_API_STATE_PLANNED;
    }
    else if(strcmp(dep_status,"PREDICTED") == 0)
    {
        Debug("PREDICTED OK");
        *dst_status = MPK_API_STATE_PREDICTED;
    }
    else if(strcmp(dep_status,"DEPARTED") == 0)
    {
        Debug("DEPARTED OK");
        *dst_status = MPK_API_STATE_DEPARTED;
    }
    else if(strcmp(dep_status, "STOPPING") == 0)
    {
        Debug("STOPPING OK");
        *dst_status = MPK_API_STATE_STOPPING;
    }
    else
    {
        Debug("UNKNOWN OK");
        *dst_status = MPK_API_STATE_UNKNOWN;
    }

    return MPK_API_OK;
}

mpk_api_status_t mpk_api_parse_actualRelativeTime(int16_t dep_actualRelativeTimeSec, char* dst_sec_left_live)
{
    if(dst_sec_left_live == NULL) return MPK_API_ERR_INPUT;

    int16_t dep_actualRelativeTimeMin = dep_actualRelativeTimeSec / 60;

    snprintf(dst_sec_left_live, SEC_LEFT_LIVE_LEN, "%d min", dep_actualRelativeTimeMin);

    return MPK_API_OK;
}

mpk_api_status_t mpk_api_parse_direction(char* dep_direction, char* dst_direction)
{
    if(dep_direction == NULL) return MPK_API_ERR_INPUT;
    if(dst_direction == NULL) return MPK_API_ERR_INPUT;

    char temp_buf[TEMP_DIRECTION_BUF_SIZE];
    memset(temp_buf,'\0',TEMP_DIRECTION_BUF_SIZE);
    strncpy(temp_buf,dep_direction,TEMP_DIRECTION_BUF_SIZE-1);

    remove_polish_diacritics(temp_buf);

    if(strlen(temp_buf) >= DIRECTION_TEXT_LEN)
    {
       strncpy(dst_direction,temp_buf,DIRECTION_TEXT_LEN);
       dst_direction[DIRECTION_TEXT_LEN-2] = '.';
       dst_direction[DIRECTION_TEXT_LEN-1] = '\0';
    }
    else strncpy(dst_direction,temp_buf,DIRECTION_TEXT_LEN);
    
    return MPK_API_OK;
}

mpk_api_status_t mpk_api_parse_line(char* dep_line, char* dst_line)
{
    if(dep_line == NULL) return MPK_API_ERR_INPUT;
    if(dst_line == NULL) return MPK_API_ERR_INPUT;

    if(strlen(dep_line) >= LINE_TEXT_LEN)
    {
       snprintf(dst_line, LINE_TEXT_LEN, "%s", dep_line);
       dst_line[LINE_TEXT_LEN-2] = '.';
       dst_line[LINE_TEXT_LEN-1] = '\0';

    }
    else snprintf(dst_line, LINE_TEXT_LEN, "%s", dep_line);

    return MPK_API_OK;
}

