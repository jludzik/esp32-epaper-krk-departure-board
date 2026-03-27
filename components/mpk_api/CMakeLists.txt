idf_component_register(SRCS "mpk_api.c"
                    INCLUDE_DIRS "."
                    REQUIRES log esp_http_client json mbedtls debug
                    #EMBED_TXTFILES "mpk_cert.pem"
                    )