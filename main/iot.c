#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
// #include "esp_task_wdt.h"
#include "dirent.h"
#include "esp_sleep.h"
#include "esp_heap_trace.h"
#include "driver/adc.h"
#include "am2320.c"

#include "config.c"
#include "storage.c"
#include "button.c"
#include "wifi.c"
#include "mqtt.c"
#include "http.c"
#include "remote_config.c"
#include "led.c"
#include "bluetooth.c"
#include "time.c"
#include "crypto.c"
#include "ota.c"
#include "soil_moisture.c"

#include "app.c"

static heap_trace_record_t trace_record[100];

void app_main(void)
{
    ESP_ERROR_CHECK(i2cdev_init());
    initialize_adc();
    ESP_ERROR_CHECK(heap_trace_init_standalone(trace_record, 100));
    ESP_LOGI("APP", "Firmware version: %f", FIRMWARE_VERSION);
    init_nvs();
    init_fs();
    init_wifi();
    init_http();
    listen_for_reset();
    show_led();

    save_str_to_nvs("ssid", "iPhone (Mateusz)");
    save_str_to_nvs("password", "12345678");
    save_str_to_nvs("token", "grWUXyuIxLoBZrkv");
    save_int_to_nvs("id", 66);

    if (!exists_in_nvs("reading_int"))
    {
        save_int_to_nvs("reading_int", 2000);
        save_int_to_nvs("push_int", 5);
    }
    save_bool_to_nvs("wifi_enabled", false);

    run();
}

// void app_main(void)
// {
//     ESP_ERROR_CHECK(i2cdev_init());
//     initialize_adc();
//     i2c_dev_t dev = {0};
//     ESP_ERROR_CHECK(am2320_init_desc(&dev, 0, 21, 22));
//     float temperature, humidity;

//     while (1)
//     {
//         float soil_moisture = read_soil_moisture_sensor();
//         printf("Soil Moisture: %.0f %%\n", soil_moisture);
//         esp_err_t res = am2320_read(&dev, &temperature, &humidity);
//         if (res == ESP_OK)
//         {
//             ESP_LOGI("APP", "Temperature: %.1f°C, Humidity: %.1f%%", temperature, humidity);
//         }
//         else
//         {
//             ESP_LOGE("APP", "Error reading data: %d (%s)", res, esp_err_to_name(res));
//         }
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }