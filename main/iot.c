#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "dirent.h"
#include "storage.c"
#include "button.c"
#include "led.c"
#include "sending.c"
// #include "bluetooth.c"
#include "http.c"
#include "time.c"
#include "crypto.c"
#include "ota.c"

#include "app.c"

void app_main(void)
{
    init_nvs();
    init_fs();
    init_wifi();
    listen_for_reset();
    show_led();

    // save_str_to_nvs("ssid", "iPhone (Mateusz)");
    // save_str_to_nvs("password", "12345678");
    // save_str_to_nvs("token", "HHiDlZkwmPKyZAjU");
    // save_int_to_nvs("id", 2);
    run();

    // start_bluetooth();
}