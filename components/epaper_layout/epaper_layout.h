#ifndef _EPAPER_LAYOUT_H_
#define _EPAPER_LAYOUT_H_

#include <stdint.h>

/** * @name Warning Box Settings 
 * Coordinates and sizes for the big warning message box.
 */
#define X_START_WARNING 50
#define Y_START_WARNING 100
#define X_WIDTH_WARNING 300
#define Y_HEIGHT_WARNING 100


/** * @name Title Settings 
 */
#define MAX_TITLE_LEN 24

/** * @name Header Settings 
 * Maximum text lengths and X/Y positions for the table columns (Line, Direction, Time).
 */
#define MAX_HEADER_LEFT_LEN 6
#define MAX_HEADER_MAIN_LEN 24
#define MAX_HEADER_RIGHT_LEN 7

#define Y_START_HEADER 50

#define X_SECTION_H_0 6
#define X_SECTION_H_1 126
#define X_SECTION_H_2 306


/** * @name Content Row Settings 
 * Settings for the actual departure data rows. 
 * MAX_CONTENT_ELEMENTS limits how many rows we can fit on the screen.
 */
#define MAX_CONTENT_MAIN_LEN 24
#define MAX_CONTENT_LEFT_LEN 4
#define MAX_CONTENT_RIGHT_LEN 10

#define MAX_CONTENT_ELEMENTS 5

#define Y_START_POS0 100
#define Y_START_POS1 140
#define Y_START_POS2 180
#define Y_START_POS3 220
#define Y_START_POS4 260

#define X_SECTION_0 6
#define X_SECTION_1 46
#define X_SECTION_2 326

#define EL_HEIGHT 20

/** * @name Global Display Settings 
 */
#define X_L_MARGIN 4
#define X_R_MARGIN 396

/**
 * @brief Return codes for the layout functions to know if drawing was successful.
 */
typedef enum {
    LAYOUT_OK = 0x00,
    LAYOUT_ERR_NO_MEMORY = 0x01,
    LAYOUT_ERR_MEMORY_ALREADY_ALOCATED = 0x02,
    LAYOUT_ERR_UNINITIALIZED = 0x03,
    LAYOUT_ERR_INPUT = 0x04
} layout_status_t;

/**
 * @brief Holds the text for a single row of departures.
 */
typedef struct {
    char main_text[MAX_CONTENT_MAIN_LEN];
    char left_text[MAX_CONTENT_LEFT_LEN];
    char right_text[MAX_CONTENT_RIGHT_LEN];
} layout_content_t;

/**
 * @brief Holds the text for the table header (column names).
 */
typedef struct {
    char main_text[MAX_CONTENT_MAIN_LEN];
    char left_text[MAX_HEADER_LEFT_LEN];
    char right_text[MAX_HEADER_RIGHT_LEN];
} layout_header_t;

/**
 * @brief Allocates memory for the image buffer and wakes up the e-paper display.
 * Must be called before drawing anything.
 * @return LAYOUT_OK on success.
 */
layout_status_t layout_init(void);

/**
 * @brief Draws the main title bar at the top of the screen.
 * @param text The title to display.
 * @return LAYOUT_OK on success.
 */
layout_status_t layout_set_title(const char* text);

/**
 * @brief Draws a single row of departure data.
 * @param content Pointer to the struct with text to draw.
 * @param pos Row number on the screen (from 0 to 4).
 * @return LAYOUT_OK on success.
 */
layout_status_t layout_set_content(layout_content_t* content, uint8_t pos);

/**
 * @brief Sends the painted image buffer to the physical e-paper screen.
 * It uses the fast refresh mode to update the display quickly.
 * @return LAYOUT_OK on success.
 */
layout_status_t layout_push_to_screen(void);

/**
 * @brief Draws the column names right below the title.
 * @param header Pointer to the struct with header names.
 * @return LAYOUT_OK on success.
 */
layout_status_t layout_set_header(const layout_header_t* header);

/**
 * @brief Wipes only the departure rows from the image buffer.
 * It keeps the title and header untouched, so we don't have to redraw them.
 * @return LAYOUT_OK on success.
 */
layout_status_t layout_clear_content(void);

/**
 * @brief Fills the entire image buffer with white color.
 * @return LAYOUT_OK on success.
 */
layout_status_t layout_set_clear_screen(void);

/**
 * @brief Draws a large warning box in the middle of the screen.
 * Used to show critical errors like missing Wi-Fi credentials.
 * @param text The warning message to show.
 * @return LAYOUT_OK on success.
 */
layout_status_t layout_set_warning_screen(const char* text);

#endif