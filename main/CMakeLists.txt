idf_component_register(SRCS "main.cpp"
                    PRIV_REQUIRES spi_flash
                    REQUIRES driver esp_wifi nvs_flash esp_http_server esp_netif
                    INCLUDE_DIRS "." "../components"
                    EMBED_FILES root.html)
