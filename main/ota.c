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
#define UPDATE_JSON_URL "https://esp32tutorial.netsons.org/https_ota/firmware.json"

// server certificates
extern const char server_cert_pem_start[] asm("_binary_certs_pem_start");
extern const char server_cert_pem_end[] asm("_binary_certs_pem_end");

char rcv_buffer[200];

esp_err_t _ota_http_event_handler(esp_http_client_event_t *evt)
{

    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        break;
    case HTTP_EVENT_ON_CONNECTED:
        break;
    case HTTP_EVENT_HEADER_SENT:
        break;
    case HTTP_EVENT_ON_HEADER:
        break;
    case HTTP_EVENT_ON_DATA:
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            strncpy(rcv_buffer, (char *)evt->data, evt->data_len);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        break;
    case HTTP_EVENT_DISCONNECTED:
        break;
    default:
        break;
    }
    return ESP_OK;
}

void check_update()
{

    printf("Looking for a new firmware...");

    // configure the esp_http_client
    esp_http_client_config_t config = {
        .url = UPDATE_JSON_URL,
        .event_handler = _ota_http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // downloading the json file
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {

        printf("Downloaded: %s\n", rcv_buffer);
        // parse the json file
        cJSON *json = cJSON_Parse(rcv_buffer);
        if (json == NULL)
            printf("downloaded file is not a valid json, aborting...\n");
        else
        {
            cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
            cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");

            // check the version
            if (!cJSON_IsNumber(version))
                printf("unable to read new version, aborting...\n");
            else
            {

                double new_version = version->valuedouble;
                if (new_version > FIRMWARE_VERSION)
                {

                    printf("current firmware version (%.1f) is lower than the available one (%.1f), upgrading...\n", FIRMWARE_VERSION, new_version);
                    if (cJSON_IsString(file) && (file->valuestring != NULL))
                    {
                        printf("downloading and installing new firmware (%s)...\n", file->valuestring);

                        esp_http_client_config_t ota_client_config = {
                            .url = file->valuestring,
                        };
                        esp_err_t ret = esp_https_ota(&ota_client_config);
                        if (ret == ESP_OK)
                        {
                            printf("OTA OK, restarting...\n");
                            esp_restart();
                        }
                        else
                        {
                            printf("OTA failed...\n");
                        }
                    }
                    else
                        printf("unable to read the new file name, aborting...\n");
                }
                else
                    printf("current firmware version (%.1f) is greater or equal to the available one (%.1f), nothing to do...\n", FIRMWARE_VERSION, new_version);
            }
        }
    }
    else
        printf("unable to download the json file, aborting...\n");

    esp_http_client_cleanup(client);
}
