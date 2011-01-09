#include <Wire.h>
#include <avr/pgmspace.h>

#include "Rainbow.h"
#include "WireCommands.h"

extern unsigned char dots_color[6][3][8][4];    // define Two Buffs (one for Display ,the other for receive data)
extern unsigned char GammaTab[3][16];           // define the Gamma value for correct the different LED matrix
extern unsigned char ASCII_Char[62][8];         // ASCII character table (READ-ONLY, it's stored in PROGMEM space)

unsigned char line, level;                      // used by the low-level rendering function
unsigned char g8Flag1;                          // NO IDEA AT THE MOMENT (derived from original code)

byte dispGamma;                                 // current gamma-ramp in use (index of GammaTab array)
byte bufFront, bufBack, bufCurr;                // used for handling the buffers

unsigned char RainbowCMD[MAX_WIRE_CMD_LEN];     // used to store command received via I2C protocol
byte paperR, paperG, paperB;                    // stores paper (background) color; used by I2C command chains
byte inkR, inkG, inkB;                          // stores ink (foreground) color; used by I2C command chains

unsigned char State=0;                          // comes from original code

void setup(void) {
  _init();                // Initialize Ports, Timer and Interrupts

  clearBuffer(paperR, paperG, paperB);

  // I2C address in white
  printChar(0, 0, 0xF, 0xF, 0xF, I2C_DEVICE_ADDRESS_CHAR);

  // Line on left in green
  drawLine(7, 0, 7, 7, 0x0, 0xF, 0x0);

  // Line on right in blue
  drawLine(0, 0, 0, 7, 0x0, 0x0, 0xF);

  swapBuffers();
}

void loop(void) {
  switch(State) {
    case waitingcmd:
    break;
    
    case processing:
      processWireCommand();
      State=checking;
    break;
    
    case checking:
      if(CheckRequest) {
        State=waitingcmd;
        ClrRequest;
      }
      break;
      
    default:
      State=waitingcmd;
      break;
  }
}
 
/*----------------------------------------------------------------------------*/
/* DRAWING FUNCTIONS                                                          */
/*----------------------------------------------------------------------------*/

void _initGfx() { // Init the graphic side
  dispGamma = 1;
  bufFront = 0;
  bufBack = 1;
  bufCurr = 0;    
}

void setGamma(byte gamma) { // Change the gamma array pointer, used during display rendering
  if ( gamma < MAX_GAMMA_SETS ) {
    dispGamma = gamma;
  }
}

void drawPixel(int x, int y, byte r, byte g, byte b) {
  if ( (x>=0 && x<8) && (y>=0 && y<8) ) { // Check if it's in the display range
    byte mask = 0xF0;
    if ( !(x % 2) ) { // If x is unpair, shift each color component four bit left
      r = (r << 4);
      g = (g << 4);
      b = (b << 4);
      mask = 0x0F;
    }
  
    x /= 2;
    replace_dot(bufBack, x, y, bufBack, x, y, r, g, b, mask);
  }
}  

void drawLine(int sx, int sy, int ex, int ey, byte r, byte g, byte b) {
  int i, lx, ly;
  
  lx = abs(ex-sx);
  ly = abs(ey-sy);
  
  if ( lx > ly) {
    for(i=0; i<=lx; i++) {
      drawPixel(map(i, 0, lx, sx, ex), map(i, 0, lx, sy, ey), r, g, b);
    }
  } else {
    for(i=0; i<=ly; i++) {
      drawPixel(map(i, 0, ly, sx, ex), map(i, 0, ly, sy, ey), r, g, b);
    }
  }
}

void drawSquare(int sx, int sy, int ex, int ey, byte r, byte g, byte b) {
  int i, ly, rw;
  
  ly = abs(ey-sy);
  
  for(i=0; i<=ly; i++) {
    rw = map(i, 0, ly, sy, ey);
    drawLine(sx, rw, ex, rw, r, g, b);
  }
}

void printChar(int x, int y, byte r, byte g, byte b, unsigned char ASCII) { // Print a char using array ASCII_Char as font source
  unsigned char idx;
  byte bitmask;
  int row, col;
  int l;

  if((ASCII > 64) && (ASCII < 91)) idx=ASCII-55; // Convert from ASCII to index of character's array
  else if((ASCII > 96) && (ASCII < 123)) idx=ASCII-61;
  else if((ASCII >= '0') && (ASCII <= '9')) idx=ASCII-48;
  
  l = 0;  // Counter used to extract each line from the character map
  for (row=(7+y); row>=y; row--) {    
    if ( row < 8 && row >= 0 ) { // Check if row is within visible area
      bitmask = pgm_read_byte(&(ASCII_Char[idx][l]));  // Extract the bitmask for each line of the character
      drawRowMask(row, x, r, g, b, bitmask);
     }
    l++;
  }
}

