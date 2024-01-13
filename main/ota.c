#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

static const char *FIRMWARE_TAG = "FIRMWARE";

bool check_update() {
    char *json = malloc(512 * sizeof(char));
    make_http_request(firmwareUrl, json);
    if (strlen(json) > 0) {
        ESP_LOGD(FIRMWARE_TAG, "FIRMWARE: %s", json);
        cJSON *root = cJSON_Parse(json);
        if (root == NULL) {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                ESP_LOGE(FIRMWARE_TAG, "Error before: %s", error_ptr);
            }
        }
        if (cJSON_HasObjectItem(root, "version")) {
            double version = cJSON_GetObjectItem(root, "version")->valuedouble;
            char *file = cJSON_GetObjectItem(root, "file")->valuestring;
            cJSON_Delete(root);
            if (version > FIRMWARE_VERSION) {
                ESP_LOGI(FIRMWARE_TAG, "New firmware version is available. %f > %f", version, FIRMWARE_VERSION);
                ESP_LOGI(FIRMWARE_TAG, "Performing automated OTA update...");

                esp_http_client_config_t config = {
                        .url = file,
                };
                esp_https_ota_config_t ota_config = {
                        .http_config = &config,
                };
                esp_err_t ret = esp_https_ota(&ota_config);
                if (ret == ESP_OK) {
                    ESP_LOGI(FIRMWARE_TAG, "Firmware update downloaded, restarting...");
                    esp_restart();
                    return true;
                } else {
                    ESP_LOGE(FIRMWARE_TAG, "OTA failed");
                    return false;
                }
            } else {
                return true;
            }
        } else {
            ESP_LOGE(FIRMWARE_TAG, "Firmware message is invalid");
        }
        cJSON_Delete(root);
    } else {
        free(json);
        ESP_LOGE(FIRMWARE_TAG, "Error fetching firmware message");
    }

    return false;
}
