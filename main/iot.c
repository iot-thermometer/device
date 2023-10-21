#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "storage.c"
#include "button.c"
#include "led.c"
#include "sending.c"
#include "bluetooth.c"
#include "app.c"

void app_main(void)
{
    init_nvs();
    init_fs();
    // listen_for_reset();
    // show_led();
    // push_data();

    // save_str_to_nvs("ssid", "iPhone (Mateusz)");
    // save_str_to_nvs("password", "12345678");

    // run();
    start_bluetooth();
}
