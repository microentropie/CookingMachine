#ifndef _COMMON_IO_H_
#define _COMMON_IO_H_

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif //uint8_t

#define UART_BAUDRATE 74880

// GPIO02: ESP-12x blue LED:
#define MODULE_BLUELED_PIN 2
// GPIO14: red LED:
#define PANEL_REDLED_PIN 14
// GPIO13: green LED:
#define PANEL_GREENLED_PIN 0
// GPIO16: blue LED:
#define PANEL_BLUELED_PIN 16

// relay/ssr output:
#define RY_MAIN_PIN 5
#define RY_HOT_PIN 4
#define RY_POUR_PIN 0
#define OUT_ACTIVE_HIGH false

// other outputs:
#define BUZZER_PIN 15
//#define WSLED_PIN 14

// inputs:
#define SENSOR_HOT_PIN 3 //RxD0
#define TEMP_PIN A0
#define BUTTON_PIN 12
/*
// display:
#define OLED_DC 15
#define OLED_RES 16
#define SPI_MISO_PIN 12
#define SPI_MOSI_PIN 13
#define SPI_SCK_PIN 14
*/

#endif //_COMMON_IO_H_
