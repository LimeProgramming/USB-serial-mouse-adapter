#include "tusb.h"
#include <stdio.h>
#include <stdlib.h>
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "hardware/divider.h"
#include "hardware/watchdog.h"

#include "include/ctypes.h"
#include "include/utils.h"

#define DEBUG false          // Debug Flag for things

// Mouse Option Pins
#define MO_THREEBTN 9       // LOGITECH     | Dip Switch 1
#define MO_WHEEL 10         // MS Wheel     | Dip Switch 2
#define MO_75SPEED 11       // 75% speed    | Dip Switch 3  | With Dip 3 + 4 depressed will set mouse speed to 25%
#define MO_50SPEED 12       // 50% speed    | Dip Switch 4  | With Dip 3 + 4 depressed will set mouse speed to 25%
#define MO_7N2 13           // Compatibility with serial controllers which expect 8+1 bits format   | Dip Switch 5
#define MO_RESERVERD 14     // Reserved for future use  | Dip Switch 6

// LEDS 
#define LED_PWR 2
#define LED_ALERT 3

//UART stuff
#define UART_ID uart1       // Use UART1, keep UART0 for DEBUG printing
#define BAUD_RATE 1200      // Mouse Baud Rate. Default for Serial mice is 1200
#define UART_TX_PIN 4       // UART1 TX --> MAX232 pin 11
#define UART_RX_PIN 5       // UART1 RX --> MAX232 pin 12
#define UART_CTS_PIN 6      // CTS      --> MAX232 pin 9
#define UART_RTS_PIN 7      // RTS      --> MAX232 pin 10

/* ---------------------------------------------------------- */
/*  Numbers for Serial Speed and Delay times
    - Thanks to Aviancer for the numbers | https://github.com/Aviancer/amouse

    1200 baud (bits/s) is 133.333333333... bytes/s
    44.44.. updates per second with 3 bytes.
    33.25.. updates per second with 4 bytes.
    ~0.0075 seconds per byte, target time calculated for 4 bytes.

    SERIALDELAY = (((7500 * (BAUD_RATE / 1200)) * NumberOfBytes) + TXWIGGLEROOM )*/
/* ---------------------------------------------------------- */

static int32_t TXWIGGLEROOM    = 0; // This is for adding a bit of padding to the serial delays, shouldn't be needed but some controllers are awkward.
//static absolute_time_t txtimer_target;      // Actual time between the sending of serial packets, calculated on startup.
static int32_t SERIALDELAY_1B;      // One Byte Delay Time, calculated on startup.
static int32_t SERIALDELAY_3B;      // Three Byte Delay Time, calculated on startup.
static int32_t SERIALDELAY_4B;      // Four Byte Delay Time, calculated on startup.
uint8_t intro_pkts[] = {0x4D,0x33,0x5A};    // M3Z | Ident info serial mouse.

/* ---------------------------------------------------------- */
// Mouse Tracking Variables
/*
typedef struct {                            // Mouse report information
    bool left, middle, right;
    int16_t  x, y, wheel;
    bool update;
} MOUSE_PKT;

typedef struct {                            // Mouse Settings and information
    uint8_t     speed;                      // Mouse Tracking Speed
    uint8_t     type;                       // Mouse Type
    uint8_t     pc_state;                   // CTS state tracker | taken from Aviancer's code since it was more straightforward than what I had already
    MOUSE_PKT   mpkt;                       // Current Mouse Packet
} MOUSE_DATA;
*/
MOUSE_DATA mouse_data;
static bool mouse_connected = false;    // Is mouse connected flag
bool btnFlipFlop[3];                    // The Button Flip Flop | Left Click + Middle Click + Right Click
bool btnUpdated[3];                     // FlipFlop Cycle Update Flag | Left Click + Middle Click + Right Click
bool btnToggle[3];                      // FlipFlop toggle flag | Left Click + Middle Click + Right Click
int16_t mLoc[3];                        // Mouse location Data | X + Y + Wheel

