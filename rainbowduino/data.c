#include <avr/pgmspace.h>

// Gamma tables
unsigned char GammaTab[3][16]= {
    {0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7},    // Default linear gamma
    {0xFF,0xFE,0xFD,0xFC,0xFB,0xF9,0xF7,0xF5,0xF3,0xF0,0xED,0xEA,0xE7,0xE4,0xE1,0xDD},    // Progressive gamma
    {0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7,0xE7}     // Undefined
};

// Display dot buffer
unsigned char dots_color[6][3][8][4] = {
//== 0 ================================================
{
  { {0x00,0x00,0x00,0x4b}, //green 
    {0x00,0x00,0x04,0xbf},
    {0x00,0x00,0x4b,0xff},
    {0x00,0x04,0xbf,0xff},
    {0x00,0x4b,0xff,0xff},
    {0x04,0xbf,0xff,0xff},
    {0x4b,0xff,0xff,0xff},
    {0xbf,0xff,0xff,0xfd}
  },
  { {0xff,0xfd,0x71,0x00}, //red 
    {0xff,0xd7,0x10,0x00},
    {0xfd,0xf1,0x00,0x00},
    {0xda,0x10,0x00,0x00},
    {0x71,0x00,0x00,0x01},
    {0x10,0x00,0x00,0x17},
    {0x00,0x00,0x01,0x7e},
    {0x00,0x00,0x17,0xef}
  },
  { {0x06,0xef,0xff,0xff}, //blue 
    {0x6e,0xff,0xff,0xff},
    {0xef,0xff,0xff,0xfa},
    {0xff,0xff,0xff,0xa3},
    {0xff,0xff,0xfa,0x30},
    {0xff,0xfa,0xa3,0x00},
    {0xff,0xfa,0x30,0x00},
    {0xff,0xa3,0x00,0x00}
  }
},
//== 1 =================================================
{
  { {0x00,0x00,0x00,0x4b}, //green
    {0x00,0x00,0x04,0xbf},
    {0x00,0x00,0x4b,0xff},
    {0x00,0x04,0xbf,0xff},
    {0x00,0x4b,0xff,0xff},
    {0x04,0xbf,0xff,0xff},
    {0x4b,0xff,0xff,0xff},
    {0xbf,0xff,0xff,0xfd}
  },
  { {0xff,0xfd,0x71,0x00}, //red 
    {0xff,0xd7,0x10,0x00},
    {0xfd,0xf1,0x00,0x00},
    {0xda,0x10,0x00,0x00},
    {0x71,0x00,0x00,0x01},
    {0x10,0x00,0x00,0x17},
    {0x00,0x00,0x01,0x7e},
    {0x00,0x00,0x17,0xef}
  },
  { {0x06,0xef,0xff,0xff}, //blue
    {0x6e,0xff,0xff,0xff},
    {0xef,0xff,0xff,0xfa},
    {0xff,0xff,0xff,0xa3},
    {0xff,0xff,0xfa,0x30},
    {0xff,0xfa,0xa3,0x00},
    {0xff,0xfa,0x30,0x00},
    {0xff,0xa3,0x00,0x00}
  }
 },
//== 2 == AUX0 =========================================
 {
  { {0xff,0xbf,0x00,0x00}, //green
    {0xff,0xbf,0x00,0x00},
    {0xff,0xbf,0x00,0x00},
    {0xff,0xbf,0x00,0x00},
    {0xff,0xbf,0x00,0x00},
    {0xff,0xbf,0x00,0x00},
    {0x01,0x23,0x45,0x67},
    {0x89,0xab,0xcd,0xef}
  },
  { {0xff,0x00,0xff,0x00}, //red
    {0xff,0x00,0xff,0x00},
    {0xff,0x00,0xff,0x00},
    {0xff,0x00,0xff,0x00},
    {0xff,0x00,0xff,0x00},
    {0xff,0x00,0xff,0x00},
    {0x01,0x23,0x45,0x67},
    {0x89,0xab,0xcd,0xef}
  },
  { {0xf0,0xf0,0xd0,0xf0}, //blue 
    {0xf0,0xf0,0xd0,0xf0},
    {0xf0,0xf0,0xd0,0xf0},
    {0xf0,0xf0,0xd0,0xf0},
    {0xf0,0xf0,0xd0,0xf0},
    {0xf0,0xf0,0xd0,0xf0},
    {0x01,0x23,0x45,0x67},
    {0x89,0xab,0xcd,0xef}
  }
 },
//== 3 == AUX1 =========================================
 {
  { {0x00,0x00,0x00,0x00}, //green 
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x12,0x34,0x57},
    {0x89,0xAB,0xCD,0xEF},
    {0x01,0x23,0x45,0x67},
    {0x89,0xAB,0xCD,0xEF},
  },
  { {0x00,0x00,0x00,0x00}, //red
    {0x00,0x00,0x00,0x00},
    {0x00,0x12,0x34,0x57},
    {0x89,0xAB,0xCD,0xEF},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x01,0x23,0x45,0x67},
    {0x89,0xAB,0xCD,0xEF},
  },
  { {0x00,0x12,0x34,0x57}, //blue
    {0x89,0xAB,0xCD,0xEF},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x01,0x23,0x45,0x67},
    {0x89,0xAB,0xCD,0xEF},
  }
 },
