#include "mqtt.h"
#include "lwip/apps/mqtt.h"

void create_mqtt_client_data(MQTT_CLIENT_DATA_T* mqtt, struct mqtt_connect_client_info_t mqtt_client_info) {
    mqtt->mqtt_client_inst = NULL;
    mqtt->mqtt_client_info = mqtt_client_info;
    mqtt->len = 0;
    mqtt->newTopic = false;
    memset(mqtt->data, 0, sizeof(mqtt->data));
    memset(mqtt->topic, 0, sizeof(mqtt->topic));
}

err_t connect_mqtt_client(MQTT_CLIENT_DATA_T *mqtt) {
    mqtt->mqtt_client_info = mqtt_client_info;

    ip_addr_t addr;
    if (!ip4addr_aton(MQTT_BROKER_IP, &addr)) {
        return ERR_VAL;
    }

    mqtt->mqtt_client_inst = mqtt_client_new();
    mqtt_set_inpub_callback(mqtt->mqtt_client_inst, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, mqtt);

    err_t err = mqtt_client_connect(
      mqtt->mqtt_client_inst,
      &addr,
      MQTT_PORT,
      &mqtt_connection_cb,
      LWIP_CONST_CAST(void*, &mqtt->mqtt_client_info),
      &mqtt->mqtt_client_info
    );

    return err;
}

void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
  MQTT_CLIENT_DATA_T* mqtt_client = (MQTT_CLIENT_DATA_T*)arg;
  strcpy(mqtt_client->topic, topic);

  LWIP_PLATFORM_DIAG(("MQTT client \"%s\" publish cb: topic %s, len %d\n", mqtt_client->mqtt_client_info.client_id,
          topic, (int)tot_len));
}

void mqtt_request_cb(void *arg, err_t err) {
  MQTT_CLIENT_DATA_T* mqtt_client = ( MQTT_CLIENT_DATA_T*)arg;

  LWIP_PLATFORM_DIAG(("MQTT client \"%s\" request cb: err %d\n", mqtt_client->mqtt_client_info.client_id, (int)err));
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
  MQTT_CLIENT_DATA_T* mqtt_client = (MQTT_CLIENT_DATA_T*)arg;
  LWIP_UNUSED_ARG(client);

  LWIP_PLATFORM_DIAG(("MQTT client \"%s\" connection cb: status %d\n", mqtt_client->mqtt_client_info.client_id, (int)status));

  if (status == MQTT_CONNECT_ACCEPTED) {
    mqtt_sub_unsub(
      client,
      "msg",
      0,
      mqtt_request_cb,
      LWIP_CONST_CAST(void*, &mqtt_client->mqtt_client_info),
      1
    );
  }
}

void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    MQTT_CLIENT_DATA_T* mqtt_client = (MQTT_CLIENT_DATA_T*)arg;
   
    strncpy(mqtt_client->data, data, len);
    LWIP_PLATFORM_DIAG(("MQTT client \"%s\" incoming data cb: topic %s, data: %s\n", mqtt_client->mqtt_client_info.client_id,
          mqtt_client->topic, mqtt_client->data));

    strncpy(mqtt_client->data, data, len);
    mqtt_client->len=len;
    mqtt_client->data[len]='\0';
    
    mqtt_client->newTopic = true;
}