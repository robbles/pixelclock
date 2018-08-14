#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "WireCommands.h"
#include "controller.h"

SoftwareSerial SoftSerial(0,1);
void sendCMD(byte address, byte CMD, ... );

void setup()
{
    // Setup watchdog timer to prevent freezing while waiting for I2C devices
    wdt_enable(WDTO_4S);
    digitalWrite(REDLED, HIGH);

    Wire.begin(); // join i2c bus (address optional for master) 
    RainbowCMD[0] = 'r'; // initialize command buffer

	// Setup 16-bit timer 1 to control time
    TIMSK1 = 0x00;
    // CTC with TOP at ICR1
	TCCR1A = 0x00; 
    // clk / 1024, 16-bit Fast PWM
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10); 

    // Overflows every 1.0s, multiplied by SPEED factor
	ICR1 = 15625 / SPEED;

	TIMSK1 = _BV(ICIE1); // Trigger interrupt when timer reaches TOP	

    red = green = blue = 0x07;

    hours = 12;
    minutes = 30;
    seconds = 0;
    last_tick = 0;
    PM = true;
    display_changed = true;

    // Start serial connection to XBee
    SoftSerial.begin(9600);
    pinMode(2, 0);

    pinMode(REDLED, OUTPUT);
    pinMode(GREENLED, OUTPUT);
    pinMode(BLUELED, OUTPUT);

    do_the_dance(2, 100);

    // Setup each matrix
    for(int addr=1; addr<=4; addr++) {
        sendCMD(addr, CMD_SET_PAPER, 0, 0, 1);
        sendCMD(addr, CMD_SET_INK, red, green, blue);
        sendCMD(addr, CMD_CLEAR_PAPER);
        sendCMD(addr, CMD_SWAP_BUF);
    }
}

void loop()
{ 
    // Reset the watchdog timer, since the sketch hasn't frozen
    wdt_reset();
    digitalWrite(REDLED, LOW);

    // Check for commands from driver	
    /* format is '[' + <hours> + <minutes> + <seconds> + <AM/PM> + <command> + ']'
       or '[' + <red> + <green> + <blue> + <null> + <command> + ']'
       or '[' + <char1> + <char2> + <char3> + <char4> + <command> + ']' */
    while(SoftSerial.available()) {
        uint8_t c = SoftSerial.read();
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
        if(display_changed) {
            display_changed = false;
            displayTime4(hours, minutes);
        }
    }
}

/*
 * Display the current time as HH:MM on LED matrices
 */
void displayTime4(unsigned char hours, unsigned char minutes) {
    for(int addr=1; addr<=4; addr++) {
        sendCMD(addr, CMD_SET_INK, red, green, blue);
    }

    // Clear matrices
    for(int addr=1; addr<=4; addr++) {
        sendCMD(addr, CMD_CLEAR_PAPER);
    }

    char digit1, digit2, digit3, digit4;
    itoa(hours/10, &digit1, 10);
    itoa(hours % 10, &digit2, 10);
    itoa(minutes/10, &digit3, 10);
    itoa(minutes % 10, &digit4, 10);

    int hour = hours - 1;
    float day_ratio = hour / 11.0;
    float hour_ratio = minutes / 60.0;
    float split_ratio = (hour + hour_ratio) / 12.0;

    color_shift(day_ratio, 1);
    color_shift(day_ratio, 2);

    // Hours tens
    if(digit1 != '0') {
        sendCMD(1, CMD_PRINT_CHAR, toByte(0), toByte(-1), digit1);
    }

    // Hours ones
    sendCMD(2, CMD_PRINT_CHAR, toByte(2), toByte(-1), digit2);

    color_shift(hour_ratio, 3);
    color_shift(hour_ratio, 4);

    // Minutes tens
    sendCMD(3, CMD_PRINT_CHAR, toByte(-1), toByte(-1), digit3);

    // Minutes ones
    sendCMD(4, CMD_PRINT_CHAR, toByte(0), toByte(-1), digit4);

    color_shift(split_ratio, 2);
    color_shift(split_ratio, 3);

    // Show divider between hours/minutes
    sendCMD(2, CMD_DRAW_LINE, toByte(0), toByte(6), toByte(0), toByte(5));
    sendCMD(2, CMD_DRAW_LINE, toByte(0), toByte(3), toByte(0), toByte(2));

    sendCMD(3, CMD_DRAW_LINE, toByte(7), toByte(6), toByte(7), toByte(5));
    sendCMD(3, CMD_DRAW_LINE, toByte(7), toByte(3), toByte(7), toByte(2));

    // Update display
    for(int addr=1; addr<=4; addr++) {
        sendCMD(addr, CMD_SWAP_BUF);
    }
}

