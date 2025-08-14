idf_component_register(SRCS "i2c_lcd.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt ads1232 esp_driver_gpio driver freertos
)