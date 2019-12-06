/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"

#include "my_wifi.h"

//if now_in_smartconfig,wifi must disconnect,don't reconnect wifi
extern bool can_reconnect_wifi;


static const char *TAG = "my_wifi";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
	    case SYSTEM_EVENT_STA_START:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
	
	      	//confing default ssid and password
			wifi_config_t wifi_config = {
				.sta.ssid = "TP-LINK_1CCF",
				.sta.password = "smart-home",
			};
			ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
			ESP_LOGI(TAG, "Start to connect WIFI:%s, PSW:%s", wifi_config.sta.ssid, wifi_config.sta.password);
		    ESP_ERROR_CHECK( esp_wifi_connect() );
	        break;
	    case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
			
			wifi_config_t w_config;
            memset(&w_config, 0x00, sizeof(wifi_config_t));
            if (ESP_OK == esp_wifi_get_config(WIFI_IF_STA, &w_config)) {
                ESP_LOGI(TAG, ">>>>>> Connected Wifi SSID:%s <<<<<<< \r\n", w_config.sta.ssid);
            } else {
                ESP_LOGE(TAG, "Got wifi config failed");
            }
	        break;
	    case SYSTEM_EVENT_STA_DISCONNECTED:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
			if(can_reconnect_wifi){
				//my_wifi_reconnect();
			}else{
				ESP_LOGI(TAG, "now_in_smartconfig,not to reconnect WIFI");
			}

	        break;
	    default:
	        break;
	}
    return ESP_OK;
}

static void my_wifi_reconnect(void)
{
	wifi_config_t w_config;
    memset(&w_config, 0x00, sizeof(wifi_config_t));
    esp_wifi_get_config(WIFI_IF_STA, &w_config);
	ESP_LOGI(TAG, "now reconnect WIFI:%s PSW:%s", w_config.sta.ssid, w_config.sta.password);
	esp_wifi_connect();
}

void initialise_wifi(void)
{
    tcpip_adapter_init();
	
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );

	//初始化完毕之后会进入wifi回调中的SYSTEM_EVENT_STA_START
}


