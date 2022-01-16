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


/*----- Functions -----*/


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
        mouse_data.mouse_conn = true;     // Flag mouse as connected
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
}
*/

// This is executed when a new device is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    switch (itf_protocol) 
    {
        // If we have a mouse!
        case HID_ITF_PROTOCOL_MOUSE:

            // Turn on Alert LED
            // TODO: Flag mouse connected ALRT
            gpio_put(LED_ALERT, 1);   

            // Flag mouse as connected
            mouse_data.mouse_conn = true;     
            
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

    if ( mouse_data.mouse_conn ) {        // IF a mouse was previously connected
        gpio_put(LED_ALERT, 0);     // Turn off Alert LED
        mouse_data.mouse_conn = false;    // Flag mouse as not connected
    }

    //sleep_ms(100);
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