//== 4 == AUX2 =========================================
 {
  { {0x00,0x00,0x00,0x4b}, //green
    {0x00,0x00,0x04,0xbf},
    {0x00,0x00,0x4b,0xff},
    {0x00,0x04,0xbf,0xff},
    {0x00,0x4b,0xff,0xff},
    {0x04,0xbf,0xff,0xff},
    {0x4b,0xff,0xff,0xff},
    {0xbf,0xff,0xff,0xfd}
  },
  { {0xff,0xfd,0x71,0x00}, //red
    {0xff,0xd7,0x10,0x00},
    {0xfd,0xf1,0x00,0x00},
    {0xda,0x10,0x00,0x00},
    {0x71,0x00,0x00,0x01},
    {0x10,0x00,0x00,0x17},
    {0x00,0x00,0x01,0x7e},
    {0x00,0x00,0x17,0xef}
  },
  { {0x06,0xef,0xff,0xff}, //blue
    {0x6e,0xff,0xff,0xff},
    {0xef,0xff,0xff,0xfa},
    {0xff,0xff,0xff,0xa3},
    {0xff,0xff,0xfa,0x30},
    {0xff,0xfa,0xa3,0x00},
    {0xff,0xfa,0x30,0x00},
    {0xff,0xa3,0x00,0x00}
  }
 },
//== 5 == AUX3 =========================================
 {
  { {0x00,0x00,0x00,0x00}, //green
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00}
  },
  { {0x0f,0xff,0xff,0xff}, //red
    {0x0f,0xff,0xff,0xff},
    {0x0f,0xff,0xff,0xff},
    {0x0f,0xff,0xff,0xff},
    {0x0f,0xff,0xff,0xff},
    {0x0f,0xff,0xff,0xff},
    {0x0f,0xff,0xff,0xff},
    {0x0f,0xff,0xff,0xff}
  },
  { {0x00,0x00,0x00,0x00}, //blue
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00}
  }
 }
};

