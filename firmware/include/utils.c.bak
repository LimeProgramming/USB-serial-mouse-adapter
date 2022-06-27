#include "tusb.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/flash.h"
#include "pico/binary_info.h"
#include "hardware/watchdog.h"

#include "utils.h"
#include "core_1.h"
#include "ctypes.h"
#include "serial.h"
#include "version.h"
#include <default_config.h>


// ==================================================================================================== //
//                                                UTILS                                                 //
// ==================================================================================================== //


int constraini(int16_t value, int16_t min, int16_t max) {
  if(value > max) { return max; }
  if(value < min) { return min; }
  return value;
}

uint8_t constrainui(uint8_t value, uint8_t min, uint8_t max) {
  if(value > max) { return max; }
  if(value < min) { return min; }
  return value;
}

// Configure GPIO for use as Pin Headers
void init_pinheader(uint pin)
{
  gpio_init(pin); 
  gpio_set_dir(pin, GPIO_IN);
  gpio_pull_up(pin);
}

// Configure GPIO to Power an LED
void init_led(uint pin)
{
  gpio_init(pin); 
  gpio_set_dir(pin, GPIO_OUT);
}

//I guess this will reboot the pico?
void machine_reboot()
{
  watchdog_reboot(0, SRAM_END, 0);
  for (;;) { __wfi(); }
  return;
}

// Blink the alert LED
void blink_aled(uint gpio, uint8_t code)
{
  bool pre_state = gpio_get(gpio);
  uint8_t errcodes[]= {1, 0, 1, 0, 1, 0, 1, 0, 1};

  gpio_put(gpio, 0);

  for ( uint8_t i = 0 ; i< sizeof(errcodes); i++ )
  {
    gpio_put(gpio, errcodes[i]);
    sleep_ms(100);
  }

  gpio_put(gpio, pre_state);
  
}



// ==================================================================================================== //
//                                          Repeating Timers                                            //
// ==================================================================================================== //


// Terminal checker timer
// --------------------------------------------------
//
// We're using a timer for the terminal since uart irq is buggy.
// Returning false from a repeating timer callback stops the timer. 

struct repeating_timer terminal_timer;

bool terminal_timer_callback(struct repeating_timer *t) {

  // Just a bit of safety
  if ( mouse_data.serial_state != 0 ) { return false; }   // Stop the timer

  // ----- Variables
  // Buffer for inputs from serial port (needs to be an array)
  uint8_t uart_data[2] = {0};                      

  // ----- Check UART0
  if( term_getc(uart0, uart_data, 2) > 0) {
      if( uart_data[0] == '\r'|| uart_data[0] == '\n' ) {
          mouse_data.serial_state = 1;                    // Set our Flag
          return false;                                   // Stop the timer
      }
  }
  
  // ----- Nuke UART DATA
  memset(uart_data, 0, 2);           
  
  // ----- Check UART1
  if( term_getc(uart1, uart_data, 2) > 0) {  
    printf("%d | %d\n", uart_data[0], uart_data[1]);

      if( uart_data[0] == '\r' || uart_data[0] == '\n' ) {
          mouse_data.serial_state = 2;                    // Set our Flag
          return false;                                   // Stop the timer
      }
  }

  return true;                                            // Keep running the timer
}

// Create 0.5 second timer for checking for terminal input
void startTerminalTimer() {                               
  add_repeating_timer_us(500000, terminal_timer_callback, NULL, &terminal_timer);
  return;
}

// Stop Our terminal timer
void stopTerminalTimer() {                                
  cancel_repeating_timer(&terminal_timer);    
  return;
}

// Terminal PWR LED Blinker
// --------------------------------------------------
//
// Timer to blink the PWR led to tell the user that the adapter is in serial terminal mode

struct repeating_timer PWR_blinker_timer;

bool PWR_blinker_timer_callback(struct repeating_timer *t) {
  // if PWR LED is on 
  if ( gpio_get(LED_PWR) ) {
    gpio_put(LED_PWR, 0);
  
  // Else it is off
  } else {
    gpio_put(LED_PWR, 1);
  }

  return true;
  
}

void startPWRBlinkerTimer() {                               
  add_repeating_timer_us(800000, PWR_blinker_timer_callback, NULL, &PWR_blinker_timer);
  return;
}

// Stop Our PWR LED blicker timer
void stopPWRBlinkerTimer() {                                
  cancel_repeating_timer(&PWR_blinker_timer);    
  return;
}

// ==================================================================================================== //
//                                         Mouse Settings Stuff                                         //
// ==================================================================================================== //


void calcSerialDelay(){

  //200 baud = 50.000 ms @ 8n1 

  // Works for bits per microsecond
  mouse_data.serialdelay_1b = ( 50000 / ( mouse_data.realbaudrate / 200.0 ) ) / 9;

  float tmp = 0.0;

  // Work out the time for 1, 3, 4 mouse packet(s)
  tmp = (mouse_data.serialdelay_1b * (mouse_data.persistent.doublestopbit ? 9 : 8) ) + TXWIGGLEROOM;
  tmp = ceilf(tmp);
  mouse_data.serialdelay_1B = (uint) tmp;
  mouse_data.serialdelay_3B =  mouse_data.serialdelay_1B * 3;
  mouse_data.serialdelay_4B =  mouse_data.serialdelay_1B * 4;

  #if DEBUG > 0
  printf("1 Bit: %4.2f | 1 Byte: %d | 3 Byte: %d | 4 Byte: %d\n" , mouse_data.serialdelay_1b, mouse_data.serialdelay_1B, mouse_data.serialdelay_3B, mouse_data.serialdelay_4B);
  #endif

  return;
}

