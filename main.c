#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"

#include "lwip/apps/mqtt.h"
#include "lwipopts.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

#define WIFI_SSID "MartinRouterKing"
#define WIFI_PASS "NieMaToJakGrunwald69"

typedef struct MQTT_CLIENT_DATA_T_ {
    mqtt_client_t* mqtt_client_inst;
    struct mqtt_connect_client_info_t mqtt_client_info;
    uint8_t data[MQTT_OUTPUT_RINGBUF_SIZE];
    uint8_t topic[100];
    uint32_t len;
    bool playing;
    bool newTopic;
} MQTT_CLIENT_DATA_T;
 
volatile MQTT_CLIENT_DATA_T *mqtt;
 
struct mqtt_connect_client_info_t mqtt_client_info=
{
  "rpico",
  NULL, /* user */
  NULL, /* pass */
  0,  /* keep alive */
  NULL, /* will_topic */
  NULL, /* will_msg */
  0,    /* will_qos */
  0     /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
  , NULL
#endif
};

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    MQTT_CLIENT_DATA_T* mqtt_client = (MQTT_CLIENT_DATA_T*)arg;
    //LWIP_UNUSED_ARG(data);
    strncpy(mqtt_client->data, data, len);
    LWIP_PLATFORM_DIAG(("MQTT client \"%s\" incoming data cb: topic %s, data: %s\n", mqtt_client_info.client_id,
          mqtt_client->topic, mqtt_client->data));

    strncpy(mqtt_client->data, data, len);
    mqtt_client->len=len;
    mqtt_client->data[len]='\0';
    
    mqtt_client->newTopic=true;
    mqtt_client->playing=false;

    printf("new top: %s \n", mqtt->newTopic == true ? "true" : "false");
}
 
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
  MQTT_CLIENT_DATA_T* mqtt_client = (MQTT_CLIENT_DATA_T*)arg;
  strcpy(mqtt_client->topic, topic);

  LWIP_PLATFORM_DIAG(("MQTT client \"%s\" publish cb: topic %s, len %d\n", mqtt_client_info.client_id,
          topic, (int)tot_len));
}
 
static void mqtt_request_cb(void *arg, err_t err) {
  MQTT_CLIENT_DATA_T* mqtt_client = ( MQTT_CLIENT_DATA_T*)arg;
 
  LWIP_PLATFORM_DIAG(("MQTT client \"%s\" request cb: err %d\n", mqtt_client_info.client_id, (int)err));
}
 
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
  MQTT_CLIENT_DATA_T* mqtt_client = (MQTT_CLIENT_DATA_T*)arg;
  LWIP_UNUSED_ARG(client);
 
  LWIP_PLATFORM_DIAG(("MQTT client \"%s\" connection cb: status %d\n", mqtt_client_info.client_id, (int)status));
 
  if (status == MQTT_CONNECT_ACCEPTED) {
    mqtt_sub_unsub(client,
            "temp", 0,
            mqtt_request_cb, LWIP_CONST_CAST(void*, &mqtt_client_info),
            1);

  }
}

void wait_connecting() {
    printf("WAIT CONNECTING \n");
}
 
void connected() {
    printf("CONNECTED \n");
}

int main() {
    stdio_init_all();
    mqtt = (MQTT_CLIENT_DATA_T*)calloc(1, sizeof(MQTT_CLIENT_DATA_T));
 
    if (!mqtt) {
        printf("mqtt client instant ini error\n");
        return 0;
    }
    mqtt->playing=false;
    mqtt->newTopic=false;
    mqtt->mqtt_client_info = mqtt_client_info;
 
    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return 1;
    }
    wait_connecting();
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("failed to connect\n");
        return 1;
    }

    ip_addr_t addr;
    if (!ip4addr_aton("192.168.0.193", &addr)) {
        printf("ip error\n");
        return 0;
    }
 
    mqtt->mqtt_client_inst = mqtt_client_new();
    mqtt_set_inpub_callback(mqtt->mqtt_client_inst, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, mqtt);

    err_t err = mqtt_client_connect(mqtt->mqtt_client_inst, &addr, MQTT_PORT, &mqtt_connection_cb, LWIP_CONST_CAST(void*, &mqtt_client_info), &mqtt->mqtt_client_info);
    if (err != ERR_OK) {
      printf("connect error\n");
      return 0;
    }
    connected();
 
    while(1) {
      if (mqtt->newTopic) { 
          printf("GET TOPIC: %s \n", mqtt->topic);
          if (strcmp(mqtt->topic, "temp") == 0) {
            printf("TOPIC HANDEL \n");
          }

          mqtt->newTopic = false;
      }
    }
}
