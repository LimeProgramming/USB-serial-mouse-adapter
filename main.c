#include "tusb.h"
#include <stdlib.h>
#include "bsp/board.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "hardware/divider.h"

#include "include/ctypes.h"
#include "include/utils.h"

#define DEBUG true			// Debug Flag for things

// Mouse Option Pins
#define MO_25SPEED 22 		// 25% speed
#define MO_50SPEED 21 		// 50% speed
#define MO_75SPEED 20 		// 75% speed
#define MO_WHEEL 19 		// MS Wheel
#define MO_THREEBTN 18 		// LOGITECH
#define MO_7N2 17 		  	// Compatibility with serial controllers which expect 8+1 bits format

// LEDS 
#define LED_PWR 2
#define LED_ALERT 3

//UART stuff
#define UART_ID uart1 		// Use UART1, keep UART0 for DEBUG printing
#define BAUD_RATE 1200 		// Mouse Baud Rate. Default for Serial mice is 1200
#define UART_TX_PIN 4 		// UART1 TX --> MAX232 pin 11
#define UART_RX_PIN 5 		// UART1 RX --> MAX232 pin 12
#define UART_CTS_PIN 6 		// CTS		--> MAX232 pin 9

/* ---------------------------------------------------------- */
/*	Numbers for Serial Speed and Delay times
	- Thanks to Aviancer for the numbers | https://github.com/Aviancer/amouse

	1200 baud (bits/s) is 133.333333333... bytes/s
	44.44.. updates per second with 3 bytes.
	33.25.. updates per second with 4 bytes.
	~0.0075 seconds per byte, target time calculated for 4 bytes.

	SERIALDELAY = (((7500 * (BAUD_RATE / 1200)) * NumberOfBytes) + TXWIGGLEROOM )
/* ---------------------------------------------------------- */

static uint32_t TXWIGGLEROOM 	= 0;		// This is for adding a bit of padding to the serial delays, shouldn't be needed but some controllers are awkward.
static uint32_t txtimer_target; 			// Actual time between the sending of serial packets, calculated on startup.
static uint32_t SERIALDELAY_1B;  			// One Byte Delay Time, calculated on startup.
static uint32_t SERIALDELAY_3B; 			// Three Byte Delay Time, calculated on startup.
static uint32_t SERIALDELAY_4B; 			// Four Byte Delay Time, calculated on startup.
uint8_t intro_pkts[] = {0x4D,0x33,0x5A}; 	// M3Z | Ident info serial mouse.

/* ---------------------------------------------------------- */
// Some structs for mouse tracking

typedef struct {							// Mouse report information
	bool left, middle, right;
	int16_t  x, y, wheel;
	bool update;
} MOUSE_PKT;

typedef struct {							// Mouse Settings and information
	uint8_t 	speed;						// Mouse Tracking Speed
	uint8_t 	type;						// Mouse Type
	uint8_t 	pc_state;					// CTS state tracker | taken from Aviancer's code since it was more straightforward than what I had already
	MOUSE_PKT 	mpkt;						// Current Mouse Packet
} MOUSE_DATA;

/* ---------------------------------------------------------- */
// Mouse Tracking Variables

MOUSE_DATA mouse_data;
static bool mouse_connected = false; 	// Is mouse connected flag
bool btnFlipFlop[3];					// The Button Flip Flop | Left Click + Middle Click + Right Click
bool btnUpdated[3];						// FlipFlop Cycle Update Flag | Left Click + Middle Click + Right Click
bool btnToggle[3]; 						// FlipFlop toggle flag | Left Click + Middle Click + Right Click
int16_t mLoc[3];						// Mouse location Data | X + Y + Wheel

/*---------------------------------------*/
//			 TinyUSB Functions 			 //
/*---------------------------------------*/

static void process_mouse_report(hid_mouse_report_t const * report);
void set_mouseclick(uint8_t spot, bool value);

