#ifndef _MPK_API_H_
#define _MPK_API_H_

#include <stdint.h>

#define MAX_HTTP_RECV_BUFFER 20*1024
#define MAX_DOWNLOADED_DEPARTURES 255
#define MAX_SAVE_DEPARTURES 10

typedef enum {
    MPK_API_STATE_UNKNOWN = 0x00,
    MPK_API_STATE_PREDICTED = 0x01,
    MPK_API_STATE_PLANNED = 0x02,
    MPK_API_STATE_DEPARTED = 0x03,
    MPK_API_STATE_STOPPING = 0x04
} mpk_api_state_t;
typedef enum {
    MPK_API_OK = 0x00,
    MPK_API_ERR_NO_MEMORY = 0x01,
    MPK_API_ERR_MEMORY_ALREADY_ALOCATED = 0x02,
    MPK_API_ERR_UNINITIALIZED = 0x03,
    MPK_API_ERR_HTTP = 0x04,
    MPK_API_ERR_NOTHING_READ = 0x05,
    MPK_API_ERR_JSON_PARSE = 0x06,
    MPK_API_ERR_JSON_ARRAY = 0x07,
    MPK_API_ERR_OVERFLOW = 0x08,
    MPK_API_ERR_INPUT = 0x09,
    MPK_API_ERR_CONV_TYPE = 0x0A,
    MPK_API_ERR_FETCH_HEADERS = 0x0B
} mpk_api_status_t;

typedef struct {
    char line[8];                   //patternText
    char direction[32];             //direction
    int16_t sec_left_live;          //actualRelativeTime
    //char time_live[8];            //actualTime
    //char time_scheduled[8];       //plannedTime
    mpk_api_state_t status;         //status
} mpk_api_departure_t;

mpk_api_status_t mpk_api_init(void);

mpk_api_status_t mpk_api_update_departure_buffer(void);

mpk_api_status_t mpk_api_get_departure(mpk_api_departure_t* dep_out, uint8_t dep_number);

mpk_api_status_t mpk_api_get_departure_save_count(uint8_t* dep_out_count);

mpk_api_state_t mpk_api_parse_state(char* status);

#endif