// Internal Font (read-only)
unsigned char ASCII_Char[62][8] PROGMEM = {
    {0x1c,0x22,0x26,0x2a,0x32,0x22,0x1c,0x0}, // 0
    {0x8,0x18,0x8,0x8,0x8,0x8,0x1c,0x0}, // 1
    {0x1c,0x22,0x2,0x4,0x8,0x10,0x3e,0x0}, // 2
    {0x3e,0x4,0x8,0x4,0x2,0x22,0x1c,0x0}, // 3
    {0x4,0xc,0x14,0x24,0x3e,0x4,0x4,0x0}, // 4
    {0x3e,0x20,0x3c,0x2,0x2,0x22,0x1c,0x0}, // 5

    {0xc,0x10,0x20,0x3c,0x22,0x22,0x1c,0x0}, // 6

    {0x3e,0x2,0x4,0x8,0x10,0x10,0x10,0x0}, // 7
    {0x1c,0x22,0x22,0x1c,0x22,0x22,0x1c,0x0}, // 8
    {0x1c,0x22,0x22,0x1e,0x2,0x4,0x18,0x0}, // 9

    {0x8,0x14,0x22,0x22,0x3e,0x22,0x22,0x0}, // A
    {0x3c,0x22,0x22,0x3c,0x22,0x22,0x3c,0x0}, // B   
    {0x1c,0x22,0x20,0x20,0x20,0x22,0x1c,0x0}, // C   
    {0x38,0x24,0x22,0x22,0x22,0x24,0x38,0x0}, // D  
    {0x3e,0x20,0x20,0x3c,0x20,0x20,0x3e,0x0}, // E
    {0x3e,0x20,0x20,0x3c,0x20,0x20,0x20,0x0}, // F
    {0x1c,0x22,0x20,0x2e,0x22,0x22,0x1e,0x0}, // G
    {0x22,0x22,0x22,0x3e,0x22,0x22,0x22,0x0}, // H
    {0x1c,0x8,0x8,0x8,0x8,0x8,0x1c,0x0}, // I
    {0xe,0x4,0x4,0x4,0x4,0x24,0x18,0x0}, // J
    {0x22,0x24,0x28,0x30,0x28,0x24,0x22,0x0}, // K
    {0x20,0x20,0x20,0x20,0x20,0x20,0x3e,0x0}, // L
    {0x22,0x36,0x2a,0x2a,0x22,0x22,0x22,0x0}, // M
    {0x22,0x22,0x32,0x2a,0x26,0x22,0x22,0x0}, // N
    {0x1c,0x22,0x22,0x22,0x22,0x22,0x1c,0x0}, // O
    {0x3c,0x22,0x22,0x3c,0x20,0x20,0x20,0x0}, // P
    {0x1c,0x22,0x22,0x22,0x2a,0x24,0x1a,0x0}, // Q
    {0x3c,0x22,0x22,0x3c,0x28,0x24,0x22,0x0}, // R
    {0x1e,0x20,0x20,0x1c,0x2,0x2,0x3c,0x0}, // S
    {0x3e,0x8,0x8,0x8,0x8,0x8,0x8,0x0}, // T
    {0x22,0x22,0x22,0x22,0x22,0x22,0x1c,0x0}, // U   
    {0x22,0x22,0x22,0x22,0x22,0x14,0x8,0x0}, // V     
    {0x22,0x22,0x22,0x2a,0x2a,0x2a,0x14,0x0}, // W    
    {0x22,0x22,0x14,0x8,0x14,0x22,0x22,0x0}, // X   
    {0x22,0x22,0x22,0x14,0x8,0x8,0x8,0x0}, // Y    
    {0x3e,0x2,0x4,0x8,0x10,0x20,0x3e,0x0}, // Z
    
    {0x0,0x0,0x1c,0x2,0x1e,0x22,0x1e,0x0}, // a   
    {0x20,0x20,0x2c,0x32,0x22,0x22,0x3c,0x0}, // b    
    {0x0,0x0,0x1c,0x20,0x20,0x22,0x1c,0x0}, // c    
    {0x2,0x2,0x1a,0x26,0x22,0x22,0x1e,0x0}, // d    
    {0x0,0x0,0x1c,0x22,0x3e,0x20,0x1c,0x0}, // e    
    {0xc,0x12,0x10,0x38,0x10,0x10,0x10,0x0}, // f    
    {0x0,0x0,0x1e,0x22,0x22,0x1e,0x2,0x1c}, // g    
    {0x20,0x20,0x2c,0x32,0x22,0x22,0x22,0x0}, // h   
    {0x8,0x0,0x18,0x8,0x8,0x8,0x1c,0x0}, // i    
    {0x4,0x0,0xc,0x4,0x4,0x4,0x24,0x18}, // j    
    {0x20,0x20,0x24,0x28,0x30,0x28,0x24,0x0}, // k    
    {0x18,0x8,0x8,0x8,0x8,0x8,0x1c,0x0}, // l    
    {0x0,0x0,0x34,0x2a,0x2a,0x22,0x22,0x0}, // m   
    {0x0,0x0,0x2c,0x32,0x22,0x22,0x22,0x0}, // n    
    {0x0,0x0,0x1c,0x22,0x22,0x22,0x1c,0x0}, // o    
    {0x0,0x0,0x3c,0x22,0x22,0x3c,0x20,0x20}, // p    
    {0x0,0x0,0x1a,0x26,0x26,0x1a,0x2,0x2}, // q   
    {0x0,0x0,0x2c,0x32,0x20,0x20,0x20,0x0}, // r    
    {0x0,0x0,0x1c,0x20,0x1c,0x2,0x3c,0x0}, // s    
    {0x10,0x10,0x38,0x10,0x10,0x12,0xc,0x0}, // t   
    {0x0,0x0,0x22,0x22,0x22,0x26,0x1a,0x0}, // u    
    {0x0,0x0,0x22,0x22,0x22,0x14,0x8,0x0}, // v   
    {0x0,0x0,0x22,0x22,0x2a,0x2a,0x14,0x0}, // w    
    {0x0,0x0,0x22,0x14,0x8,0x14,0x22,0x0}, // x   
    {0x0,0x0,0x22,0x22,0x22,0x1e,0x2,0x1c}, // y   
    {0x0,0x0,0x3e,0x4,0x8,0x10,0x3e,0x0}, // z
};
