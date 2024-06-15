#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"

#include "lwipopts.h"
#include "drivers/mqtt.h"

#include "drivers/SSD1306_I2C.h"

SSD1306 oled = {};

#define OLED_SDA 10
#define OLED_SCL 11

void oled_init()
{
    i2c_init(i2c1, SSD1306_I2C_CLK * 1000);
    gpio_set_function(OLED_SDA, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_SDA);
    gpio_pull_up(OLED_SCL);

    SSD1306_init(&oled, i2c1);
    SSD1306_SetFont(&oled, "8x8");
}

struct mqtt_connect_client_info_t mqtt_client_info = {
  .client_id = "rpico",
  .client_user = NULL,
  .client_pass = NULL,
  .keep_alive = 0,
  .will_topic = NULL,
  .will_msg = NULL,
  .will_qos = 0,
  .will_retain = 0
#if LWIP_ALTCP && LWIP_ALTCP_TLS
  , NULL
#endif
};

int main() {
    stdio_init_all();

    /** OLED */
    oled_init();

    volatile MQTT_CLIENT_DATA_T *mqtt = (MQTT_CLIENT_DATA_T*)malloc(sizeof(MQTT_CLIENT_DATA_T));

    if (!mqtt) {
        printf("mqtt client instant ini error\n");
        return 0;
    }
 
    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return 1;
    }
    
    /** CONNECT TO WIFI */
    cyw43_arch_enable_sta_mode();
    while(1) {
      if (!cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
          break;
      }
      printf("failed to connect, retrying...\n");
      SSD1306_WriteText(&oled, "failed to connect, retrying...");
      SSD1306_NextLine(&oled);
    }

    SSD1306_WriteText(&oled, "Connected!");
    SSD1306_NextLine(&oled);

    /** INIT AND CONNECT MQTT */
    create_mqtt_client_data(mqtt, mqtt_client_info);
    if (connect_mqtt_client(mqtt) != ERR_OK) { printf("connect error\n"); return 0; }

    SSD1306_ClearDisplay(&oled);
    /** MQTT LOOP */
    char oled_buffer[20];
    while(1) {
       if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP) {
        printf("Disconnected from WiFi, trying to reconnect...\n");
        while(1) {
          if (!cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            printf("Reconnected to WiFi\n");
            break;
          }
          printf("Failed to reconnect, retrying...\n");
        }
      }

      if (mqtt->newTopic) { 
          if (strcmp(mqtt->topic, "msg") == 0) {
            SSD1306_ClearDisplay(&oled);
            SSD1306_WriteText(&oled, mqtt->data);
            memset(oled_buffer, 0, sizeof(oled_buffer));
          }

          mqtt->newTopic = false;
          err_t err = mqtt_publish(mqtt->mqtt_client_inst, "volt", "25", 2, 0, 0, mqtt_request_cb, LWIP_CONST_CAST(void*, mqtt));
          if (err != ERR_OK) printf("publish error\n");
      }
    }

    free((void *)mqtt);
    return 0;
}
