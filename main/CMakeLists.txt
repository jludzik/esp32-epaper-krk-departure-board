idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES epaper_lib wifi_manager mpk_api debug ntp_connect
                    )