// This is executed when a new device is mounted.
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
	uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

	// If connected device is not a mouse or compatible
	if ( itf_protocol != HID_ITF_PROTOCOL_MOUSE ) {

		// Debug Print out
		if ( DEBUG) { printf("Device with address %d, instance %d is not a mouse.\r\n", dev_addr, instance); }

		// Call ALRT Code flasher for incompatible device. 
		// TODO

		return;	// Exit func
	}

	gpio_put(LED_ALERT, 1);		// Turn on Alert LED
	mouse_connected = true;		// Flag mouse as connected

	if ( DEBUG ) { 
		const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
		printf("Device with address %d, instance %d is a %s.\r\n", dev_addr, instance, protocol_str[itf_protocol]);
	}
}

// This is executed when a device is unmounted.
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{	
	// DEBUG printout
	if ( DEBUG ) { printf("Device with address %d, instance %d was unmounted.\r\n", dev_addr, instance); }

	if ( mouse_connected ) {		// IF a mouse was previously connected
		gpio_put(LED_ALERT, 0);		// Turn off Alert LED
		mouse_connected = false;	// Flag mouse as not connected
	}

	uart_deinit(UART_ID);			// DeInit UART for sanity sake
	machine_reboot(); 				// There's a bug in TinyUSB, a reboot should bypass it
}

// This is executed when data is received from the mouse.
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{	
	// This is to ensure that anything which isn't a mouse is ignored
	if ( tuh_hid_interface_protocol(dev_addr, instance) == HID_ITF_PROTOCOL_MOUSE ) {
		process_mouse_report((hid_mouse_report_t const*) report);
	}
}

/*---------------------------------------*/
//			  Mouse Processing			 //
/*---------------------------------------*/

// Point the mouse report data to the correct places
static void process_mouse_report(hid_mouse_report_t const * report)
{
	// Send the button report data off to the flip flop tracker
	set_mouseclick(0, report->buttons & MOUSE_BUTTON_LEFT);
	set_mouseclick(2, report->buttons & MOUSE_BUTTON_RIGHT);

	// Limiting the max motion of a mouse movement axis helps prevent an overflow on the serial side of things 
	mLoc[0] = constrain( (mLoc[0] + report->x) , -127, 127);	// Process the X axis
	mLoc[1] = constrain( (mLoc[1] + report->y) , -127, 127);	// Process the Y axis

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
        btnUpdated[i] = false;				// Reset update counter to False

        // If the toggle flag has been set, toggle the value of the flip-flop
        // Note:
        //      This would happen if the user clicked and let go during the same cycle. 
        if ( btnToggle[i] )
        {
			btnFlipFlop[i] = !(btnFlipFlop[i]);

            btnToggle[i] = false;	// Reset toggle flag
			btnUpdated[i] = true;	// Flag Button State as updated
        }
		mLoc[i] = 0;				// Reset mLoc array
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
    else 		// If the FlipFlop Cycle Update Flag have been set
    {
        if 		( value && !btnFlipFlop[spot] ) { btnFlipFlop[spot] = value; }	// If value is true and the FlipFlop is false, prioritize a click over a non click
        else if ( !value && btnFlipFlop[spot] )	{ btnToggle[spot] = true; } 	// if value is false, flag flip-flop for toggling on the next cycle
    }
}

