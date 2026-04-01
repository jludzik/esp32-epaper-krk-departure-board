idf_component_register(SRCS "epaper_layout.c" "epaper_layout"
                    INCLUDE_DIRS "."
                    REQUIRES epaper_lib driver freertos
                    )