/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "my_head.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spi_flash.h"

//TerminalControl
#include "TerminalControlService.h"


#define APP_TAG "HELLO_WORLD_MAIN"

extern MY_INFO_T My_Info;

void app_main()
{	
    ESP_LOGI(APP_TAG, "[APP] [%s][%s]Startup..", __FILE__, __func__);
    ESP_LOGI(APP_TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(APP_TAG, "[APP] IDF version: %s", esp_get_idf_version());
	//必要
	ESP_ERROR_CHECK(nvs_flash_init());
	
	//开启串口指令控制功能
	terminalControlActive();

	Get_SystemInfo(&My_Info);
	ESP_AUDIO_LOGI(APP_TAG, "Get_SystemInfo success:\n  mac:%s\n  brightness:%d\n  vol:%d", \
					My_Info.device_mac, My_Info.brightness, My_Info.vol);
    
	
    printf("----------0.0.0.1-----------------\n");

}