// Update stored mouse packet
void update_mousepacket()
{
	MOUSE_PKT retpkt;

	// Dump in the current mouse data
	retpkt.left = 	btnFlipFlop[0];
	retpkt.middle = btnFlipFlop[1];
	retpkt.right = 	btnFlipFlop[2];
	retpkt.wheel = 	mLoc[2];
	retpkt.update = false;

	// Limit mouse xy travel rate if required
	if 		( mouse_data.speed == SPEED75 )	{ retpkt.x = travel_limit75(mLoc[0]); retpkt.y = travel_limit75(mLoc[1]); }
	else if ( mouse_data.speed == SPEED50 )	{ retpkt.x = travel_limit50(mLoc[0]); retpkt.y = travel_limit50(mLoc[1]); }
	else if ( mouse_data.speed == SPEED25 )	{ retpkt.x = travel_limit25(mLoc[0]); retpkt.y = travel_limit25(mLoc[1]); }
	else 	 								{ retpkt.x = mLoc[0]; retpkt.y = mLoc[1]; }

	// If an update is required from a previous cycle, IE a button has been let go of.
	for ( u_short i=0; i<=2; i++ ) { if ( btnUpdated[i] == true ) { retpkt.update = true; } }

	if ( !retpkt.update )
	{
		if 		( mouse_data.mpkt.left && !(retpkt.left) )				{ retpkt.update = true; }
		else if ( mouse_data.mpkt.middle && !(retpkt.middle) )			{ retpkt.update = true; }
		else if ( mouse_data.mpkt.right && !(retpkt.right) ) 			{ retpkt.update = true; }
		else if ( retpkt.x != 0 || retpkt.y != 0 || retpkt.wheel != 0 ) { retpkt.update = true; }
	}

    reset_cycle();				// Prepare for the next cycle
	mouse_data.mpkt = retpkt;	// Overwrite previous mouse packet
}

/*---------------------------------------*/
//			   SERIAL STUFF 			 //
/*---------------------------------------*/

// Post off data over UART. 
void serial_putc(uint8_t *buffer, int size)
{ 
	for( uint8_t i=0; i <= size; i++ ) 	{ uart_putc_raw( UART_ID, buffer[i] ); } 
}

// Serial mouse negotiation
void mouse_id_nego() 
{
	/*---------------------------------------*/
	//	      Serial Mouse negotiation	     //
	/*---------------------------------------*/
	// Byte1:Always M                        //
	// Byte2:[None]=MS 3=Logitech Z=MSWheel  // 
	/*---------------------------------------*/

	// Wait for UART to be writable
	while ( !uart_is_writable ) { sleep_us(SERIALDELAY_1B); }	

	uart_putc_raw( UART_ID, intro_pkts[0] );  		// M
	
	if ( mouse_data.type == THREEBTN ) {
		sleep_us(SERIALDELAY_1B);			
		uart_putc_raw( UART_ID, intro_pkts[1] ); 	// 3
	}
	else if ( mouse_data.type == WHEELBTN ){
		sleep_us(SERIALDELAY_1B);		
		uart_putc_raw( UART_ID, intro_pkts[2] ); 	// Z
	}

	sleep_us( SERIALDELAY_1B );						// Wait for packet to send				
}

// Update stored mouse data and post
void post_serial_mouse() 
{
	update_mousepacket();

	// If there's nothing worth talking about, feck it
	if ( !mouse_data.mpkt.update ) { return; }

	uint8_t packet[4];

	packet[0] = ( 0x40 | (mouse_data.mpkt.left ? 0x20 : 0) | (mouse_data.mpkt.right ? 0x10 : 0) | ((mouse_data.mpkt.y >> 4) & 0xC) | ((mouse_data.mpkt.x >> 6) & 0x3));
	packet[1] = ( 0x00 | (mouse_data.mpkt.x & 0x3F)); 
	packet[2] = ( 0x00 | (mouse_data.mpkt.y & 0x3F));

	if ( mouse_data.type == WHEELBTN )
	{
		packet[3] = (0x00 | (mouse_data.mpkt.middle ? 0x20 : 0) | (-mouse_data.mpkt.wheel & 0x0f));
		serial_putc(packet,  3);
		txtimer_target = time_us_32() + SERIALDELAY_4B;
	}
	else if ( mouse_data.type == THREEBTN )
	{
		packet[3] = (0x00 | (mouse_data.mpkt.middle ? 0x20 : 0));
		serial_putc(packet,  3);
		txtimer_target = time_us_32() + SERIALDELAY_4B;
	}
	else
	{
		serial_putc(packet,  2);
		txtimer_target = time_us_32() + SERIALDELAY_3B;
	}
}

