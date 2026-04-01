#include "epaper_layout.h"

#include "DEV_Config.h"
#include "EPD_4in2_V2.h"
#include "Fonts/fonts.h"
#include "GUI_Paint.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static uint8_t* Image = NULL;

layout_status_t layout_init(void)
{
    if(Image != NULL) return LAYOUT_ERR_MEMORY_ALREADY_ALOCATED;

    Image = (uint8_t*)malloc(EPD_4IN2_V2_SIZE);
    if(Image == NULL) return LAYOUT_ERR_NO_MEMORY;

    EPD_4IN2_V2_Init();
    DEV_Delay_ms(2000);

    EPD_4IN2_V2_Clear();
    DEV_Delay_ms(2000);

    Paint_NewImage(Image,EPD_4IN2_V2_WIDTH,EPD_4IN2_V2_HEIGHT,0,WHITE);
    Paint_SelectImage(Image);
    Paint_Clear(WHITE);
    Paint_DrawRectangle(2,2,398,40,BLACK,DOT_PIXEL_2X2,DRAW_FILL_EMPTY);

    EPD_4IN2_V2_Display(Image);
    DEV_Delay_ms(2000);

    EPD_4IN2_V2_Sleep();

    return LAYOUT_OK;
}

layout_status_t layout_push_to_screen(void)
{
    if(Image == NULL) return LAYOUT_ERR_UNINITIALIZED;

    EPD_4IN2_V2_Init_Fast(Seconds_1_5S); 
    DEV_Delay_ms(2000);
    
    EPD_4IN2_V2_Display_Fast(Image);
    DEV_Delay_ms(2000);
    
    EPD_4IN2_V2_Sleep();
    return LAYOUT_OK;
}

layout_status_t layout_set_title(char* text)
{
    if(text == NULL) return LAYOUT_ERR_INPUT;
    if(Image == NULL) return LAYOUT_ERR_UNINITIALIZED;
    if(strlen(text) > MAX_TITLE_LEN) return LAYOUT_ERR_INPUT;
    
    Paint_DrawRectangle(4,4,396,38,WHITE,DOT_PIXEL_1X1,DRAW_FILL_FULL);
    Paint_DrawString_EN(10,10,text,&Font24,BLACK,WHITE);

    return LAYOUT_OK;
}

layout_status_t layout_set_header(const layout_content_t* content)
{
    if(Image == NULL) return LAYOUT_ERR_UNINITIALIZED;
    if(content == NULL) return LAYOUT_ERR_INPUT;

    Paint_DrawRectangle(X_L_MARGIN,Y_START_HEADER,X_R_MARGIN,(Y_START_HEADER+EL_HEIGHT*2),BLACK,DOT_PIXEL_1X1,DRAW_FILL_EMPTY);

    Paint_DrawString_EN(X_SECTION_H_0,(Y_START_HEADER + ((EL_HEIGHT*2)-Font20.Height)/2),content->left_text,&Font20,BLACK,WHITE);
    Paint_DrawString_EN(X_SECTION_H_1,(Y_START_HEADER + ((EL_HEIGHT*2)-Font20.Height)/2),content->main_text,&Font20,BLACK,WHITE);
    Paint_DrawString_EN(X_SECTION_H_2,(Y_START_HEADER + ((EL_HEIGHT*2)-Font20.Height)/2),content->right_text,&Font20,BLACK,WHITE);

    return LAYOUT_OK;
}

layout_status_t layout_set_content(layout_content_t* content, uint8_t pos)
{
    if(Image == NULL) return LAYOUT_ERR_UNINITIALIZED;
    if(content == NULL) return LAYOUT_ERR_INPUT;
    if(pos >= MAX_CONTENT_ELEMENTS) return LAYOUT_ERR_INPUT;
    
    switch(pos)
    {
        case 0:
            Paint_DrawRectangle(X_L_MARGIN,Y_START_POS0,X_R_MARGIN,(Y_START_POS0+EL_HEIGHT),BLACK,DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
            Paint_DrawString_EN(X_SECTION_0,(Y_START_POS0 + (EL_HEIGHT-Font16.Height)/2),content->left_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_1,(Y_START_POS0 + (EL_HEIGHT-Font16.Height)/2),content->main_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_2,(Y_START_POS0 + (EL_HEIGHT-Font16.Height)/2),content->right_text,&Font16,BLACK,WHITE);
        break;
        case 1:
            Paint_DrawRectangle(X_L_MARGIN,Y_START_POS1,X_R_MARGIN,(Y_START_POS1+EL_HEIGHT),BLACK,DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
            Paint_DrawString_EN(X_SECTION_0,(Y_START_POS1 + (EL_HEIGHT-Font16.Height)/2),content->left_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_1,(Y_START_POS1 + (EL_HEIGHT-Font16.Height)/2),content->main_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_2,(Y_START_POS1 + (EL_HEIGHT-Font16.Height)/2),content->right_text,&Font16,BLACK,WHITE);
        break;
        case 2:
            Paint_DrawRectangle(X_L_MARGIN,Y_START_POS2,X_R_MARGIN,(Y_START_POS2+EL_HEIGHT),BLACK,DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
            Paint_DrawString_EN(X_SECTION_0,(Y_START_POS2 + (EL_HEIGHT-Font16.Height)/2),content->left_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_1,(Y_START_POS2 + (EL_HEIGHT-Font16.Height)/2),content->main_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_2,(Y_START_POS2 + (EL_HEIGHT-Font16.Height)/2),content->right_text,&Font16,BLACK,WHITE);
        break;
        case 3:
            Paint_DrawRectangle(X_L_MARGIN,Y_START_POS3,X_R_MARGIN,(Y_START_POS3+EL_HEIGHT),BLACK,DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
            Paint_DrawString_EN(X_SECTION_0,(Y_START_POS3 + (EL_HEIGHT-Font16.Height)/2),content->left_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_1,(Y_START_POS3 + (EL_HEIGHT-Font16.Height)/2),content->main_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_2,(Y_START_POS3 + (EL_HEIGHT-Font16.Height)/2),content->right_text,&Font16,BLACK,WHITE);
        break;
        case 4:
            Paint_DrawRectangle(X_L_MARGIN,Y_START_POS4,X_R_MARGIN,(Y_START_POS4+EL_HEIGHT),BLACK,DOT_PIXEL_1X1,DRAW_FILL_EMPTY);
            Paint_DrawString_EN(X_SECTION_0,(Y_START_POS4 + (EL_HEIGHT-Font16.Height)/2),content->left_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_1,(Y_START_POS4 + (EL_HEIGHT-Font16.Height)/2),content->main_text,&Font16,BLACK,WHITE);
            Paint_DrawString_EN(X_SECTION_2,(Y_START_POS4 + (EL_HEIGHT-Font16.Height)/2),content->right_text,&Font16,BLACK,WHITE);
        break;
        default:
            return LAYOUT_ERR_INPUT;
    }

    return LAYOUT_OK;
}

layout_status_t layout_clear_content(void)
{
    if(Image == NULL) return LAYOUT_ERR_UNINITIALIZED;

    Paint_DrawRectangle(X_L_MARGIN,Y_START_POS0,X_R_MARGIN,EPD_4IN2_V2_HEIGHT,WHITE,DOT_PIXEL_1X1,DRAW_FILL_FULL);

    return LAYOUT_OK;
}