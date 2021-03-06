#ifndef _MY_HEAD_H_
#define _MY_HEAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//log
#include "esp_audio_log.h"
//esp_err_t
#include "esp_err.h"
//mac
#include "esp_system.h"
//flash
#include "nvs_flash.h"
//ota
#include "esp_https_ota.h"
//freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

//esp_timer
#include "esp_timer.h"

//my test
#include "my_wifi.h"
#include "my_smartconfig.h"
#include "my_mqtt.h"
#include "my_airkiss.h"
#include "my_Semaphore.h"
#include "my_flash.h"
#include "my_ota.h"
#include "my_timer.h"

#endif