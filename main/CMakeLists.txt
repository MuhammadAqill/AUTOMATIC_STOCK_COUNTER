idf_component_register(
    SRCS "ble.c" "main.c"
    INCLUDE_DIRS "."
    REQUIRES bt ads1232 esp_driver_gpio driver freertos 14_I2C_LCD_16x2
    PRIV_REQUIRES nvs_flash esp_http_server spi_flash
    PRIV_INCLUDE_DIRS ~/esp/esp-idf/components/spi_flash/include/spi_flash
)
