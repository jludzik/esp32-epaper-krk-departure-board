idf_component_register(SRCS "DEV_Config.c" "EPD_4in2_V2.c" "GUI_Paint.c" "Fonts/font8.c" "Fonts/font12.c" "Fonts/font16.c" "Fonts/font20.c" "Fonts/font24.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver freertos debug
                    )