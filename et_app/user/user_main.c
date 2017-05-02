/**
  ******************************************************************************
  * @FileName:     user_main.c
  * @Author:       zlk
  * @Version:      V1.0
  * @Date:         2017-5-1 œ¬ŒÁ 4:29:41
  * @Description:  This file provides all the user_main.c functions. 
  ******************************************************************************
  */


#include "esp_common.h"
#include "user_config.h"
#include "../../include/lwip/lwip/sockets.h"

#define AP_NAME        "JQM-206"
#define AP_PASSWORD    "fjjqm123"

#define SERVER_IP      "192.168.1.102"
#define SERVER_PORT    8888

/**
  ******************************************************************************
  * Function:     Set_WIFI_Option()
  * Description:  √Ë ˆ
  * Date:         2017-05-01
  * Others:       add by zlk
  ******************************************************************************
  */ 
bool Set_WIFI_Option()
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
	ret = wifi_station_set_config(configWIFI);                  //…Ë÷√WIFI¡¨Ω”
    if (FALSE == ret) {
        os_printf("wifi_station_set_config() error\n");
        goto EXIT;
    }
	ETS_UART_INTR_ENABLE();
    
    ret = wifi_station_connect();
    if (FALSE == ret) {
        os_printf("wifi_station_connect() error\n");
        goto EXIT;
    }

EXIT:
    if (NULL != configWIFI) {
        free(configWIFI);
    }
    
    return ret;
}

bool Build_Client_Socket()
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
    clientAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    clientAddr.sin_port = htons(SERVER_PORT);

    while (1) {
        if (0 != connect(socketfd, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr_in))) {
            os_printf("connect error\n");
            vTaskDelay(10);
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
        return -1;
    }
    
    os_printf(".................\n");
    return 0;
        
}

/**
  ******************************************************************************
  * Function:     user_init()
  * Description:  √Ë ˆ
  * Date:         2017-05-01
  * Others:       add by zlk
  ******************************************************************************
  */ 
void user_init(void)
{	
    bool ret = TRUE;
    
    os_printf("software version: %s\n", system_get_sdk_version()); //¥Ú”°»Ìº˛∞Ê±æ∫≈
    os_printf("ESP8266 chip ID: 0x%x\n", system_get_chip_id()); //¥Ú”°–æ∆¨ID

    ret = Set_WIFI_Option();
    if (FALSE == ret) {
        os_printf("Set_WIFI_Option() error\n");
    } else {
        os_printf("set WIFI succeed\n");
    }
    os_printf("11111\n");
    vTaskDelay(500);
    os_printf("222222\n");

#if 0
    ret = Build_Client_Socket();
    if (-1 == ret) {
        os_printf("Build_Client_Socket() error\n");
    }
#endif
    
}
