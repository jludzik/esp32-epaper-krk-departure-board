#ifndef _MPK_API_H_
#define _MPK_API_H_

#include <stdint.h>

#define MAX_DOWNLOADED_DEPARTURES 255
#define MAX_SAVE_DEPARTURES 15

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
    MPK_API_ERR_CONV_TYPE = 0x0A
} mpk_api_status_t;

typedef struct {
    char line[8];                   //patternText
    char direction[32];             //direction
    int16_t sec_left_live;          //actualRelativeTime
    char time_live[8];              //actualTime
    char time_scheduled[8];         //plannedTime
    char status[16];                //status
} mpk_api_departure_t;

mpk_api_status_t mpk_api_init(void);

mpk_api_status_t mpk_api_update_departure_buffer(void);

mpk_api_status_t mpk_api_get_departure(mpk_api_departure_t* dep_out, uint8_t dep_number);

mpk_api_status_t mpk_api_get_departure_save_count(uint8_t* dep_out_count);

#endif