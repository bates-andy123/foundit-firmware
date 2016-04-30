#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
//#include "uart.h"

//Global variables for led 4
LOCAL os_timer_t blink_timer;
LOCAL uint8_t alive_led_state=0;

#define LED_GPIO 5
LOCAL uint8_t udp_led_state=0;

LOCAL struct espconn conn1;
LOCAL esp_udp udp1;

LOCAL void recvCB(void *arg, char *pData, unsigned short len);
LOCAL void eventCB(System_Event_t *event);
LOCAL void setupUDP();
LOCAL void initDone();

//Flickers the led 4
LOCAL void ICACHE_FLASH_ATTR blink_cb(void *arg)
{
    alive_led_state = !alive_led_state;
    GPIO_OUTPUT_SET(4, alive_led_state);
}

LOCAL void initDone() {
    wifi_set_opmode_current(STATION_MODE);
    struct station_config stationConfig;
    strncpy(stationConfig.ssid, "bates", 32);
    strncpy(stationConfig.password, "JackBates", 64);
    wifi_station_set_config_current(&stationConfig);
    wifi_station_connect();
}// end of initDone

LOCAL void recvCB(void *arg, char *pData, unsigned short len) {
    struct espconn *pEspConn = (struct espconn *)arg;
    os_printf("Received data!! - length = %d\n", len);
    os_printf("%c", pData[0]);
    /*if (len == 0 || (pData[0] != '0' && pData[0] != '1')) {
        return;
    }*/
    udp_led_state = !udp_led_state;
    GPIO_OUTPUT_SET(LED_GPIO, udp_led_state);
    //int v = (pData[0] == '1');
    //GPIO_OUTPUT_SET(LED_GPIO, v);
} // End of recvCB

LOCAL void setupUDP() {
    conn1.type = ESPCONN_UDP;
    conn1.state = ESPCONN_NONE;
    udp1.local_port = 25867;
    conn1.proto.udp = &udp1;
    espconn_create(&conn1);
    espconn_regist_recvcb(&conn1, recvCB);
    os_printf("Listening for data\n");
} // End of setupUDP

LOCAL void eventCB(System_Event_t *event) {
    switch (event->event) {
    case EVENT_STAMODE_GOT_IP:
        os_printf("IP: %d.%d.%d.%d\n", IP2STR(&event->event_info.got_ip.ip));
        setupUDP();
        break;
     }
} // End of eventCB

void user_rf_pre_init(void) {
}

//Init function 
void ICACHE_FLASH_ATTR
user_init()
{

    //Blinking led pin 4
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
    os_timer_disarm(&blink_timer);
    os_timer_setfn(&blink_timer, (os_timer_func_t *)blink_cb, (void *)0);
    os_timer_arm(&blink_timer, 1000, 1);

    //0 refers to uart0
    uart_div_modify(0, UART_CLK_FREQ / 115200);
    
    //Selecting pin 5 as the output
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);

    // Call "initDone" when the ESP8266 has initialized
   system_init_done_cb(initDone);
   wifi_set_event_handler_cb(eventCB);
}//end of user_init
