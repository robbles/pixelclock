#ifndef CONTROLLER_H
#define CONTROLLER_H

#define SPEED 1
//#define HOLIDAY

// [ H, M, S, AM/PM ]
#define COMMAND_ON '@'
#define COMMAND_OFF '*'
#define COMMAND_TIME '%'
#define COMMAND_COLOR '#'
#define COMMAND_ASCII '$'
typedef struct {
    uint8_t data1;
    uint8_t data2;
    uint8_t data3;
    uint8_t data4;
    uint8_t command;
} CommandPacket;

#define REDLED 6
#define GREENLED 5
#define BLUELED 9

typedef struct {
	float red;
	float green;
	float blue;
} RGB_color;

typedef struct {
	float hue;
	float saturation;
	float value;
} HSV_color;

int packet_index = 0;
unsigned char buffer[24];
unsigned char driver_index = 0;

unsigned char red, green, blue;

uint8_t enabled = true;

unsigned char RainbowCMD[20];
unsigned char State = 0;  
unsigned long timeout;
unsigned long delay_time = 1;

volatile int hours, minutes, seconds;
volatile uint8_t PM;
volatile uint8_t display_changed;

volatile int last_tick;

#define LED_ON() digitalWrite(13, 1)
#define LED_OFF() digitalWrite(13, 0)

#endif

