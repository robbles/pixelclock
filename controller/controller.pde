#include <Wire.h>
#include <NewSoftSerial.h>

#include "controller.h"


void setup()
{

    Wire.begin(); // join i2c bus (address optional for master) 

	// Setup 16-bit timer 1 to control time
    TIMSK1 = 0x00;
	TCCR1A = 0x00; // CTC with TOP at ICR1
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10); // clk / 1024, 16-bit Fast PWM
	ICR1 = 15625; // Overflows every 1.0s
	TIMSK1 = _BV(ICIE1); // Trigger interrupt when timer reaches TOP	

    red = green = blue = 0x07;

    hours = 12;
    minutes = 0;
    seconds = 0;
    PM = true;
    time_changed = true;

    colorMatrix(1, 0, 0, 0);    
    colorMatrix(2, 0, 0, 0);    
    colorMatrix(3, 0, 0, 0);    
    colorMatrix(4, 0, 0, 0);    

    // Start serial connection to XBee
    xbee.begin(9600);
    pinMode(2, 0);

}

void loop()
{ 

    // Check for commands from driver	
    // { 0:'[', 1:'hours', 2:'minutes', 3:'seconds', 4:'PM', 5:'Command', 5:']' }
    while(xbee.available()) {

        uint8_t c = xbee.read();

        switch(c) {

            case '[': // Opening byte
                packet_index = 0;
                break;

            case ']': // Closing byte
                if(packet_index == 5) { // valid length
                    LED_ON();
                    CommandPacket *packet = (CommandPacket *)buffer;
                    handlePacket(packet);
                }
                break;

            default:
                buffer[packet_index++] = c;
                if(packet_index > 5) {
                    packet_index = 0;
                }	
        }
    }

    // Update the dot matrix display, if enabled
    if(enabled) {
        displayTime(hours, minutes);
    }
}

/* 
 * Handles a packet received from the XBee
 */
void handlePacket(CommandPacket *packet) {
    switch(packet->command) {
        case COMMAND_ON: /* Turns the display on */
            enabled = true;
            time_changed = true;
            break;

        case COMMAND_OFF: /* Turns the display off */
            enabled = false;

            // Clear all matrices
            colorMatrix(1, 0, 0, 0);
            colorMatrix(2, 0, 0, 0);
            colorMatrix(3, 0, 0, 0);
            colorMatrix(4, 0, 0, 0);
            break;

        case COMMAND_TIME: /* Sets the time */
            /* Fields == 0xFF are ignored so that specific
             * parts of the time can be set (e.g. only seconds) */
            hours = (packet->data1 != 0xFF)? packet->data1 : hours;
            minutes = (packet->data2 != 0xFF)? packet->data2 : minutes;
            seconds = (packet->data3 != 0xFF)? packet->data3 : seconds;
            PM = (packet->data4 != 0xFF)? packet->data4 : PM;

            time_changed = true;
            break;

        case COMMAND_COLOR: /* Sets the display color */
            red = packet->data1 >> 4;
            green = packet->data2 >> 4;
            blue = packet->data3 >> 4;
            time_changed = true;
            break;

        case COMMAND_ASCII: /* Sets display text */
            displayASCII((uint8_t *)packet);
            break;
    }
}

/*
 * Updates the hours, minutes and seconds in typical clock-like fashion
 * every one second
 */
SIGNAL(TIMER1_CAPT_vect) {
    seconds = (seconds + 1) % 60;
    if(!seconds) {
        minutes = (minutes + 1) % 60;
        time_changed = true;
        if(!minutes) {
            hours = (hours != 12)? (hours + 1) : 1;
            if(hours == 12) {
                PM ^= 1;
            }
        }
    }
}

/*
 * Display the current time as HH:MM on LED matrices
 */
