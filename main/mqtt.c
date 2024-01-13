#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "mqtt_client.h"

#define MQTT_BROKER_URI "mqtt://srv3.enteam.pl:1883"
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

static esp_mqtt_client_handle_t client;

static EventGroupHandle_t mqtt_event_group;

#define MQTT_CONNECTED_BIT BIT0
#define MQTT_DISCONNECTED_BIT BIT1

static const char *MQTT_TAG = "MQTT";
bool published = false;

void disconnect_mqtt() {
    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);

    ESP_LOGI(MQTT_TAG, "Disconnected");
    vTaskDelay(pdMS_TO_TICKS(1000));
}

static esp_err_t mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    ESP_LOGD(MQTT_TAG, "Event: %d", event->event_id);
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(MQTT_TAG, "Connected");
            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        case MQTT_EVENT_DISCONNECTED:
            if (mqtt_event_group != NULL) {
                xEventGroupSetBits(mqtt_event_group, MQTT_DISCONNECTED_BIT);
            }
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            published = true;
            break;
        default:
            break;
    }
    return ESP_OK;
}

bool connect_mqtt() {
    mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
            .broker.address.uri = MQTT_BROKER_URI,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    EventBits_t bits = xEventGroupWaitBits(mqtt_event_group,
                                           MQTT_CONNECTED_BIT | MQTT_DISCONNECTED_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    esp_mqtt_client_unregister_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler);

    vEventGroupDelete(mqtt_event_group);
    mqtt_event_group = NULL;

    if (bits & MQTT_CONNECTED_BIT) {
        return true;
    } else if (bits & MQTT_DISCONNECTED_BIT) {
        return false;
    }
    return false;
}

bool send_message(char *topic, char *data) {
    published = false;
    esp_mqtt_client_publish(client, topic, data, 0, 0, 0);

    while (!published) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return true;
}