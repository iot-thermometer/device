#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"

static const char *TIME_TAG = "TIME";

void obtain_time(void) {
    time_t now;
    time(&now);
    struct tm timeInfo = {0};
    localtime_r(&now, &timeInfo);
    if (timeInfo.tm_year >= (2016 - 1900))
        return;
    ESP_LOGI(TIME_TAG, "Initializing SNTP");
    if (esp_sntp_enabled()) {
        esp_sntp_stop();
    }
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    int c = 0;
    while (timeInfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TIME_TAG, "Waiting for system time to be set... (%d)", timeInfo.tm_year);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeInfo);
        if (c > 10) {
            break;
            return;
        }
        c++;
    }

    ESP_LOGI(TIME_TAG, "Time is set");
}