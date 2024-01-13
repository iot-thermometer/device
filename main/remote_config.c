#include "cJSON.h"

static const char *CONFIG_TAG = "CONFIG";

void parse_remote_config(const char *json) {
    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(CONFIG_TAG, "Error before: %s\n", error_ptr);
        }
    }

    int reading_interval = cJSON_GetObjectItem(root, "reading_interval")->valueint;
    int push_interval = cJSON_GetObjectItem(root, "push_interval")->valueint;

    ESP_LOGI(CONFIG_TAG, "Setting reading interval: %d", reading_interval);
    ESP_LOGI(CONFIG_TAG, "Setting push interval: %d", push_interval);
    save_int_to_nvs("reading_interval", reading_interval);
    save_int_to_nvs("push_interval", push_interval);

    cJSON_Delete(root);
}

void pull_config() {
    const char *json = make_http_request(
            "https://gist.githubusercontent.com/matisiekpl/cd086500b92dfa3b0493aa6f518ad5b7/raw/9848b277efde3ef75485a088b24e3ee188d6a1b7/device.json");
    printf("CONFIG JSON: %s", json);
    // parse_remote_config(json);
}