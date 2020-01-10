#include "my_head.h"

#include "my_timer.h"

#define TAG "my_timer"

int count;

#if 0
static TimerHandle_t shutdown_timer;

void test_timer_cb(xTimerHandle tmr)
{
	ESP_AUDIO_LOGI(TAG, "hello world!!");
	count++;
	if(count == 5){
		xTimerChangePeriod(shutdown_timer,2000/portTICK_RATE_MS,0);
	}else if(count == 10){
		count = 0;
		xTimerChangePeriod(shutdown_timer,1000/portTICK_RATE_MS,0);
	}
}

void test_timer_start(void)
{
	shutdown_timer = xTimerCreate("shutdown_timer", 
											1000/portTICK_RATE_MS,
											pdTRUE,//AutoReload
											NULL,
											test_timer_cb);

	xTimerStart(shutdown_timer, 0);
	ESP_AUDIO_LOGI(TAG, "xTimerStart!!");
}

void test_timer_stop(void)
{
	xTimerStop(shutdown_timer, 0);
	ESP_AUDIO_LOGI(TAG, "xTimerStop!!");
}

#else

static esp_timer_handle_t shutdown_timer;

static void test_timer_cb(void *timer_arg)
{
   ESP_AUDIO_LOGI(TAG, "hello world!!");
	count++;
	if(count == 5){
		esp_timer_stop(shutdown_timer);
		esp_timer_start_periodic(shutdown_timer,2000/portTICK_RATE_MS * 1000 * 10);
	}else if(count == 10){
		count = 0;
		esp_timer_stop(shutdown_timer);
		esp_timer_start_periodic(shutdown_timer,1000/portTICK_RATE_MS * 1000 * 10);
	}
}

void test_timer_start(void)
{
    esp_timer_create_args_t create_args = {
        .callback = &test_timer_cb,
        .arg = NULL,
        .name = "shutdown_timer",
    };
    esp_timer_create(&create_args, &shutdown_timer);
    esp_timer_start_periodic(shutdown_timer, 1000/portTICK_RATE_MS * 1000 * 10);
	ESP_AUDIO_LOGI(TAG, "esp_timer_start!!");
}

void test_timer_stop(void)
{
	if (shutdown_timer) {
		esp_timer_stop(shutdown_timer);
		esp_timer_delete(shutdown_timer);
	    shutdown_timer = NULL;
	}
	ESP_AUDIO_LOGI(TAG, "esp_timer_start!!");
}

#endif

	

