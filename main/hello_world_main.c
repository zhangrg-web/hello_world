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

void test_timer_periodic_cb(void *arg);
void test_timer_once_cb(void *arg);



//定义2个定时器句柄
esp_timer_handle_t test_p_handle = 0;
esp_timer_handle_t test_o_handle = 0;

//定义一个单次运行的定时器结构体
esp_timer_create_args_t test_once_arg = { 
	.callback = &test_timer_once_cb, //设置回调函数
	.arg = NULL, //不携带参数
	.name = "TestOnceTimer" //定时器名字
};
//定义一个周期重复运行的定时器结构体
esp_timer_create_args_t test_periodic_arg = { 
	.callback = &test_timer_periodic_cb, //设置回调函数
	.arg = NULL, //不携带参数
	.name = "TestPeriodicTimer" //定时器名字
};

void test_timer_periodic_cb(void *arg) {

	int64_t tick = esp_timer_get_time();

	printf("方法回调名字: %s , 距离定时器开启时间间隔 = %lld \r\n", __func__, tick);

	if (tick > 100000000) {
		//停止定时器工作，并获取是否停止成功
		esp_err_t err = esp_timer_stop(test_p_handle);
		printf("要停止的定时器名字：%s , 是否停止成功：%s", test_periodic_arg.name,
				err == ESP_OK ? "ok!\r\n" : "failed!\r\n");
		err = esp_timer_delete(test_p_handle);
		printf("要删除的定时器名字：%s , 是否停止成功：%s", test_periodic_arg.name,
				err == ESP_OK ? "ok!\r\n" : "failed!\r\n");
	}
	//低电平
	gpio_set_level(16, 0);
	//延迟
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	//高电平
	gpio_set_level(16, 1);
	//延迟
	vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void test_timer_once_cb(void *arg) {
    int64_t tick = esp_timer_get_time();
	printf("方法回调名字: %s , 距离定时器开启时间间隔 = %lld \r\n", __func__, tick);
	esp_err_t err = esp_timer_delete(test_o_handle);
	printf("要删除的定时器名字：%s , 是否停止成功：%s", test_periodic_arg.name,
			err == ESP_OK ? "ok!\r\n" : "failed!\r\n");
}

void app_main()
{
    printf("Hello world!\n");

	 //GPIO16初始化
	gpio_pad_select_gpio(16);
	gpio_set_direction(16, GPIO_MODE_OUTPUT);

		
   //开始创建一个重复周期的定时器并且执行
	esp_err_t err = esp_timer_create(&test_periodic_arg, &test_p_handle);
	err = esp_timer_start_periodic(test_p_handle, 1000 * 1000);
	printf("重复周期运行的定时器创建状态码: %s", err == ESP_OK ? "ok!\r\n" : "failed!\r\n");

		
	//开始创建一个单次周期的定时器并且执行
	err = esp_timer_create(&test_once_arg, &test_o_handle);
	err = esp_timer_start_once(test_o_handle, 10 * 1000 * 1000);
	printf("单次运行的定时器创建状态码: %s", err == ESP_OK ? "ok!\r\n" : "failed!\r\n");

#if 0
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);

#endif
	//重启
    //esp_restart();
}
