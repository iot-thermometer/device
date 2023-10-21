float read_sensor()
{
    return 42.0;
}

void push_data()
{
    printf("Pushing...\n");
    char ssid[33];
    char password[65];
    printf("DEBUG1\n");
    read_str_from_nvs("ssid", ssid, 32);
    printf("DEBUG2\n");
    read_str_from_nvs("password", password, 64);
    printf("DEBUG3\n");
    save_bool_to_nvs("wifi_enabled", true);
    printf("DEBUG4\n");
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

    char existing_data[1000];
    read_str_from_fs(existing_data, 1000);

    int suc = send_message(existing_data);
    if (!suc)
    {
        printf("Failed to push!\n");
        save_bool_to_nvs("wifi_enabled", false);
        disconnect_wifi();
        return;
    }
    save_str_to_fs("");
    printf("Pushed!\n");
    save_bool_to_nvs("wifi_enabled", false);
    disconnect_wifi();
}

void read_loop()
{
    int reading_interval = 1000;
    int push_interval = 7;
    int counter = 0;
    while (1)
    {
        if (exists_in_nvs("reading_interval"))
        {
            read_int_from_nvs("reading_interval", &reading_interval);
        }
        if (exists_in_nvs("push_interval"))
        {
            read_int_from_nvs("push_interval", &push_interval);
        }

        float reading = read_sensor();
        printf("Reading: %f\n", reading);

        char existing_data[1000];

        read_str_from_fs(existing_data, 1000);
        char *new_data = malloc(2000);
        sprintf(new_data, "%s%f\n", existing_data, reading);
        save_str_to_fs(new_data);

        if (counter >= push_interval)
        {
            counter = 0;
            xTaskCreate(push_data, "push_data", 2048, NULL, 10, NULL);
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(reading_interval));
    }
}

void run()
{
    if (exists_in_nvs("ssid"))
    {
        read_loop();
    }
    else
    {
        start_bluetooth();
    }
}