/*---------------------------------------*/
//             TinyUSB Stuff             //
/*---------------------------------------*/

/*----- Variables -----*/

#define MAX_HID_REPORT  4 // Each HID instance can has multiple reports

static struct{
  uint8_t report_count;
  tuh_hid_report_info_t report_info[MAX_HID_REPORT];
}hid_info[CFG_TUH_HID];

CFG_TUSB_MEM_SECTION static char serial_in_buffer[64] = { 0 };

/*----- Functions -----*/

void set_mouseclick(uint8_t spot, bool value);

//static inline void process_mouse_report(mouse_state_t *mouse, hid_mouse_report_t const *p_report);
void process_mouse_report(hid_mouse_report_t const * report);

// Handle generic USB Report
void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    (void) dev_addr;

    uint8_t const rpt_count = hid_info[instance].report_count;
    tuh_hid_report_info_t* rpt_info_arr = hid_info[instance].report_info;
    tuh_hid_report_info_t* rpt_info = NULL;

    if ( rpt_count == 1 && rpt_info_arr[0].report_id == 0) {    // Simple report without report ID as 1st byte
        rpt_info = &rpt_info_arr[0];        
    }
    
    else {
        uint8_t const rpt_id = report[0];                       // Composite report, 1st byte is report ID, data starts from 2nd byte

        for ( uint8_t i=0; i<rpt_count; i++ ) {
            if ( rpt_id == rpt_info_arr[i].report_id ) {
                rpt_info = &rpt_info_arr[i];
                break;
            }}

        report++;   len--;
    }

    // For complete list of Usage Page & Usage checkout src/class/hid/hid.h. && Assume mouse follow boot report layout
    if ( rpt_info && rpt_info->usage_page == HID_USAGE_PAGE_DESKTOP && rpt_info->usage == HID_USAGE_DESKTOP_MOUSE ) {
        gpio_put(LED_ALERT, 1);     // Turn on Alert LED
        mouse_connected = true;     // Flag mouse as connected
        process_mouse_report((hid_mouse_report_t const*) report); 
    }
}

/*
// invoked ISR context I don't think this does anything
void tuh_cdc_xfer_isr(uint8_t dev_addr, xfer_result_t event, cdc_pipeid_t pipe_id, uint32_t xferred_bytes)
{
    (void) event;
    (void) pipe_id;
    (void) xferred_bytes;

    tu_memclr(serial_in_buffer, sizeof(serial_in_buffer));

    tuh_cdc_receive(dev_addr, serial_in_buffer, sizeof(serial_in_buffer), true); // waiting for next data
}*/

// This is executed when a new device is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    switch (itf_protocol) 
    {
        case HID_ITF_PROTOCOL_MOUSE:    // Process Mouse Report
            gpio_put(LED_ALERT, 1);     // Turn on Alert LED
            mouse_connected = true;     // Flag mouse as connected
            // TODO: Flag mouse connected ALRT
            break;

        case HID_ITF_PROTOCOL_NONE:    // Process Mouse Report
            // By default host stack will use activate boot protocol on supported interface.
            hid_info[instance].report_count = tuh_hid_parse_report_descriptor(hid_info[instance].report_info, MAX_HID_REPORT, desc_report, desc_len);
            // TODO: Flag mouse unsure ALRT
            break;

        default:                        // Process Generic Report
            // TODO: Flag incompatible ALRT
            break;
    }

    if ( DEBUG ) { 
        const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
        printf("Device with address %d, instance %d, protocol %d, has mounted.\r\n", dev_addr, instance, itf_protocol); 
        printf("Device with address %d, instance %d, protocol %d, is a %s, has mounted.\r\n", dev_addr, instance, itf_protocol, protocol_str[itf_protocol]); 
    }
}

