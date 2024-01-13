#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#define FIRMWARE_VERSION 0.1
static const char *FIRMWARE_TAG = "FIRMWARE";

void check_update() {
    const char *json = make_http_request(firmwareUrl);
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
            printf("%s",file);

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
                } else {
                    ESP_LOGE(FIRMWARE_TAG, "OTA failed");
                }
            }
        } else {
            ESP_LOGE(FIRMWARE_TAG, "Firmware message is invalid");
        }
        cJSON_Delete(root);
    } else {
        ESP_LOGE(FIRMWARE_TAG, "Error fetching firmware message");
    }

//    printf("Looking for a new firmware...");
//
//    // configure the esp_http_client
//    esp_http_client_config_t config = {
//        .url = UPDATE_JSON_URL,
//        .event_handler = _ota_http_event_handler,
//    };
//    esp_http_client_handle_t client = esp_http_client_init(&config);
//
//    // downloading the json file
//    esp_err_t err = esp_http_client_perform(client);
//    if (err == ESP_OK)
//    {
//
//        printf("Downloaded: %s\n", rcv_buffer);
//        // parse the json file
//        cJSON *json = cJSON_Parse(rcv_buffer);
//        if (json == NULL)
//            printf("downloaded file is not a valid json, aborting...\n");
//        else
//        {
//            cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
//            cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");
//
//            // check the version
//            if (!cJSON_IsNumber(version))
//                printf("unable to read new version, aborting...\n");
//            else
//            {
//
//                double new_version = version->valuedouble;
//                if (new_version > FIRMWARE_VERSION)
//                {
//
//                    printf("current firmware version (%.1f) is lower than the available one (%.1f), upgrading...\n", FIRMWARE_VERSION, new_version);
//                    if (cJSON_IsString(file) && (file->valuestring != NULL))
//                    {
//                        printf("downloading and installing new firmware (%s)...\n", file->valuestring);
//
//                        esp_http_client_config_t ota_client_config = {
//                            .url = file->valuestring,
//                        };
//                        esp_err_t ret = esp_https_ota(&ota_client_config);
//                        if (ret == ESP_OK)
//                        {
//                            printf("OTA OK, restarting...\n");
//                            esp_restart();
//                        }
//                        else
//                        {
//                            printf("OTA failed...\n");
//                        }
//                    }
//                    else
//                        printf("unable to read the new file name, aborting...\n");
//                }
//                else
//                    printf("current firmware version (%.1f) is greater or equal to the available one (%.1f), nothing to do...\n", FIRMWARE_VERSION, new_version);
//            }
//        }
//    }
//    else
//        printf("unable to download the json file, aborting...\n");
//
//    esp_http_client_cleanup(client);
}
