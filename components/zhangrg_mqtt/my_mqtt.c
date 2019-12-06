#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "my_mqtt.h"

static const char *TAG = "MQTTS_SAMPLE";


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "MQTT2", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d, TOPIC:MQTT2", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "MQTT3", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d TOPIC:MQTT3", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
		case MQTT_EVENT_BEFORE_CONNECT:
			ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            break;
		default:
			ESP_LOGI(TAG, "MQTT_EVENT  faile!!");
            break;
    }
    return ESP_OK;
}

void mqtt_app_start(void)
{
	uint8_t mac[6] = {0};
	char death_msg[24] = {0};
	esp_read_mac((uint8_t *)mac, ESP_MAC_ETH);
	sprintf(death_msg, "death:%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
    const esp_mqtt_client_config_t mqtt_cfg = {
		.event_handle = mqtt_event_handler,
        .uri = "mqtts://tels.giec.cn:8883",    // for mqtt over ssl
        //.uri = "mqtt://192.168.140.241:1883", //for mqtt over tcp

		.keepalive = 10,
        .lwt_topic = "MQTT3",                /*!< LWT (Last Will and Testament) message topic (NULL by default) */
   		.lwt_msg = death_msg,                /*!< LWT message (NULL by default) */
    	.lwt_qos = 2,                        /*!< LWT message qos */
    	.lwt_retain = 1,                     /*!< LWT retained message flag */
    	.lwt_msg_len = sizeof(death_msg),  
	
        .client_cert_pem = CLIENT_CA_CRT,
        .client_key_pem = PRIVATE_KEY_RSA,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}