// This is executed when a device is unmounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{   
    // DEBUG printout
    if ( DEBUG ) { printf("Device with address %d, instance %d was unmounted.\r\n", dev_addr, instance); }

    if ( mouse_connected ) {        // IF a mouse was previously connected
        gpio_put(LED_ALERT, 0);     // Turn off Alert LED
        mouse_connected = false;    // Flag mouse as not connected
    }

    sleep_ms(100);
    uart_deinit(UART_ID);           // DeInit UART for sanity sake
    machine_reboot();               // There's a bug in TinyUSB, a reboot should bypass it
}

// This is executed when data is received from the mouse
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{   
    switch (tuh_hid_interface_protocol(dev_addr, instance)) 
    {
        case HID_ITF_PROTOCOL_MOUSE:    // Process Mouse Report
            process_mouse_report((hid_mouse_report_t const*) report );
            break;

        case HID_ITF_PROTOCOL_KEYBOARD: // Throw Away Keyboard Reports
            break;

        default:                        // Process Generic Report
            process_generic_report(dev_addr, instance, report, len);
            break;
    }
}

/*---------------------------------------*/
//            Mouse Processing           //
/*---------------------------------------*/

// Point the mouse report data to the correct places
void process_mouse_report(hid_mouse_report_t const * report)
{
    // Send the button report data off to the flip flop tracker
    set_mouseclick(0, report->buttons & MOUSE_BUTTON_LEFT);
    set_mouseclick(2, report->buttons & MOUSE_BUTTON_RIGHT);

    // Limiting the max motion of a mouse movement axis helps prevent an overflow on the serial side of things 
    mLoc[0] = constrain( (mLoc[0] + report->x) , -127, 127);    // Process the X axis
    mLoc[1] = constrain( (mLoc[1] + report->y) , -127, 127);    // Process the Y axis

    if ( mouse_data.type == WHEELBTN ){
        set_mouseclick(1, report->buttons & MOUSE_BUTTON_MIDDLE);
        mLoc[2] = constrain( (mLoc[2] + report->pan) , -15, 15);
    } 
    else if ( mouse_data.type == THREEBTN ){
        set_mouseclick(1, report->buttons & MOUSE_BUTTON_MIDDLE);
    }
}

// Reset mouse state for the next cycle
void reset_cycle()
{
    for( u_short i=0; i<=2; i++ ){
        btnUpdated[i] = false;              // Reset update counter to False

        // If the toggle flag has been set, toggle the value of the flip-flop
        // Note:
        //      This would happen if the user clicked and let go during the same cycle. 
        if ( btnToggle[i] )
        {
            btnFlipFlop[i] = !(btnFlipFlop[i]);

            btnToggle[i] = false;   // Reset toggle flag
            btnUpdated[i] = true;   // Flag Button State as updated
        }
        mLoc[i] = 0;                // Reset mLoc array
    }
}

// Mouse values from TinyUSB are sent here
void set_mouseclick(uint8_t spot, bool value)
{      
    // If the value has not been updated during this cycle, accept value and toggle the FlipFlop Cycle Update Flag
    if ( !btnUpdated[spot] )
    {
        btnFlipFlop[spot] = value;
        btnUpdated[spot] = true;
    }
    else        // If the FlipFlop Cycle Update Flag have been set
    {
        if      ( value && !btnFlipFlop[spot] ) { btnFlipFlop[spot] = value; }  // If value is true and the FlipFlop is false, prioritize a click over a non click
        else if ( !value && btnFlipFlop[spot] ) { btnToggle[spot] = true; }     // if value is false, flag flip-flop for toggling on the next cycle
    }
}