/* 
 * Handles a packet received from the XBee
 */
void handlePacket(CommandPacket *packet) {
    switch(packet->command) {
        case COMMAND_ON: /* Turns the display on */
            enabled = true;
            display_changed = true;
            break;

        case COMMAND_OFF: /* Turns the display off */
            enabled = false;

            // Clear all matrices
            for(int addr=1; addr<=4; addr++) {
                sendCMD(addr, CMD_CLEAR_BUF, 0, 0, 0);
                sendCMD(addr, CMD_SWAP_BUF);
            }
            break;

        case COMMAND_TIME: /* Sets the time */
            /* Fields == 0xFF are ignored so that specific
             * parts of the time can be set (e.g. only seconds) */
            hours = (packet->data1 != 0xFF)? packet->data1 : hours;
            minutes = (packet->data2 != 0xFF)? packet->data2 : minutes;
            seconds = (packet->data3 != 0xFF)? packet->data3 : seconds;
            PM = (packet->data4 != 0xFF)? packet->data4 : PM;

            display_changed = true;
            break;

        case COMMAND_COLOR: /* Sets the display color */
            red = packet->data1 >> 4;
            green = packet->data2 >> 4;
            blue = packet->data3 >> 4;
            clear_color(red, green, blue);
            delay(500);
            clear_color(0, 0, 0x1);
            display_changed = true;
            break;

        case COMMAND_STROBE: /* Sets the display color */
            red = packet->data1 >> 4;
            green = packet->data2 >> 4;
            blue = packet->data3 >> 4;
            clear_color(red, green, blue);
            break;

        case COMMAND_ASCII: /* Sets display text */
            sendCMD(0x01, CMD_CLEAR_PAPER);
            sendCMD(0x02, CMD_CLEAR_PAPER);
            sendCMD(0x03, CMD_CLEAR_PAPER);
            sendCMD(0x04, CMD_CLEAR_PAPER);
            sendCMD(0x01, CMD_PRINT_CHAR, toByte(0), toByte(0), packet->data1);
            sendCMD(0x02, CMD_PRINT_CHAR, toByte(0), toByte(0), packet->data2);
            sendCMD(0x03, CMD_PRINT_CHAR, toByte(0), toByte(0), packet->data3);
            sendCMD(0x04, CMD_PRINT_CHAR, toByte(0), toByte(0), packet->data4);
            sendCMD(0x01, CMD_SWAP_BUF);
            sendCMD(0x02, CMD_SWAP_BUF);
            sendCMD(0x03, CMD_SWAP_BUF);
            sendCMD(0x04, CMD_SWAP_BUF);
            break;
    }
}

/*
 * Updates the hours, minutes and seconds in typical clock-like fashion
 * every one second
 */
SIGNAL(TIMER1_CAPT_vect) {
    seconds = (seconds + 1) % 60;
    display_changed = true;
    if(!seconds) {
        minutes = (minutes + 1) % 60;
        if(!minutes) {
            hours = (hours != 12)? (hours + 1) : 1;
            if(hours == 12) {
                PM ^= 1;
            }
        }
    }
}