/*---------------------------------------*/
//			Mouse Settings Stuff   		 //
/*---------------------------------------*/

// Updates Mouse settings from the headers.
void setMouseOptions()
{	
	// Configure Travel Rate Divider settings
	if 		( !gpio_get(MO_25SPEED) ) 	{ mouse_data.speed = SPEED25; }
	else if ( !gpio_get(MO_50SPEED) ) 	{ mouse_data.speed = SPEED50; }
	else if ( !gpio_get(MO_75SPEED) ) 	{ mouse_data.speed = SPEED75; }
	else 								{ mouse_data.speed = SPEED100; }

	// Configure mouse type
	if 		( !gpio_get(MO_WHEEL) ) 	{ mouse_data.type = WHEELBTN; }
	else if ( !gpio_get(MO_THREEBTN) ) 	{ mouse_data.type = THREEBTN; }
	else 								{ mouse_data.type = TWOBTN; }
}
	
/*
I spent a bit of time trying to figure out the simplest way to have mouse settings updatable without restarting the pi pico.
I could have just called setMouseOptions() in the main loop all the time but that felt wasteful.
Debounce in a gpio_callback doesn't work well, give it a Google, some fellow tried it and it was spotty.
I implemented a hardware debounce but decided against it, since it made the final PCB bigger for something not integral. 

Ultimately I decided it was best to add an alarm from a GPIO callback to act as a shitty debounce.
While the IRQ is triggering from the bounce the mouse movement could be spotty but you're not moving the mouse while changing pin headers, so it doesn't matter.
Plus, I want as little as possible in the main loop to avoid the possibility of missing input.
*/

/* ===== Mouse Options Callback ===== */

volatile bool MO_IRQ_TRIGGERED = false;

int64_t MO_alarm_callback(alarm_id_t id, void *user_data)
{
	setMouseOptions();							// Update Mouse Options
	MO_IRQ_TRIGGERED = false;					// Unset IRQ Bool
    return 0;									// Ret 0
}

void MO_gpio_callback(uint gpio, uint32_t events) 
{
	gpio_acknowledge_irq(gpio, events);			// ACK GPIO IRQ
	if ( !MO_IRQ_TRIGGERED ) 					// If IRQ bool is not set
		MO_IRQ_TRIGGERED = true;				// Set IRQ bool
		add_alarm_in_ms(2000, MO_alarm_callback, NULL, true); 	// Call MO_alarm_callback in 2 seconds
}

/* ===== Stop Bit Callback ===== */
// Made this seperate since I don't want to touch the UART settings unless required.
volatile bool SB_IRQ_TRIGGERED = false;

int64_t SB_alarm_callback(alarm_id_t id, void *user_data)
{
	// Set Double Stop Toggle
	if ( !gpio_get(MO_7N2) )	{ uart_set_format(UART_ID, 7, 2, UART_PARITY_NONE); }	// 7N2
	else						{ uart_set_format(UART_ID, 7, 1, UART_PARITY_NONE); }	// 7N1
	SB_IRQ_TRIGGERED = false;					// Unset IRQ Bool
    return 0;									// Ret 0
}

void SB_gpio_callback(uint gpio, uint32_t events) 
{
	gpio_acknowledge_irq(gpio, events);			// ACK GPIO IRQ
	if ( !SB_IRQ_TRIGGERED ) 					// If IRQ bool is not set
		SB_IRQ_TRIGGERED = true;				// Set IRQ bool
		// Call SB_alarm_callback in 2 seconds
		add_alarm_in_ms(2000, SB_alarm_callback, NULL, true);
}

