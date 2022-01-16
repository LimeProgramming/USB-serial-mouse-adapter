#include "tusb.h"
#include <stdio.h>
//#include <stdlib.h>
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/timer.h"

#include "include/utils.h"
#include "include/ctypes.h"
#include "include/serial.h"
#include "include/version.h"
#include "include/hid_app.h"

#include "default_config.h"

/* ---------------------------------------------------------- */
// Mouse Tracking Variables

MOUSE_DATA mouse_data;

/*---------------------------------------*/
//                  Main                 //
/*---------------------------------------*/
int main(){
    stdio_init_all();           // pico SDK
    board_init();               // init board from TinyUSB
    tusb_init();                // init TinyUSB

    /*---------------------------------------*/
    //                 LEDS                  //
    /*---------------------------------------*/

    init_led(LED_PWR);          // Init Power LED
    gpio_put(LED_PWR, 1);       // Turn on Power LED
    init_led(LED_ALERT);        // Init Alert LED

    /*---------------------------------------*/
    //            Reset the flash            //
    /*---------------------------------------*/

    // Init the reset pin
    init_pinheader(RESET_FLASH);  
    sleep_us(500);         // wait a bit to avoid false postitive

    // If the reset pin is closed on startup
    if ( !gpio_get(RESET_FLASH) ) {   

        board_led_write(1);     // Turn on Built in LED for feedback
        sleep_ms(3000);         // Wait for 3 seconds.
        
        // IF reset pin is still held
        if ( !gpio_get(RESET_FLASH) ) {   
            
            loadPersistentSetDefaults();    // Load the default Settings
            savePersistentSet();            // Save the default settings

            // Loop forever waiting for user to reboot device
            while(1) {   
                blink_led_task();           // Blink the built in LED
            }
        } else {
            board_led_write(0);     // Turn off Built in LED
        }
    }
    
    /*---------------------------------------*/
    //              Mouse Options            //
    /*---------------------------------------*/
    
    // Setup Dip Switch Pins
    int dipswpins[6] = { DIPSW_THREEBTN, DIPSW_WHEEL, DIPSW_75XYSPEED, DIPSW_50XYSPEED, DIPSW_7N2, DIPSW_2400 };
    for ( uint8_t i = 0; i < 6; i++ ) { 
        
        init_pinheader(dipswpins[i]);           // init Dip Switch Pins
        
        sleep_us(20000);                        // wait a bit to avoid false postitive

        // Set up IRQ callbacks on Dip Switch Pins
        gpio_set_irq_enabled_with_callback(dipswpins[i], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &dipswGPIOCallback);
        }

    initPersistentSet();        // try to load persistent settings.
    
    /*---------------------------------------*/
    //              UART STUFF               //
    /*---------------------------------------*/


    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Set the TX pins
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // Set the RX pins

    gpio_init(UART_CTS_PIN);                    // CTS Pin
    gpio_set_dir(UART_CTS_PIN, GPIO_IN);        // CTS Pin direction

    init_serial_uart();         // Init our uart

    /*----------   PACKET DELAYS   ----------*/
    
    calcSerialDelay();

    mouse_data.intro_pkts[0] = 0x4D;    // M
    mouse_data.intro_pkts[1] = 0x33;    // 3
    mouse_data.intro_pkts[2] = 0x5A;    // Z
    mouse_data.mouse_conn = false;

    mouse_data.serial_state = 0;
    
    /*----------   TIMERS   ----------*/
    startTerminalTimer();             // Start Our Terminal Timer
    startMouseTimer();                // Start Our Mouse Timer

    /*---------------------------------------*/
    //               Main Loop               //
    /*---------------------------------------*/
    while(1)
    {   
        /*----------   Serial Terminal   ----------*/
        // Serial State is used to say if the pico should be a serial terminal or a mouse. 
        if ( mouse_data.serial_state != 0 )
        {   
            // Stop Mouse Timer
            stopMouseTimer();               
            
            // If the UART we got data from is the one we're using for the mouse
            if ( ( mouse_data.serial_state == 1 && UART_ID == uart0 ) || ( mouse_data.serial_state == 2 && UART_ID == uart1 ) ) {

                // 10ms baseline for 1200 baud
                uint64_t delay = (10000 / (int) ( mouse_data.persistent.baudrate / 1200 ));
                serial_terminal(UART_ID, delay);  

            } else {  // Else the user connected a TTL to the unused uart
                // 10000 / (115200 / 1200 ) ~= 105 
                serial_terminal(mouse_data.serial_state == 1 ? uart0 : uart1, 1500);  
            }

            mouse_data.serial_state = 0;    // Set serial State to mouse mode
            startMouseTimer();              // Resume Mouse Timer
            startTerminalTimer();           // Resume Terminal Timer
        }

        /*----------   Blink Built in LED to show the main loop is running   ----------*/
        // Blink the built in LED when in DEBUG Mode
        if ( DEBUG ) { blink_led_task(); } 


        /*----------   Main Mouse Part of the mouse  ----------*/
        bool cts_pin = gpio_get(UART_CTS_PIN);
        
        // Check if mouse driver trying to initialize
        // Computers RTS is low, with MAX3232 this shows reversed as high instead? Check spec. <-- Thanks Aviancer
        if(cts_pin) { 
            /* ----- LOGITECH DRIVERS COMBATIBILITY -----
            Sometimes the PC sporadically pulls the CTS pin low when loads games with Logitech drivers.
            So to avoid spamming the serial port, we'll wait the time to required to send 4 packets; if the serial mouse was already connected.
            */

            if ( mouse_data.pc_state != CTS_TOGGLED ) {
                mouse_data.pc_state = CTS_LOW_INIT;
            } else {
                busy_wait_us_32(mouse_data.serialdelay_4B);

                bool cts_pin = gpio_get(UART_CTS_PIN);
                
                if ( cts_pin ) { mouse_data.pc_state = CTS_LOW_INIT; }
            }
        }

        // Mouse initializing request detected
        if( !cts_pin && mouse_data.pc_state == CTS_LOW_INIT ) {
            serialMouseNego();
            mouse_data.pc_state = CTS_TOGGLED;
        }

        if ( !cts_pin && mouse_data.pc_state == CTS_UNINIT) {
            /* ----- CTMOUSE DRIVERS COMBATIBILITY -----
            Unlike the Logitech drivers, the CTMouse driver doesn't seem to pull CTS low when it looses connection with the mouse. 
            If the USB mouse is replaced or lost connection for a moment:
                - the pico reboots because TinyUSB panics
                - CTMouse driver the loop should fall here to resume mouse functionality
                - Logitech driver should fall in the top if to resume mouse functionality. */

            mouse_data.pc_state = CTS_LOW_INIT;
        }

        /*** Mouse update loop ***/
        if( mouse_data.pc_state == CTS_TOGGLED || DEBUG ) {
            tuh_task(); // tinyusb host task
        }

        // we fall in here for the thing (yank out serial cable )
        if( (mouse_data.pc_state != CTS_TOGGLED && mouse_data.mouse_conn) )
        {   
            mouse_data.mouse_conn = false;

            //if ( DEBUG ) { printf("Serial Connection not found"); }

            // Call ALRT Code flasher for Serial Connection Lost. 
            // TODO

        }   


    }


}
