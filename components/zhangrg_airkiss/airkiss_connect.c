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

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "airkiss.h"
#include "airkiss_connect.h"



#define AIRKISS_DEBUG_ON                0

#ifdef CONFIG_AIRKISS_NOTIFY
#define AIRKISS_NOTIFY_TASK_PRIORITY    3
#define AIRKISS_NOTIFY_TASK_STACK_SIZE  4096
#define AIRKISS_DEVICE_TYPE             "gh_9e2cff3dfa51"   // wechat public number
#define AIRKISS_DEVICE_ID               "122475"            // model ID
#define AIRKISS_DEFAULT_LAN_PORT        12476
#endif

#define AIRKISS_ACK_TASK_PRIORITY       2
#define AIRKISS_ACK_TASK_STACK_SIZE     2048
#define AIRKISS_ACK_PORT                10000
#define AIRKISS_CHANNEL_CHANGE_PERIOD   130
#define AIRKISS_MAX_CHANNEL_NUM         17
#define AIRKISS_MIN_RSSI                -90

typedef struct {
    bool ap_exist;
    uint8_t primary_chan;
    wifi_second_chan_t second_chan;
} airkiss_chan_t;

static const char *TAG = "airkiss";

/* FreeRTOS event group to signal when we are connected & airkiss is done */
EventGroupHandle_t airkiss_evt_group = NULL;

static int s_sniffer_stop_flag = 1;
static int s_cur_chan_idx = AIRKISS_MAX_CHANNEL_NUM - 1;
static airkiss_context_t *ak_ctx;
static uint8_t ak_random_num = 0;

static esp_timer_handle_t channel_change_timer;
static esp_timer_handle_t airkiss_timeout_timer;
static TaskHandle_t air_answer_task_handle;

static airkiss_chan_t s_airkiss_chan_tab[AIRKISS_MAX_CHANNEL_NUM] = {
    {false, 1, WIFI_SECOND_CHAN_ABOVE},
    {false, 2, WIFI_SECOND_CHAN_ABOVE},
    {false, 3, WIFI_SECOND_CHAN_ABOVE},
    {false, 4, WIFI_SECOND_CHAN_ABOVE},
    {false, 5, WIFI_SECOND_CHAN_ABOVE},
    {false, 5, WIFI_SECOND_CHAN_BELOW},
    {false, 6, WIFI_SECOND_CHAN_ABOVE},
    {false, 6, WIFI_SECOND_CHAN_BELOW},
    {false, 7, WIFI_SECOND_CHAN_ABOVE},
    {false, 7, WIFI_SECOND_CHAN_BELOW},
    {false, 8, WIFI_SECOND_CHAN_BELOW},
    {false, 9, WIFI_SECOND_CHAN_BELOW},
    {false, 10, WIFI_SECOND_CHAN_BELOW},
    {false, 11, WIFI_SECOND_CHAN_BELOW},
    {false, 12, WIFI_SECOND_CHAN_BELOW},
    {false, 13, WIFI_SECOND_CHAN_BELOW},
    {false, 14, WIFI_SECOND_CHAN_NONE},
};

const airkiss_config_t ak_conf = {
    (airkiss_memset_fn) &memset,
    (airkiss_memcpy_fn) &memcpy,
    (airkiss_memcmp_fn) &memcmp,
    (airkiss_printf_fn) &printf,
};

static int airkiss_get_errno(int fd)
{
    int sock_errno = 0;
    u32_t optlen = sizeof(sock_errno);

    getsockopt(fd, SOL_SOCKET, SO_ERROR, &sock_errno, &optlen);

    return sock_errno;
}

