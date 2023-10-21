#include "cJSON.h"

void parse_remote_config(const char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
    }

    int reading_interval = cJSON_GetObjectItem(root, "reading_interval")->valueint;
    int push_interval = cJSON_GetObjectItem(root, "push_interval")->valueint;

    printf("Reading interval: %d\n", reading_interval);
    printf("Push interval: %d\n", push_interval);
    save_int_to_nvs("reading_interval", reading_interval);
    save_int_to_nvs("push_interval", push_interval);

    cJSON_Delete(root);
}