// Update stored mouse packet
void update_mousepacket()
{
    MOUSE_PKT retpkt;

    // Dump in the current mouse data
    retpkt.left =   btnFlipFlop[0];
    retpkt.middle = btnFlipFlop[1];
    retpkt.right =  btnFlipFlop[2];
    retpkt.wheel =  mLoc[2];
    retpkt.update = false;

    // Limit mouse xy travel rate if required
    if      ( mouse_data.speed == SPEED75 ) { retpkt.x = travel_limit75(mLoc[0]); retpkt.y = travel_limit75(mLoc[1]); }
    else if ( mouse_data.speed == SPEED50 ) { retpkt.x = travel_limit50(mLoc[0]); retpkt.y = travel_limit50(mLoc[1]); }
    else if ( mouse_data.speed == SPEED25 ) { retpkt.x = travel_limit25(mLoc[0]); retpkt.y = travel_limit25(mLoc[1]); }
    else                                    { retpkt.x = mLoc[0]; retpkt.y = mLoc[1]; }

    // If an update is required from a previous cycle, IE a button has been let go of.
    for ( u_short i=0; i<=2; i++ ) { if ( btnUpdated[i] == true ) { retpkt.update = true; } }

    if ( !retpkt.update )
    {
        if      ( mouse_data.mpkt.left && !(retpkt.left) )              { retpkt.update = true; }
        else if ( mouse_data.mpkt.middle && !(retpkt.middle) )          { retpkt.update = true; }
        else if ( mouse_data.mpkt.right && !(retpkt.right) )            { retpkt.update = true; }
        else if ( retpkt.x != 0 || retpkt.y != 0 || retpkt.wheel != 0 ) { retpkt.update = true; }
    }

    reset_cycle();              // Prepare for the next cycle
    mouse_data.mpkt = retpkt;   // Overwrite previous mouse packet
}

/*---------------------------------------*/
//             SERIAL STUFF              //
/*---------------------------------------*/

// Post off data over UART. 
void serial_putc(uint8_t *buffer, int size){ 
    for( uint8_t i=0; i <= size; i++ )  { uart_putc_raw( UART_ID, buffer[i] ); } 
}

// Serial mouse negotiation
void mouse_id_nego() {
    /*---------------------------------------*/
    //        Serial Mouse negotiation       //
    /*---------------------------------------*/
    // Byte1:Always M                        //
    // Byte2:[None]=MS 3=Logitech Z=MSWheel  // 
    /*---------------------------------------*/

    // Wait for UART to be writable
    while ( !uart_is_writable(UART_ID) ) { sleep_us(SERIALDELAY_1B); }   

    uart_putc_raw( UART_ID, intro_pkts[0] );        // M
    
    if ( mouse_data.type == THREEBTN ) {
        sleep_us(SERIALDELAY_1B);           
        uart_putc_raw( UART_ID, intro_pkts[1] );    // 3
    }
    else if ( mouse_data.type == WHEELBTN ) {
        sleep_us(SERIALDELAY_1B);       
        uart_putc_raw( UART_ID, intro_pkts[2] );    // Z
    }

    sleep_us( SERIALDELAY_1B );                     // Wait for packet to send              
}

// Update stored mouse data and post
void post_serial_mouse() {
    uint8_t packet[4];                          // Create packet array
    update_mousepacket();                       // Get Current Mouse Data
    if ( !mouse_data.mpkt.update ) { return; }  // Skip if no update.
    
    packet[0] = ( 0x40 | (mouse_data.mpkt.left ? 0x20 : 0) | (mouse_data.mpkt.right ? 0x10 : 0) | ((mouse_data.mpkt.y >> 4) & 0xC) | ((mouse_data.mpkt.x >> 6) & 0x3));
    packet[1] = ( 0x00 | (mouse_data.mpkt.x & 0x3F)); 
    packet[2] = ( 0x00 | (mouse_data.mpkt.y & 0x3F));

    if ( mouse_data.type == WHEELBTN ){         // Add Wheel Data + Third Button
        packet[3] = (0x00 | (mouse_data.mpkt.middle ? 0x20 : 0) | (-mouse_data.mpkt.wheel & 0x0f));
        serial_putc(packet,  3);
    }
    else if ( mouse_data.type == THREEBTN ){    // Add Third Button
        packet[3] = (0x00 | (mouse_data.mpkt.middle ? 0x20 : 0));
        serial_putc(packet,  3);
    }
    else{
        serial_putc(packet,  2);
    }
}

/*---------------------------------------*/
//          Mouse Settings Stuff         //
/*---------------------------------------*/