void sendCMD(byte address, byte CMD, ... ) {
  int i;
  unsigned char v;
  byte t;
  va_list args;                     // Create a variable argument list
  va_start(args, CMD);              // Initialize the list using the pointer of the variable next to CMD;
  
  RainbowCMD[1] = CMD;              // Stores the command name
  t = pgm_read_byte(&(CMD_totalArgs[CMD]))+2;  // Retrieve the number of arguments for the command
  for (i=2; i < t; i++) {
    v = va_arg(args, int);          // Retrieve the argument from the va_list    
    RainbowCMD[i] = v;              // Store the argument
  }
  
  sendWireCommand(address, t);      // Transmit the command via I2C
}

unsigned char toByte(int i) {
  return map(i, -128, 127, 0, 255);
}

// ### The following lines are adapted from the original code ###

void sendWireCommand(int Add, byte len) {
  unsigned char OK=0;
  unsigned char i,temp;
  int tries = 0;
  
  while(!OK)
  {                          
    switch (State)
    { 	

    case 0:                          
      Wire.beginTransmission(Add);
      for (i=0; i<len ;i++) Wire.write(RainbowCMD[i]);
      Wire.endTransmission();    
      delay(delay_time);   
      State=1;                      
      break;

    case 1:
      Wire.requestFrom(Add,1);   
      if (Wire.available()>0) 
        temp=Wire.read();    
      else {
        temp=0xFF;
        timeout++;
      }

      if ((temp==1)||(temp==2)) State=2;
      else if (temp==0) State=0;

      if (timeout>5000) {
        if(tries > 3) {
            return;
        }
        tries++;
        timeout=0;
        State=0;
      }

      delay(delay_time);
      break;

    case 2:
      OK=1;
      State=0;
      break;

    default:
      State=0;
      break;
    }
  }
}

/*
 * Sets the color based on the current time
 */
void color_shift(float ratio, int addr) {
    /*HSV_color hsv = { (hours + (minutes / 60.0)) / 2.0, 1.0, 1.0 };*/
    HSV_color hsv = { ratio * 6.0, 1.0, 1.0 };
    RGB_color rgb = HSV_to_RGB(hsv);
    
    red = rgb.red * 15;
    green = rgb.green * 15;
    blue = rgb.blue * 15;

    // Set new paper color
    sendCMD(addr, CMD_SET_INK, red, green, blue);
}

RGB_color HSV_to_RGB( HSV_color HSV ) {

    // H is given on [0, 6] or UNDEFINED. S and V are given on [0, 1].
    // RGB are each returned on [0, 1].
    float h = HSV.hue, s = HSV.saturation, v = HSV.value, m, n, f;
    int i;
    RGB_color RGB;
    i = floor(h);
    f = h - i;
    if ( !(i&1) ) f = 1 - f; // if i is even
    m = v * (1 - s);
    n = v * (1 - s * f);
    switch (i) {
        case 6:
        case 0: RETURN_RGB(v, n, m);
        case 1: RETURN_RGB(n, v, m);
        case 2: RETURN_RGB(m, v, n)
        case 3: RETURN_RGB(m, n, v);
        case 4: RETURN_RGB(n, m, v);
        case 5: RETURN_RGB(v, m, n);
    }
}


void clear_color(int red, int green, int blue) {
    // Setup each matrix
    for(int addr=1; addr<=4; addr++) {
        sendCMD(addr, CMD_SET_PAPER, red, green, blue);
        sendCMD(addr, CMD_CLEAR_PAPER);
        sendCMD(addr, CMD_SWAP_BUF);
    }
}

/*
 * Self-explanatory
 */
void do_the_dance(int numTimes, int interDelay) {
    for(int i=0; i<numTimes; i++) {
        digitalWrite(REDLED, HIGH);
        clear_color(0xF, 0, 0);
        delay(interDelay);

        digitalWrite(REDLED, LOW);
        digitalWrite(GREENLED, HIGH);
        clear_color(0xF, 0xF, 0);
        delay(interDelay);

        digitalWrite(GREENLED, LOW);
        digitalWrite(BLUELED, HIGH);
        clear_color(0x0, 0xF, 0xF);
        delay(interDelay);

        digitalWrite(BLUELED, LOW);
        clear_color(0, 0, 0xF);
        delay(interDelay * 2);
    }
}