void drawRowMask(int row, int off, byte r, byte g, byte b, byte bitmask) {
  byte col;
  
  if ( (abs(off) < 8) && (row < 8) && (row >= 0) ) {
    if ( off > 0 ) bitmask = (bitmask << off);  // Shift the bitmask according to x-offset
    else bitmask = (bitmask >> -off);
  
    for (col=0; col<4; col++) {     // Check the first bit of the bitmask
      if ( bitmask & B00000001 ) {
        replace_dot(bufBack, col, row, bufBack, col, row, (r << 4), (g << 4), (b << 4), 0x0F);
      }
      if ( bitmask & B00000010 ) {  // Check the second bit of the bitmask
        replace_dot(bufBack, col, row, bufBack, col, row, r, g, b, 0xF0);
      }
      bitmask = (bitmask >> 2);     // Shift the bitmask two bits left
    }
  }
}

/* BUFFERS HANDLING */

void swapBuffers() { // Swap Front with Back buffer
  if ( bufFront > 1 ) { // If we're showing an Aux Buffer
    bufFront = bufBack;
    bufBack = !bufBack;
  } else {
    bufFront = !bufFront;
    bufBack = !bufBack;
  }
  while(bufCurr != bufFront) {    // Wait for display to change.
    delayMicroseconds(10);
  }
}

void showAuxBuffer(byte n) { // Show an Auxiliary buffer
  if (n < MAX_AUX_BUFFERS) {
    bufFront = n+2;
   while(bufCurr != bufFront) {  // Wait for display to change.
      delayMicroseconds(10);
    }
  }
}

void storeToAuxBuffer(byte buf, byte n) { // Store the buffer to Aux Buffer #
  byte x, y;
  if (n < MAX_AUX_BUFFERS) {
    n+=2;
    for (y=0; y<8; y++) {
      for (x=0; x<4; x++) {
        dots_color[n][R][y][x]=dots_color[buf][R][y][x];
        dots_color[n][G][y][x]=dots_color[buf][G][y][x];
        dots_color[n][B][y][x]=dots_color[buf][B][y][x];
      }
    }
  }
}

void getFromAuxBuffer(byte n) { // Retrieve Aux Buffer # to Back (not visibile) buffer;
  byte x, y;
  
  if (n < MAX_AUX_BUFFERS) {
    n+=2;
    for (y=0; y<8; y++) {
      for (x=0; x<4; x++) {
        dots_color[bufBack][R][y][x]=dots_color[n][R][y][x];
        dots_color[bufBack][G][y][x]=dots_color[n][G][y][x];
        dots_color[bufBack][B][y][x]=dots_color[n][B][y][x];
      }
    }
  }
}

void clearBuffer(byte r, byte g, byte b) {  // Clear the Back buffer and fill with selected color
  byte x, y;
  r = r | (r << 4);
  g = g | (g << 4);
  b = b | (b << 4);

  for (y=0; y<8; y++) {
    for (x=0; x<4; x++) {
      dots_color[bufBack][G][y][x]=g;
      dots_color[bufBack][R][y][x]=r;
      dots_color[bufBack][B][y][x]=b;
    }
  }
}

void copyFrontBuffer(int x, int y) { // Copy the Front buffer to the Back one, offsetting it by x and y
  int row, col;
  int offy, offx;
  byte t0, t1, t2;
  for (row=0; row<8; row++) {
    
    offy = row-y;
    if ( offy>=0 && offy<8 ) {
      for (col=0; col<4; col++) {
        
        offx = col+(x/2);
        if ( offx>=0 && offx<4 ) {
          if ( !(x%2) ) { // if x-offset is pair, we just need to copy the byte by the offset
            dots_color[bufBack][R][row][col]=dots_color[bufFront][R][offy][offx];
            dots_color[bufBack][G][row][col]=dots_color[bufFront][G][offy][offx];
            dots_color[bufBack][B][row][col]=dots_color[bufFront][B][offy][offx];
          } else { // if x-offset is unpair, we need to shift each byte by four-bit in the offset direction
            t0 = dots_color[bufFront][R][offy][offx];
            t1 = dots_color[bufFront][G][offy][offx];
            t2 = dots_color[bufFront][B][offy][offx];
            
            if ( x >= 0 ) { // if x-offset is positive (and unpair) shift four-bit to the right
              replace_dot(bufBack, col, row, bufBack, col, row, (t0 >> 4), (t1 >> 4), (t2 >> 4), 0xF0);
              offx = col+1; // NOTE: I reuse offx variable to avoid initializing a new one. This affect only the 2 lines below.
              if ( offx < 4) // if it's not the right end side column
                replace_dot(bufBack, offx, row, bufBack, offx, row, (t0 << 4), (t1 << 4), (t2 << 4), 0x0F);
            } else { // if x-offset is negative (and unpair) shift four-bit to the left
              replace_dot(bufBack, col, row, bufBack, col, row, (t0 << 4), (t1 << 4), (t2 << 4), 0x0F);
              offx = col-1; // NOTE: I reuse offx variable to avoid initializing a new one. This affect only the 2 lines below.
              if ( offx >= 0) // if it's not the left end side column
                replace_dot(bufBack, offx, row, bufBack, offx, row, (t0 >> 4), (t1 >> 4), (t2 >> 4), 0xF0);
            }
            
          }
        }
        
      }
    }
    
  }
}

