#define WDT_TIMEOUT 10000

static const char *APP_TAG = "APP";

void push_data() {
    bool wifi_enabled = false;
    read_bool_from_nvs("wifi_enabled", &wifi_enabled);
    if (wifi_enabled) {
        ESP_LOGI(APP_TAG, "Aborting push, because another process is ongoing");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(APP_TAG, "Attempting to push readings...");

    char ssid[33];
    char password[65];
    read_str_from_nvs("ssid", ssid, 32);
    read_str_from_nvs("password", password, 64);
    save_bool_to_nvs("wifi_enabled", true);
    connect_wifi(ssid, password);

    bool wifi_connected = false;
    read_bool_from_nvs("wifi_connected", &wifi_connected);
    int breaker = 0;
    while (!wifi_connected) {
        if (breaker > 10) {
            ESP_LOGE(APP_TAG, "Attempting to connect to Wifi failed. Will try in next push attempt");
            disconnect_wifi();
            save_bool_to_nvs("wifi_enabled", false);
            vTaskDelay(pdMS_TO_TICKS(300));
            vTaskDelete(NULL);
            return;
        }
        read_bool_from_nvs("wifi_connected", &wifi_connected);
        ESP_LOGW(APP_TAG, "Waiting for wifi (%d seconds)...", breaker);
        vTaskDelay(pdMS_TO_TICKS(1000));
        breaker++;
    }
    ESP_LOGI(APP_TAG, "Device is online!");

    // check_update();

    pull_config();

    obtain_time();
    connect_mqtt();

    DIR *dir;
    struct dirent *ent;
    dir = opendir("/spiffs");

    int counter = 0;

    int id;
    read_int_from_nvs("id", &id);

    while (true) {
        struct dirent *de = readdir(dir);
        if (!de)
            break;

        char *filename = malloc(512 * sizeof(char));
        sprintf(filename, "/spiffs/%s", de->d_name);
        char *data = read_str_from_fs(filename);

        if (strlen(data) == 0)
            continue;

        char *final_data = (char *) malloc((strlen(data)) * sizeof(char) + 17);
        sprintf(final_data, "%d;%s", id, data);

        int suc = send_message(final_data);
        if (suc) {
            counter++;
            remove(filename);
            ESP_LOGI(APP_TAG, "Sent file: %s", de->d_name);
        } else {
            ESP_LOGE(APP_TAG, "Failed to send file: %s", de->d_name);
        }
    }

    char *summary = (char *) malloc(100 * sizeof(char));
    sprintf(summary, "Sent %d files", counter);
    int suc = send_message(summary);
    if (!suc) {
        disconnect_mqtt();
        ESP_LOGE(APP_TAG, "Failed to push!");
        save_bool_to_nvs("wifi_enabled", false);
        disconnect_wifi();
    }

    disconnect_mqtt();
    ESP_LOGI(APP_TAG, "Data pushed!");
    save_bool_to_nvs("wifi_enabled", false);
    disconnect_wifi();

    vTaskDelete(NULL);
}

void main_loop() {
    int reading_interval = 100000;
    int push_interval = 100;
    int counter = 0;
    while (1) {
        esp_task_wdt_reset();
        read_int_from_nvs("push_int", &push_interval);
        read_int_from_nvs("reading_int", &reading_interval);

        cJSON *root = cJSON_CreateObject();
        time_t now;
        time(&now);
        char token[20];
        read_str_from_nvs("token", token, 19);

        float temperature = esp_random();
        ESP_LOGI(APP_TAG, "[%d] Temperature is %f", push_interval - counter, temperature);

        if (time(&now) > 10000) {
            int id;
            read_int_from_nvs("id", &id);
            cJSON_AddNumberToObject(root, "device_id", id);
            cJSON_AddStringToObject(root, "type", "TEMPERATURE");
            cJSON_AddNumberToObject(root, "value", temperature);

            cJSON_AddNumberToObject(root, "time", time(&now));

            char *new_data = cJSON_PrintUnformatted(root);
            char *encrypted_data = encrypt_text(new_data, token);

            cJSON_Delete(root);

            char *filename = (char *) malloc(40 * sizeof(char));
            sprintf(filename, "/spiffs/%d.txt", (int) esp_random());
            // save_str_to_fs(filename, encrypted_data);
            // printf("Saved file: %s\n", filename);
        }

        if (counter == push_interval) {
            counter = 0;
            xTaskCreate(push_data, "push_data", 32768, NULL, 10, NULL);
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(reading_interval));
    }
}

void run() {
    if (exists_in_nvs("ssid")) {
        esp_task_wdt_config_t twdt_config = {
                .timeout_ms = WDT_TIMEOUT,
                .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
                .trigger_panic = false,
        };
        esp_task_wdt_init(&twdt_config);
        esp_task_wdt_add(NULL);
        main_loop();
    } else {
        start_bluetooth();
    }
}