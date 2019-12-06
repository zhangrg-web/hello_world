/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

//TerminalControl
#include "TerminalControlService.h"
//nvs_flash_init
#include "nvs_flash.h"
//log
#include "esp_audio_log.h"


#define APP_TAG "HELLO_WORLD_MAIN"

void app_main()
{	
    ESP_LOGI(APP_TAG, "[APP] Startup..");
    ESP_LOGI(APP_TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(APP_TAG, "[APP] IDF version: %s", esp_get_idf_version());
	//必要
	ESP_ERROR_CHECK(nvs_flash_init());
	
	//开启串口指令控制功能
	terminalControlActive();
    
	
    printf("Hello world!\n");

}
