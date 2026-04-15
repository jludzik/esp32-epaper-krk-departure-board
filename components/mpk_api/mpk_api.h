#ifndef _MPK_API_H_
#define _MPK_API_H_

#include <stdint.h>

// Maximum size of the buffer for downloading HTTP data (20 KB)
#define MAX_HTTP_RECV_BUFFER 20*1024

// Temporary buffer size used when replacing Polish characters
#define TEMP_DIRECTION_BUF_SIZE 64

// Absolute limit of departures we can download at once to prevent errors
#define MAX_DOWNLOADED_DEPARTURES 255

// Maximum number of departures we actually save
#define MAX_SAVE_DEPARTURES 10

// Maximum text length for destination name (23 chars + null terminator)
#define DIRECTION_TEXT_LEN 24

// Maximum text length for line number (3 chars + null terminator)
#define LINE_TEXT_LEN 4

// Maximum text length for time left string
#define SEC_LEFT_LIVE_LEN 10

/**
 * @brief Enum representing the real-time status of the vehicle.
 */
typedef enum {
    MPK_API_STATE_UNKNOWN = 0x00,
    MPK_API_STATE_PREDICTED = 0x01,
    MPK_API_STATE_PLANNED = 0x02,
    MPK_API_STATE_DEPARTED = 0x03,
    MPK_API_STATE_STOPPING = 0x04
} mpk_api_state_t;

/**
 * @brief Enum representing all possible errors in the API module.
 */
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

/**
 * @brief Structure holding data for a single departure.
 */
typedef struct {
    char line[LINE_TEXT_LEN];                     //patternText
    char direction[DIRECTION_TEXT_LEN];           //direction
    char sec_left_live[SEC_LEFT_LIVE_LEN];        //actualRelativeTime
    mpk_api_state_t status;                       //status
} mpk_api_departure_t;

/**
 * @brief Allocates memory for the HTTP download buffer.
 * Must be called once before fetching any data.
 * @return MPK_API_OK if memory is allocated, or error code on failure.
 */
mpk_api_status_t mpk_api_init(void);

/**
 * @brief Connects to the MPK API, downloads JSON data, and parses it.
 * Overwrites the internal array with new departures.
 * @return MPK_API_OK if data is successfully downloaded and parsed.
 */
mpk_api_status_t mpk_api_update_departure_buffer(void);

/**
 * @brief Gets a single departure struct from the internal array.
 * @param[out] dep_out Pointer to the struct where data will be copied.
 * @param[in] dep_number Index of the departure (0 to MAX_SAVE_DEPARTURES - 1).
 * @return MPK_API_OK on success.
 */
mpk_api_status_t mpk_api_get_departure(mpk_api_departure_t* dep_out, uint8_t dep_number);

/**
 * @brief Gets the exact number of departures that were successfully downloaded and saved.
 * @param[out] dep_out_count Pointer to the variable that will hold the count.
 * @return MPK_API_OK on success.
 */
mpk_api_status_t mpk_api_get_departure_save_count(uint8_t* dep_out_count);

/**
 * @brief Converts the text status from JSON into an enum value.
 * @param[in] dep_status Raw text string from JSON.
 * @param[out] dst_status Pointer to the enum where result will be saved.
 * @return MPK_API_OK on success.
 */
mpk_api_status_t mpk_api_parse_state(char* dep_status, mpk_api_state_t* dst_status);

/**
 * @brief Converts seconds left into a readable minute string.
 * For example, converts 120 seconds into "2 min".
 * @param[in] dep_actualRelativeTimeSec Time left in seconds.
 * @param[out] dst_sec_left_live Buffer to store the final text.
 * @return MPK_API_OK on success.
 */
mpk_api_status_t mpk_api_parse_actualRelativeTime(int16_t dep_actualRelativeTimeSec, char* dst_sec_left_live);

/**
 * @brief Cleans up the destination text to fit on the screen.
 * Removes Polish characters 'ł' becomes 'l') and adds dots "..." 
 * if the text is too long for the buffer.
 * @param[in] dep_direction Raw destination text from API.
 * @param[out] dst_direction Buffer to store the cleaned text.
 * @return MPK_API_OK on success.
 */
mpk_api_status_t mpk_api_parse_direction(char* dep_direction, char* dst_direction);

/**
 * @brief Cleans up the line number text.
 * Trims the line number if it's too long to prevent memory issues.
 * @param[in] dep_line Raw line text from API.
 * @param[out] dst_line Buffer to store the safe text.
 * @return MPK_API_OK on success.
 */
mpk_api_status_t mpk_api_parse_line(char* dep_line, char* dst_line);

#endif