#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

static EventGroupHandle_t http_event_group;
static char *result = "";

#define HTTP_BIT BIT0

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;
    static int output_len;
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        if (output_len == 0 && evt->user_data)
        {
            memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
        }
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            int copy_len = 0;
            if (evt->user_data)
            {
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                int content_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)calloc(content_len + 1, sizeof(char));
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (content_len - output_len));
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        if (output_buffer != NULL)
        {
            printf("%s\n", output_buffer);
            result = (char *)malloc(output_len);
            strncpy(result, output_buffer, output_len);
            result[output_len] = '\0';
            free(output_buffer);
            output_buffer = NULL;
            xEventGroupSetBits(http_event_group, HTTP_BIT);
        }
        output_len = 0;
        break;
    default:
        break;
    }
    return ESP_OK;
}

char *make_http_request(const char *url)
{
    http_event_group = xEventGroupCreate();
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_perform(client);

    xEventGroupWaitBits(http_event_group,
                        HTTP_BIT,
                        pdFALSE,
                        pdFALSE,
                        portMAX_DELAY);

    esp_http_client_cleanup(client);
    vEventGroupDelete(http_event_group);

    return result;
}