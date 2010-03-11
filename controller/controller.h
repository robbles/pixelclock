#ifndef CONTROLLER_H
#define CONTROLLER_H

#define XBEE_RETRIES 3

const char turk_init_packet[] = "SPAWN\x00\x00\x00\x00\x00\x00\x00\x08";

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

int packet_index = 0;
unsigned char buffer[24];
unsigned char driver_index = 0;

unsigned char red, green, blue;

uint8_t enabled = true;

unsigned char RainbowCMD[5];
unsigned char State = 0;  
unsigned long timeout;

volatile int hours, minutes, seconds;
volatile uint8_t PM;
volatile uint8_t time_changed;

#define LED_ON() digitalWrite(13, 1)
#define LED_OFF() digitalWrite(13, 0)

NewSoftSerial xbee(2, 3);

#endif

