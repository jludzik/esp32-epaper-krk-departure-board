#ifndef _EPAPER_LAYOUT_H_
#define _EPAPER_LAYOUT_H_

#include <stdint.h>


//--------- TITLE --------------
#define MAX_TITLE_LEN 24

//--------- HEADER --------------
#define MAX_HEADER_LEFT_LEN 6
#define MAX_HEADER_MAIN_LEN 24
#define MAX_HEADER_RIGHT_LEN 7

#define Y_START_HEADER 50

#define X_SECTION_H_0 6
#define X_SECTION_H_1 126
#define X_SECTION_H_2 306

//--------- CONTENT --------------
#define MAX_CONTENT_MAIN_LEN 24     //23+'\0'
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

//--------- GLOBAL --------------
#define X_L_MARGIN 4
#define X_R_MARGIN 396


typedef enum {
    LAYOUT_OK = 0x00,
    LAYOUT_ERR_NO_MEMORY = 0x01,
    LAYOUT_ERR_MEMORY_ALREADY_ALOCATED = 0x02,
    LAYOUT_ERR_UNINITIALIZED = 0x03,
    LAYOUT_ERR_INPUT = 0x04
} layout_status_t;

typedef struct {
    char main_text[MAX_CONTENT_MAIN_LEN];
    char left_text[MAX_CONTENT_LEFT_LEN];
    char right_text[MAX_CONTENT_RIGHT_LEN];
} layout_content_t;

typedef struct {
    char main_text[MAX_CONTENT_MAIN_LEN];
    char left_text[MAX_HEADER_LEFT_LEN];
    char right_text[MAX_HEADER_RIGHT_LEN];
} layout_header_t;

layout_status_t layout_init(void);

layout_status_t layout_set_title(const char* text);

layout_status_t layout_set_content(layout_content_t* content, uint8_t pos);

layout_status_t layout_push_to_screen(void);

layout_status_t layout_set_header(const layout_header_t* header);

layout_status_t layout_clear_content(void);

#endif