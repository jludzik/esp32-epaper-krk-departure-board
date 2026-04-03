#include "wifi_manager.h"
#include "mpk_api.h"
#include "ntp_connect.h"
#include "debug.h"
#include "epaper_layout.h"

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

    DEV_Module_Init();

    layout_init();

    char text_title[] = "UR Al. 29 Listopada";
    const layout_content_t header_content = {
        .left_text = "Linia",
        .main_text = "Kierunek",
        .right_text = "Odjazd"
    };

    layout_set_title(text_title);
    layout_set_header(&header_content);

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

                Debug("Linia: %s Kierunek: %s Odjazd: %s\n", dep_new.line, dep_new.direction, dep_new.sec_left_live);
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
//ZBYT DLUGIE NAZWY UCIAC DODAC KROPKE
//POPRAWIC LICZENIE MINUT PORPZEZ STWORZEINE FUNKCJI PARSE LUB CALCULATE MIN
//POLSKIE ZNAKI
//FILTROWANIE KIERUNKOW
//NIE POKAZUJ WARTOSCI <= 0 min


/*
layout_content_t sample_content0 = {
        .left_text = "199\0",
        .main_text = "Pradnik Czerwony P+R\0",
        .right_text = "1 min\0"
    };

    layout_content_t sample_content1 = {
        .left_text = "503\0",
        .main_text = "Nowy Biezanow Poludnie\0",
        .right_text = "5 min\0"
    };

    layout_content_t sample_content2 = {
        .left_text = "139\0",
        .main_text = "Przybyszewskiego\0",
        .right_text = "11 min\0"
    };

    layout_content_t sample_content3 = {
        .left_text = "159\0",
        .main_text = "Cichy Kacik\0",
        .right_text = "14 min\0"
    };

   layout_content_t sample_content4 = {
        .left_text = "169\0",
        .main_text = "Zajezdnia Wola Duchacka\0",
        .right_text = "18 min\0"
    };
 layout_set_content(&sample_content0, 0);
    layout_set_content(&sample_content1, 1);
    layout_set_content(&sample_content2, 2);
    layout_set_content(&sample_content3, 3);
    layout_set_content(&sample_content4, 4);

*/