void displayTime(unsigned char hours, unsigned char minutes) {
    // NOTE: the slower changing digits must be overlaid
    // over the faster ones (e.g. minutes first, then hours)

    // Show the minutes
    if(time_changed) {

        time_changed = false;

        char digit1, digit2, digit3, digit4;
        itoa(hours/10, &digit1, 10);
        itoa(hours % 10, &digit2, 10);
        itoa(minutes/10, &digit3, 10);
        itoa(minutes % 10, &digit4, 10);


        setMatrixChar(2, digit3, red, green, blue, 5);
        setMatrixChar(3, digit3, red, green, blue, 11);

        overlayMatrixChar(3, digit4, red, green, blue, 3);
        setMatrixChar(4, digit4, red, green, blue, 13);

        // Show divider between hours/minutes
        overlayMatrixChar(2, ':', red, green, blue, 1);

        // Show hours, hiding leading zero
        setMatrixChar(1, digit2, red, green, blue, 4);
        overlayMatrixChar(2, digit2, red, green, blue, 12);
        if(digit1 != '0') {
            overlayMatrixChar(1, digit1, red, green, blue, 10);
        }

        // Show AM/PM
        if(PM) {
            overlayMatrixChar(4, 'P', red, green, blue, 1);
        } else {
            overlayMatrixChar(4, 'A', red, green, blue, 1);
        }
    }

}

/*
 * Display a string of ASCII characters on the LEDs
 */
void displayASCII(unsigned char *str) {
    setMatrixChar(1, str[0], red, green, blue, 0);
    setMatrixChar(2, str[1], red, green, blue, 0);
    setMatrixChar(3, str[2], red, green, blue, 0);
    setMatrixChar(4, str[3], red, green, blue, 0);
}

/*
 * Sends the current Rainbowduino API packet via TWI to I2C address addr
 */
void sendCommand(unsigned char addr) {

    Wire.beginTransmission(addr);
    for (int i=0;i<5;i++) Wire.send(RainbowCMD[i]);
    Wire.endTransmission();    
    delay(5);   

}

/* 
 * Colors the matrix with I2C address addr a solid color
 */
void colorMatrix(unsigned char addr, unsigned char red, unsigned char green, unsigned char blue) {
    RainbowCMD[0]='R';
    RainbowCMD[1]=0x03;
    RainbowCMD[2]=red;
    RainbowCMD[3]=((green<<4)|(blue));

    sendCommand(addr);
}

/*
 * Displays an ASCII character on the matrix with I2C address addr in a solid RGB color.
 * Clears the current pattern on the matrix in the process.
 */
void setMatrixChar(unsigned char addr, unsigned char code, unsigned char red, unsigned char green ,unsigned char blue, unsigned char shift)
{
    if(((code > 64) && (code < 91)) || 
       ((code > 96) && (code < 123)) || 
       ((code >= '0') && (code <= ':'))) {
        // Draw known ASCII symbol
        RainbowCMD[0]='R';
        RainbowCMD[1]=0x02;
        RainbowCMD[2]=((shift<<4)|(red));
        RainbowCMD[3]=((green<<4)|(blue));
        RainbowCMD[4]=code;

        sendCommand(addr);
    } else {
        // Unknown symbol, blank matrix
       colorMatrix(addr, 0, 0, 0);
    }
}

/*
 * Overlays an ASCII character on the matrix with I2C address addr in a solid RGB color.
 * Only changes the pixels that are set in the pixel map for the new character.
 * i.e. Adds the new character pixels to the current pattern
 */
void overlayMatrixChar(unsigned char addr, unsigned char code, unsigned char red, unsigned char green ,unsigned char blue, unsigned char shift)
{
    if(code == ' ' || code == 0) {
       return;
    }
    RainbowCMD[0]='R';
    RainbowCMD[1]=0x04;
    RainbowCMD[2]=((shift<<4)|(red));
    RainbowCMD[3]=((green<<4)|(blue));
    RainbowCMD[4]=code;

    sendCommand(addr);
}

/*
 * Replaces the current pattern on the matrix with I2C address addr with a pattern stored in the flash
 * memory of the Rainbowduino. Displays the pattern associated with key.
 */
void setMatrixImage(unsigned char addr, unsigned char key, unsigned char shift)
{
    RainbowCMD[0]='R';
    RainbowCMD[1]=0x01;
    RainbowCMD[2]=(shift<<4);
    RainbowCMD[4]=key;

    sendCommand(addr);
}


/*
 * Sends an "Association Indication" command to the XBee
 * module and reads back a byte indicating the network
 * status of the module
 */
int get_xbee_status() {
    unsigned char status = 0;

    // Enter command mode
    delay(1000);
    xbee.print("+++");
    delay(1000);

    // Send AI command
    xbee.print("ATAI\r");

    // Wait for 0.5s and check for data
    delay(500);
    if(xbee.available()) {
        // A value of 0x00 means the module associated successfully
        if(xbee.read() == 0x00) {
            status = 1;
        }
    }

    // Exit command mode
    xbee.print("ATCN\r");

    return status;
}



