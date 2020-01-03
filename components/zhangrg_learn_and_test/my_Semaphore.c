#include "esp_err.h"
#include "esp_audio_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"



static SemaphoreHandle_t xSemaphore = NULL;
	
static const char *TAG = "my_Semaphore_test";

void TaskA( void *pvParameters )
{
    for( ;; )
    {
        xSemaphoreTake( xSemaphore, portMAX_DELAY );
        {
            printf("Task A\r\n");
        }
        xSemaphoreGive( xSemaphore );
        vTaskDelay( 1000/portTICK_RATE_MS );
    }
}
 
void TaskB( void *pvParameters )
{
    for( ;; )
    {
        xSemaphoreTake( xSemaphore, portMAX_DELAY );
        {
            printf("Task B\r\n");
        }
        xSemaphoreGive( xSemaphore );
        vTaskDelay( 1000/portTICK_RATE_MS );
    }
}

void my_Semaphore_start(void)
{

	 /* 创建互斥信号量 */
    xSemaphore = xSemaphoreCreateMutex();
    /* 建立任务 */
    xTaskCreate( TaskA, "TaskA", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, NULL );
    xTaskCreate( TaskB, "TaskB", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+4, NULL );
}

