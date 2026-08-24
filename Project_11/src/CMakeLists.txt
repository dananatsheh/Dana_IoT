# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

idf_component_register(SRCS "main.c"
                        INCLUDE_DIRS "."
                        REQUIRES driver esp_driver_gpio esp_driver_ledc esp_adc freertos)