// Set Serial Baid rate from the headers
void setDipSerialBaud() {
  if ( !gpio_get(DIPSW_19200) )  { mouse_data.persistent.baudrate = 19200; }
  else                           { mouse_data.persistent.baudrate = 1200; }

  return;
}

// Sets Mouse Speed from the headers.
void setDipMouseType() {   
  if      ( !gpio_get(DIPSW_WHEEL) )     { mouse_data.persistent.mousetype = WHEELBTN; }
  else if ( !gpio_get(DIPSW_THREEBTN) )  { mouse_data.persistent.mousetype = THREEBTN; }
  else                                   { mouse_data.persistent.mousetype = TWOBTN; }

  return;
}

// Updates Mouse Travel Rate Divider from the headers.
void setDipMouseSpeed()
{   
  if      ( !gpio_get(DIPSW_75XYSPEED) && !gpio_get(DIPSW_50XYSPEED) )    { mouse_data.persistent.xytravel_percentage = 25; }
  else if ( !gpio_get(DIPSW_50XYSPEED) )                                  { mouse_data.persistent.xytravel_percentage = 50; }
  else if ( !gpio_get(DIPSW_75XYSPEED) )                                  { mouse_data.persistent.xytravel_percentage = 75; }
  else                                                                    { mouse_data.persistent.xytravel_percentage = 100; }

  return;
}



// ==================================================================================================== //
//                                          Dip Switch IRQ                                              //
// ==================================================================================================== //


/*
I spent a bit of time trying to figure out the simplest way to have mouse settings updatable without restarting the pi pico.
I could have just called the above functions in the main loop all the time but that felt wasteful.
Debounce in a gpio_callback doesn't work well, give it a Google, some fellow tried it and it was spotty.
I implemented a hardware debounce but decided against it, since it made the final PCB bigger for something not integral. 

Ultimately I decided it was best to add an alarm from a GPIO callback to act as a poor mans debounce.
*/

volatile bool DIPSW_MOUSE_IRQ = false;
volatile bool DIPSW_XYSPEED_IRQ = false;
volatile bool DIPSW_7N2_IRQ = false;
volatile bool DIPSW_19200_IRQ = false;

int64_t mouse_type_callback(alarm_id_t id, void *user_data) {  

  #if DEBUG > 0 
    printf("Mouse Type Callback called\n"); 
  #endif

  // Exit if the mouse is acting as a terminal
  if ( mouse_data.serial_state != 0 ) { return 0; }

  mouse_data.serial_state = 3;          // Stop the serial mouse timer 
  stop_core1();                         // Stop Core 1
  setDipMouseType();                    // Set Mouse Type from headers
  updateStoredDipswitchs();             // Get Dip Switch state
  savePersistentSet();                  // Save Updated Settings
  calcSerialDelay();                    // Update Serial Timings
  DIPSW_MOUSE_IRQ = false;              // Flag that we are done with this func
  mouse_data.serial_state = 0;          // Start the serial mouse timer 
  start_core1(0);                       // Start Core 1
  
  return 0;                                   
}

int64_t mouse_speed_callback(alarm_id_t id, void *user_data) {   

  #if DEBUG > 0 
    printf("Mouse Speed Callback called\n"); 
  #endif
  
    // Exit if the mouse is acting as a terminal
  if ( mouse_data.serial_state != 0 ) { return 0; }

  mouse_data.serial_state = 3;          // Stop the serial mouse timer 
  stop_core1();                         // Stop Core 1
  setDipMouseSpeed();                   // Get mouse speed from the headers
  updateStoredDipswitchs();             // Get Dip Switch state
  savePersistentSet();                  // Save persistent settings       
  DIPSW_XYSPEED_IRQ = false;            // Flag that we are done with this func
  mouse_data.serial_state = 0;          // Start the serial mouse timer 
  start_core1(0);                        // Start Core 1

  return 0;                                   
}

int64_t serial_format_callback(alarm_id_t id, void *user_data) {   

  #if DEBUG > 0 
    printf("Serial Format Callback called\n");
  #endif

  // Exit if the mouse is acting as a terminal
  if ( mouse_data.serial_state != 0 ) { return 0; }

  mouse_data.serial_state = 3;          // Stop the serial mouse timer 
  stop_core1();                         // Stop Core 1
  mouse_data.persistent.doublestopbit = !gpio_get(DIPSW_7N2);     // get the header state
  refresh_serial_uart();                // Refresh the uart with updated settings
  calcSerialDelay();
  updateStoredDipswitchs();             // Store header state
  savePersistentSet();                  // save persistent
  DIPSW_7N2_IRQ = false;                // Flag that the func is done
  mouse_data.serial_state = 0;          // Start the serial mouse timer 
  start_core1(0);                        // Start Core 1
  
  return 0;                                   
}

