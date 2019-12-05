/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_audio_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"

#include "esp_shell.h"
#include "TerminalControlService.h"



#define TERM_TAG                            "TERM_CTRL"

static void reboot(void *ref, int argc, char *argv[])
{
    esp_restart();
}

void EspAudioPrintMemory(const char *Tag)
{
	multi_heap_info_t info;
	heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
	ESP_AUDIO_LOGI(Tag, "Totals:\n	free %d allocated %d min_free %d largest_free_block %d\n", \
						info.total_free_bytes, info.total_allocated_bytes, info.minimum_free_bytes, info.largest_free_block);
	ESP_AUDIO_LOGI(Tag, "Memory,total:%d, inter:%d, Dram:%d", esp_get_free_heap_size(),
				heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
	   

}

static void getMem(void *ref, int argc, char *argv[])
{
    EspAudioPrintMemory(TERM_TAG);
}

static void show_version(void *ref, int argc, char *argv[])
{
    printf("\r\n");
    printf("----------------------------- Firmware Version -------------------------------\n");
    printf("|\n" );
    printf("|                  Compile date: %s-%s                        |\n", __DATE__, __TIME__);
    printf("|                  Commit SHA1:%s      |\n", "549f635687dc8157c69aa3dec7710e3bf42e9ca4");
    printf("|                  ESP-IDF :%s                         |\n", IDF_VER);
    printf("------------------------------------------------------------------------------\r\n");
}

const ShellCommand command[] = {
    //system
    {"------system-------", NULL},
    {"mem", getMem},
    {"reboot", reboot},
    {"version", show_version},

    {NULL, NULL}
};

void terminalControlActive(void)
{
	ESP_AUDIO_LOGI(TERM_TAG, "terminalControlActive\r\n");
    shell_init(command, NULL);
}
void terminalControlDeactive(void)
{
    ESP_AUDIO_LOGI(TERM_TAG, "terminalControlStop\r\n");
    shell_stop();
}

