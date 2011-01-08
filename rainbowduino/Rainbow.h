#ifndef Rainbow_h
#define Rainbow_h

#define I2C_DEVICE_ADDRESS 0x01
#define I2C_DEVICE_ADDRESS_CHAR (I2C_DEVICE_ADDRESS + 48)

#define FRONT_BUFFER       bufFront
#define BACK_BUFFER        bufBack

#define MAX_AUX_BUFFERS    4
#define MAX_GAMMA_SETS     3
#define MAX_WIRE_CMD_LEN   20

#define BLACK              0x0, 0x0, 0x0
#define WHITE              0xF, 0xF, 0xF
#define BLUE               0x0, 0x0, 0xF
#define GREEN              0x0, 0xF, 0x0
#define RED                0xF, 0x0, 0x0

#define G                  0
#define R                  1
#define B                  2

//=============================================
//MBI5168
#define SH_DIR_OE          DDRC
#define SH_DIR_SDI         DDRC
#define SH_DIR_CLK         DDRC
#define SH_DIR_LE          DDRC

#define SH_BIT_OE          0x08
#define SH_BIT_SDI         0x01
#define SH_BIT_CLK         0x02
#define SH_BIT_LE          0x04

#define SH_PORT_OE         PORTC
#define SH_PORT_SDI        PORTC
#define SH_PORT_CLK        PORTC
#define SH_PORT_LE         PORTC
//============================================
#define clk_rising         {SH_PORT_CLK&=~SH_BIT_CLK;SH_PORT_CLK|=SH_BIT_CLK;}
#define le_high            {SH_PORT_LE|=SH_BIT_LE;}
#define le_low             {SH_PORT_LE&=~SH_BIT_LE;}
#define enable_oe          {SH_PORT_OE&=~SH_BIT_OE;}
#define disable_oe         {SH_PORT_OE|=SH_BIT_OE;}

#define shift_data_1       {SH_PORT_SDI|=SH_BIT_SDI;}
#define shift_data_0       {SH_PORT_SDI&=~SH_BIT_SDI;}

//============================================
#define close_all_line	   {PORTD&=~0xf8;PORTB&=~0x07;}

//============================================
#define CheckRequest       (g8Flag1&0x01)
#define SetRequest         (g8Flag1|=0x01)
#define ClrRequest         (g8Flag1&=~0x01)

//==============================================
#define waitingcmd         0x00
#define processing         0x01
#define checking           0x02

#endif