int64_t serial_speed_callback(alarm_id_t id, void *user_data) {  

  #if DEBUG > 0 
    printf("Serial Speed Callback called\n");
  #endif

  // Exit if the mouse is acting as a terminal
  if ( mouse_data.serial_state != 0 ) { return 0; }

  mouse_data.serial_state = 3;          // Stop the serial mouse timer 
  stop_core1();                         // Stop Core 1
  setDipSerialBaud();                   // Set new Baud rate based on dip switches
  refresh_serial_uart();                // Reinit serial with updated settings
  calcSerialDelay();                    // Recalulate serial delay used by the main mouse timer
  updateStoredDipswitchs();             // Poll all dip switches for state
  savePersistentSet();                  // Store all settings
  mouse_data.serial_state = 0;          // Start the serial mouse timer 
  DIPSW_19200_IRQ = false;              // Flag that this func is finished
  start_core1(0);                        // Start Core 1

  return 0;                                   
}

// Baically this is acting as a poor mans debounce on the dip switches
void dipswGPIOCallback(uint gpio, uint32_t events) {
  gpio_acknowledge_irq(gpio, events);         // ACK GPIO IRQ

  switch ( gpio ) {

    case DIPSW_THREEBTN: case DIPSW_WHEEL:

      if ( DIPSW_MOUSE_IRQ ) { break; }
      
      DIPSW_MOUSE_IRQ = true;
      add_alarm_in_ms(1500, mouse_type_callback, NULL, true);

    break;
    case DIPSW_75XYSPEED: case DIPSW_50XYSPEED: // 50% speed | 75% speed  | Dip 3 + 4 depressed will set mouse speed to 25%

      if ( DIPSW_XYSPEED_IRQ ) { break; }

      DIPSW_XYSPEED_IRQ = true; 
      add_alarm_in_ms(1500, mouse_speed_callback, NULL, true); 

    break;
    case DIPSW_7N2:

      if ( DIPSW_7N2_IRQ ) { break; }

      DIPSW_7N2_IRQ = true;
      add_alarm_in_ms(1500, serial_format_callback, NULL, true);

    break;
    case DIPSW_19200:
      
      if ( DIPSW_19200_IRQ ) { break; }

      DIPSW_19200_IRQ = true;
      add_alarm_in_ms(1500, serial_speed_callback, NULL, true); 

    break;
  }
}



// ==================================================================================================== //
//                                         Persistent Settings                                          //
// ==================================================================================================== //


#define FLASH_TARGET_OFFSET (384 * 1024)
#define FLASH_SECTOR_SIZE_C (1u << 16) // <-- Custom Sector Size

// Load persistent data from flash | Load defaults if no data in flash found
// There's a pile of if's to try to combat silly users.
void loadPersistentSetDefaults() {
  mouse_data.persistent.firstrun = false;
  
  // Assume good because I set them.
  mouse_data.persistent.FW_V_MAJOR = V_MAJOR;
  mouse_data.persistent.FW_V_MINOR = V_MINOR;
  mouse_data.persistent.FW_V_REVISION = V_REVISION;

  // Simple Contrains
  mouse_data.persistent.xytravel_percentage =  constrainui(default_xytravel_percentage, 1, 200); //int8_t min, int8_t max) 
  mouse_data.persistent.xtravel_percentage =   constrainui(default_xtravel_percentage,  1, 200);
  mouse_data.persistent.ytravel_percentage =   constrainui(default_ytravel_percentage,  1, 200);

  // DipSwitches 
  // 1 = open | 0 = closed
  mouse_data.persistent.ST_DIPSW_THREEBTN = 1;
  mouse_data.persistent.ST_DIPSW_WHEEL = 1;
  mouse_data.persistent.ST_DIPSW_75XYSPEED = 1;
  mouse_data.persistent.ST_DIPSW_50XYSPEED = 1;
  mouse_data.persistent.ST_DIPSW_7N2 = 1;
  mouse_data.persistent.ST_DIPSW_19200 = 1;

  // Added version 1.1.X
  mouse_data.persistent.mouse_movt_type = default_mouse_movt_type;
  mouse_data.persistent.use_cosine_smoothing = default_use_cosine_smoothing;

  // Mouse type range
  switch ( default_mousetype )
  {
  case 0: case 1: case 2:
    mouse_data.persistent.mousetype = default_mousetype;
    break;

  default:
    mouse_data.persistent.mousetype = 0;
  }

  // Baud Rate range 
  switch ( default_baudrate ) 
  {
    case 1200: case 2400: case 4800: case 9600: case 19200:
      mouse_data.persistent.baudrate =      default_baudrate;
      break;
    default:
      mouse_data.persistent.baudrate =      1200;
      break;
  }

  // Double Stop Bit range
  if ( default_doublestopbit == 1 )         { mouse_data.persistent.doublestopbit = true; }
  else                                      { mouse_data.persistent.doublestopbit = false; }

  // Swap the left and right buttons 
  if ( default_swap_left_right == 1 )       { mouse_data.persistent.swap_left_right = true; }
  else                                      { mouse_data.persistent.swap_left_right = false; }

  // use forward and backward as ALT left and right buttons
  if ( default_use_forward_backward == 1 )  { mouse_data.persistent.use_forward_backward = true; }
  else                                      { mouse_data.persistent.use_forward_backward = false; }

  // Swap forward and backwards
  if ( default_swap_forward_backward == 1 ) { mouse_data.persistent.swap_forward_backward = true; }
  else                                      { mouse_data.persistent.swap_forward_backward = false; }
  
  // Invert X axis (Left and Right movement)
  if ( default_invert_x == 1 ) { mouse_data.persistent.invert_x = true; }
  else                         { mouse_data.persistent.invert_x = false; }
  
  // Invert Y axis (Up and Down movement)
  if ( default_invert_y == 1 ) { mouse_data.persistent.invert_y = true; }
  else                         { mouse_data.persistent.invert_y = false; }
  
  // Added 1.2.X Language
  if ( (default_language == 0) || (default_language == 1) ) { mouse_data.persistent.language = default_language; }
  else                                                      { mouse_data.persistent.language = 0; }

}