#ifdef CONFIG_AIRKISS_NOTIFY
//#######################~send notify~#########################
static void airkiss_notify_task(void *pvParameters)
{
    airkiss_lan_pack_param *lan_param = (airkiss_lan_pack_param *)pvParameters;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    struct sockaddr_in broad_addr;
    struct timeval tv;
    fd_set rfds, exfds;
    socklen_t addr_len = sizeof(remote_addr);
    int fd = -1;
    int send_socket = -1;
    uint16_t buf_len = 200;
    uint16_t resp_len;
    uint16_t recv_len;
    uint16_t req_len;
    uint8_t *buf = NULL;
    uint8_t *req_buf = NULL;
    int ret, err;
    airkiss_lan_ret_t lan_ret;

    buf = malloc(buf_len);
    if (buf == NULL) {
        ESP_LOGE(TAG, "buf allocate fail");
        goto _fail;
    }

    req_buf = malloc(buf_len);
    if (req_buf == NULL) {
        ESP_LOGE(TAG, "lan buf allocate fail");
        goto _fail;
    }

    memset(req_buf, 0, buf_len);
    req_len = buf_len;
    ret = airkiss_lan_pack(AIRKISS_LAN_SSDP_NOTIFY_CMD,
                           lan_param->appid , lan_param->deviceid, 0, 0, req_buf, &req_len, &ak_conf);

    if (ret != AIRKISS_LAN_PAKE_READY) {
        ESP_LOGE(TAG, "Pack lan packet error!");
        goto _fail;
        return;
    }

    do {
        send_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (send_socket == -1) {
            ESP_LOGE(TAG, "failed to create sock!");
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while (send_socket == -1);

    memset(&broad_addr, 0, sizeof(broad_addr));
    broad_addr.sin_family = AF_INET;
    broad_addr.sin_addr.s_addr = INADDR_BROADCAST;
    broad_addr.sin_port = htons(AIRKISS_DEFAULT_LAN_PORT);
    broad_addr.sin_len = sizeof(broad_addr);

    do {
        fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //IPPROTO_UDP
        if (fd == -1) {
            ESP_LOGE(TAG, "failed to create sock!");
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while (fd == -1);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(AIRKISS_DEFAULT_LAN_PORT);
    local_addr.sin_len = sizeof(local_addr);

    ret = bind(fd, (const struct sockaddr *)&local_addr, sizeof(local_addr));
    if (ret) {
        err = airkiss_get_errno(fd);
        ESP_LOGE(TAG, "airkiss bind local port ERROR! errno %d", err);
        goto _out;
    }

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    uint8_t re_sent_num = 10;

    while (re_sent_num) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        FD_ZERO(&exfds);
        FD_SET(fd, &exfds);

        ret = select(fd + 1, &rfds, NULL, &exfds, &tv);
        if (ret > 0) {
            if (FD_ISSET(fd, &exfds) || !FD_ISSET(fd, &rfds)) {
                ESP_LOGE(TAG, "Receive AIRKISS_LAN_SSDP_REQ select error!");
                goto _out;
            }

            memset(buf, 0, buf_len);
            recv_len = recvfrom(fd, buf, buf_len, 0,
                                (struct sockaddr *)&remote_addr, (socklen_t *)&addr_len);

            lan_ret = airkiss_lan_recv(buf, recv_len, &ak_conf);
            if (lan_ret == AIRKISS_LAN_SSDP_REQ) {
                ESP_LOGD(TAG, "AIRKISS_LAN_SSDP_REQ");

                memset(buf, 0, buf_len);
                resp_len = buf_len;
                lan_ret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD,
                                           lan_param->appid , lan_param->deviceid, 0, 0, buf, &resp_len, &ak_conf);
                if (lan_ret != AIRKISS_LAN_PAKE_READY) {
                    ESP_LOGE(TAG, "Pack lan packet error! errno %d", lan_ret);
                    goto _out;
                }

                ret = sendto(fd, buf, resp_len, 0,
                             (struct sockaddr *)&remote_addr, sizeof(remote_addr));
                if (ret < 0) {
                    err = airkiss_get_errno(fd);
                    if (err != ENOMEM && err != EAGAIN) {
                        ESP_LOGE(TAG, "send notify msg ERROR! errno %d", err);
                        goto _out;
                    }
                } else {
                    ESP_LOGD(TAG, "send notify msg OK!");
                    re_sent_num --;
                }
            }
        } else {
            ret = sendto(send_socket, req_buf, req_len, 0,
                         (const struct sockaddr *)&broad_addr, sizeof(broad_addr));
            if (ret < 0) {
                err = airkiss_get_errno(fd);
                if (err != ENOMEM && err != EAGAIN) {
                    ESP_LOGE(TAG, "send notify msg ERROR! errno %d", err);
                    goto _out;
                }
            } else {
                ESP_LOGI(TAG, "send notify msg %d OK!", re_sent_num);
                re_sent_num --;
            }
        }
    }

_out:
    close(fd);
    close(send_socket);
_fail:
    if (buf) {
        free(buf);
        buf = NULL;
    }
    if (req_buf) {
        free(req_buf);
        req_buf = NULL;
    }
    free(lan_param->appid);
    free(lan_param->deviceid);
    free(lan_param);
    xEventGroupClearBits(airkiss_evt_group, WIFI_CONNECTED_BIT | AIRKISS_STOP_BIT | AIRKISS_TIMEOUT_BIT);
    vTaskDelete(NULL);
}

void airkiss_ssdp_notify(airkiss_lan_pack_param *lan_param)
{
    if (lan_param == NULL) {
        ESP_LOGE(TAG, "lan_param invalid");
        return;
    }
    airkiss_lan_pack_param *pack = malloc(sizeof(airkiss_lan_pack_param));
    pack->appid = strdup(lan_param->appid);
    pack->deviceid = strdup(lan_param->deviceid);
    xTaskCreate(airkiss_notify_task, "airkiss_notify_task", AIRKISS_NOTIFY_TASK_STACK_SIZE, pack, AIRKISS_NOTIFY_TASK_PRIORITY, NULL);
}

#endif //CONFIG_AIRKISS_NOTIFY

static void airkiss_send_ack_task(void *pvParameters)
{
    tcpip_adapter_ip_info_t local_ip;
    struct sockaddr_in server_addr;
    socklen_t sin_size = sizeof(server_addr);
    int send_socket = 0;
    uint8_t buf[7];
    size_t sendlen;
    int count = 1;
    int ret, err;

    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_BROADCAST;
    server_addr.sin_port = htons(AIRKISS_ACK_PORT);

//0.0.0.32
/*
	uint8_t mac[6] = {0};
	char macaddr[18] = {0};
	esp_read_mac((uint8_t *)mac, ESP_MAC_ETH);
	sprintf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	//ESP_LOGI(TAG, "%s", macaddr);
*/
    buf[0] = (uint8_t)ak_random_num;
    esp_wifi_get_mac(WIFI_IF_STA, &buf[1]);

    vTaskDelay(200 / portTICK_RATE_MS);

    while (1) {
        /* Get local IP address of station */
        ret = tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &local_ip);
        if ((ESP_OK == ret) && (local_ip.ip.addr != INADDR_ANY)) {
            /* Create UDP socket. */
            send_socket = socket(AF_INET, SOCK_DGRAM, 0);
            if (send_socket < 0) {
                ESP_LOGE(TAG, "Creat airkiss udp socket failed");
                vTaskDelay(1000 / portTICK_RATE_MS);
                continue;
            }

            while (1) {
                /* Send airkiss ACK every 100ms. */
                vTaskDelay(100 / portTICK_RATE_MS);
				//0.0.0.32
                sendlen = sendto(send_socket, buf, 7, 0,
                                 (struct sockaddr *) &server_addr, sin_size);
						
						//sendto(send_socket, macaddr, 18, 0,
                                 //(struct sockaddr *) &server_addr, sin_size);
                if (sendlen > 0) {
                    /* Totally send 30 airkiss ACKs. Then airkiss is successful. */
                    if (count++ >= 30) {
                        goto _out;
                    }
                } else {
                    err = airkiss_get_errno(send_socket);
                    if (err == ENOMEM || err == EAGAIN) {
                        ESP_LOGD(TAG, "send failed, errno %d", err);
                        continue;
                    }
                    ESP_LOGE(TAG, "send failed, errno %d", err);
                    goto _out;
                }
            }
        } else {
            vTaskDelay((portTickType) (100 / portTICK_RATE_MS));
        }
    }

_out:
    close(send_socket);
#ifdef CONFIG_AIRKISS_NOTIFY
    airkiss_lan_pack_param lan_param;
    memset(&lan_param, 0, sizeof(lan_param));
    lan_param.appid = AIRKISS_DEVICE_TYPE;
    lan_param.deviceid = AIRKISS_DEVICE_ID;
    airkiss_ssdp_notify(&lan_param); //LAN discover
#else
    xEventGroupClearBits(airkiss_evt_group, WIFI_CONNECTED_BIT | AIRKISS_STOP_BIT | AIRKISS_TIMEOUT_BIT);
#endif
    ESP_LOGI(TAG, "airkiss_send_ack_task exit");
    air_answer_task_handle = NULL;
    vTaskDelete(NULL);
}

//reply UDP to cell phone to notify that cell phone has connected to router
void airkiss_answer(void)
{
    if (air_answer_task_handle) {
        return;
    }
    xTaskCreate(airkiss_send_ack_task, "KISS_Send_task", AIRKISS_ACK_TASK_STACK_SIZE, NULL,
                AIRKISS_ACK_TASK_PRIORITY, &air_answer_task_handle);
}

static int airkiss_get_next_channel_idx(void)
{
    do {
        if (s_cur_chan_idx >= AIRKISS_MAX_CHANNEL_NUM - 1) {
            s_cur_chan_idx = 0;
        } else {
            s_cur_chan_idx++;
        }
    } while (s_airkiss_chan_tab[s_cur_chan_idx].ap_exist == false);

    return s_cur_chan_idx;
}

//switch channel
static void channel_change_callback(void *timer_arg)
{
    int chan_idx = 0;

    if (s_sniffer_stop_flag == 1) {
        return;
    }

    chan_idx = airkiss_get_next_channel_idx();

    ESP_LOGD(TAG, "ch%d-%d", s_airkiss_chan_tab[chan_idx].primary_chan,
             s_airkiss_chan_tab[chan_idx].second_chan);

    esp_wifi_set_channel(s_airkiss_chan_tab[chan_idx].primary_chan,
                         s_airkiss_chan_tab[chan_idx].second_chan);
    airkiss_change_channel(ak_ctx);
}

static void airkiss_finish(void)
{
    airkiss_result_t result;
    wifi_config_t wifi_config;
    int err;

    err = airkiss_get_result(ak_ctx, &result);
    if (err == 0) {
        ESP_LOGI(TAG,
                 "ssid = \"%s\", pwd = \"%s\", ssid_length = %d, pwd_length = %d, random = %x",
                 result.ssid, result.pwd, result.ssid_length, result.pwd_length,
                 result.random);

        // xEventGroupSetBits(airkiss_evt_group, AIRKISS_DONE_BIT);

        ak_random_num = result.random;

        bzero(&wifi_config.sta, sizeof(wifi_sta_config_t));
        memcpy(wifi_config.sta.ssid, result.ssid, result.ssid_length);
        memcpy(wifi_config.sta.password, result.pwd, result.pwd_length);

        if (wifi_config.sta.ssid[0] != 0x00) {
            ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...",
                     wifi_config.sta.ssid);
            esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
            esp_wifi_connect();
        }
    } else {
        ESP_LOGI(TAG, "airkiss_get_result() failed !");
    }
    free(ak_ctx);
    ak_ctx = NULL;
}

