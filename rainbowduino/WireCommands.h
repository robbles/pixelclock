/*

  -0- -1- -2- -3- -.- -L-

  'r' CMD [o] [o] ... 0x0     byte0: 'r' identify a Rainbowduino Firmware 3.0 command string
                              byte1: CMD Hex code of the command
                              byte2 to ...: [o] Optional parameters for the command CMD (as many byte up to MAX_WIRE_CMD_LEN)
                              last byte:  0x0 the command string terminator
                              
  -CMD- -1- -2- -3-
  
  0x00                        RESET
  
> 0x10                        swapBuffers();
> 0x11   X   Y                copyFrontBuffer(X, Y);
> 0x12   N                    showAuxBuffer(N);
  0x13   B   N                storeToAuxBuffer(B, N);
  0x14   N                    getFromAuxBuffer(N);
  0x19   G                    setGamma(G);    

> 0x20   R   G   B            clearBuffer(R, G, B);
> 0x21   R   G   B            setPaperColor(R, G, B);
> 0x22   R   G   B            setInkColor(R, G, B);
> 0x25                        clearPaper();
> 0x26   X   Y                drawPixel(X, Y);
> 0x27   SX  SY  EX  EY       drawLine(SX, SY, EX, EY);
> 0x28   SX  SY  EX  EY       drawSquare(SX, SY, EX, EY);
  0x29   OX  OY  RAD          drawCircle(OX, OY, RAD);
> 0x2A   X   Y   C            printChar(X, Y, C);
> 0x2B   RW  OFX M            drawRowMask(RW, OFX, M);
  
  0x40                        scrollLeft();
  0x41                        scrollRight();
  0x42                        scrollUp();
  0x43                        scrollDown();
  0x45   L                    insLineLeft(L);
  0x46   L                    insLineRight(L);
  0x47   L                    insLineUp(L);
  0x48   L                    insLineDown(L);
  
  0x50                        clearStringBuffer();
  0x51   C                    charToStringBuffer(C);    (includes termination with /0)
  0x52   C   C   [...]   /0   wordToStringBuffer(W);    (W = C + C + C + ... + /0)
  0x55   D                    scrollStringBuffer(D);
  
  0x60                        clearColorBuffer();
  0x61   PR PG PB  IR IG IB   addToColorBuffer(PR, PG, PB, IR, IG, IB);
  0x62   S                    setColorBufferIterationStep(S);
  
*/

#ifndef WireCommands_h
#define WireCommands_h

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

#endif