// Run on launch 
void initPersistentSet() {
  
  // If settings need to be saved
  bool saveset = false;

  // Load current settings from flash. A read from blank returns an array of bytes set to "255".
  loadPersistentSet();

  /* ----- First Run or FW Required Reload ----- */
  // ==================================================
  // IF First run or revision number higher than stored revision number, load default
  if ( mouse_data.persistent.firstrun == 255 || V_REVISION > mouse_data.persistent.FW_V_REVISION  ) {

    #if DEBUG > 0

    printf("fristrun: %d\n", mouse_data.persistent.firstrun ); 
    printf("revision: %d\n", mouse_data.persistent.FW_V_REVISION);

    #endif
    
    loadPersistentSetDefaults();  // Load the default settings from default_config.h
    savePersistentSet();          // Save new persistent Data
    machine_reboot();             // Reboot Pico
  } 
  
  /* ----- If The Firmware Version Has Changed ----- */
  // ==================================================
  // Just store the new verion information
  if ( (V_MAJOR > mouse_data.persistent.FW_V_MAJOR || V_MINOR > mouse_data.persistent.FW_V_MINOR ) && saveset == false ){
    mouse_data.persistent.FW_V_MAJOR = V_MAJOR;
    mouse_data.persistent.FW_V_MINOR = V_MINOR;

    saveset = true;
  }

  /* ----- DIPSWITCHES----- */
  // ==================================================
  // Use Dipswitch data if dipswitch is closed and it doesn't match the stored data

  // If stored dipswitch state does not match current dipswitch state.
  if ( ( mouse_data.persistent.ST_DIPSW_THREEBTN != gpio_get(DIPSW_THREEBTN) ) || ( mouse_data.persistent.ST_DIPSW_WHEEL != gpio_get(DIPSW_WHEEL) ) ) {
    setDipMouseType();
    mouse_data.persistent.ST_DIPSW_THREEBTN = gpio_get(DIPSW_THREEBTN);
    mouse_data.persistent.ST_DIPSW_WHEEL = gpio_get(DIPSW_WHEEL);
    saveset = true;
    }

  // Mouse xytracking speed Dip Switches
  if ( ( mouse_data.persistent.ST_DIPSW_75XYSPEED != gpio_get(DIPSW_75XYSPEED) ) || ( mouse_data.persistent.ST_DIPSW_50XYSPEED != gpio_get(DIPSW_50XYSPEED) ) ) {
    setDipMouseSpeed();
    mouse_data.persistent.ST_DIPSW_75XYSPEED = gpio_get(DIPSW_75XYSPEED);
    mouse_data.persistent.ST_DIPSW_50XYSPEED = gpio_get(DIPSW_50XYSPEED);
    saveset = true;
    }
  
  // Double stop bit Dip Switch
  if ( mouse_data.persistent.ST_DIPSW_7N2 != gpio_get(DIPSW_7N2) ) {
    mouse_data.persistent.doublestopbit = !gpio_get(DIPSW_7N2);
    mouse_data.persistent.ST_DIPSW_7N2 = gpio_get(DIPSW_7N2);
    saveset = true;
  } 

  // Baud Rate Dip Switch
  if ( mouse_data.persistent.ST_DIPSW_19200 != gpio_get(DIPSW_19200) ) {
    if ( !gpio_get(DIPSW_19200) )   { mouse_data.persistent.baudrate = 19200; }
    else                            { mouse_data.persistent.baudrate = 1200; }
    mouse_data.persistent.ST_DIPSW_19200 = gpio_get(DIPSW_19200);
    saveset = true;
  } 


  /* ----- Save Any Changes----- */
  // ==================================================
  if ( saveset ) { savePersistentSet(); }


  /* ----- Handy DEBUG printout----- */
  // ==================================================
  #if DEBUG > 0
    printf("Mouse Settings print out\n");

    sleep_ms(5);

    printf("FW_V_MAJOR: %d | FW_V_MINOR: %d | FW_V_REVISION: %d | FW_Language: %d\n",
      mouse_data.persistent.FW_V_MAJOR,
      mouse_data.persistent.FW_V_MINOR,
      mouse_data.persistent.FW_V_REVISION,
      mouse_data.persistent.language
    );

    sleep_ms(5);

    printf("xytravel_percentage: %d | xtravel_percentage: %d | ytravel_percentage: %d\n",
      mouse_data.persistent.xytravel_percentage,
      mouse_data.persistent.xtravel_percentage,
      mouse_data.persistent.ytravel_percentage
    );

    sleep_ms(5);

    printf("mouse_movt_type: %d | use_cosine_smoothing: %d\n",
      mouse_data.persistent.mouse_movt_type,
      mouse_data.persistent.use_cosine_smoothing
    );

    sleep_ms(5);

    printf("mousetype: %d | doublestopbit: %d | baudrate: %d\n",
      mouse_data.persistent.mousetype,
      mouse_data.persistent.doublestopbit,
      mouse_data.persistent.baudrate
    );

    sleep_ms(5);

    printf("swap_left_right: %d | use_forward_backward: %d | swap_forward_backward: %d\n",
      mouse_data.persistent.swap_left_right,
      mouse_data.persistent.use_forward_backward,
      mouse_data.persistent.swap_forward_backward
    );

    sleep_ms(5);

    printf("Invert X: %d | Invert Y: %d\n",
      mouse_data.persistent.invert_x,
      mouse_data.persistent.invert_y
    );
  #endif
}


/*
* Some kind of wear leveling for the pico's flash memeory
* The only time this reports 0 is for the first time run
* If this reports 16, then flash needs to be erased
*/
ushort findEmptyPage() {
  const uint8_t* flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

  for ( ushort i = 0 ; i <= FLASH_SECTOR_SIZE_C/FLASH_PAGE_SIZE ; i++ ) {
    // If the curent page being looked at is blank, this is the first run var
    if ( flash_target_contents[ (i*FLASH_PAGE_SIZE) ] == 255 ) {
      return i;
    }
  }
}

void savePersistentSet() {

  // declare the variables we need
  uint32_t ints;
  uint8_t buffer[FLASH_PAGE_SIZE];

  // Write persistend data to buffer array
  buffer[0] = mouse_data.persistent.firstrun;
  buffer[1] = mouse_data.persistent.FW_V_MAJOR;
  buffer[2] = mouse_data.persistent.FW_V_MINOR;
  buffer[3] = mouse_data.persistent.FW_V_REVISION;

  buffer[4] = mouse_data.persistent.xytravel_percentage;
  buffer[5] = mouse_data.persistent.xtravel_percentage;
  buffer[6] = mouse_data.persistent.ytravel_percentage;
  buffer[7] = mouse_data.persistent.mousetype;
  buffer[8] = mouse_data.persistent.doublestopbit;
  buffer[9] = (uint8_t) ( mouse_data.persistent.baudrate / 1200 );
  
  buffer[10] = mouse_data.persistent.swap_left_right;
  buffer[11] = mouse_data.persistent.use_forward_backward;
  buffer[12] = mouse_data.persistent.swap_forward_backward;
  buffer[13] = mouse_data.persistent.invert_x;
  buffer[14] = mouse_data.persistent.invert_y;

  buffer[15] = mouse_data.persistent.ST_DIPSW_THREEBTN;
  buffer[16] = mouse_data.persistent.ST_DIPSW_WHEEL;
  buffer[17] = mouse_data.persistent.ST_DIPSW_75XYSPEED;
  buffer[18] = mouse_data.persistent.ST_DIPSW_50XYSPEED;
  buffer[19] = mouse_data.persistent.ST_DIPSW_7N2;
  buffer[20] = mouse_data.persistent.ST_DIPSW_19200;

  // Added version 1.1.X
  buffer[21] = mouse_data.persistent.mouse_movt_type;
  buffer[22] = mouse_data.persistent.use_cosine_smoothing;

  // Added Version 1.2.X
  buffer[23] = mouse_data.persistent.language;

  // Halt all interrupts to avoid errors
  ints = save_and_disable_interrupts();                                  
  
  ushort pagenum = findEmptyPage();

  switch ( pagenum ) {
    case FLASH_SECTOR_SIZE_C/FLASH_PAGE_SIZE:  // Blank area if it's full
      pagenum = 0;
    case 0: // Prime area for first use.
      flash_range_erase( FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE_C );

      #if DEBUG > 0
      printf("Flash erased");
      #endif

    break;
  }

  // Program Target Area
  flash_range_program(FLASH_TARGET_OFFSET + (pagenum * FLASH_PAGE_SIZE), buffer, FLASH_PAGE_SIZE );

  // Restore previouslt halted interrupts
  restore_interrupts(ints);

  #if DEBUG > 0
  printf("Page Written");
  #endif

  return;
}