/*
::TODO::
I do want to have mouse type hot changeable but I just have to sit and experiment with it. 
Hitting reset it fine for now
*/

// Sets Mouse Speed from the headers.
void setMouseType()
{   
    if      ( !gpio_get(MO_WHEEL) )     { mouse_data.type = WHEELBTN; }
    else if ( !gpio_get(MO_THREEBTN) )  { mouse_data.type = THREEBTN; }
    else                                { mouse_data.type = TWOBTN; }
}

// Updates Mouse Travel Rate Divider from the headers.
void setMouseSpeed()
{   
    if      ( !gpio_get(MO_75SPEED) && !gpio_get(MO_50SPEED) )  { mouse_data.speed = SPEED25; }
    else if ( !gpio_get(MO_50SPEED) )                           { mouse_data.speed = SPEED50; }
    else if ( !gpio_get(MO_75SPEED) )                           { mouse_data.speed = SPEED75; }
    else                                                        { mouse_data.speed = SPEED100; }
}

/*
I spent a bit of time trying to figure out the simplest way to have mouse settings updatable without restarting the pi pico.
I could have just called the above functions in the main loop all the time but that felt wasteful.
Debounce in a gpio_callback doesn't work well, give it a Google, some fellow tried it and it was spotty.
I implemented a hardware debounce but decided against it, since it made the final PCB bigger for something not integral. 

Ultimately I decided it was best to add an alarm from a GPIO callback to act as a poor mans debounce.
*/

volatile bool MS_IRQ_TRIGGERED = false;
volatile bool DS_IRQ_TRIGGERED = false;

int64_t ms_alarm_callback(alarm_id_t id, void *user_data)   // Mouse Speed Alarm
{
    setMouseSpeed();                            // Update Mouse Options
    MS_IRQ_TRIGGERED = false;                   // Unset IRQ Bool
    return 0;                                   // Ret 0
}

int64_t ds_alarm_callback(alarm_id_t id, void *user_data)   // Double Stop Bit Alarm
{
    if ( !gpio_get(MO_7N2) )    { uart_set_format(UART_ID, 7, 2, UART_PARITY_NONE); }   // 7N2
    else                        { uart_set_format(UART_ID, 7, 1, UART_PARITY_NONE); }   // 7N1
    DS_IRQ_TRIGGERED = false;                   // Unset IRQ Bool
    return 0;                                   // Ret 0
}

void dipsw_gpio_callback(uint gpio, uint32_t events) 
{
    gpio_acknowledge_irq(gpio, events);         // ACK GPIO IRQ
    
    if ( gpio == MO_50SPEED || gpio == MO_75SPEED ) {
        if ( !MS_IRQ_TRIGGERED ) {                  // If IRQ bool is not set
            MS_IRQ_TRIGGERED = true;                // Set IRQ bool
            add_alarm_in_ms(2000, ms_alarm_callback, NULL, true);   // Call MO_alarm_callback in 2 seconds
    }}

    else if ( gpio == MO_7N2 ) {
        if ( !DS_IRQ_TRIGGERED ) {                  // If IRQ bool is not set
            DS_IRQ_TRIGGERED = true;                // Set IRQ bool
            add_alarm_in_ms(2000, ds_alarm_callback, NULL, true);// Call SB_alarm_callback in 2 seconds
    }}
}

/*---------------------------------------*/
//             Repeating Timer           //
/*---------------------------------------*/
struct repeating_timer serial_timer;
volatile bool serial_timer_running = false;

bool serial_timer_callback(struct repeating_timer *t) {
    if ( (mouse_data.pc_state == CTS_TOGGLED && mouse_connected) || DEBUG ) { post_serial_mouse(); }
    return true;
}

void toggle_serial_timer() {
    if  ( serial_timer_running )  { 
        cancel_repeating_timer(&serial_timer); 
        serial_timer_running = false;
    } else {
        int delay_ms = 0;

        if  ( mouse_data.type == TWOBTN )   { delay_ms = SERIALDELAY_3B; } 
        else                                { delay_ms = SERIALDELAY_4B; }

        add_repeating_timer_us(delay_ms, serial_timer_callback, NULL, &serial_timer);
        serial_timer_running = true;
    }
}

/*---------------------------------------*/
//                  Main                 //
/*---------------------------------------*/
int main(){
    stdio_init_all();       // pico SDK
    board_init();           // init board from TinyUSB
    tusb_init();            // init TinyUSB

    /*---------------------------------------*/
    //                 LEDS                  //
    /*---------------------------------------*/

    init_led(LED_PWR);          // Init Power LED
    gpio_put(LED_PWR, 1);       // Turn on Power LED
    init_led(LED_ALERT);        // Init Alert LED

    /*---------------------------------------*/
    //              Mouse Options            //
    /*---------------------------------------*/
    
    // Setup Dip Switch Pins
    int dipswpins[6] = { MO_WHEEL, MO_THREEBTN, MO_50SPEED, MO_75SPEED, MO_7N2, MO_RESERVERD};
    for ( uint8_t i = 0; i < 6; i++ ) { init_pinheader(dipswpins[i]); }

    // Set Mouse Type
    setMouseType();
    
    // Set up IRQ callbacks on Dip Switch Pins
    gpio_set_irq_enabled_with_callback(MO_50SPEED, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &dipsw_gpio_callback);
    gpio_set_irq_enabled_with_callback(MO_75SPEED, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &dipsw_gpio_callback);
    gpio_set_irq_enabled_with_callback(MO_7N2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &dipsw_gpio_callback);

    /*---------------------------------------*/
    //              UART STUFF               //
    /*---------------------------------------*/

    uart_init(UART_ID, BAUD_RATE);              // Set up the UART with the required speed.
    uart_set_hw_flow(UART_ID, false, false);    // Set UART flow control CTS/RTS
    uart_set_translate_crlf(UART_ID, false);    // Turn off crlf conversion
    uart_set_fifo_enabled(UART_ID, false);      // Turn off FIFO's - we want to do this character by character

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Set the TX pins
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // Set the RX pins

    gpio_init(UART_CTS_PIN);                    // CTS Pin
    gpio_set_dir(UART_CTS_PIN, GPIO_IN);        // CTS Pin direction

    // Set Double Stop Toggle
    if ( !gpio_get(MO_7N2) )    { uart_set_format(UART_ID, 7, 2, UART_PARITY_NONE); }   // 7N2
    else                        { uart_set_format(UART_ID, 7, 1, UART_PARITY_NONE); }   // 7N1

    /*----------   PACKET DELAYS   ----------*/

    SERIALDELAY_1B = (( 7500 * (BAUD_RATE / 1200)) +      TXWIGGLEROOM );
    SERIALDELAY_3B = (((7500 * (BAUD_RATE / 1200)) * 3) + TXWIGGLEROOM );
    SERIALDELAY_4B = (((7500 * (BAUD_RATE / 1200)) * 4) + TXWIGGLEROOM );

    /*----------   TIMER   ----------*/

    toggle_serial_timer();
    sleep_ms(1110);
    toggle_serial_timer();
    sleep_ms(1110);
    toggle_serial_timer();
    sleep_ms(1110);

    /*---------------------------------------*/
    //               Main Loop               //
    /*---------------------------------------*/
    while(1)
    {
        
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
                busy_wait_us_32(SERIALDELAY_4B);

                bool cts_pin = gpio_get(UART_CTS_PIN);
                
                if ( cts_pin ) { mouse_data.pc_state = CTS_LOW_INIT; }
            }
        }

        // Mouse initializing request detected
        if( !cts_pin && mouse_data.pc_state == CTS_LOW_INIT ) {
            mouse_id_nego();
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
        if( (mouse_data.pc_state != CTS_TOGGLED && mouse_connected) )
        {   
            //if ( DEBUG ) { printf("Serial Connection not found"); }

            // Call ALRT Code flasher for Serial Connection Lost. 
            // TODO

        }   


    }


}
