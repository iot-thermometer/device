#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BUTTON_PIN GPIO_NUM_0

static const char *BUTTON_TAG = "BUTTON";

void button_observer() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    int counter = 0;

    while (1) {
        int button_state = gpio_get_level(BUTTON_PIN);

        if (button_state == 0) {
            ESP_LOGI(BUTTON_TAG, "Button is pressed for %d seconds!", counter);
            counter++;
        } else {
            counter = 0;
        }

        if (counter == 5) {
            ESP_LOGI(BUTTON_TAG, "Resetting...");
            DIR *dir;
            dir = opendir("/spiffs");
            while (true) {
                struct dirent *de = readdir(dir);
                if (!de)
                    break;

                char *filename = malloc(512 * sizeof(char));
                sprintf(filename, "/spiffs/%s", de->d_name);
                remove(filename);
                ESP_LOGI(BUTTON_TAG, "Deleted: %s", de->d_name);
            }

            ESP_ERROR_CHECK(reset_nvs());
            esp_restart();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void listen_for_reset() {
    xTaskCreate(button_observer, "button_observer", 2048, NULL, 10, NULL);
}