void loadPersistentSet() {
  const uint8_t* flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

  ushort i = 0;

  for ( ushort i = 0 ; i <= FLASH_SECTOR_SIZE_C/FLASH_PAGE_SIZE ; i++ ) {
    if ( flash_target_contents[ (i*FLASH_PAGE_SIZE) ] == 255 ) { break; }
  }

  if ( i != 0 ) { i--; i = i*256; }

  mouse_data.persistent.firstrun = flash_target_contents[i];
  mouse_data.persistent.FW_V_MAJOR = flash_target_contents[++i];
  mouse_data.persistent.FW_V_MINOR = flash_target_contents[++i];
  mouse_data.persistent.FW_V_REVISION = flash_target_contents[++i];

  mouse_data.persistent.xytravel_percentage = flash_target_contents[++i];
  mouse_data.persistent.xtravel_percentage = flash_target_contents[++i];
  mouse_data.persistent.ytravel_percentage = flash_target_contents[++i];
  mouse_data.persistent.mousetype = flash_target_contents[++i];
  mouse_data.persistent.doublestopbit = flash_target_contents[++i];

  mouse_data.persistent.baudrate = (uint) ( flash_target_contents[++i] * 1200 );

  mouse_data.persistent.swap_left_right = flash_target_contents[++i];
  mouse_data.persistent.use_forward_backward = flash_target_contents[++i];
  mouse_data.persistent.swap_forward_backward = flash_target_contents[++i];
  mouse_data.persistent.invert_x = flash_target_contents[++i];
  mouse_data.persistent.invert_y = flash_target_contents[++i];

  mouse_data.persistent.ST_DIPSW_THREEBTN = flash_target_contents[++i];
  mouse_data.persistent.ST_DIPSW_WHEEL = flash_target_contents[++i];
  mouse_data.persistent.ST_DIPSW_75XYSPEED = flash_target_contents[++i];
  mouse_data.persistent.ST_DIPSW_50XYSPEED = flash_target_contents[++i];
  mouse_data.persistent.ST_DIPSW_7N2 = flash_target_contents[++i];
  mouse_data.persistent.ST_DIPSW_19200 = flash_target_contents[++i];

  // Added version 1.1.X
  mouse_data.persistent.mouse_movt_type = flash_target_contents[++i];
  mouse_data.persistent.use_cosine_smoothing = flash_target_contents[++i]; 

  // Added version 1.2.X
  mouse_data.persistent.language = flash_target_contents[++i];
  
  return;
}

void updateStoredDipswitchs() {
  mouse_data.persistent.ST_DIPSW_THREEBTN =   gpio_get(DIPSW_WHEEL);
  mouse_data.persistent.ST_DIPSW_WHEEL =      gpio_get(DIPSW_THREEBTN);
  mouse_data.persistent.ST_DIPSW_75XYSPEED =  gpio_get(DIPSW_75XYSPEED);
  mouse_data.persistent.ST_DIPSW_50XYSPEED =  gpio_get(DIPSW_50XYSPEED);
  mouse_data.persistent.ST_DIPSW_7N2 =        gpio_get(DIPSW_7N2);
  mouse_data.persistent.ST_DIPSW_19200 =      gpio_get(DIPSW_19200);
}



// ==================================================================================================== //
//                                              DEBUG Tools                                             //
// ==================================================================================================== //


// I use this as a visual que that the pico is actually running the main loop.
void blink_led_task(void)
{
	const uint32_t interval_ms = 1000;
	static uint32_t start_ms = 0;
	static bool led_state = false;
	if(board_millis() - start_ms < interval_ms) {
		return;
	}
	start_ms += interval_ms;
	board_led_write(led_state);
	led_state = !led_state;
}



// ==================================================================================================== //
//                                             Travel Limits                                            //
// ==================================================================================================== //


int16_t travel_limit(int16_t val, uint8_t percentage, uint16_t constainval)
{
  // Do that first to avoid unnesseccary processing 
  if ( val == 0 || percentage == 0 ) {
    return 0;

  }else{

    int16_t j;

    // If there is a travel percentage 
    if ( percentage != 100 ) {

      // Do the actual math
      j = (int16_t) round( ( val * (percentage / 100.0) ) );

      // Return now if zero 
      if (j == 0) {return 0;}

    }else{
      j = val;
    }

    // Constrain 
    if ( constainval != 0 ){
      j = constraini(j, (constainval * -1), constainval);
    }

    return j;
  }
}

int16_t travel_limit_d(int16_t val, double percentage, uint16_t constainval)
{
  // Do that first to avoid unnesseccary processing 
  if ( val == 0 || percentage == 0 ) {
    return 0;

  }else{

    int16_t j;

    // If there is a travel percentage 
    if ( percentage != 100 ) {

      // Do the actual math
      j = (int16_t) round( val * percentage );

      // Return now if zero 
      if (j == 0) {return 0;}

    }else{
      j = val;
    }

    // Constrain 
    if ( constainval != 0 ){
      j = constraini(j, (constainval * -1), constainval);
    }

    return j;
  }
}

/*---------------------------------------*/
//            Mouse Processing           //
/*---------------------------------------*/

