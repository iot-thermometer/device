#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
//#include "esp_task_wdt.h"
#include "dirent.h"
#include "esp_sleep.h"
#include "esp_heap_trace.h"

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

#include "app.c"

static heap_trace_record_t trace_record[100];

void app_main(void) {
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

//    if (!exists_in_nvs("reading_int")) {
    save_int_to_nvs("reading_int", 2000);
    save_int_to_nvs("push_int", 1);
//    }
    save_bool_to_nvs("wifi_enabled", false);

    run();
}