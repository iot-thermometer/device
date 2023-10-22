float read_sensor()
{
    return 42.0;
}

void pull_config()
{
    const char *json = make_http_request("https://gist.githubusercontent.com/matisiekpl/cd086500b92dfa3b0493aa6f518ad5b7/raw/9848b277efde3ef75485a088b24e3ee188d6a1b7/device.json");
    parse_remote_config(json);
}

void push_loop()
{
    int push_interval = 3000;

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

        pull_config();

        char existing_data[1000];
        // read_str_from_fs(existing_data, 1000);

        printf("Existing data: %s\n", existing_data);
        connect_mqtt();
        int suc = send_message(existing_data);
        if (!suc)
        {
            disconnect_mqtt();
            printf("Failed to push!\n");
            save_bool_to_nvs("wifi_enabled", false);
            disconnect_wifi();
        }

        disconnect_mqtt();
        save_str_to_fs("");
        printf("Pushed!\n");
        save_bool_to_nvs("wifi_enabled", false);
        disconnect_wifi();

        vTaskDelay(pdMS_TO_TICKS(push_interval));
    }
}

void read_loop()
{
    int reading_interval = 1000;
    while (1)
    {
        if (exists_in_nvs("reading_interval"))
        {
            read_int_from_nvs("reading_interval", &reading_interval);
            printf("Reading interval: %d\n", reading_interval);
        }

        float reading = read_sensor();
        printf("Reading: %f\n", reading);

        cJSON *root = cJSON_CreateObject();

        cJSON_AddNumberToObject(root, "device_id", 1);
        cJSON_AddStringToObject(root, "type", "TEMPERATURE");
        cJSON_AddNumberToObject(root, "value", reading);
        cJSON_AddNumberToObject(root, "time", 1696699342);

        char *new_data = cJSON_PrintUnformatted(root);

        char *existing_data = read_str_from_fs();

        char *final_data = malloc(strlen(existing_data) + 1 + strlen(new_data) + 1);
        strcpy(final_data, existing_data);
        strcat(final_data, "\n");
        strcat(final_data, new_data);

        cJSON_Delete(root);

        save_str_to_fs(final_data);
        printf("Saved!\n");
        printf("Final: %s\n", final_data);

        vTaskDelay(pdMS_TO_TICKS(reading_interval));
    }
}

StaticTask_t xTaskBuffer;

void run()
{
    if (exists_in_nvs("ssid"))
    {
        // xTaskCreate(push_loop, "push_data", 32768, NULL, 10, NULL);
        read_loop();
    }
    else
    {
        start_bluetooth();
    }
}