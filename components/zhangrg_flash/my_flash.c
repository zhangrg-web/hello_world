#include "my_head.h"

#include "my_flash.h"

static const char* SYSINFO_NAMESPACE = "SYSINFO";
static const char* MY_DATA = "MY_DATA";

#define Tag "my_flash"

MY_INFO_T My_Info;

void Set_Default_SystemInfo(void)
{
	uint8_t mac[6] = {0};
	esp_read_mac((uint8_t *)mac, ESP_MAC_ETH);
	sprintf(My_Info.device_mac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	My_Info.brightness = 50;
	My_Info.vol	= 20;
	
	ESP_AUDIO_LOGI(Tag, "Set_Default_SystemInfo start... ...");
	Set_SystemInfo(My_Info);
}

void Set_SystemInfo(MY_INFO_T My_Info)
{
	nvs_handle handle;
	
    ESP_ERROR_CHECK( nvs_open( SYSINFO_NAMESPACE, NVS_READWRITE, &handle) );
  	size_t len = sizeof(MY_INFO_T);
    ESP_ERROR_CHECK( nvs_set_blob( handle, MY_DATA, &My_Info, len));

    ESP_ERROR_CHECK( nvs_commit(handle) );
	
    nvs_close(handle);
	ESP_AUDIO_LOGI(Tag, "Set_SystemInfo success:\n  mac:%s\n  brightness:%d\n  vol:%d", \
							My_Info.device_mac, My_Info.brightness, My_Info.vol);
}

void Get_SystemInfo(MY_INFO_T *My_Info)
{
	nvs_handle handle;

    ESP_ERROR_CHECK( nvs_open( SYSINFO_NAMESPACE, NVS_READWRITE, &handle) );
   	size_t len = sizeof(MY_INFO_T);
	#if 1
    esp_err_t err = nvs_get_blob( handle, MY_DATA, My_Info, &len);
	if(err != ESP_OK){
		ESP_AUDIO_LOGW(Tag, "nvs_get_blob fail, now Set_Default_SystemInfo");
		Set_Default_SystemInfo();
		nvs_get_blob( handle, MY_DATA, My_Info, &len);
	}
	#else
	ESP_ERROR_CHECK( nvs_get_blob( handle, MY_DATA, &My_Info, &len));
	#endif
    nvs_close(handle);
}

