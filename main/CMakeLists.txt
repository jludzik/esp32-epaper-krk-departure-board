idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES epaper_lib epaper_layout wifi_manager mpk_api debug ntp_connect
                    )
