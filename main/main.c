/*
*                     !!!!!!!!! READ THIS !!!!!!!!!!!
*
* Project: Automatic Component Weight & Stock Counter System
* Description:
*  This program reads weight data from the ADS1232 load cell
*  amplifier and processes the measurements to determine
*  component quantity. It also provides a web interface for
*  real-time monitoring using ESP32.
*
* License:
* This software is released for educational and non commercial use.
* You are free to modify, distribute, and use this code
*
* Provided that:
*  1. Proper credit is given to the original author.
*  2. This notice remains intact in all copies or
*     modifications.
*  3. The software is provided "as-is" without warranty of
*     any kind.
*
* Author: MUHAMMAD AQIL BIN MUHAMMAD SHAHRIL
* Date: 24 August 2025
* project:
* https://github.com/MuhammadAqill/AUTOMATIC_STOCK_COUNTER.git
*
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADS1232.c"
#include "i2c_lcd.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <inttypes.h>
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include <esp_http_server.h>
#include "spi_flash_mmap.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define BUTTON_PIN 13
#define RESET_DURATION_MS 5000

volatile int raw_offset;
float web_raw_calibration = 0.0;
volatile int web_component_count = 0;
float web_raw_offset_calibration = 0.0;


esp_err_t http_get_handler(httpd_req_t *req) {
    const char* html = 
    "<!DOCTYPE html>"
    "<html lang='en'>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Automatic Stock Counter</title>"
    "<style>"
    "body {"
    "  font-family: Arial, sans-serif;"
    "  background: #f0f4f8;"
    "  color: #333;"
    "  display: flex;"
    "  flex-direction: column;"
    "  align-items: center;"
    "  justify-content: flex-start;"
    "  min-height: 100vh;"
    "  margin: 0;"
    "  padding: 40px 20px;"
    "}"
    "h1 {"
    "  color: #0077cc;"
    "  margin-bottom: 40px;"
    "  text-align: center;"
    "}"
    ".row {"
    "  display: flex;"
    "  flex-wrap: wrap;"
    "  gap: 20px;"
    "  justify-content: center;"
    "  margin-bottom: 20px;"
    "}"
    ".card {"
    "  background: #fff;"
    "  padding: 20px;"
    "  border-radius: 8px;"
    "  box-shadow: 0 4px 12px rgba(0,0,0,0.1);"
    "  min-width: 150px;"
    "  text-align: center;"
    "  flex: 1 1 140px;"
    "}"
    ".full-width {"
    "  width: 100%;"
    "  max-width: 340px;"
    "}"
    "p {"
    "  margin: 0 0 10px;"
    "  font-size: 18px;"
    "}"
    "span {"
    "  font-weight: bold;"
    "  color: #0077cc;"
    "  font-size: 20px;"
    "}"
    "</style>"

    "</head>"
    "<body>"
    "<h1>Automatic Stock Counter</h1>"
    "<div class='row'>"
    "  <div class='card'>"
    "    <p>Offset | gram (g)</p>"
    "    <span id='raw_offset_calibration'>--</span>"
    "  </div>"
    "  <div class='card'>"
    "    <p>Calib | gram (g)</p>"
    "    <span id='raw_calibration'>--</span>"
    "  </div>"
    "</div>"
    "<div class='card full-width'>"
    "  <p>components</p>"
    "  <span id='komponen'>--</span>"
    "</div>"

    "<script>"
    "setInterval(() => {"
    "  fetch('/data').then(r => r.json()).then(d => {"
    "    document.getElementById('raw_offset_calibration').innerText = d.raw_offset_calibration;"
    "    document.getElementById('raw_calibration').innerText = d.raw_calibration;"

    "let web_component_count = d.komponen;"
    "if (web_component_count === -1) {"
    "  document.getElementById('komponen').innerHTML = 'Invalid Reading';"
    "} else {"
    "  document.getElementById('komponen').innerHTML = web_component_count;"
    "}"

    "  });"
    "}, 500);"
    "</script>"

    "</body>"
    "</html>";



    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t data_handler(httpd_req_t *req) {
    char response[126];
    snprintf(response, sizeof(response),
             "{\"raw_offset_calibration\": %.2f, \"raw_calibration\": %.2f, \"komponen\": %d}",
             web_raw_offset_calibration,
             web_raw_calibration,
             web_component_count);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


// Fungsi mula web server
void start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = http_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_get);
        httpd_uri_t uri_data = {
            .uri = "/data",
            .method = HTTP_GET,
            .handler = data_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_data);
    }
}

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("✅ Got IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}


void calibrate(char buffer[], size_t buffer_size, int x, float gram_offset[], char str[15]) {
    snprintf(buffer, buffer_size, "Raw[%d]: %.2fg", x, gram_offset[x]); // convert int to string
    lcd_clear();            
    lcd_put_cursor(0, 0);   
    lcd_send_string(buffer);
    lcd_put_cursor(1, 0);
    lcd_send_string(str);
    vTaskDelay(pdMS_TO_TICKS(500));
}

void offset_total_average(int64_t total, int32_t average, char Tstr[9], char Astr[11]) {
    printf("-----------------------------------------------\n");
    printf("%s", Tstr);
    printf("%" PRId64 "\n", total);
    printf("%s", Astr);
    printf("%" PRId32 "\n", average);
    printf("-----------------------------------------------\n");
}

void pressure_total_average(int32_t total, int32_t average, float average_component, char Tstr[9], char Astr[11], char Cstr[22]) {
    printf("-----------------------------------------------\n");
    printf("%s", Tstr);
    printf("%" PRId32 "\n", total);
    printf("%s", Astr);
    printf("%" PRId32 "\n", average);
    printf("%s", Cstr);
    printf("%f\n", average_component);
    printf("-----------------------------------------------\n");
}

void display_lcd_i2c(char firstStr[14], char secondStr[15]) {
    lcd_clear();
    lcd_put_cursor(0, 0);
    lcd_send_string(firstStr);
    lcd_put_cursor(1,0);
    lcd_send_string(secondStr);
}

void samples_components(char buffer[], size_t buffer_size, char SamplingStr[9], int i, int x) {
    snprintf(buffer, buffer_size, "%s%d/%d", SamplingStr, i, x);
    lcd_clear();
    lcd_put_cursor(0, 0);
    lcd_send_string(buffer);
}

void check_reset_button() {
    static int64_t button_press_time = 0;
    static bool button_pressed = false;
    
    int button_state = gpio_get_level(BUTTON_PIN);
    
    if (button_state == 0 && !button_pressed) {
        // Tombol baru ditekan
        button_press_time = esp_timer_get_time();
        button_pressed = true;
    } 
    else if (button_state == 1 && button_pressed) {
        // Tombol dilepas
        button_pressed = false;
    }
    else if (button_state == 0 && button_pressed) {
        // Tombol masih ditekan, cek durasi
        int64_t press_duration = (esp_timer_get_time() - button_press_time) / 1000; // Konversi ke ms
        if (press_duration >= RESET_DURATION_MS) {
            lcd_clear();
            lcd_put_cursor(0, 0); 
            lcd_send_string("Resetting");
            lcd_put_cursor(1,0);
            lcd_send_string("Device...");
            printf("Resetting device...\n");
            for(int reset_two_times = 0; reset_two_times < 2; reset_two_times++) {
                esp_restart();
            }

        }
    }
}


void app_main(void) { 

    // 🔧 WAJIB: Init NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "itik", // Redmi 9C
            .password = "cucutimah", // aqilsem#
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();




    xEventGroupWaitBits(wifi_event_group,
                        CONNECTED_BIT,
                        pdFALSE,
                        pdFALSE,
                        portMAX_DELAY);

        esp_netif_ip_info_t ip_info;
        esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_get_ip_info(netif, &ip_info);

        printf("📡 IP Address (manual fetch): " IPSTR "\n", IP2STR(&ip_info.ip));
        char ip_address[16];
        sprintf(ip_address, IPSTR, IP2STR(&ip_info.ip));

    start_webserver();

    ads1232_gpio_init();
    lcd_init();

    lcd_put_cursor(0, 0); 
    lcd_send_string("Web Server:");
    lcd_put_cursor(1,0);              
    lcd_send_string(ip_address);
    vTaskDelay(pdMS_TO_TICKS(2000));

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // 1. CALIBRATION OFFSET

    int64_t total_raw_offset = 0;
    for(int x = 1; x < 21; x++) {

        float gram_offset[x];
        int32_t raw_offset[x];
        char buffer[20];

        raw_offset[x] = ads1232_read_raw();
        gram_offset[x] = raw_offset[x] * -0.00773575;
        web_raw_offset_calibration = gram_offset[x];
        total_raw_offset += raw_offset[x];

        check_reset_button();
        vTaskDelay(pdMS_TO_TICKS(10));

        calibrate(buffer, sizeof(buffer), x, gram_offset, "Components : 0");
        printf("Raw Offset : %" PRId32 " | Gram : %.2f\n ", raw_offset[x], gram_offset[x]);
        
        if(x == 20) {
            display_lcd_i2c("Put 1 Component", "and press button");
        }

    }
    int32_t average_raw_offset = total_raw_offset / 20;
    offset_total_average(total_raw_offset, average_raw_offset, "Total : ", "Average : ");

    // 2. CALIBRATION PRESURE

    float total_component = 0.0;
    int32_t total_raw_pressure = 0;
    bool running = true;
    while(running) {
        int button_state = gpio_get_level(BUTTON_PIN);
        printf("Button State : %d\n", button_state);
        check_reset_button();
        vTaskDelay(pdMS_TO_TICKS(10));
        if(button_state == 0) {
            for(int i = 1; i < 21; i++) {
                int32_t raw_pressure[i];
                float gram_pressure[i];
                char buffer[20];
                raw_pressure[i] = ads1232_read_raw();
                total_raw_pressure += raw_pressure[i];
                int32_t pressure_value = average_raw_offset - raw_pressure[i];
                float scale = 100.00 / pressure_value;
                gram_pressure[i] = pressure_value * -0.0076017;
                total_component += gram_pressure[i];
                web_raw_calibration = gram_pressure[i];
                check_reset_button();
                vTaskDelay(pdMS_TO_TICKS(10));
                calibrate(buffer, sizeof(buffer), i, gram_pressure, "               ");
                // printf("Raw_Pressure : %" PRId32 " | Pressure : %" PRId32 " | Gram : %.2f\n", raw_pressure[i], pressure_value, gram_pressure[i]);
                printf("Raw_Pressure : %" PRId32 " | Pressure : %" PRId32 "| Scale : %f\n", raw_pressure[i], pressure_value, scale);
                running = false;
            }

        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    int32_t average_raw_pressure = total_raw_pressure / 20;
    float average_component = total_component / 20;
    pressure_total_average(total_raw_pressure, average_raw_pressure, average_component, "Total : ", "Average : ", "Average Component :   ");

    // 3. CALCULATE THE COMPONENTS

    display_lcd_i2c("Put Components", "And Press Button"); // 14,16
    while(1) {
        
        int button_state = gpio_get_level(BUTTON_PIN);

        check_reset_button();
        vTaskDelay(pdMS_TO_TICKS(10));
        printf("Button State : %d\n", button_state);

        if(button_state == 0) {
            int x = 21;
            float total_components = 0.0;
            int32_t total_raw_components = 0;

            for(int i = 1; i < x; i++) {
                int32_t raw_samples[i];
                char buffer[20];
                float gram[i];
                raw_samples[i] = ads1232_read_raw();
                total_raw_components += raw_samples[i];

                check_reset_button();
                vTaskDelay(pdMS_TO_TICKS(10));
                samples_components(buffer, sizeof(buffer), "Sampling ", i, 20);

                int32_t pressure_value = raw_samples[i] - average_raw_offset;
                gram[i] = pressure_value * 0.00780725;
                total_components += gram[i];

                printf("Raw_Pressure : %" PRId32 " | Pressure : %" PRId32 " | Gram : %.2f\n", raw_samples[i], pressure_value, gram[i]);
                vTaskDelay(pdMS_TO_TICKS(500));
            }

            int32_t average_raw_components = total_raw_components / 20;
            int32_t current_pressure = average_raw_components - average_raw_offset;
            int32_t component_raw_weight = average_raw_pressure - average_raw_offset;
            float average_components = total_components / 20.00;

            pressure_total_average(total_raw_components, average_raw_components, average_components, "Total : ", "Average : ", "Average Components :  ");

            // Elak pembahagi sifar
            if(component_raw_weight == 0) {
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Error: Calib Fail");
                vTaskDelay(pdMS_TO_TICKS(2000));
                continue;
            }

            // kira jumlah komponen dengan bacaan berat
            float exact_components = (float)current_pressure / (float)component_raw_weight;

            int n = (int)round(exact_components);
            n = n < 0 ? 0 : n;
            printf("n : %d | current_pressure : %" PRId32 " | component_raw_weight : %" PRId32 "\n", n, current_pressure, component_raw_weight);


            float count_components = average_components / average_component;
            float rounded = round(count_components * 10) / 10.0;
            int result_components = (int)rounded;

            web_component_count = n;

            // semak toleransi ±10%
            int32_t tolerance = (int32_t)(0.1 * abs(component_raw_weight));
            int32_t lower_bound = n * (component_raw_weight - tolerance);
            int32_t upper_bound = n * (component_raw_weight + tolerance);


            printf("%" PRId32 " >= %" PRId32 " && %" PRId32 " <= %" PRId32 " \n", current_pressure, lower_bound, current_pressure, upper_bound);

            char buffer[100];

            // Devloping the decision
            if(current_pressure >= lower_bound && current_pressure <= upper_bound) {
                snprintf(buffer, sizeof(buffer), "Components: %d", n);
                printf("Components in analog : %d\n", n);
                printf("Conponents in weight : %d\n", result_components);
                printf("%" PRId32 " >= %" PRId32 " && %" PRId32 " <= %" PRId32 " \n", current_pressure, lower_bound, current_pressure, upper_bound);
            } else {
                snprintf(buffer, sizeof(buffer), "Invalid Reading!");
                printf("Invalid Reading\n");
                printf("%" PRId32 " >= %" PRId32 " && %" PRId32 " <= %" PRId32 " \n", current_pressure, lower_bound, current_pressure, upper_bound);
                web_component_count = -1;
            }

            lcd_clear();
            lcd_put_cursor(0, 0);
            lcd_send_string(buffer);
            vTaskDelay(pdMS_TO_TICKS(500));

        }

        vTaskDelay(pdMS_TO_TICKS(500));

    }

}