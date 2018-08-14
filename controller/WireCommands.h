#ifndef WireCommands_h
#define WireCommands_h

#define MAX_WIRE_CMD          0x80

#define CMD_NOP               0x00

#define CMD_SWAP_BUF          0x10
#define CMD_COPY_FRONT_BUF    0x11
#define CMD_SHOW_AUX_BUF      0x12

#define CMD_CLEAR_BUF         0x20
#define CMD_SET_PAPER         0x21
#define CMD_SET_INK           0x22
#define CMD_CLEAR_PAPER       0x25
#define CMD_DRAW_PIXEL        0x26
#define CMD_DRAW_LINE         0x27
#define CMD_DRAW_SQUARE       0x28
#define CMD_PRINT_CHAR        0x2A
#define CMD_DRAW_ROW_MASK     0x2B

const extern unsigned char CMD_totalArgs[MAX_WIRE_CMD];

#endif
