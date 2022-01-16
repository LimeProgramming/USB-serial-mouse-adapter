#include <stdbool.h>

/*---------------------------------------*/
//                 DEBUG                 //
/*---------------------------------------*/

// All this really does is give printf's out via UART1
#define DEBUG false          // Debug Flag for things


/*---------------------------------------*/
//                GPIO PINS              //
/*---------------------------------------*/

// DIP Switch Pins
#define DIPSW_THREEBTN 9        // Dip Switch 1 | LOGITECH
#define DIPSW_WHEEL 10          // Dip Switch 2 | MS Wheel
#define DIPSW_75XYSPEED 11      // Dip Switch 3 | 75% speed | Dip 3 + 4 depressed will set mouse speed to 25%
#define DIPSW_50XYSPEED 12      // Dip Switch 4 | 50% speed | Dip 3 + 4 depressed will set mouse speed to 25%
#define DIPSW_7N2 13            // Dip Switch 5 | 7N2 format
#define DIPSW_2400 14           // Dip Switch 6 | Reserved

// LEDS 
#define LED_PWR 2
#define LED_ALERT 3

//UART stuff
#define UART_ID uart1       // Use UART1, keep UART0 for DEBUG printing
#define UART_TX_PIN 4       // UART1 TX --> MAX232 pin 11
#define UART_RX_PIN 5       // UART1 RX --> MAX232 pin 12
#define UART_CTS_PIN 6      // CTS      --> MAX232 pin 9
#define UART_RTS_PIN 7      // RTS      --> MAX232 pin 10


#define RESET_FLASH 17      // Button for resetting the stored settings back to default, defined below.


/*---------------------------------------*/
//            Settings Defaults          //
/*---------------------------------------*/
// These are used until they are changed via software for dipswitches

// global limit | Range 1 -> 200
static uint8_t default_xytravel_percentage = 100;

// seperate x and y limits. | Range 1 -> 200
static uint8_t default_xtravel_percentage = 100;
static uint8_t default_ytravel_percentage = 100;

// TWOBTN = 0 | THREEBTN = 1 | WHEELBTN = 2 
static int8_t default_mousetype = 0;

// Double Stop Bit
// 7N1 = false | 7N2 = true
static bool default_doublestopbit = false;

// Baud Rate
/* ---------- Acceptible baud rates ----------
            1200 | 2400 | 4800 | 9600
            Recommended value is 1200 

Any values outside of the ones above, are rejected
and the default value of 1200 is used.

Note:
As far as I can see from docs, serial mice ran at
1200 Baud, so any value higher then that may result in issues.
Max value is 9600 because of hardware limits of the 
max232 

   ------------------------------------------ */

static uint16_t default_baudrate = 1200;

// Swap left and right mouse buttons
// Could be handy for left handed people.
static bool default_swap_left_right = false;

// Uses the forward and back buttons of a mouse as alturnative left and right mouse buttons. 
// The forward and back butons are on the side of the mouse and usually act as the page forward and backwards in file and web browsers. 
// Forward button -> left click | Back Button -> right click
static bool default_use_forward_backward = false;

// Changes above to 
// Forward button -> right click | Back Button -> left click
static bool default_swap_forward_backward = false;

// Invert X axis (Left and Right movement)
static bool default_invert_x = false;

// Invert Y axis (Up and Down movement)
static bool default_invert_y = false;


/*---------------------------------------*/
//            Advanced Settings          //
/*---------------------------------------*/


/* ---------------------------------------------------------- */
/*  Numbers for Serial Speed and Delay times
    - Thanks to Aviancer for the numbers | https://github.com/Aviancer/amouse

    1200 baud (bits/s) is 133.333333333... bytes/s
    44.44.. updates per second with 3 bytes.
    33.25.. updates per second with 4 bytes.
    ~0.0075 seconds per byte, target time calculated for 4 bytes.

    SERIALDELAY = (((7500 / (BAUD_RATE / 1200)) * NumberOfBytes) + TXWIGGLEROOM )

    SerialDelay is the time between when serial packets are sent, based on the math above */
/* ---------------------------------------------------------- */

// This is for adding a bit of padding to the serial delays, shouldn't be needed but some controllers are awkward.
static uint32_t TXWIGGLEROOM    = 0; 
