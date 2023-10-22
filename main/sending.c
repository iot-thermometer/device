#include <stdio.h>
#include "wifi.c"
#include "mqtt.c"
#include "remote_config.c"

void hello()
{
    printf("Hello world!\n");
}

// void push_data()
// {
//     printf("Pushing data...\n");
//     connect_wifi("iPhone (Mateusz)", "12345678");
//     // connect_mqtt();
//     // send_message();
//     while (1)
//     {
//         printf("Pulling config...\n");
//         bool wifi_connected = false;
//         esp_err_t err = read_bool_from_nvs("wifi_connected", &wifi_connected);
//         if (wifi_connected)
//         {
//             char *result = "";
//             result = make_http_request("https://gist.githubusercontent.com/matisiekpl/cd086500b92dfa3b0493aa6f518ad5b7/raw/615acf095ce559ce3738ff5bcac5387b28e27810/device.json");
//             if (strlen(result) > 0)
//                 parse_remote_config(result);
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }