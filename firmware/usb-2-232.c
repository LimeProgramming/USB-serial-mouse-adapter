#include "tusb.h"
#include <stdio.h>
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "pico/multicore.h"

#include "include/utils.h"
#include "include/core_1.h"
#include "include/ctypes.h"
#include "include/serial.h"
#include "include/version.h"
#include "include/hid_app.h"

#include "default_config.h"

/* ---------------------------------------------------------- */
// Mouse Tracking Variables

MOUSE_DATA mouse_data;

// Aggregate movements before sending
CFG_TUSB_MEM_SECTION static hid_mouse_report_t usb_mouse_report_prev;

static absolute_time_t usb_polling_target;


/*---------------------------------------*/
//                  Main                 //
/*---------------------------------------*/
int main(){

    // Mild underclock
    set_sys_clock_khz(125000, true);

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

        // Blink built-in LED forever waiting for user to reboot device
        while(1) {   
          board_led_write(0);   
          sleep_ms(800);
          board_led_write(1); 
          sleep_ms(800);
        }

      } else {
        board_led_write(0);     // Turn off Built in LED
      }
    }
    
    /*---------------------------------------*/
    //              Mouse Options            //
    /*---------------------------------------*/
    
    // Setup Dip Switch Pins
    int dipswpins[6] = { DIPSW_THREEBTN, DIPSW_WHEEL, DIPSW_75XYSPEED, DIPSW_50XYSPEED, DIPSW_7N2, DIPSW_19200 };
    for ( uint8_t i = 0; i < 6; i++ ) { 
        
      init_pinheader(dipswpins[i]);           // init Dip Switch Pins
      
      sleep_us(20000);                        // Wait a bit to avoid false postitive

      // Set up IRQ callbacks on Dip Switch Pins
      gpio_set_irq_enabled_with_callback(dipswpins[i], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &dipswGPIOCallback);
      }

    initPersistentSet();        // try to load persistent settings.
    
    /*---------------------------------------*/
    //              UART STUFF               //
    /*---------------------------------------*/

    /* ----- UART SETUP ----- */
    // ==================================================

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Set the TX pins
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // Set the RX pins

    gpio_init(UART_CTS_PIN);                        // CTS Pin
    gpio_set_dir(UART_CTS_PIN, GPIO_IN);            // CTS Pin direction

    init_serial_uart(7);         // Init our uart
    calcSerialDelay();

    start_core1(0);

    /*----------   TIMERS   ----------*/
    startTerminalTimer();               // Start Our Terminal Timer

    /*---------------------------------------*/
    //               Main Loop               //
    /*---------------------------------------*/
    while(1)
    {  
        /*----------   Blink Built in LED to show the main loop is running   ----------*/
        // Blink the built in LED when in DEBUG Mode
        #if DEBUG > 0
        blink_led_task();
        #endif
        
        /*---------- Core 1 talking to Core 0 ----------*/
        if ( multicore_fifo_rvalid() )
        {
          switch ( multicore_fifo_pop_blocking() )
          {
            case cf_update:
            
              // Update the mouse packet from Core0 so Core1 can pull the correct data
              update_mousepacket();

              // Wait for fifo to be writable
              while ( !multicore_fifo_wready() ) { tight_loop_contents(); } 

              // Reply to Core1
              multicore_fifo_push_blocking( mouse_data.mpkt.update ? cf_post : cf_nopost );

            break;
          }
        }

        /*----------   Serial Terminal   ----------*/
        // Serial State is used to say if the pico should be a serial terminal or a mouse. 
        if ( mouse_data.serial_state == 1  || mouse_data.serial_state == 2 )
        {   
          stop_core1();                   // Stop Core 1 to stop it fecking up the nand flash
          startPWRBlinkerTimer();         // Blink the PWR led 

          // If the UART we got data from is the one we're using for the mouse
          if ( ( mouse_data.serial_state == 1 && UART_ID == uart0 ) || ( mouse_data.serial_state == 2 && UART_ID == uart1 ) ) {
            serial_terminal(UART_ID,  (10000 / (int) ( mouse_data.realbaudrate / 1200 )) );  

          } else {  // Else the user connected a TTL to the unused uart
              serial_terminal(mouse_data.serial_state == 1 ? uart0 : uart1, 1500);  // 10000 / (115200 / 1200 ) ~= 105 
          }

          stopPWRBlinkerTimer();          // Stop Blinking the PWR LED
          gpio_put(LED_PWR, 1);           // Turn on PWR LED
          mouse_data.serial_state = 0;    // Set serial State to mouse mode
          start_core1(0);                 // Resume Core 1
          startTerminalTimer();           // Resume Terminal Timer
        }


        // USB Mouse Polling
        // -----------------------------
        // TinyUSB support USB polling rates up to 1000hz
        // However
        // AVG movement @1000hz feels very very wrong. 
        // Adding these delays for the AVG movement is acting as a sort of handicap on the USB polling rate 

        switch ( mouse_data.persistent.mouse_movt_type )
        {
          case MO_MVT_ADDITIVE: case MO_MVT_COAST:
            tuh_task();
          break;
  
          case MO_MVT_AVERAGE:  
            if ( time_reached(usb_polling_target) ) {
              switch ( mouse_data.persistent.baudrate ) {
                case 19200:
                  usb_polling_target = delayed_by_us( get_absolute_time(), 1000);
                break;
                
                case 9600:
                  usb_polling_target = delayed_by_us( get_absolute_time(), 2000);
                break;

                case 4800:  case 2400:  case 1200:  default:
                  usb_polling_target = delayed_by_us( get_absolute_time(), 4000);
                break;
              }

              // TinyUSB usb host task
              tuh_task(); 
            }

          break;

        }

    }

}