void replace_dot(byte fromBuf, int fromX, int fromY, byte toBuf, int toX, int toY, byte vR, byte vG, byte vB, byte mask) {
  // Replace a single dot ( 4 byte ) in toBuf buffer with the value from fromBuf buffer
  dots_color[toBuf][0][toY][toX]=(vG | (dots_color[fromBuf][0][fromY][fromX] & mask));
  dots_color[toBuf][1][toY][toX]=(vR | (dots_color[fromBuf][1][fromY][fromX] & mask));
  dots_color[toBuf][2][toY][toX]=(vB | (dots_color[fromBuf][2][fromY][fromX] & mask));
}

/*----------------------------------------------------------------------------*/
/* I2C EVENT HANDLING FUNCTIONS                                               */
/*----------------------------------------------------------------------------*/

void _initWire() {
  int i;
  for (i=0; i<MAX_WIRE_CMD_LEN; i++) {
    RainbowCMD[i] = 0x00;         // Clear Command Buffer
  }
  paperR = 0x0; paperG = 0x0; paperB = 0x0;
  inkR = 0xF; inkG = 0x0; inkB = 0x0;

  Wire.begin(I2C_DEVICE_ADDRESS); // join I2C bus (address optional for master) 
  Wire.onReceive(receiveEvent);   // define the receive function for receiving data from master
  Wire.onRequest(requestEvent);   // define the request function for the request from master
}

#define CMD_ACKNOWLEDGED  {RainbowCMD[1] = CMD_NOP;}

void processWireCommand() {
  int x, y, ex, ey;
  byte n;
  byte r, g, b;
  switch(RainbowCMD[1]) {
    case CMD_SWAP_BUF:
      CMD_ACKNOWLEDGED;
      swapBuffers();
      break;
    case CMD_COPY_FRONT_BUF:
      CMD_ACKNOWLEDGED;
      x = toInt(RainbowCMD[2]); y = toInt(RainbowCMD[3]);
      copyFrontBuffer(x, y);
      break;
    case CMD_SHOW_AUX_BUF:
      CMD_ACKNOWLEDGED;
      n = RainbowCMD[2];
      showAuxBuffer(n);
      break;
    case CMD_CLEAR_BUF:
      CMD_ACKNOWLEDGED;
      r = RainbowCMD[2]; g = RainbowCMD[3]; b = RainbowCMD[4];
      clearBuffer(r, g, b);
      break;
    case CMD_SET_PAPER:
      CMD_ACKNOWLEDGED;
      paperR = RainbowCMD[2]; paperG = RainbowCMD[3]; paperB = RainbowCMD[4];
      break;
    case CMD_SET_INK:
      CMD_ACKNOWLEDGED;
      inkR = RainbowCMD[2]; inkG = RainbowCMD[3]; inkB = RainbowCMD[4];
      break;
    case CMD_CLEAR_PAPER:
      CMD_ACKNOWLEDGED;
      clearBuffer(paperR, paperG, paperB);
      break;
    case CMD_DRAW_PIXEL:
      CMD_ACKNOWLEDGED;
      x = toInt(RainbowCMD[2]); y = toInt(RainbowCMD[3]);
      drawPixel(x, y, inkR, inkG, inkB);
      break;
    case CMD_DRAW_LINE:
      CMD_ACKNOWLEDGED;
      x = toInt(RainbowCMD[2]); y = toInt(RainbowCMD[3]);
      ex = toInt(RainbowCMD[4]); ey = toInt(RainbowCMD[5]);
      drawLine(x, y, ex, ey, inkR, inkG, inkB);
      break;
    case CMD_DRAW_SQUARE:
      CMD_ACKNOWLEDGED;
      x = toInt(RainbowCMD[2]); y = toInt(RainbowCMD[3]);
      ex = toInt(RainbowCMD[4]); ey = toInt(RainbowCMD[5]);
      drawSquare(x, y, ex, ey, inkR, inkG, inkB);
      break;
    case CMD_PRINT_CHAR:
      CMD_ACKNOWLEDGED;
      x = toInt(RainbowCMD[2]); y = toInt(RainbowCMD[3]);
      n = RainbowCMD[4];
      printChar(x, y, inkR, inkG, inkB, n);
      break;
    case CMD_DRAW_ROW_MASK:
      CMD_ACKNOWLEDGED;
      y = toInt(RainbowCMD[2]); x = toInt(RainbowCMD[3]);
      n = RainbowCMD[4];
      drawRowMask(y, x, inkR, inkG, inkB, n);
      break;
  }
}

