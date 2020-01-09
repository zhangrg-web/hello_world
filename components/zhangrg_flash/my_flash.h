#ifndef _MY_FLASH_H_
#define _MY_FLASH_H_

typedef struct{
	char device_mac[18];
	char brightness;
	char vol;
}MY_INFO_T;

void Set_SystemInfo(MY_INFO_T My_Info);
void Get_SystemInfo(MY_INFO_T *My_Info);

void Set_Default_SystemInfo(void);

#endif