/*---------------------------------------*/
//		   		    Main				 //
/*---------------------------------------*/
int main(void)
{
	/*---------------------------------------*/
	//			       LEDS      			 //
	/*---------------------------------------*/

	init_led(LED_PWR);			// Init Power LED
	gpio_put(LED_PWR, 1);		// Turn on Power LED
	init_led(LED_ALERT);		// Init Alert LED

	/*---------------------------------------*/
	//			    Mouse Options 			 //
	/*---------------------------------------*/

	// 25% Travel Rate Pin | 50% Travel Rate Pin | 75% Travel Rate Pin | Three BTN Type Pin | Mouse Wheel Type Pin 
	int MO_arr[5] = { MO_25SPEED, MO_50SPEED, MO_75SPEED, MO_WHEEL, MO_THREEBTN };

	for (short i = 0; i < 5; i++) {		// Set up Mouse Options Pins
		init_pinheader(MO_arr[i]);
		gpio_set_irq_enabled_with_callback(MO_arr[i], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &MO_gpio_callback);
	}

	setMouseOptions();				// Configure Mouse settings from cold boot 
	init_pinheader(MO_7N2); 		// Configure Awkward Compatibility Pin Header
	gpio_set_irq_enabled_with_callback(MO_7N2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SB_gpio_callback);
	
	/*---------------------------------------*/
	//			    UART STUFF  			 //
	/*---------------------------------------*/

    uart_init(UART_ID, BAUD_RATE);				// Set up the UART with the required speed.
    uart_set_hw_flow(UART_ID, false, false);	// Set UART flow control CTS/RTS
    uart_set_translate_crlf(UART_ID, false);	// Turn off crlf conversion
	uart_set_fifo_enabled(UART_ID, false);		// Turn off FIFO's - we want to do this character by character

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);	// Set the TX pins
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);	// Set the RX pins

	gpio_init(UART_CTS_PIN); 					// CTS Pin
	gpio_set_dir(UART_CTS_PIN, GPIO_IN);		// CTS Pin direction

	// Set Double Stop Toggle
	if ( !gpio_get(MO_7N2) )	{ uart_set_format(UART_ID, 7, 2, UART_PARITY_NONE); }	// 7N2
	else						{ uart_set_format(UART_ID, 7, 1, UART_PARITY_NONE); }	// 7N1

	/*---------------------------------------*/
	//	  	   	  PACKET DELAYS  			 //
	/*---------------------------------------*/

	SERIALDELAY_1B = ((7500 * (BAUD_RATE / 1200)) + TXWIGGLEROOM );
	SERIALDELAY_3B = (((7500 * (BAUD_RATE / 1200)) * 3) + TXWIGGLEROOM );
	SERIALDELAY_4B = (((7500 * (BAUD_RATE / 1200)) * 4) + TXWIGGLEROOM );

	/*---------------------------------------*/
	//	  	   	 	  TinyUSB     			 //
	/*---------------------------------------*/
	
	board_init();			// init board from TinyUSB
	tusb_init();			// init TinyUSB

	/*---------------------------------------*/
	//	  	   	 	 Main Loop     			 //
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

				txtimer_target = time_us_32() + SERIALDELAY_4B;
			}
		}

		// Mouse initializing request detected
		if( !cts_pin && mouse_data.pc_state == CTS_LOW_INIT ) {
			mouse_data.pc_state = CTS_TOGGLED;
			mouse_id_nego();
			  
  			txtimer_target = time_us_32() + SERIALDELAY_3B;  // Set initial serial transmit timer target
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
		if( mouse_data.pc_state == CTS_TOGGLED ) {
			// mouse_data.pc_state = CTS_TOGGLED;
			//led_state ^= 1; // Flip state between 0/1 // DEBUG

			tuh_task(); // tinyusb host task

			if ( mouse_connected )
			{
				if( time_reached(txtimer_target) ) {
					post_serial_mouse();
				}
			}
		}

		// we fall in here for the thing (yank out serial cable )
		if( (mouse_data.pc_state != CTS_TOGGLED && mouse_connected) )
		{	
			if ( DEBUG ) { printf("Serial Connection lost"); }

			// Call ALRT Code flasher for Serial Connection Lost. 
			// TODO

		}

	}
}