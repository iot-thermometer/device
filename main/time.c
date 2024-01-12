#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"

void obtain_time(void)
{
    time_t now;
    time(&now);
    struct tm timeinfo = {0};
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year >= (2016 - 1900))
        return;
    printf("Initializing SNTP\n");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    while (timeinfo.tm_year < (2016 - 1900))
    {

        printf("Waiting for system time to be set... (%d)\n", timeinfo.tm_year);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}