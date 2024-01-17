#include "esp_log.h"

static uint blink_delay = 5000;
static uint blink_timeout = 150;

#define BLINK_GPIO 2

void led_watcher()
{
    while (1)
    {
        bool wifi_connected = false;
        bool wifi_enabled = false;
        read_bool_from_nvs("wifi_connected", &wifi_connected);
        read_bool_from_nvs("wifi_enabled", &wifi_enabled);
        if (wifi_enabled && !wifi_connected)
        {
            blink_delay = 250;
            blink_timeout = 250;
        }
        else
        {
            blink_delay = 5000;
            blink_timeout = 150;
        }

        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(blink_delay / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(blink_timeout / portTICK_PERIOD_MS);
    }
}

void show_led()
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    xTaskCreate(led_watcher, "led_watcher", 2048, NULL, 10, NULL);
}