#include "tusb.h"
#include "pico.h"
#include <stdio.h>
#include <stdlib.h>
#include "bsp/board.h"
#include "pico/stdlib.h"

#include "utils.h"
#include "ctypes.h"
#include "hid_app.h"
#include <default_config.h>

/*---------------------------------------*/
//             TinyUSB Stuff             //
/*---------------------------------------*/

/*----- Memory -----*/

CFG_TUSB_MEM_SECTION static char serial_in_buffer[64] = { 0 };

static uint8_t const keycode2ascii[128][2] =  { HID_KEYCODE_TO_ASCII };


/*---------------------------------------*/
//           TinyUSB Callbacks           //
/*---------------------------------------*/

// ==================================================
// ---------- This is executed when a new device is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
    switch ( tuh_hid_interface_protocol(dev_addr, instance) ) 
    {
        // If we have a mouse!
        case HID_ITF_PROTOCOL_MOUSE:

            // Turn on Alert LED
            // TODO: Flag mouse connected ALRT
            gpio_put(LED_ALERT, 1);   

            // Increment our mouse counter
            ++mouse_data.mouse_count;
            
            break;

        // If we have a keyboard!
        case HID_ITF_PROTOCOL_KEYBOARD:
            break;


        // Process HID Report and hope it's a mouse
        case HID_ITF_PROTOCOL_NONE:    
            // By default host stack will use activate boot protocol on supported interface.
            hid_info[instance].report_count = tuh_hid_parse_report_descriptor(hid_info[instance].report_info, MAX_HID_REPORT, desc_report, desc_len);
            // TODO: Flag mouse unsure ALRT
            break;
        
        // Process Generic Report
        default:                        
            // TODO: Flag incompatible ALRT
            break;
    }
    
    // Manually tell TinyUSB that we do actually want data from the connected USB device
    // I guess only weirdos want their conneced USB device to do something ¯\_(ツ)_/¯
    const bool claim_endpoint = tuh_hid_receive_report(dev_addr, instance);


    #if DEBUG > 0

    // ---------- Print out the type of device connected
    const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
    const uint8_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
    printf("HID device with address %d, instance %d, protocol %d, is a %s, has mounted.\r\n", dev_addr, instance, itf_protocol, protocol_str[itf_protocol]); 

    // ---------- Print out for bad USB device.
    if ( !claim_endpoint ) {
        printf("Error: cannot request to receive report\r\n");
    }

    #endif

    return;
}

// ==================================================
// ---------- This is executed when a device is unmounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{   
    switch ( tuh_hid_interface_protocol(dev_addr, instance) )
    {
        case HID_ITF_PROTOCOL_MOUSE:

            // Deincrement the mouse counter
            --mouse_data.mouse_count;

            if ( mouse_data.mouse_count <= 0 )
            {   
                // Make sure mouse count is actually zero
                mouse_data.mouse_count = 0;

                // Turn off mouse connected LED
                gpio_put(LED_ALERT, 0);
            }

        break;
    }

    #if DEBUG

    printf("HID device with address %d, instance %d was unmounted.\r\n", dev_addr, instance);

    #endif
    
    // Old bug fix for TinyUSB, it liked to crash when something was disconnected.
    //machine_reboot();               // There's a bug in TinyUSB, a reboot should bypass it
}

// ==================================================
// ---------- This is executed when data is received from the mouse
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{   
    switch (tuh_hid_interface_protocol(dev_addr, instance)) 
    {   
         // Process Mouse Report
        case HID_ITF_PROTOCOL_MOUSE:   
            process_mouse_report((hid_mouse_report_t const*) report );
            break;

        // Throw Away Keyboard Reports
        case HID_ITF_PROTOCOL_KEYBOARD: 
            break;

        // Process Generic Report
        default:                        
            process_generic_report(dev_addr, instance, report, len);
            break;
    }

    // Manually tell TinyUSB that we do actually want data from the connected USB device
    // I guess only weirdos want their conneced USB device to do something ¯\_(ツ)_/¯
    const bool claim_endpoint = tuh_hid_receive_report(dev_addr, instance);

    #if DEBUG > 0

    // ---------- Print out for bad USB device.
    if ( !claim_endpoint ) {
        printf("Error: cannot request to receive report\r\n");
    }

    #endif

}


/*---------------------------------------*/
//             HID Processors            //
/*---------------------------------------*/

// ==================================================
// ---------- Handle generic USB Report
void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) 
{   
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
            
        report++;   
        len--;
    }
    
    // If the Hid usage isn't something we care about
    if ( rpt_info->usage_page != HID_USAGE_PAGE_DESKTOP ) { return; }

    
    // For complete list of Usage Page & Usage checkout src/class/hid/hid.h. && Assume mouse follow boot report layout
    switch ( rpt_info->usage ) 
    {
        case HID_USAGE_DESKTOP_MOUSE:

            if ( mouse_data.mouse_count < 1 ){
                // Increment the mouse counter
                ++mouse_data.mouse_count;

                // Turn on Alert LED
                gpio_put(LED_ALERT, 1);       
            }

            // Process the hopefully mouse report
            process_mouse_report((hid_mouse_report_t const*) report); 
        break;

        case HID_USAGE_DESKTOP_KEYBOARD:
        break;
    }

}


//--------------------------------------------------------------------+
// Keyboard Testing
//--------------------------------------------------------------------+

// look up new key in previous keys
static inline bool find_key_in_report(hid_keyboard_report_t const *report, uint8_t keycode)
{
  for(uint8_t i=0; i<6; i++)
  {
    if (report->keycode[i] == keycode)  return true;
  }

  return false;
}

static void process_kbd_report(hid_keyboard_report_t const *report)
{
  static hid_keyboard_report_t prev_report = { 0, 0, {0} }; // previous report to check key released

  //------------- example code ignore control (non-printable) key affects -------------//
  for(uint8_t i=0; i<6; i++)
  {
    if ( report->keycode[i] )
    {
      if ( find_key_in_report(&prev_report, report->keycode[i]) )
      {
        // exist in previous report means the current key is holding
      }else
      {
        // not existed in previous report means the current key is pressed
        bool const is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
        uint8_t ch = keycode2ascii[report->keycode[i]][is_shift ? 1 : 0];
        putchar(ch);
        if ( ch == '\r' ) putchar('\n'); // added new line for enter key

        fflush(stdout); // flush right away, else nanolib will wait for newline
      }
    }
    // TODO example skips key released
  }

  prev_report = *report;
}