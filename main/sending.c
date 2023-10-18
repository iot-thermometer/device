#include <stdio.h>
#include "network.c"
#include "mqtt.c"

void hello() {
    printf("Hello world!\n");
}

void push_data(){
    printf("Pushing data...\n");
    connect_wifi("iPhone (Mateusz)", "12345678");
    connect_mqtt();
    send_message();
}