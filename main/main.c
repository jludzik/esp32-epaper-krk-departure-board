#include "wifi_manager.h"
#include "mpk_api.h"
#include "ntp_connect.h"
#include "debug.h"

#include "DEV_Config.h"
#include "EPD_4in2_V2.h"
#include "Fonts/fonts.h"
#include "GUI_Paint.h"

#include <stdio.h>

void app_main(void)
{
    wifi_manager_init_sta();
    ntp_connect();
    mpk_api_init();
    mpk_api_update_departure_buffer();

    DEV_Module_Init();
    EPD_4IN2_V2_Init();
    
    EPD_4IN2_V2_Clear();
    DEV_Delay_ms(2000);
    
    uint8_t* Image = (uint8_t*)malloc(EPD_4IN2_V2_SIZE);
    if(Image == NULL)
    {
        Debug("MALLOC ERROR");
        return;
    }
    const char text_line1[] = "UR Al. 29 Listopada";
    Paint_NewImage(Image,EPD_4IN2_V2_WIDTH,EPD_4IN2_V2_HEIGHT,0,WHITE);
    Paint_SelectImage(Image);
    Paint_Clear(WHITE);

    Paint_DrawRectangle(2,2,398,40,BLACK,DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
    Paint_DrawString_EN(10,10,text_line1,&Font24,BLACK,WHITE);

    EPD_4IN2_V2_Display(Image);
    Debug("..");
    DEV_Delay_ms(2000);
    EPD_4IN2_V2_Sleep();

    mpk_api_departure_t test_dep;

    while(1)
    {
        EPD_4IN2_V2_Init_Fast(Seconds_1_5S); 
        DEV_Delay_ms(2000);

        mpk_api_status_t status = mpk_api_get_departure(&test_dep, 1);
        if(status == MPK_API_OK)
        {
            Debug("Line: %s, Direction: %s, sec to left: %d\n", test_dep.line, test_dep.direction, test_dep.sec_left_live);

            int min = test_dep.sec_left_live / 60;
            char time_str[16];
            snprintf(time_str, sizeof(time_str), "%d min", min);
            
            Paint_DrawRectangle(10, 100, 300, 250, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
            
            Paint_DrawString_EN(10, 100, test_dep.line, &Font16, BLACK, WHITE);
            Paint_DrawString_EN(10, 150, test_dep.direction, &Font16, BLACK, WHITE);
            Paint_DrawString_EN(10, 200, time_str, &Font16, BLACK, WHITE);

            Debug("DISP FAST START\n");
            EPD_4IN2_V2_Display_Fast(Image);
            DEV_Delay_ms(2000);
            Debug("DISP FAST DONE\n");
        }
        else
        {
            Debug("BLAD POBIERANIA API - ekran nie zostanie odswiezony: %d\n", status);
        }

        EPD_4IN2_V2_Sleep();

        DEV_Delay_ms(10000);
    }
}
