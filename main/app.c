#define WDT_TIMEOUT 10000
float read_sensor()
{
    return 42.0;
}

void pull_config()
{
    const char *json = make_http_request("https://gist.githubusercontent.com/matisiekpl/cd086500b92dfa3b0493aa6f518ad5b7/raw/9848b277efde3ef75485a088b24e3ee188d6a1b7/device.json");
    printf("CONFIG JSON: %s\n", json);
    // parse_remote_config(json);
}

void push_loop()
{
    int push_interval = 10000;

    while (1)
    {
        if (exists_in_nvs("push_interval"))
        {
            read_int_from_nvs("push_interval", &push_interval);
            printf("Push interval: %d\n", push_interval);
        }

        printf("Pushing...\n");

        char ssid[33];
        char password[65];
        read_str_from_nvs("ssid", ssid, 32);
        read_str_from_nvs("password", password, 64);
        save_bool_to_nvs("wifi_enabled", true);
        connect_wifi(ssid, password);

        bool wifi_connected = false;
        read_bool_from_nvs("wifi_connected", &wifi_connected);
        while (!wifi_connected)
        {
            read_bool_from_nvs("wifi_connected", &wifi_connected);
            printf("Waiting for wifi...\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        printf("Connected!\n");

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

        while (true)
        {
            struct dirent *de = readdir(dir);
            if (!de)
                break;

            char *filename = malloc(512 * sizeof(char));
            sprintf(filename, "/spiffs/%s", de->d_name);
            char *data = read_str_from_fs(filename);

            if (strlen(data) == 0)
                continue;

            char *final_data = (char *)malloc((strlen(data)) * sizeof(char) + 17);
            sprintf(final_data, "%d;%s", id, data);

            int suc = send_message(final_data);
            if (suc)
            {
                counter++;
                remove(filename);
                printf("Sent file: %s\n", de->d_name);
            }
            else
            {
                printf("Failed to send file: %s\n", de->d_name);
            }
        }

        char *summary = (char *)malloc(100 * sizeof(char));
        sprintf(summary, "Sent %d files", counter);
        int suc = send_message(summary);
        if (!suc)
        {
            disconnect_mqtt();
            printf("Failed to push!\n");
            save_bool_to_nvs("wifi_enabled", false);
            disconnect_wifi();
        }

        disconnect_mqtt();
        printf("Pushed!\n");
        save_bool_to_nvs("wifi_enabled", false);
        disconnect_wifi();

        vTaskDelay(pdMS_TO_TICKS(push_interval));
    }
}

void read_loop()
{
    int reading_interval = 4000;
    while (1)
    {
        esp_task_wdt_reset();
        if (exists_in_nvs("reading_interval"))
        {
            read_int_from_nvs("reading_interval", &reading_interval);
            printf("Reading interval: %d\n", reading_interval);
        }

        float reading = read_sensor();

        cJSON *root = cJSON_CreateObject();

        time_t now;
        time(&now);

        char token[20];
        read_str_from_nvs("token", token, 19);

        if (time(&now) > 10000)
        {
            int id;
            read_int_from_nvs("id", &id);
            cJSON_AddNumberToObject(root, "device_id", id);
            cJSON_AddStringToObject(root, "type", "TEMPERATURE");
            cJSON_AddNumberToObject(root, "value", esp_random());

            cJSON_AddNumberToObject(root, "time", time(&now));

            char *new_data = cJSON_PrintUnformatted(root);
            char *encrypted_data = encrypt_text(new_data, token);

            cJSON_Delete(root);

            char *filename = (char *)malloc(40 * sizeof(char));
            sprintf(filename, "/spiffs/%d.txt", (int)esp_random());
            // save_str_to_fs(filename, encrypted_data);
            // printf("Saved file: %s\n", filename);
        }

        vTaskDelay(pdMS_TO_TICKS(reading_interval));
    }
}

StaticTask_t xTaskBuffer;

void run()
{
    if (exists_in_nvs("ssid"))
    {
        esp_task_wdt_config_t twdt_config = {
            .timeout_ms = WDT_TIMEOUT,
            .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // Bitmask of all cores
            .trigger_panic = false,
        };
        esp_task_wdt_init(&twdt_config);
        esp_task_wdt_add(NULL);
        xTaskCreate(push_loop, "push_data", 32768, NULL, 10, NULL);
        read_loop();
    }
    else
    {
        start_bluetooth();
    }
}