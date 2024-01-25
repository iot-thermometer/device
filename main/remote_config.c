#include "cJSON.h"

static const char *CONFIG_TAG = "CONFIG";

void parse_remote_config(char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            ESP_LOGE(CONFIG_TAG, "Error before: %s\n", error_ptr);
        }
    }

    if (cJSON_HasObjectItem(root, "ReadingInterval"))
    {
        int reading_interval = cJSON_GetObjectItem(root, "ReadingInterval")->valueint;
        int push_interval = cJSON_GetObjectItem(root, "PushInterval")->valueint;

        ESP_LOGI(CONFIG_TAG, "Setting reading interval: %d", reading_interval);
        ESP_LOGI(CONFIG_TAG, "Setting push interval: %d", push_interval);
        // DEMO w trakcie demo nie zapisujemy do NVS, bo szkoda czasu na czekanie
        // save_int_to_nvs("reading_int", reading_interval);
        // save_int_to_nvs("push_int", push_interval);
    }
    else
    {
        ESP_LOGE(CONFIG_TAG, "Config message is invalid");
    }

    cJSON_Delete(root);
}

char token[20];

void pull_config()
{
    read_str_from_nvs("token", token, 19);
    char url[100];
    sprintf(url, "http://srv3.enteam.pl:3009/api/iot/%s/config", token);
    ESP_LOGI(CONFIG_TAG, "Fetching config from %s", url);

    char json[512];
    make_http_request(url, json);

    if (strlen(json) > 0)
    {
        ESP_LOGD(CONFIG_TAG, "CONFIG: %s", json);
        parse_remote_config(json);
    }
    else
    {
        ESP_LOGE(CONFIG_TAG, "Error fetching config");
    }
}