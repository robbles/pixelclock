#include <Wire.h>

#include "WProgram.h"
void setup();
void loop();
void displayTime(unsigned char hours, unsigned char minutes);
void ShowChar(int Address, unsigned char ASCII, unsigned char red, unsigned char blue ,unsigned char green, unsigned char shift);
void ShowCharOverlay(int Address, unsigned char ASCII, unsigned char red, unsigned char blue ,unsigned char green, unsigned char shift);
void ShowColor(int Address,unsigned char red , unsigned char green, unsigned char blue);
void ShowImage(int Address, unsigned char number,unsigned char shift);
void SentCMD(int  Add);
unsigned char RainbowCMD[5];
unsigned char State = 0;  
unsigned long timeout;

volatile int hours, minutes, seconds;
volatile boolean PM;
volatile boolean minutes_changed;

void setup()
{
    Wire.begin(); // join i2c bus (address optional for master) 

	// Setup 16-bit timer 1 to control servo
    TIMSK1 = 0x00;
	TCCR1A = 0x00; // CTC with TOP at ICR1
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10); // clk / 1024, 16-bit Fast PWM
	ICR1 = 15625; // Overflows every 1.0s
	TIMSK1 = _BV(ICIE1); // Trigger interrupt when timer reaches TOP	

    hours = 12;
    minutes = 0;
    seconds = 0;
    PM = true;
    minutes_changed = true;

    ShowColor(1, 0, 0, 0);    
    ShowColor(2, 0, 0, 0);    
    ShowColor(3, 0, 0, 0);    
    ShowColor(4, 0, 0, 0);    
}

void loop()
{ 

    displayTime(hours, minutes);

    delay(1);
}

/*
 * Updates the hours, minutes and seconds in typical clock-like fashion
 * every one second
 */
SIGNAL(TIMER1_CAPT_vect) {
    seconds = (seconds + 1) % 60;
    if(!seconds) {
        minutes = (minutes + 1) % 60;
        minutes_changed = true;
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
    char digit1, digit2, digit3, digit4;
    itoa(hours/10, &digit1, 10);
    itoa(hours % 10, &digit2, 10);
    itoa(minutes/10, &digit3, 10);
    itoa(minutes % 10, &digit4, 10);

    // NOTE: the slower changing digits must be overlaid
    // over the faster ones (e.g. minutes first, then hours)

    // Show the minutes
    if(minutes_changed) {
        minutes_changed = false;

        ShowChar(2, digit3, 15, 0, 15, 5);
        ShowChar(3, digit3, 15, 0, 15, 11);

        ShowCharOverlay(3, digit4, 15, 15, 0, 3);
        ShowChar(4, digit4, 15, 15, 0, 13);

        // Show divider between hours/minutes
        ShowCharOverlay(2, ':', 15, 15, 15, 1);

        // Show hours, hiding leading zero
        ShowChar(1, digit2, 0, 15, 15, 4);
        ShowCharOverlay(2, digit2, 0, 15, 15, 12);
        if(digit1 != '0') {
            ShowCharOverlay(1, digit1, 15, 15, 15, 10);
        }

        // Show AM/PM
        if(PM) {
            ShowCharOverlay(4, 'P', 15, 0, 0, 1);
        } else {
            ShowCharOverlay(4, 'A', 15, 0, 0, 1);
        }
    }

}


//--------------------------------------------------------------------------
//Name:ShowChar
//function: Send a conmand to Rainbowduino for showing a character
//parameter: Address: rainbowduino IIC address
//                 red,green,blue:  the color RGB  
//                 shift: the picture  shift bit for display
//                 ASCII:the char or Number want to show
//----------------------------------------------------------------------------
void ShowChar(int Address, unsigned char ASCII, unsigned char red, unsigned char blue ,unsigned char green, unsigned char shift)
{
    RainbowCMD[0]='R';
    RainbowCMD[1]=0x02;
    RainbowCMD[2]=((shift<<4)|(red));
    RainbowCMD[3]=((green<<4)|(blue));
    RainbowCMD[4]=ASCII;

    SentCMD(Address);
}

//--------------------------------------------------------------------------
//Name:ShowCharOverlay
//function: Send a conmand to Rainbowduino for showing a character
//parameter: Address: rainbowduino IIC address
//                 red,green,blue:  the color RGB  
//                 shift: the picture  shift bit for display
//                 ASCII:the char or Number want to show
//----------------------------------------------------------------------------
void ShowCharOverlay(int Address, unsigned char ASCII, unsigned char red, unsigned char blue ,unsigned char green, unsigned char shift)
{
    RainbowCMD[0]='R';
    RainbowCMD[1]=0x04;
    RainbowCMD[2]=((shift<<4)|(red));
    RainbowCMD[3]=((green<<4)|(blue));
    RainbowCMD[4]=ASCII;

    SentCMD(Address);
}


//--------------------------------------------------------------------------
//Name:ShowColor
//function: Send a conmand to Rainbowduino for showing a color
//parameter: Address: rainbowduino IIC address
//                 red,green,blue:  the color RGB    
//----------------------------------------------------------------------------
void ShowColor(int Address,unsigned char red , unsigned char green, unsigned char blue)
{
    RainbowCMD[0]='R';
    RainbowCMD[1]=0x03;
    RainbowCMD[2]=red;
    RainbowCMD[3]=((green<<4)|(blue));

    SentCMD(Address);
}


//--------------------------------------------------------------------------
//Name:ShowImage
//function: Send a conmand to Rainbowduino for showing a picture which was pre-set in Rainbowduino Flash
//parameter: Address: rainbowduino IIC address
//                 number:  the pre-set picture position
//                 shift: the picture  shift bit for display
//----------------------------------------------------------------------------
void ShowImage(int Address, unsigned char number,unsigned char shift)
{
    RainbowCMD[0]='R';
    RainbowCMD[1]=0x01;
    RainbowCMD[2]=(shift<<4);
    RainbowCMD[4]=number;

    SentCMD(Address);
}





//--------------------------------------------------------------------------
//Name:SentCMD
//function: Send a 5 byet Rainbow conmand out 
//parameter: NC  
//----------------------------------------------------------------------------
void SentCMD(int  Add)
{   unsigned char OK=0;
    unsigned char i,temp;

    while(!OK)
    {                          
        switch (State)
        { 	

            case 0:                          
                Wire.beginTransmission(Add);
                for (i=0;i<5;i++) Wire.send(RainbowCMD[i]);
                Wire.endTransmission();    
                delay(5);   
                State=1;                      
                break;

            case 1:

                Wire.requestFrom(Add,1);   
                if (Wire.available()>0) 
                    temp=Wire.receive();    
                else {
                    temp =0xFF;
                    timeout++;
                }

                if ((temp==1)||(temp==2)) State=2;
                else if (temp==0) State=0;

                if (timeout>5000)
                {
                    timeout=0;
                    State=0;
                }

                delay(5);
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

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

