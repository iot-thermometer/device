#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

static EventGroupHandle_t wifi_event_group;

#define WIFI_DISCONNECTED_BIT BIT0
#define WIFI_CONNECTED_BIT BIT1

static const char *WIFI_TAG = "WIFI";

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        save_bool_to_nvs("wifi_connected", false);
        if (wifi_event_group != NULL)
            xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        bool wifi_enabled = false;
        read_bool_from_nvs("wifi_enabled", &wifi_enabled);
        if (wifi_enabled) {
            read_bool_from_nvs("wifi_enabled", &wifi_enabled);
            ESP_LOGI(WIFI_TAG, "Wifi is enabled, reconnecting");
            esp_wifi_connect();
        } else {
            ESP_LOGI(WIFI_TAG, "Wifi is disabled, not reconnecting");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(WIFI_TAG, "Got ip: "
        IPSTR, IP2STR(&event->ip_info.ip));
        save_bool_to_nvs("wifi_connected", true);
        if (wifi_event_group != NULL)
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;

void disconnect_wifi() {
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));

    esp_wifi_disconnect();
    esp_wifi_stop();
}

void init_wifi() {
    save_bool_to_nvs("wifi_enabled", false);
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void connect_wifi(const char *ssid, const char *pass) {
    ESP_LOGI(WIFI_TAG, "Connecting to %s...", ssid);
    wifi_event_group = xEventGroupCreate();


    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
            .sta = {
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };
    strncpy((char *) wifi_config.sta.ssid, (char *) ssid, 32);
    strncpy((char *) wifi_config.sta.password, (char *) pass, 64);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_DISCONNECTED_BIT | WIFI_CONNECTED_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    }

    vEventGroupDelete(wifi_event_group);

    wifi_event_group = NULL;
}