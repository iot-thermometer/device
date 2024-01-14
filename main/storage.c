#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_log.h"

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

esp_err_t save_bool_to_nvs(const char *key, bool value) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READWRITE, &nvs);
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

esp_err_t read_bool_from_nvs(const char *key, bool *value) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READONLY, &nvs);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_get_u8(nvs, key, (uint8_t *) value);
    if (ret == ESP_OK) {
        *value = (*value != 0);
    }

    nvs_close(nvs);
    return ret;
}

esp_err_t save_str_to_nvs(const char *key, const char *value) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READWRITE, &nvs);
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

esp_err_t read_str_from_nvs(const char *key, char *value, size_t max_length) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READONLY, &nvs);
    if (ret != ESP_OK) {
        printf("Failed to open NVS\n");
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
    } else {
        printf("Failed to get NVS\n");
    }

    nvs_close(nvs);
    return ret;
}

esp_err_t save_int_to_nvs(const char *key, int value) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_set_i32(nvs, key, value);
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs);
    }

    nvs_close(nvs);
    return ret;
}

esp_err_t read_int_from_nvs(const char *key, int32_t *value) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READONLY, &nvs);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_get_i32(nvs, key, value);
    if (ret != ESP_OK) {
        printf(ret);
        return ret;
    }

    nvs_close(nvs);
    return ret;
}

esp_err_t reset_nvs() {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_erase_all(nvs);
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs);
    }

    nvs_close(nvs);
    return ret;
}

bool exists_in_nvs(const char *key) {
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READONLY, &nvs);
    if (ret != ESP_OK) {
        return false;
    }

    bool b = false;
    size_t required_size;
    if (nvs_get_str(nvs, key, NULL, &required_size) == ESP_OK || nvs_get_u8(nvs, key, (uint8_t * ) & b) == ESP_OK) {
        return true;
    }

    nvs_close(nvs);
    return false;
}

void save_str_to_fs(char *filename, char *value) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(f, "%s", value);

    fclose(f);
}

void read_str_from_fs(char *filename, char *out) {
    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    fread(out, fsize, 1, f);
    fclose(f);
    out[fsize] = '\0';
}

void init_fs() {
    esp_vfs_spiffs_conf_t conf = {
            .base_path = "/spiffs",
            .partition_label = NULL,
            .max_files = 5,
            .format_if_mount_failed = true};

    esp_err_t
            ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE("", "Failed to mount SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }
}

int count_files() {
    DIR *dir;
    dir = opendir("/spiffs");
    int count = 0;
    while (true) {
        struct dirent *de = readdir(dir);
        if (!de)
            break;
        count++;
    }
    closedir(dir);
    return count;
}