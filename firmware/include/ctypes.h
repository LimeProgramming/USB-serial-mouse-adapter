// These two lines prevents the file from being included multiple
// times in the same source file
#ifndef CTYPES_H_
#define CTYPES_H_1

#include "pico.h"


/* -------------------- Some Basic emums -------------------- */

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

// Mouse Movement Type
enum MO_MVT_TYPE {
  MO_MVT_ADDITIVE  = 0, // Cumulate and constrain the X and Y values.
  MO_MVT_AVERAGE = 1, // Average X and Y values.
  MO_MVT_COAST  = 2  // Coast X and Y values.
};

// Cosine Smoothed
enum MO_MVT_COS {
  MO_MVT_COS_DISABLED = 0,
  MO_MVT_COS_LOW = 1,
  MO_MVT_COS_MEDIUM = 2,
  MO_MVT_COS_HIGH = 3,
  MO_MVT_COS_VERYHIGH = 4
};

// Mouse Type enum
enum CORE_FLAGS {
  cf_nothing = 0,
  cf_stop = 1, 
  cf_resume = 2,
  cf_update = 3,
  cf_post = 4,
  cf_nopost = 5
};


/* -------------------- Mouse Packet Structure -------------------- */
/*  This is the hold the mouse state for the previous cycle. 
    Each cycle is defined by the Baud rate. */

typedef struct {    
    // State of left, middle and right buttons                        
    bool left, middle, right;

    // mouse location delta
    int16_t  x, y, wheel;

    // Is update required this cycle?
    bool update;

} MOUSE_PKT;

/* -------------------- Raw Mouse Packet Structure -------------------- */
/*  This is the hold the mouse state for the previous cycle. 
    Each cycle is defined by the Baud rate. */
typedef struct {    

  // The Button Flip Flop       | Left Click + Middle Click + Right Click
  bool btnFlipFlop[3];  

  // FlipFlop Cycle Update Flag | Left Click + Middle Click + Right Click       
  bool btnUpdated[3];   

  // FlipFlop toggle flag       | Left Click + Middle Click + Right Click                
  bool btnToggle[3];                     

  // Mouse location delta       | X + Y + Wheel
  int16_t  x, y, wheel;

} MOUSE_RPKT;

/* -------------------- Persistent Data Structure -------------------- */
/*  This is to hold the mouse data that is to survive hard reboots */

typedef struct  {
    // 255 = first time execution | false = ran before.
    uint8_t firstrun;

    // global limit | Range 1 -> 200
    uint8_t xytravel_percentage;

    // seperate x and y limits. | Range 1 -> 200
    uint8_t xtravel_percentage;
    uint8_t ytravel_percentage;

    // TWOBTN = 0 | THREEBTN = 1 | WHEELBTN = 2 
    uint8_t mousetype;

    // Double Stop Bit
    // 7N1 = 0 | 7N2 = 1
    bool doublestopbit;

    // Baud rate 
    // 1200 | 2400 | 4800 | 9600 | 19200
    uint baudrate;

    // Swap the left and right buttons 
    bool swap_left_right;

    // use forward and backward as ALT left and right buttons
    bool use_forward_backward;

    // Swap forward and backwards
    bool swap_forward_backward;

    // Invert X and Y movement
    bool invert_x;
    bool invert_y;

    // Mouse Movement Type enum MO_MVT_TYPE
    uint8_t mouse_movt_type;

    // USe cosine smoothing
    uint8_t use_cosine_smoothing;

    // Firmware Versioning
    uint8_t FW_V_MAJOR;
    uint8_t FW_V_MINOR;
    uint8_t FW_V_REVISION;

    // Previous DipSwitch Button State
    bool ST_DIPSW_THREEBTN;         // DIP 1
    bool ST_DIPSW_WHEEL;            // DIP 2
    bool ST_DIPSW_75XYSPEED;        // DIP 3
    bool ST_DIPSW_50XYSPEED;        // DIP 4
    bool ST_DIPSW_7N2;              // DIP 5
    bool ST_DIPSW_19200;            // DIP 6

    uint8_t language;

} PERSISTENT_MOUSE_DATA; 

extern PERSISTENT_MOUSE_DATA pmData;

/* -------------------- Mouse Settings -------------------- */
// Mouse Settings and information

typedef struct {                            

  // CTS state tracker | taken from Aviancer's code since it was more straightforward than what I had already
  uint8_t pc_state;

  // yeah that should be a bitwise thing but eh
  // serial state tracker. 0 = Mouse mode | 1 = terminal mode uart 0 | 2 = terminal mode uart 1 | 3 = paused
  uint8_t serial_state;

  // Current Processed Mouse Packet.                    
  MOUSE_PKT mpkt;                       

  // Raw Mouse data.                    
  MOUSE_RPKT rmpkt;    

  // Persisten Mouse data, survives reboots.
  PERSISTENT_MOUSE_DATA persistent;               

  // Is mouse connected flag
  uint8_t mouse_count;
  
  // counr the number of mouse updates between serial cycles. USed for AVG mouse movement
  uint8_t mouse_movt_ticker;

  // The real Baudrate returned by pico
  uint realbaudrate;

  // One Bit Delay Time, calculated on startup.
  float serialdelay_1b;    

  // One Byte Delay Time, calculated on startup.
  uint serialdelay_1B;        

  // Three Byte Delay Time, calculated on startup.         
  uint serialdelay_3B;

  // Four Byte Delay Time, calculated on startup.
  uint serialdelay_4B;


} MOUSE_DATA;

// Extern value, declared again in usb-2-232.c, can be used everywhere ctypes is included.
extern MOUSE_DATA mouse_data;


/* -------------------- Make the file work stuff -------------------- */

#if CTYPES_C_ 

enum MO_SPEED MO_SPEED;
enum MO_TYPE MO_TYPE;
enum PC_INIT_STATES PC_INIT_STATES;
enum MO_MVT_TYPE MO_MVT_TYPE;
enum MO_MVT_COS MO_MVT_COS;
enum CORE_FLAGS CORE_FLAGS;
//PersistentData persistentData;

#else

extern enum MO_SPEED MO_SPEED;
extern enum MO_TYPE MO_TYPE;
extern enum PC_INIT_STATES PC_INIT_STATES;
extern enum MO_MVT_TYPE MO_MVT_TYPE;
extern enum MO_MVT_COS MO_MVT_COS;
extern enum CORE_FLAGS CORE_FLAGS;
//extern PersistentData persistentData;

#endif

#endif // CTYPES_H_