#include "wifi_manager.h"
#include "mpk_api.h"
#include "ntp_connect.h"
#include "debug.h"
#include "epaper_layout.h"
#include "epaper_layout_const_texts.h"

#include "DEV_Config.h"

#include <stdio.h>

void app_main(void)
{
    wifi_manager_init_sta();
    ntp_connect();
    mpk_api_init();

    DEV_Module_Init();

    layout_init();
    layout_set_title(epaper_layout_title);
    layout_set_header(&epaper_layout_header);

    mpk_api_status_t dep_update_status;
    uint8_t dep_count = 0;
    mpk_api_departure_t dep_new;
    layout_content_t dep_new_layout;

    while(1)
    {
        layout_clear_content();

        dep_update_status = mpk_api_update_departure_buffer();
        if(dep_update_status == MPK_API_OK)
        {
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

            layout_push_to_screen(); 
        }
        else
        {
            Debug("BLAD POBIERANIA API - ekran nie zostanie odswiezony: %d\n", dep_update_status);
        }

        DEV_Delay_ms(20000);
    }
}

//TODO
//POLSKIE ZNAKI
//FILTROWANIE KIERUNKOW
//NIE POKAZUJ WARTOSCI <= 0 min