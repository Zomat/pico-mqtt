#ifndef MQTT_FUNCTIONS_H
#define MQTT_FUNCTIONS_H

#include "lwip/apps/mqtt.h"
#include "stdbool.h"
#include "string.h"

#define WIFI_SSID "NAZWA_WIFI"
#define WIFI_PASS "HASLO_WIFI"
#define MQTT_BROKER_IP "192.168.1.186"

typedef struct MQTT_CLIENT_DATA_T {
    mqtt_client_t* mqtt_client_inst;
    struct mqtt_connect_client_info_t mqtt_client_info;
    uint8_t data[MQTT_OUTPUT_RINGBUF_SIZE];
    uint8_t topic[100];
    uint32_t len;
    bool newTopic;
} MQTT_CLIENT_DATA_T;

extern struct mqtt_connect_client_info_t mqtt_client_info;
extern volatile MQTT_CLIENT_DATA_T *mqtt;

void create_mqtt_client_data(MQTT_CLIENT_DATA_T* mqtt, struct mqtt_connect_client_info_t mqtt_client_info);
err_t connect_mqtt_client(MQTT_CLIENT_DATA_T *mqtt);
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
void mqtt_request_cb(void *arg, err_t err);
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);

#endif // MQTT_FUNCTIONS_H