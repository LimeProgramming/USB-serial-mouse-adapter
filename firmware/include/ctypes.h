// These two lines prevents the file from being included multiple
// times in the same source file
#ifndef CTYPES_H_
#define CTYPES_H_1

// Mouse tracking speed modifier
enum LED_STATE {
  LED_OFF = 0,  
  LED_ON = 1,   
  LED_UNKNOWN_DEVICE = 2,
  LED_UNKNOWN_DEVICE_1 = 3,
    LED_SERIAL_CONNECTED = 4,
    LED_SERIAL_CONNECTED_1 = 5,
    LED_SERIAL_DISCONNECTED = 6,
    LED_SERIAL_DISCONNECTED_1 = 7,
};

// Mouse tracking speed modifier
enum MO_SPEED {
  SPEED100 = 0, // 100% speed
  SPEED75 = 1,  // 75% speed
  SPEED50 = 2,  // 50% speed 
  SPEED25 = 3   // 50% speed 
};

// Mouse Type enum
enum MO_TYPE {
  TWOBTN = 0,   // MS Two Button
  THREEBTN = 1,   // Logitech
  WHEELBTN = 2  // MS Wheel
};

// States of mouse init request from PC
enum PC_INIT_STATES {
  CTS_UNINIT   = 0, // Initial state
  CTS_LOW_INIT = 1, // CTS pin has been set low, wait for high.
  CTS_TOGGLED  = 2  // CTS was low, now high -> do ident.
};


#if CTYPES_C_ 

enum MO_SPEED MO_SPEED;
enum MO_TYPE MO_TYPE;
enum PC_INIT_STATES PC_INIT_STATES;

#else

extern enum MO_SPEED MO_SPEED;
extern enum MO_TYPE MO_TYPE;
extern enum PC_INIT_STATES PC_INIT_STATES;

#endif

#endif // CTYPES_H_