static void wifi_promiscuous_rx(void *buf, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *) buf;
    uint8_t *payload;
    uint16_t len;
    int ret;

    if (s_sniffer_stop_flag == 1 || buf == NULL) {
        return;
    }

    payload = pkt->payload;
    len = pkt->rx_ctrl.sig_len;

    ret = airkiss_recv(ak_ctx, payload, len);

    if (ret == AIRKISS_STATUS_CHANNEL_LOCKED) {
        esp_timer_stop(channel_change_timer);
        esp_timer_delete(channel_change_timer);
        channel_change_timer = NULL;
        esp_timer_stop(airkiss_timeout_timer);
        esp_timer_delete(airkiss_timeout_timer);
        airkiss_timeout_timer = NULL;
        ESP_LOGI(TAG, "AIRKISS_STATUS_CHANNEL_LOCKED");
    } else if (ret == AIRKISS_STATUS_COMPLETE) {
        esp_wifi_set_promiscuous(false);
        s_sniffer_stop_flag = 1;
        airkiss_finish();
        ESP_LOGI(TAG, "AIRKISS_STATUS_COMPLETE");
    } else {
        //ESP_LOGI(TAG, "AIRKISS_STATUS: %d", ret);
    }
}

static void airkiss_wifi_scan_ap(void)
{
    wifi_scan_config_t *scan_config = NULL;
    uint16_t ap_num = 0;
    wifi_ap_record_t *ap_record = NULL;

    scan_config = malloc(sizeof(wifi_scan_config_t));
    if (scan_config == NULL) {
        ESP_LOGE(TAG, "scan config allocate fail");
        return;
    }

    for (int scan_cnt = 0; scan_cnt < 2; scan_cnt++) {
        bzero(scan_config, sizeof(wifi_scan_config_t));
        scan_config->show_hidden = true;
        esp_wifi_scan_start(scan_config, true);

        esp_wifi_scan_get_ap_num(&ap_num);
        if (ap_num) {
            ap_record = malloc(ap_num * sizeof(wifi_ap_record_t));
            if (ap_record == NULL) {
                ESP_LOGE(TAG, "ap record allocate fail");
                continue;
            }

            esp_wifi_scan_get_ap_records(&ap_num, ap_record);

#if AIRKISS_DEBUG_ON
            ESP_LOGI(TAG, "scan ap number: %d", ap_num);
            for (int i = 0; i < ap_num; i++) {
                ESP_LOGI(TAG, "scan ap: %s, "MACSTR", %u, %u, %d", ap_record[i].ssid,
                         MAC2STR(ap_record[i].bssid), ap_record[i].primary,
                         ap_record[i].second, ap_record[i].rssi);
            }
#endif

            for (int i = 0; i < AIRKISS_MAX_CHANNEL_NUM; i++) {
                if (s_airkiss_chan_tab[i].ap_exist == true) {
                    continue;
                }
                for (int j = 0; j < ap_num; j++) {
                    if (ap_record[j].rssi < AIRKISS_MIN_RSSI) {
                        continue;
                    }
                    if (ap_record[j].primary == s_airkiss_chan_tab[i].primary_chan) {
                        s_airkiss_chan_tab[i].ap_exist = true;
                    }
                }
            }
            free(ap_record);
        }
    }

    free(scan_config);
}

