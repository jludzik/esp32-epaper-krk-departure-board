#ifndef _MPK_API_H_
#define _MPK_API_H_

#include <stdint.h>

typedef enum {
    MPK_API_OK = 0x00,
    MPK_API_ERR_NO_MEMORY = 0x01,
    MPK_API_ERR_MEMORY_ALREADY_ALOCATED = 0x02,
    MPK_API_ERR_UNINITIALIZED = 0x03,
    MPK_API_ERR_HTTP = 0x04,
    MPK_API_ERR_NOTHING_READ = 0x05,
    MPK_API_ERR_JSON_PARSE = 0x06,
    MPK_API_ERR_JSON_ARRAY_EMPTY = 0x07,
} mpk_api_status_t;

typedef struct {
    char line[8];                   //patternText
    char direction[32];             //direction
    int16_t sec_left_live;          //actualRelativeTime
    char dep_time_live[8];          //actualTime
    char dep_time_planned[8];       //plannedTime
    char status[16];                //status
} mpk_api_departure_t;

mpk_api_status_t mpk_api_init(void);

mpk_api_status_t mpk_api_get_departure_data(mpk_api_departure_t* dep_out);

#endif