// Mouse values from TinyUSB are sent here
void set_mouseclick(uint8_t spot, bool value)
{      
  // If the value has not been updated during this cycle, accept value and toggle the FlipFlop Cycle Update Flag
  if ( !mouse_data.rmpkt.btnUpdated[spot] ) 
  { 
    // If no change in value, feck it.
    if ( mouse_data.rmpkt.btnFlipFlop[spot] != value ) 
    {
      mouse_data.rmpkt.btnFlipFlop[spot] = value;
      mouse_data.rmpkt.btnUpdated[spot] = true;
    }

  // If the FlipFlop Cycle Update Flag have been set
  } else {
    // If value is true and the FlipFlop is false, prioritize a click over a non click
    if      ( value && !mouse_data.rmpkt.btnFlipFlop[spot] ) { mouse_data.rmpkt.btnFlipFlop[spot] = value; } 

    // if value is false, flag flip-flop for toggling on the next cycle
    else if ( !value && mouse_data.rmpkt.btnFlipFlop[spot] ) { mouse_data.rmpkt.btnToggle[spot] = true; }     
  }

  return;
}


// Point the mouse report data to the correct places
// Called by TinyUSB
void process_mouse_report(hid_mouse_report_t const * report)
{ 
  /* ---------- LEft & Right Button State ---------- */

  // If Left and right buttons are swapped
  uint8_t leftbutton = mouse_data.persistent.swap_left_right ? 2 : 1;
  uint8_t rightbutton = mouse_data.persistent.swap_left_right ? 1 : 2;

  // If we're using the forward and backward buttons.
  if ( mouse_data.persistent.use_forward_backward ) { 
    // If the forward and backward buttons are swapped.
    leftbutton += mouse_data.persistent.swap_forward_backward ? 16 : 8;
    rightbutton += mouse_data.persistent.swap_forward_backward ? 8 : 16;
  }

  // Post data for left click
  set_mouseclick(0, report->buttons & leftbutton );

  // Post data for right click
  set_mouseclick(2, report->buttons & rightbutton );
  

  /* ---------- Mouse Location Tracking ---------- */

  // Dump our report values into variables.
  int8_t mousex = report->x;
  int8_t mousey = report->y;

  // Is mouse movement inverted
  if ( mouse_data.persistent.invert_x ) { mousex = -( mousex ); }
  if ( mouse_data.persistent.invert_y ) { mousey = -( mousey ); }

  // Add and Constrain mouse axis movement | prevent overflow.
  mouse_data.rmpkt.x = constraini( ( mouse_data.rmpkt.x + mousex ), -30000, 30000);
  mouse_data.rmpkt.y = constraini( ( mouse_data.rmpkt.y + mousey ), -30000, 30000);

  // Increment mouse movement ticker for AVG movement style.
  mouse_data.mouse_movt_ticker++;

  // Handle our two different mouse types.
  // I need the middle button data for wheel also, so no breaks.
  switch ( mouse_data.persistent.mousetype) {
  case WHEELBTN:
    mouse_data.rmpkt.wheel = mouse_data.rmpkt.wheel + report->wheel;
  case THREEBTN:
    set_mouseclick(1, report->buttons & MOUSE_BUTTON_MIDDLE);
  }
  return;
}


// Reset mouse state for the next cycle
void reset_cycle() {

  /* ----- Handle Buffered Button Clicks ----- */
  // ==================================================
  for( uint8_t i=0; i<=2; i++ ){

    // Reset update counter to False
    mouse_data.rmpkt.btnUpdated[i] = false;              

    // If the toggle flag has been set, toggle the value of the flip-flop
    // Note: This would happen if the user clicked and let go during the same cycle. 
    if ( mouse_data.rmpkt.btnToggle[i] ) { 
      
      mouse_data.rmpkt.btnFlipFlop[i] = !(mouse_data.rmpkt.btnFlipFlop[i]); // Invert the value of the flipflop
      mouse_data.rmpkt.btnToggle[i] = false;                                // Reset toggle flag
      mouse_data.rmpkt.btnUpdated[i] = true;                                // Flag Button State as updated
    }       
  }

  /* ----- Handle Specific mouse movement options ----- */
  // ==================================================
  switch ( mouse_data.persistent.mouse_movt_type ) 
  {
    case MO_MVT_ADDITIVE:   case MO_MVT_AVERAGE: 
      mouse_data.rmpkt.x = 0;           // Reset Buffered x-axis movement
      mouse_data.rmpkt.y = 0;           // Reset Buffered y-axis movement

    case MO_MVT_COAST:
      mouse_data.rmpkt.wheel = 0;       // Reset Buffered wheel movement
      mouse_data.mouse_movt_ticker = 0; // Reset mouse poll count
    break;
  }

  return;
}

