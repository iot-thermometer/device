#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "mqtt_client.h"

#define MQTT_BROKER_URI "mqtt://srv3.enteam.pl:1883"
#define MQTT_TOPIC "sensors"
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

static esp_mqtt_client_handle_t client;

static EventGroupHandle_t mqtt_event_group;

#define MQTT_CONNECTED_BIT BIT0
#define MQTT_DISCONNECTED_BIT BIT1

static esp_err_t mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    ESP_LOGD("", "Event: %d", event->event_id);
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI("", "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI("", "MQTT_EVENT_DISCONNECTED");
        break;
    default:
        break;
    }
    return ESP_OK;
}

void disconnect_mqtt()
{
    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);

    printf("Disconnected from mqtt\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
}

bool connect_mqtt()
{
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
    vEventGroupDelete(mqtt_event_group);

    if (bits & MQTT_CONNECTED_BIT)
    {
        ESP_LOGI("", "connected to mqtt");
        return true;
    }
    else if (bits & MQTT_DISCONNECTED_BIT)
    {
        ESP_LOGI("", "failed to connect to mqtt");
        return false;
    }
    return false;
}

bool send_message(char *data)
{
    esp_mqtt_client_publish(client, MQTT_TOPIC, data, 0, 0, 0);
    return true;
}