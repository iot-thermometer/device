
static bool blink_led = false;
#define BLINK_GPIO 2

static void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void led_watcher() {
    while(1) {
        const char* ssid = NULL;
        read_str_from_nvs("ssid", &ssid, 32);
        if (ssid==NULL){
            printf("No ssid found in nvs\n");
            blink_led = true;
        } else {
            printf("ssid found in nvs: %s\n", ssid);
            blink_led = false;
        }

        gpio_set_level(BLINK_GPIO, 0);
        if (blink_led){
            vTaskDelay(500 / portTICK_PERIOD_MS);
            gpio_set_level(BLINK_GPIO, 1);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

void show_led(){
    configure_led();
    xTaskCreate(led_watcher, "led_watcher", 2048, NULL, 10, NULL);
}