#define WDT_TIMEOUT 10000000

static const char *APP_TAG = "APP";

void sleep_if_possible(int timeout) {
    esp_sleep_enable_timer_wakeup(timeout * 1000);
    esp_light_sleep_start();
}

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
    bool mqtt_connected = connect_mqtt();
    if (!mqtt_connected) {
        ESP_LOGE(APP_TAG,
                 "MQTT broker is unavailable. Contact manufacturer status website to ensure broker is online.");
        disconnect_mqtt();
        disconnect_wifi();
        vTaskDelete(NULL);
    }

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

        if (strlen(data) == 0) {
            free(data);
            free(filename);
            continue;
        }

        char *final_data = (char *) malloc((strlen(data)) * sizeof(char) + 17);
        sprintf(final_data, "%d;%s", id, data);

        char kind[5];
        kind[4] = 0;
        kind[3] = filename[strlen(filename) - 1];
        kind[2] = filename[strlen(filename) - 2];
        kind[1] = filename[strlen(filename) - 3];
        kind[0] = filename[strlen(filename) - 4];
        char *topic = (char *) malloc((strlen(data)) * sizeof(char) + 17);
        sprintf(topic, "%s/%d/%s", "sensors", id, kind);
        int send_result = send_message(topic, final_data);
        if (send_result) {
            counter++;
            remove(filename);
            ESP_LOGI(APP_TAG, "Sent file %s to topic %s", de->d_name, topic);
        } else {
            ESP_LOGE(APP_TAG, "Failed to send file %s to topic %s", de->d_name, topic);
        }
        free(data);
        free(filename);
        free(final_data);
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

    xTaskCreate(push_data, "push_data", 32768, NULL, 10, NULL);

    while (1) {
//        esp_task_wdt_reset();
        read_int_from_nvs("push_int", &push_interval);
        read_int_from_nvs("reading_int", &reading_interval);

        time_t now;
        time(&now);
        char token[20];
        read_str_from_nvs("token", token, 19);

        bool temperature_valid = true;
        bool soil_moisture_valid = true;
        float temperature = esp_random();
        float soil_moisture = esp_random();

        if (time(&now) > 10000) {
            int id;
            read_int_from_nvs("id", &id);
            if (temperature_valid) {

                cJSON *temperature_payload = cJSON_CreateObject();
                cJSON_AddNumberToObject(temperature_payload, "device_id", id);
                cJSON_AddStringToObject(temperature_payload, "type", "TEMPERATURE");
                cJSON_AddNumberToObject(temperature_payload, "value", temperature);
                cJSON_AddNumberToObject(temperature_payload, "time", time(&now));
                char *marshaled_temperature_payload = cJSON_PrintUnformatted(temperature_payload);
                char *encrypted_temperature_payload = encrypt_text(marshaled_temperature_payload, token);
                cJSON_Delete(temperature_payload);
                char *temperature_filename = (char *) malloc(40 * sizeof(char));
                sprintf(temperature_filename, "/spiffs/%d.TEMP", (int) esp_random());
                save_str_to_fs(temperature_filename, encrypted_temperature_payload);
                free(encrypted_temperature_payload);

                ESP_LOGI(APP_TAG, "[%d] Temperature is %f | File: %s", push_interval - counter, temperature,
                         temperature_filename);
            } else {
                ESP_LOGW(APP_TAG, "Reading temperature was incorrect, skipping saving onto device");
            }

            if (soil_moisture_valid) {
                cJSON *soil_moisture_payload = cJSON_CreateObject();
                cJSON_AddNumberToObject(soil_moisture_payload, "device_id", id);
                cJSON_AddStringToObject(soil_moisture_payload, "type", "SOIL_MOISTURE");
                cJSON_AddNumberToObject(soil_moisture_payload, "value", soil_moisture);
                cJSON_AddNumberToObject(soil_moisture_payload, "time", time(&now));
                char *marshaled_soil_moisture_payload = cJSON_PrintUnformatted(soil_moisture_payload);
                char *encrypted_soil_moisture_payload = encrypt_text(marshaled_soil_moisture_payload, token);
                cJSON_Delete(soil_moisture_payload);
                char *soil_moisture_filename = (char *) malloc(40 * sizeof(char));
                sprintf(soil_moisture_filename, "/spiffs/%d.SOIL", (int) esp_random());
                save_str_to_fs(soil_moisture_filename, encrypted_soil_moisture_payload);
                free(encrypted_soil_moisture_payload);

                ESP_LOGI(APP_TAG, "[%d] Soil moisture is %f | File: %s", push_interval - counter, soil_moisture,
                         soil_moisture_filename);
            } else {
                ESP_LOGW(APP_TAG, "Reading soil moisture was incorrect, skipping saving onto device");
            }
        } else {
            ESP_LOGW(APP_TAG, "Device need at least once connect to Wifi to obtain time. Skipping reading.");
        }
        ESP_LOGD(APP_TAG, "Memory: %d", (int) esp_get_free_heap_size());

        if (counter == push_interval) {
            counter = 0;
            xTaskCreate(push_data, "push_data", 32768, NULL, 10, NULL);
        }

        counter++;

        bool wifi_enabled = false;
        read_bool_from_nvs("wifi_enabled", &wifi_enabled);
        if (wifi_enabled) {
            vTaskDelay(pdMS_TO_TICKS(reading_interval));
        } else {
            ESP_LOGI(APP_TAG, "Going to take nap (%d ms), good night", reading_interval);
            vTaskDelay(pdMS_TO_TICKS(100));
            sleep_if_possible(reading_interval);
            ESP_LOGI(APP_TAG, "Odpowiedzialnosc to wstawac rano");
        }

    }
}

void run() {
    if (exists_in_nvs("ssid")) {
//        esp_task_wdt_config_t twdt_config = {
//                .timeout_ms = WDT_TIMEOUT,
//                .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
//                .trigger_panic = false,
//        };
//        esp_task_wdt_init(&twdt_config);
//        esp_task_wdt_add(NULL);
        main_loop();
    } else {
        start_bluetooth();
    }
}