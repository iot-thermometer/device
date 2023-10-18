#include "nvs_flash.h"
#include "nvs.h"

esp_err_t init_nvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        printf("Failed to init NVS\n");
    }
    return ret;
}

esp_err_t save_bool_to_nvs(const char* namespace, const char* key, bool value) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open(namespace, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_set_u8(nvs, key, value);
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs);
    }

    nvs_close(nvs);
    return ret;
}

esp_err_t read_bool_from_nvs(const char* namespace, const char* key, bool* value) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open(namespace, NVS_READONLY, &nvs);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_get_u8(nvs, key, (uint8_t*)value);
    if (ret == ESP_OK) {
        *value = (*value != 0);
    }

    nvs_close(nvs);
    return ret;
}

esp_err_t save_str_to_nvs(const char* namespace, const char* key, const char* value) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open(namespace, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_set_str(nvs, key, value);
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs);
    }

    nvs_close(nvs);
    return ret;
}

esp_err_t read_str_from_nvs(const char* namespace, const char* key, char* value, size_t max_length) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open(namespace, NVS_READONLY, &nvs);
    if (ret != ESP_OK) {
        return ret;
    }

    size_t required_size;
    ret = nvs_get_str(nvs, key, NULL, &required_size);
    if (ret == ESP_OK) {
        if (required_size <= max_length) {
            ret = nvs_get_str(nvs, key, value, &required_size);
        } else {
            ret = ESP_ERR_NVS_INVALID_LENGTH;
        }
    }

    nvs_close(nvs);
    return ret;
}

