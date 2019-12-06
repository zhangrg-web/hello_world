/* AIRKISS example

   
*/
#include "my_airkiss.h"
#include "esp_err.h"
#include "esp_audio_log.h"
#include "airkiss_connect.h"

static const char *TAG = "my_airkiss";

void my_airkiss_start(void)
{
	esp_err_t res = ESP_FAIL;
	ESP_AUDIO_LOGI(TAG, "*****Airkiss connect start*****");
    res = airkiss_connect(60 * 1000);
	
    switch (res) {
        case ESP_OK:
            ESP_AUDIO_LOGI(TAG, "airkissconfig success");
            break;
        case ESP_ERR_INVALID_STATE:
            ESP_AUDIO_LOGE(TAG, "Now in airkissconfig, please wait or close airkissconfig");
            break;
        case ESP_ERR_TIMEOUT:
            ESP_AUDIO_LOGI(TAG, "airkissconfig timeout");
            break;
        case ESP_FAIL:
            ESP_AUDIO_LOGW(TAG, "airkissconfig fail");
            break;
        default:
            break;
    }
}

