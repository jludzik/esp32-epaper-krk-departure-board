idf_component_register(SRCS "ntp_connect.c"
                    INCLUDE_DIRS "."
                    REQUIRES lwip esp_netif debug
                    )