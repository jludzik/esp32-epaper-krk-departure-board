#include "wifi_manager.h"
#include "mpk_api.h"
#include "ntp_connect.h"
#include "debug.h"
#include "epaper_layout.h"
#include "epaper_layout_const_texts.h"
#include "DEV_Config.h"

#include <stdio.h>
typedef enum {
    STATE_INIT,
    STATE_WIFI_CONNECTING,
    STATE_SYNC_TIME,
    STATE_API_INIT,
    STATE_SET_HEADERS,
    STATE_API_UPDATING,
    STATE_DRAW_DISPLAY,
    STATE_WAIT_FOR_REFRESH,
    STATE_ERROR
} app_state_t;

void app_main(void)
{
    mpk_api_status_t dep_update_status = MPK_API_OK;
    uint8_t dep_count = 0;
    mpk_api_departure_t dep_new;
    layout_content_t dep_new_layout;

    app_state_t current_state = STATE_INIT;

    while(1)
    {
        switch (current_state)
        {
            case STATE_INIT:
                Debug("STATE_INIT");

                wifi_reset_key_config();
                DEV_Module_Init();
                layout_init();

                if(wifi_manager_check_reset() == true) layout_set_warning_screen(layout_warning_wifi_prov);
                else layout_set_warning_screen(layout_warning_wifi_init);
  
                layout_push_to_screen();

                current_state = STATE_WIFI_CONNECTING;
            break;
            case STATE_WIFI_CONNECTING:
                Debug("STATE_WIFI_CONNECTING");

                wifi_manager_init_sta();

                current_state = STATE_SYNC_TIME;
            break;
            case STATE_SYNC_TIME:
                Debug("STATE_SYNC_TIME");

                ntp_connect();

                current_state = STATE_API_INIT;
            break;
            case STATE_API_INIT:
                Debug("STATE_API_INIT");

                mpk_api_init();

                current_state = STATE_SET_HEADERS;
            break;
            case STATE_SET_HEADERS:
                Debug("STATE_SET_HEADERS");

                layout_set_clear_screen();
                layout_set_title(layout_title);
                layout_set_header(&layout_header);

                current_state = STATE_API_UPDATING;
            break;
            case STATE_API_UPDATING:
                Debug("STATE_API_UPDATING");

                dep_update_status = mpk_api_update_departure_buffer();
                if(dep_update_status == MPK_API_OK)
                {
                    layout_clear_content();

                    mpk_api_get_departure_save_count(&dep_count);
                    for(uint8_t i=0;i<dep_count;i++)
                    {
                        mpk_api_get_departure(&dep_new, i);

                        strncpy(dep_new_layout.left_text, dep_new.line, sizeof(dep_new_layout.left_text));
                        strncpy(dep_new_layout.main_text, dep_new.direction, sizeof(dep_new_layout.main_text));
                        strncpy(dep_new_layout.right_text, dep_new.sec_left_live, sizeof(dep_new_layout.right_text));
                        layout_set_content(&dep_new_layout,i);

                        Debug("Linia: %s Kierunek: %s Odjazd: %s", dep_new.line, dep_new.direction, dep_new.sec_left_live);
                    }

                    current_state = STATE_DRAW_DISPLAY;
                }
                else current_state = STATE_ERROR;
            break;
            case STATE_DRAW_DISPLAY:
                Debug("STATE_DRAW_DISPLAY");

                layout_push_to_screen();

                current_state = STATE_WAIT_FOR_REFRESH;
            break;
            case STATE_WAIT_FOR_REFRESH:
                Debug("STATE_WAIT_FOR_REFRESH");

                DEV_Delay_ms(20000);

                current_state = STATE_API_UPDATING;
            break;
            case STATE_ERROR:
                Debug("STATE_ERROR");
                
                Debug("API fetch failed. Display will not be updated. Status: %d\n", dep_update_status);

                DEV_Delay_ms(5000);
                current_state = STATE_WIFI_CONNECTING;
            break;   
        }

        DEV_Delay_ms(10);
    }
}