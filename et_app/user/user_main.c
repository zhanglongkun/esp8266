/**
  ******************************************************************************
  * @FileName:     user_main.c
  * @Author:       zlk
  * @Version:      V1.0
  * @Date:         2017-5-1 ÏÂÎç 4:29:41
  * @Description:  This file provides all the user_main.c functions.
  ******************************************************************************
  */


#include "esp_common.h"
#include "user_config.h"
#include "../../include/lwip/lwip/sockets.h"

#define AP_NAME        "JQM-206"
#define AP_PASSWORD    "fjjqm123"

#define TCP_SERVER_IP      "192.168.1.108"
#define TCP_SERVER_PORT    8888
#define LISTEN_MAX_CONN    10

void wifi_handle_event_cb(System_Event_t *evt)
{
    printf("event %x\n", evt->event_id);
    
    switch (evt->event_id) {
        case EVENT_STAMODE_CONNECTED:
            printf("connect to ssid	%s,	channel	%d\n",
                   evt->event_info.connected.ssid,
                   evt->event_info.connected.channel);
            break;
            
        case EVENT_STAMODE_DISCONNECTED:
            printf("disconnect	from ssid %s, reason %d\n",
                   evt->event_info.disconnected.ssid,
                   evt->event_info.disconnected.reason);
            break;
            
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            printf("mode: %d -> %d\n",
                   evt->event_info.auth_change.old_mode,
                   evt->event_info.auth_change.new_mode);
            break;
            
        case EVENT_STAMODE_GOT_IP:
            printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
                   IP2STR(&evt->event_info.got_ip.ip),
                   IP2STR(&evt->event_info.got_ip.mask),
                   IP2STR(&evt->event_info.got_ip.gw));
            printf("\n");
            break;
            
        case EVENT_SOFTAPMODE_STACONNECTED:
            printf("station: " MACSTR "join, AID = %d\n",
                   MAC2STR(evt->event_info.sta_connected.mac),
                   evt->event_info.sta_connected.aid);
            break;
            
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            printf("station: " MACSTR "leave, AID = %d\n",
                   MAC2STR(evt->event_info.sta_disconnected.mac),
                   evt->event_info.sta_disconnected.aid);
            break;
            
        default:
            break;
    }
}

/**
  ******************************************************************************
  * Function:     Set_WIFI_Option()
  * Description:  ÃèÊö
  * Date:         2017-05-01
  * Others:       add by zlk
  ******************************************************************************
  */
void Set_WIFI_Option()
{
    bool ret = TRUE;
    struct station_config *configWIFI;
    configWIFI = (struct station_config *)zalloc(sizeof(struct station_config));

    bzero(configWIFI, sizeof(struct station_config));
    ret = wifi_set_opmode(STATION_MODE);
    if (FALSE == ret) {
        os_printf("wifi_set_opmode() error\n");
        goto EXIT;
    }

    sprintf(configWIFI->ssid, AP_NAME);
    sprintf(configWIFI->password, AP_PASSWORD);

    ETS_UART_INTR_DISABLE();
    ret = wifi_station_set_config(configWIFI);                  //ÉèÖÃWIFIÁ¬½Ó
    if (FALSE == ret) {
        os_printf("wifi_station_set_config() error\n");
        goto EXIT;
    }
    ETS_UART_INTR_ENABLE();

    ret = wifi_station_connect();
#if 0
    if (FALSE == ret) {
        os_printf("wifi_station_connect() error\n");
        goto EXIT;
    }
#endif

    wifi_set_event_handler_cb(wifi_handle_event_cb);

EXIT:
    if (NULL != configWIFI) {
        free(configWIFI);
    }
	vTaskDelete(NULL);

    return ;
}

void Build_TCP_Servers()
{
    int ret;
    int listenfd, clientfd;
    int clientAddrLen, bufLen;
    char recvbuf[20] = {0};
    struct sockaddr_in servAddr, remoteAddr;

    memset(&servAddr, 0, sizeof(struct sockaddr_in));

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listenfd) {
        os_printf("creat socket failed, line = %d\n", __LINE__);
        return;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(TCP_SERVER_PORT);
    servAddr.sin_len = sizeof(struct sockaddr_in);

    ret = bind(listenfd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr_in));
    if (0 != ret) {
        os_printf("bind failed, ret = %d, line = %d\n", ret, __LINE__);
        return ;
    }

    ret = listen(listenfd, LISTEN_MAX_CONN);
    if (0 != ret) {
        os_printf("listen failed, ret = %d, line = %d\n", ret, __LINE__);
        return ;
    }

    clientAddrLen = sizeof(struct sockaddr_in);

    for (; ;) {
        memset(&remoteAddr, 0, sizeof(struct sockaddr_in));

        os_printf("ESP8266 TCP server task > wait client\n");
        clientfd = accept(listenfd, (struct sockaddr *)&remoteAddr, &clientAddrLen);
        if (clientfd < 0) {
            continue;
        }

        os_printf("remote addr:%s, port = %d\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));

        bufLen = read(clientfd, recvbuf, sizeof(recvbuf));
        os_printf("buf len = %d\n", bufLen);
        if (bufLen > 0) {
            os_printf("recv buffer %s\n", recvbuf);
        }
    }
}

void Build_TCP_Client()
{
    int socketfd;
    char buf[] = "aaaaaaa";
    int len = 0;
    struct sockaddr_in clientAddr;

    memset(&clientAddr, 0, sizeof(struct sockaddr_in));

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == socketfd) {
        os_printf("socket() error\n");
    }

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(TCP_SERVER_IP);
    clientAddr.sin_port = htons(TCP_SERVER_PORT);

    while (1) {
        if (0 != connect(socketfd, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr_in))) {
            os_printf("connect error\n");
            vTaskDelay(1000);
            continue;
        }
        os_printf("connect succeed\n");
        break;
    }

#if 0
    if (0 != connect(socketfd, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr_in))) {
        os_printf("connect error\n");
    }
#endif

    len = sizeof(buf);
    if (write(socketfd, buf, len) < len) {
        os_printf("write error\n");
        return ;
    }

    os_printf(".................\n");
	vTaskDelete(NULL);
    return ;

}


/**
  ******************************************************************************
  * Function:     user_init()
  * Description:  ÃèÊö
  * Date:         2017-05-01
  * Others:       add by zlk
  ******************************************************************************
  */
void user_init(void)
{
    bool ret = TRUE;

    os_printf("software version: %s\n", system_get_sdk_version()); //´òÓ¡Èí¼þ°æ±¾ºÅ
    os_printf("ESP8266 chip ID: 0x%x\n", system_get_chip_id()); //´òÓ¡Ð¾Æ¬ID

	xTaskCreate(Set_WIFI_Option, "wifiTask", 256, NULL, 2, NULL);
	xTaskCreate(Build_TCP_Servers, "TCP_Client", 256, NULL, 2, NULL);

#if 0
    ret = Set_WIFI_Option();
    if (FALSE == ret) {
        os_printf("Set_WIFI_Option() error\n");
    } else {
        os_printf("set WIFI succeed\n");
    }
    os_printf("11111\n");
    vTaskDelay(5000);
    os_printf("222222\n");

    ret = Build_Client_Socket();
    if (-1 == ret) {
        os_printf("Build_Client_Socket() error\n");
    }
#endif

}