void requestEvent(void) { // While processing or checking previous directive, request other commands
  Wire.send(State); 
  if ((State==processing) || (State==checking)) SetRequest;
}

// Adapted from the original code (with minor changes)
void receiveEvent(int howMany) {
  unsigned char i=0;
  while(Wire.available()>0) { 
    RainbowCMD[i]=Wire.receive();
    if ( i < MAX_WIRE_CMD_LEN) i++;
  }

  if(RainbowCMD[0]=='r') State=processing;
  else State=waitingcmd;

}

int toInt(byte b) {
  return map(b, 0, 255, -128, 127);
}

/*----------------------------------------------------------------------------*/
/* LED MATRIX DISPLAY HANDLING                                                */
/*----------------------------------------------------------------------------*/

void flash_next_line(unsigned char ln, unsigned char lvl) {  // Scan one line
  disable_oe;
  close_all_line;

  if(ln < 3) {    // Open the line and close others
    PORTB  = (PINB & ~0x07) | 0x04 >> ln;
    PORTD  = (PIND & ~0xF8);
  } else {
    PORTB  = (PINB & ~0x07);
    PORTD  = (PIND & ~0xF8) | 0x80 >> (ln - 3);
  }
  
  shift_24_bit(ln, lvl);
  enable_oe;
}

void shift_24_bit(unsigned char ln, unsigned char lvl) {   // Display one line by the color level in buff
  unsigned char color=0,row=0;
  unsigned char data0=0,data1=0;
  
  le_high;
  for(color=0;color<3;color++) {   // The color order is G-B-R
    for(row=0;row<4;row++) {
      data1=dots_color[bufCurr][color][ln][row]&0x0f;
      data0=dots_color[bufCurr][color][ln][row]>>4;

      if(data0>lvl) {    //gray scale, 0x0f aways light (original comment, not sure what it means)
        shift_data_1;
        clk_rising;
      } else {
        shift_data_0;
        clk_rising;
      }
      
      if(data1>lvl) {
        shift_data_1;
        clk_rising;
      } else {
        shift_data_0;
        clk_rising;
      }
    }
  }
  le_low;
}


/*----------------------------------------------------------------------------*/
/* INIT TIMERS AND INTERRUPT FUNCTIONS                                        */
/*----------------------------------------------------------------------------*/

void _init(void) {  // define the pin mode
  DDRD=0xff;        // Configure ports (see http://www.arduino.cc/en/Reference/PortManipulation): digital pins 0-7 as OUTPUT
  DDRC=0xff;        // analog pins 0-5 as OUTPUT
  DDRB=0xff;        // digital pins 8-13 as OUTPUT
  PORTD=0;          // Configure ports data register (see link above): digital pins 0-7 as READ
  PORTB=0;          // digital pins 8-13 as READ

  level = 0;
  line = 0;

  _initWire();      // init I2C communication protocol
  _initGfx();       // init Graphic
  
  init_timer2();    // init the timer for flashing the LED matrix
}

void init_timer2(void) {                  // Initialize Timer2
  TCCR2A |= (1 << WGM21) | (1 << WGM20);   
  TCCR2B |= (1<<CS22);                    // by clk/64
  TCCR2B &= ~((1<<CS21) | (1<<CS20));     // by clk/64
  TCCR2B &= ~((1<<WGM21) | (1<<WGM20));   // Use normal mode
  ASSR |= (0<<AS2);                       // Use internal clock - external clock not used in Arduino
  TIMSK2 |= (1<<TOIE2) | (0<<OCIE2B);     //Timer2 Overflow Interrupt Enable
  TCNT2 = GammaTab[dispGamma][0];
  sei();   
}

ISR(TIMER2_OVF_vect) {  // Timer2 Service 
  TCNT2 = GammaTab[dispGamma][level];  // Set the flashing time using gamma value table
  flash_next_line(line,level);         // flash the next line in LED matrix for the current level.
  line++;

  if(line > 7) {                // After flashing all LED's lines, go back to line 0 and increase the level
    line = 0;
    level++;
    if(level>15) {              // After flashng all levels, go back to level 0 and, eventually, swaps the buffers
      level = 0;
      bufCurr = bufFront;       // do the actual swapping, synced with display refresh.
    }
  }
}