// Update stored mouse packet
void update_mousepacket()
{
  MOUSE_PKT retpkt;

  /* ----- Dump in the current mouse button data ----- */
  // ==================================================
  retpkt.left   = mouse_data.rmpkt.btnFlipFlop[0];  
  retpkt.middle = mouse_data.rmpkt.btnFlipFlop[1];
  retpkt.right  = mouse_data.rmpkt.btnFlipFlop[2];
  retpkt.wheel  = constraini( mouse_data.rmpkt.wheel, -15, 15);
  retpkt.update = false;
  
  /* ----- Handle Specific mouse movement options ----- */
  // ==================================================
  switch ( mouse_data.persistent.mouse_movt_type )
  {
    case MO_MVT_ADDITIVE: 
    
      retpkt.x = mouse_data.rmpkt.x;
      retpkt.y = mouse_data.rmpkt.y;

    break;
  
    case MO_MVT_COAST:

      retpkt.x = 0; 
      retpkt.y = 0; 

      if ( mouse_data.rmpkt.x != 0) // If x-axis buffer is not 0
      {
        if      ( mouse_data.rmpkt.x >= 127 )   { retpkt.x = 127; }
        else if ( mouse_data.rmpkt.x <= -127 )  { retpkt.x = -127; }
        else                                    { retpkt.x = mouse_data.rmpkt.x; }
        
        mouse_data.rmpkt.x += ( retpkt.x * -1); // Remove used x-axis movement from the x-axis buffer
      }

      if ( mouse_data.rmpkt.y != 0) // If y-axis buffer is not 0
      {
        if      ( mouse_data.rmpkt.y >= 127 )   { retpkt.y = 127; }
        else if ( mouse_data.rmpkt.y <= -127 )  { retpkt.y = -127; }
        else                                    { retpkt.y = mouse_data.rmpkt.y; }

        mouse_data.rmpkt.y += ( retpkt.y * -1); // Remove used y-axis movement from the y-axis buffer
      }

    break;

    case MO_MVT_AVERAGE:

      // Avoid a Div by zero err
      if ( mouse_data.mouse_movt_ticker == 0 ) { break; }

      // Divide the total mouse XY axis values by number of mouse updates
      retpkt.x = ( int16_t ) round( mouse_data.rmpkt.x / mouse_data.mouse_movt_ticker );
      retpkt.y = ( int16_t ) round( mouse_data.rmpkt.y / mouse_data.mouse_movt_ticker );
      
    break;
  }

  /* ----- Handle Travel Limit Options ----- */
  // ==================================================
  retpkt.x = travel_limit(retpkt.x, mouse_data.persistent.xtravel_percentage, 0); // Limit X
  retpkt.y = travel_limit(retpkt.y, mouse_data.persistent.ytravel_percentage, 0); // Limit Y
  retpkt.x = travel_limit(retpkt.x, mouse_data.persistent.xytravel_percentage, 127); // Limit XY
  retpkt.y = travel_limit(retpkt.y, mouse_data.persistent.xytravel_percentage, 127); // Limit XY
  
  /* ----- Co-Sine Mouse Smoothing ----- */
  // ==================================================
  // Low = 1280 | Medium = 1024 | High = 768 | Very High = 512
  
  if ( mouse_data.persistent.use_cosine_smoothing > 0){

    double radianVal;
    
    // Process X Axis
    if (retpkt.x !=0 ){

      radianVal = retpkt.x * (3.14 / ( 1536 - (256 * mouse_data.persistent.use_cosine_smoothing ) ) );
      retpkt.x = travel_limit_d(retpkt.x, cos(radianVal), 0);

      #if DEBUG > 0
      printf("Cosine smoothed x: old: %d | new: %d | cos: %f\n", retpkt.x, travel_limit_d(retpkt.x, cos(radianVal), 0), cos(radianVal));
      #endif
    }

    // Process Y Axis
    if (retpkt.y !=0 ){

      radianVal = retpkt.y * (3.14 / ( 1536 - (256 * mouse_data.persistent.use_cosine_smoothing ) ) );
      retpkt.y = travel_limit_d(retpkt.y, cos(radianVal), 0);

      #if DEBUG > 0
      printf("Cosine smoothed y: old: %d | new: %d | cos: %f\n", retpkt.y, travel_limit_d(retpkt.y, cos(radianVal), 0), cos(radianVal));
      #endif
    }
  }
  
  /* ----- Work Out IF There Is A Button Update ----- */
  // ==================================================

  // If an update is required from a previous cycle, flag update flag
  retpkt.update = mouse_data.rmpkt.btnUpdated[0] || mouse_data.rmpkt.btnUpdated[1] || mouse_data.rmpkt.btnUpdated[2];

  // If still not true, do a pile of if's
  // If the current button state is unpressed and previous state was pressed
  if ( !retpkt.update )
  {
    if      ( mouse_data.mpkt.left && !(retpkt.left) )              { retpkt.update = true; }
    else if ( mouse_data.mpkt.middle && !(retpkt.middle) )          { retpkt.update = true; }
    else if ( mouse_data.mpkt.right && !(retpkt.right) )            { retpkt.update = true; }
    else if ( retpkt.x != 0 || retpkt.y != 0 || retpkt.wheel != 0 ) { retpkt.update = true; }
  }

  /* ----- Wrap it up ----- */
  // ==================================================
  reset_cycle();              // Prepare for the next cycle
  mouse_data.mpkt = retpkt;   // Overwrite previous mouse packet

  return;

}