static void airkiss_timeout_callback(void *timer_arg)
{
    if (airkiss_evt_group) {
        xEventGroupSetBits(airkiss_evt_group, AIRKISS_TIMEOUT_BIT);
    }
}
/*
#define CONFIG_AIRKISS_CRYPT
#define CONFIG_AIRKISS_KEY "e1c361cc29e43fb8"
*/
static void airkiss_start(int timeout)
{
    int chan_idx = 0;
    int8_t ret;

#ifdef CONFIG_AIRKISS_CRYPT
    const char *key = CONFIG_AIRKISS_KEY;
#endif
    ak_ctx = malloc(sizeof(airkiss_context_t));
    if (ak_ctx == NULL) {
        ESP_LOGE(TAG, "Airkiss context allocate fail");
        return;
    }

    ret = airkiss_init(ak_ctx, &ak_conf);
    if (ret < 0) {
        ESP_LOGE(TAG, "Airkiss init failed!");
        return;
    }

#ifdef CONFIG_AIRKISS_CRYPT
	ESP_LOGW(TAG, "Airkiss I need key!");
    airkiss_set_key(ak_ctx, (uint8_t *)key, strlen(key));
#else
	ESP_LOGW(TAG, "Airkiss I needn't key!");
#endif

    ESP_LOGI(TAG, "Airkiss version:%s", airkiss_version());

    esp_wifi_disconnect();
    airkiss_wifi_scan_ap();
    esp_timer_start_once(airkiss_timeout_timer, timeout * 1000);

    chan_idx = airkiss_get_next_channel_idx();
    esp_wifi_set_channel(s_airkiss_chan_tab[chan_idx].primary_chan,
                         s_airkiss_chan_tab[chan_idx].second_chan);
    esp_timer_create_args_t create_args = {
        .callback = &channel_change_callback,
        .arg = NULL,
        .name = "channel_change",
    };
    esp_timer_create(&create_args, &channel_change_timer);
    esp_timer_start_periodic(channel_change_timer, AIRKISS_CHANNEL_CHANGE_PERIOD * 1000);

    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(wifi_promiscuous_rx);
    esp_wifi_set_promiscuous(true);
    s_sniffer_stop_flag = 0;
}
bool isAirKissconfig = false;
esp_err_t airkiss_connect(int timeout)
{
	isAirKissconfig = false;
	if(isAirKissconfig){
        return ESP_ERR_INVALID_STATE;
    }
	EventBits_t uxBits;
    esp_err_t res = ESP_OK;
    airkiss_evt_group = xEventGroupCreate();

    esp_timer_create_args_t tmr_args = {
        .callback = &airkiss_timeout_callback,
        .arg = NULL,
        .name = "airkiss_timeout",
    };
    ESP_LOGI(TAG, "Start airkiss!");
    esp_timer_create(&tmr_args, &airkiss_timeout_timer);
    xEventGroupClearBits(airkiss_evt_group, WIFI_CONNECTED_BIT | AIRKISS_STOP_BIT | AIRKISS_TIMEOUT_BIT);

    airkiss_start(timeout);
	isAirKissconfig = true;

    uxBits = xEventGroupWaitBits(airkiss_evt_group,
                                 WIFI_CONNECTED_BIT
                                 | AIRKISS_STOP_BIT | AIRKISS_TIMEOUT_BIT, true, false,
                                 portMAX_DELAY);
    if (uxBits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi Connected to ap");
        airkiss_answer();
    }
    // if (uxBits & AIRKISS_DONE_BIT) {
    //     ESP_LOGI(TAG, "Airkiss done");
    //     res = ESP_OK;
    // }
    if (uxBits & AIRKISS_STOP_BIT) {
        ESP_LOGI(TAG, "Airkiss stop");
        s_sniffer_stop_flag = 1;
        if (channel_change_timer) {
            esp_timer_stop(channel_change_timer);
            esp_timer_delete(channel_change_timer);
            channel_change_timer = NULL;
        }
        esp_wifi_set_promiscuous(false);
        free(ak_ctx);
        ak_ctx = NULL;
        res = ESP_FAIL;
    }
    if (uxBits & AIRKISS_TIMEOUT_BIT) {
        ESP_LOGI(TAG, "Airkiss timeout(30s)");
        s_sniffer_stop_flag = 1;
        if (channel_change_timer) {
            esp_timer_stop(channel_change_timer);
            esp_timer_delete(channel_change_timer);
            channel_change_timer = NULL;
        }
        esp_wifi_set_promiscuous(false);
        free(ak_ctx);
        ak_ctx = NULL;
        res = ESP_ERR_TIMEOUT;
    }
    if (airkiss_timeout_timer) {
        esp_timer_stop(airkiss_timeout_timer);
        esp_timer_delete(airkiss_timeout_timer);
        airkiss_timeout_timer = NULL;
    }
    //vEventGroupDelete(airkiss_evt_group);
    ESP_LOGI(TAG, "Start quit!");
	isAirKissconfig = false;
    //airkiss_evt_group = NULL;
    return res;
}

void airkiss_stop(void)
{
    if (airkiss_evt_group) {
        xEventGroupSetBits(airkiss_evt_group, AIRKISS_STOP_BIT);
    }
}
