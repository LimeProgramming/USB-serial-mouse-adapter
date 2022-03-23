#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/uart.h"


#include "utils.h"
#include "ctypes.h"
#include "serial.h"
#include "version.h"
#include <default_config.h>


/*---------------------------------------*/
//             Uart Functions            //
/*---------------------------------------*/
// Functions which handle the uart and it's settings directly

void init_serial_uart()
{   

    // Set up the UART with the required speed.
    uart_init(UART_ID, mouse_data.persistent.baudrate);              

    // Set UART flow control CTS/RTS
    uart_set_hw_flow(UART_ID, false, false);   

    // Turn off crlf conversion
    uart_set_translate_crlf(UART_ID, false); 

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);      

    // Set Double Stop Toggle
    if  ( mouse_data.persistent.doublestopbit ) { uart_set_format(UART_ID, 7, 1, UART_PARITY_NONE); } // 7N2
    else                                        { uart_set_format(UART_ID, 7, 2, UART_PARITY_NONE); } // 7N1
}

// Called when serial settings are updated. 
void refresh_serial_uart()
{
    uart_deinit(UART_ID);   

    busy_wait_us(250000);

    init_serial_uart();

    busy_wait_us(250000);
}

/*---------------------------------------*/
//           Serial Mouse Funcs          //
/*---------------------------------------*/

// Post mouse data over UART. 
void serial_putc(uint8_t *buffer, int size){ 
    for( uint8_t i=0; i <= size; i++ )  { uart_putc_raw( UART_ID, buffer[i] ); } 
}

// Serial mouse negotiation
void serialMouseNego() {
    /*---------------------------------------*/
    //        Serial Mouse negotiation       //
    /*---------------------------------------*/
    // Byte1:Always M                        //
    // Byte2:[None]=MS 3=Logitech Z=MSWheel  // 
    /*---------------------------------------*/

    // Wait for UART to be writable
    while ( !uart_is_writable(UART_ID) ) { sleep_us(mouse_data.serialdelay_1B); }   

    uart_putc_raw( UART_ID, mouse_data.intro_pkts[0] );        // M
    
    if ( mouse_data.persistent.mousetype == THREEBTN ) {
        sleep_us(mouse_data.serialdelay_1B);           
        uart_putc_raw( UART_ID, mouse_data.intro_pkts[1] );    // 3
    }
    else if ( mouse_data.persistent.mousetype == WHEELBTN ) {
        sleep_us(mouse_data.serialdelay_1B);       
        uart_putc_raw( UART_ID, mouse_data.intro_pkts[2] );    // Z
    }

    sleep_us( mouse_data.serialdelay_1B );                     // Wait for packet to send              
}

// Update stored mouse data and post
void postSerialMouse() {
    uint8_t packet[4];                          // Create packet array
    update_mousepacket();                       // Get Current Mouse Data
    if ( !mouse_data.mpkt.update ) { return; }  // Skip if no update.

    packet[0] = ( 0x40 | (mouse_data.mpkt.left ? 0x20 : 0) | (mouse_data.mpkt.right ? 0x10 : 0) | ((mouse_data.mpkt.y >> 4) & 0xC) | ((mouse_data.mpkt.x >> 6) & 0x3));
    packet[1] = ( 0x00 | (mouse_data.mpkt.x & 0x3F)); 
    packet[2] = ( 0x00 | (mouse_data.mpkt.y & 0x3F));

    if ( mouse_data.persistent.mousetype == WHEELBTN ){         // Add Wheel Data + Third Button
        packet[3] = (0x00 | (mouse_data.mpkt.middle ? 0x20 : 0) | (-mouse_data.mpkt.wheel & 0x0f));
        serial_putc(packet,  3);
    }
    else if ( mouse_data.persistent.mousetype == THREEBTN ){    // Add Third Button
        packet[3] = (0x00 | (mouse_data.mpkt.middle ? 0x20 : 0));
        serial_putc(packet,  3);
    }
    else{
        serial_putc(packet,  2);
    }

    if ( DEBUG ){

        // Mouse position.
        printf("Mouse: (%d %d %d)", mouse_data.mpkt.x, mouse_data.mpkt.y, mouse_data.mpkt.wheel);

        printf(" %c%c%c\n",
            mouse_data.mpkt.left   ? 'L' : '-',
            mouse_data.mpkt.middle ? 'M' : '-',
            mouse_data.mpkt.right  ? 'R' : '-');
    }
}



/*---------------------------------------*/
//            Serial Terminal            //
/*---------------------------------------*/

    /*-------------- Variables --------------*/

#define TERM_BUFFER 256         // Should be lots of space.
#define RAW_TERM_BUFFER 10      // 2 should do but just in case. 

volatile bool settingsUpdated = false;      // Flag the need to save settings
volatile bool rebootNeeded = false;         // Flag the need to reboot
volatile bool uartSettingsUpdated = false;  // Flag the need to reboot
volatile int64_t consoleDelay= 10000;       // Force the Uart to run slow to avoid overloading the PC

// Numbers 0 -> 9 
const char validInput[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

const char terminal_header[] =     
    " _  _  ____  ____      ____      ____  ____  ____ \n" \
    "/ )( \\/ ___)(  _ \\ ___(___ \\ ___(___ \\( __ \\(___ \\ \n" \
    ") \\/ (\\___ \\ ) _ ((___)/ __/(___)/ __/ (__ ( / __/ \n" \
    "\\____/(____/(____/    (____)    (____)(____/(____) \n" \
    "USB to Serial Mouse Adapter | By: CalamityLime";

const char terminal_menu_headings[5][30] = 
{
    { "===== Main Menu =====\n" }, { "===== Mouse Travel =====\n" }, { "===== Mouse Buttons =====\n" }, { "===== Serial Settings =====\n" }, { "===== Firmware =====\n" }
};

const char terminal_menus[5][140] =
{
    { "1: Mouse Travel\n2: Mouse Buttons\n3: Serial Settings\n4: Firmware\n0: Exit\n" },
    { "1: List Config\n2: XY Travel\n3: X Travel\n4: Y Travel\n5: Invert X\n6: Invert Y\n7: Movement Type\n8: Cosine Smoothing\n0: Back\n" },
    { "1: List Config\n2: Swap Left and Right\n3: Use Forward and Backward\n4: Swap Forward and Backward\n0: Back\n" },
    { "1: List Config\n2: Format\n3: Baud Rate\n4: Mouse Type\n0: Back\n" },
    { "1: Information\n2: Reset\n3: List Default Settings\n0: Back\n"}
};

const uint8_t terminal_valids[5][8] = 
{
    { 1, 2, 3, 4},
    { 1, 2, 3, 4, 5, 6, 7, 8},
    { 1, 2, 3, 4},
    { 1, 2, 3, 4},
    { 1, 2, 3}
};

const char terminal_guide[] = "Use the numbers to navigate menu. Clear screen: CTRL + L | Menu: CTRL + E\nReminder: Changes saved on exit";

const char terminal_prompt[] = ">>> ";

const char terminal_footer[] = "Returning to mouse mode\n";

const char terminal_invalid[] = "Invalid Input";

    /*------------- Handy Funcs -------------*/

// Write a string to uart
void term_writes_crlf(uart_inst_t * com, const char *s) {   
    int16_t size = (int16_t)strlen(s);

    for (int16_t i = 0; i <= size; i++) {
        if( s[i] == '\0' ) { return; }              // Quit if there's nothing
        else if( s[i] == '\n' ) {                   // Convert to CRLF manually
            uart_putc_raw(com, '\r'); 
            sleep_us(consoleDelay); 
        }

        uart_putc_raw(com, s[i]); 
        sleep_us(consoleDelay);
    } 
}

// Write a single char to uart
void term_writec_crlf(uart_inst_t * com, char c) {      
    
    if( c == '\0' ){ return; }                      // Quit if there's nothing
    else if( c == '\n' ) {                          // Convert to CRLF manually
        uart_putc_raw(com, '\r'); 
        sleep_us(consoleDelay); 
    }

    uart_putc_raw(com, c); 
    sleep_us(consoleDelay); 
}

// Read data from uart
int16_t term_getc(uart_inst_t * com, uint8_t * buffer, int16_t num) { 
    int16_t bytes = 0;
    char i;

    for(int16_t j = 0 ; j < num; j++ ) {
        
        // Exit if UART is unreadable
        if ( !uart_is_readable(com) ) {
            return bytes;
        }
        
        // Pull the UART data
        i = uart_getc(com);

        // Save our data 
        buffer[bytes] = i;
        bytes++;
    } 
  
  return bytes;
}

bool term_menu_option_valid(uint buff, ushort level) {
    
    uint8_t size = sizeof(terminal_valids[level]);  // Is entered menu option valid
    bool valid = false;

    for ( uint8_t i = 0; i < size; i++){
        if ( terminal_valids[level][i] == buff ){
            valid = true;
        }
    }

    return valid;
}

void term_save_settings(uart_inst_t * com) {
    term_writes_crlf(com, "Settings will be saved on exit\n");
    settingsUpdated = true;
}

// Reverse array of chars
void reverse(char s[]) {
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

// Convert int to array of chars
void itoa(int n, char s[]) {
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';

    reverse(s);
}

// Convert array of chars to an int
int atoi(char* str) {

    int res = 0;                                // Initialize result
    int sign = 1;                               // Initialize sign as positive
    int i = 0;                                  // Initialize index of first digit

    if (str[0] == '-') {                        // Update Sign if number if it's neg
        sign = -1;
        i++; }                                  // Also update index of first digit

    for (; str[i] != '\0'; ++i)                 // Iterate through all digit and update the result
        res = res * 10 + str[i] - '0';

    return sign * res;                          // Return result with sign
}

void post_header(uart_inst_t * com) {
    
    term_writes_crlf(com, terminal_header);     // Print the Header for the serial mouse
    term_writec_crlf(com, '\n');                // New Line
    
    // GitHub
    term_writes_crlf(com, "GitHub URL: https://github.com/LimeProgramming/USB-serial-mouse-adapter");
    term_writec_crlf(com, '\n');                // New Line
    term_writes_crlf(com, "Version: ");         // Version
    term_writes_crlf(com, V_FULL);
}

void post_menu(uart_inst_t * com, ushort i){   
    term_writes_crlf(com, terminal_menu_headings[i]);   // Print the menu heading
    term_writes_crlf(com, terminal_menus[i]);           // Print the terminal menu
    term_writec_crlf(com, '\n');                        // New Line
}

void post_prompt(uart_inst_t * com) {
    sleep_ms(50);                                   // Wait a bit for fun
    term_writes_crlf(com, terminal_prompt);         // Cursor prompt
}

void post_invalid(uart_inst_t * com) {   
    term_writes_crlf(com, terminal_invalid);        // Print Invalid input MSG
    term_writec_crlf(com, '\n');                    // New Line
}

int16_t get_option_input(uart_inst_t * com, int16_t min, int16_t max, char msg[], bool baud)
// Can you tell that I didn't plan the serial terminal very well?
{
    uint8_t raw_buff[RAW_TERM_BUFFER] = {0};        // Raw user input
    uint8_t filter_buff[TERM_BUFFER] = {0};         // Filtered user input
    uint filter_buffi = 0;                          // Filtered user input as int
    ushort buff_spot = 0;                           // Keep track of how many chars the user entered

    memset(raw_buff, 0, RAW_TERM_BUFFER);           // Nuke Raw user input
    memset(filter_buff, 0, TERM_BUFFER);            // Nuke Filtered user input
    
    term_writes_crlf(com, msg);             // Repost MSG
    term_writec_crlf(com, '\n');            // New Line
    post_prompt(com);                       // Print the prompt
    
    while(1) {
        // Get user input
        term_getc(com, raw_buff, RAW_TERM_BUFFER);
    
        switch (raw_buff[0]) {

        case 0x08: case 0x7F: // Delete and backspace keys

            if ( buff_spot > 0 ) {   
                term_writes_crlf(com, "\b\x20\b");  // Issue a back space
                buff_spot--;                        // Step back the write pos
                filter_buff[buff_spot] = 0;         // Zero out the last spot
            }
            break;

        case 0x0c: // Handle Clear Screen | CTRL + L
            term_writec_crlf(com, 0x0c);            // Clear Screen

        case 0x05: // Handle Help | CTRL + E

            buff_spot = 0;                          // Reset buffer spot
            memset(filter_buff, 0, TERM_BUFFER);    // Nuke Filtered user input
            term_writes_crlf(com, msg);             // Repost MSG
            term_writec_crlf(com, '\n');            // New Line
            post_prompt(com);                       // Print the prompt

            break;

        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34:
        case 0x35: case 0x36: case 0x37: case 0x38: case 0x39:

            // Prevent an overflow
            if ( buff_spot == (TERM_BUFFER - 1) ) {
                term_writes_crlf(com, "\nBuffer Overflow Error... Press enter to continue\n");
                break;
            }

            filter_buff[buff_spot] = raw_buff[0];   // Store raw buffer value
            buff_spot++;                            // Inc buffer spot
            term_writec_crlf(com, raw_buff[0]);     // Write input back to console

            break;

        case 0x0D:  // Handle the enter

            if ( buff_spot != 0 ) {
                filter_buffi = atoi(filter_buff);   // Convert input to int
                term_writec_crlf(com, '\n');        // New line
                
                if ( baud ) {
                    if ( filter_buffi > 0 && filter_buffi <= 9600 && filter_buffi % 1200 == 0 ) { 
                        return filter_buffi; 
                    }
                    else { return 32000; } 

                } else { 
                    if ( filter_buffi > max || filter_buffi < min ) { return 32000; }
                    else                                            { return filter_buffi; }
                }
            }
            else { return 32000; }

        case 0x00: default: // user didn't type anything or non number
            
            sleep_us(1000);                         // Sleep for a bit
            continue;                               // Continue loop

            break;                                  // Just in case

        }   // End Switch

        memset(raw_buff, 0, RAW_TERM_BUFFER);   // Nuke Raw input

    }   // End While

}

void term_handle_options(uart_inst_t * com, ushort level1, ushort level2) {  
    int16_t ret = 0;                        // Make return var now
    char helpMSG[250] = {0};                // Make our Help MSG string    
    char tmp[20] = {0};                     // TMP string for things                  
    term_writec_crlf(com, '\n');            // New Line

    switch (level1)
    {
    // 1: Mouse Travel   
    case 1:

        switch ( level2 ) {
            case 2: // 2: XY Travel
                memset(tmp, 0, 20); 
                memset(helpMSG, 0, 250);  
                strcpy(helpMSG, "Set both X & Y axis mouse travel. Range: 1 -> 200\x25 | Current value:\x20");
                itoa(mouse_data.persistent.xytravel_percentage, tmp);
                strcat(helpMSG, tmp );
                strcat(helpMSG, "\x25");

                ret = get_option_input(com, 1, 200,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.xytravel_percentage )    { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.xytravel_percentage = ret;
                    term_save_settings(com); 
                    }
                break;
            
            case 3: // 3: X Travel
                memset(tmp, 0, 20); 
                memset(helpMSG, 0, 250);  

                strcpy(helpMSG, "Set X axis mouse travel. Range: 1 -> 200\x25 | Current value:\x20");
                itoa(mouse_data.persistent.xtravel_percentage, tmp);
                strcat(helpMSG, tmp);
                strcat(helpMSG, "\x25");

                ret = get_option_input(com, 1, 200,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.xtravel_percentage )     { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.xtravel_percentage = ret;
                    term_save_settings(com); 
                    }
                break;
            
            case 4: // 4: Y Travel
                memset(tmp, 0, 20); 
                memset(helpMSG, 0, 250);  

                strcpy(helpMSG, "Set Y axis mouse travel. Range: 1 -> 200\x25 | Current value:\x20");
                itoa(mouse_data.persistent.ytravel_percentage, tmp);
                strcat(helpMSG, tmp);
                strcat(helpMSG, "\x25");

                ret = get_option_input(com, 1, 200,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.ytravel_percentage )     { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.ytravel_percentage = ret;
                    term_save_settings(com); 
                    }
                break;

            case 5: // 5: Invert X
                memset(helpMSG, 0, 250);  

                if ( mouse_data.persistent.invert_x ){
                    strcpy(helpMSG, "Set Invert X. 0 -> Disabled | 1 -> Enabled | Currently: Enabled");
                } else {
                    strcpy(helpMSG, "Set Invert X. 0 -> Disabled | 1 -> Enabled | Currently: Disabled");
                }

                ret = get_option_input(com, 0, 1,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.invert_x )               { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.invert_x = ret;
                    term_save_settings(com); 
                    }
                break;
            
            case 6: //6: Invert Y
                memset(helpMSG, 0, 250);  

                if ( mouse_data.persistent.invert_y ){
                    strcpy(helpMSG, "Set Invert Y. 0 -> Disabled | 1 -> Enabled | Currently: Enabled");
                } else {
                    strcpy(helpMSG, "Set Invert Y. 0 -> Disabled | 1 -> Enabled | Currently: Disabled");
                }

                ret = get_option_input(com, 0, 1,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.invert_y )               { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.invert_y = ret;
                    term_save_settings(com); 
                    }
                break;

            case 7: // 7 Movement Type
                memset(helpMSG, 0, 250);  

                term_writes_crlf(com, "Set Movement Type. USB mice talk faster than serial mice, movement type decides what to do with the backlog between serial mouse updates. \n");
                term_writes_crlf(com, "Additive: Sum the mouse movement (Sensitive)\n");
                term_writes_crlf(com, "Average: Avg the mouse movement  (Insensitive)\n");
                term_writes_crlf(com, "Coast: Send the mouse movement incrementally (Slippy)\n");

                switch ( mouse_data.persistent.mouse_movt_type )
                {
                    case MO_MVT_ADDITIVE:
                        strcpy(helpMSG, "0 -> Additive | 1 -> Average | 2 -> Coast | Currently: Additive");
                    break;
                    case MO_MVT_AVERAGE:
                        strcpy(helpMSG, "0 -> Additive | 1 -> Average | 2 -> Coast | Currently: Average");
                    break;
                    case MO_MVT_COAST:
                        strcpy(helpMSG, "0 -> Additive | 1 -> Average | 2 -> Coast | Currently: Coast");
                    break;
                }

                ret = get_option_input(com, 0, 2,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.mouse_movt_type )        { term_writes_crlf(com, "No Change\n"); } 
                else    {
                    mouse_data.persistent.mouse_movt_type = ret;
                    term_save_settings(com); 
                    }
                break;

            case 8: // Cosine smoothing
                memset(helpMSG, 0, 250); 
                
                term_writes_crlf(com, "Set Cosine Smoothing. Makes the cursor proportionally less sensitive at high speeds, leaving the movement mostly one to one at low speeds.\n"); 

                switch ( mouse_data.persistent.use_cosine_smoothing )
                {
                    case MO_MVT_COS_DISABLED:
                        strcpy(helpMSG, "0 -> Disabled | 1 -> Low | 2 -> Medium | 3 -> High | 4 -> Very High\nCurrently: Disabled");
                    break;
                    case MO_MVT_COS_LOW:
                        strcpy(helpMSG, "0 -> Disabled | 1 -> Low | 2 -> Medium | 3 -> High | 4 -> Very High\nCurrently: Low");
                    break;
                    case MO_MVT_COS_MEDIUM:
                        strcpy(helpMSG, "0 -> Disabled | 1 -> Low | 2 -> Medium | 3 -> High | 4 -> Very High\nCurrently: Medium");
                    break;
                    case MO_MVT_COS_HIGH:
                        strcpy(helpMSG, "0 -> Disabled | 1 -> Low | 2 -> Medium | 3 -> High | 4 -> Very High\nCurrently: High");
                    break;
                    case MO_MVT_COS_VERYHIGH:
                        strcpy(helpMSG, "0 -> Disabled | 1 -> Low | 2 -> Medium | 3 -> High | 4 -> Very High\nCurrently: Very High");
                    break;
                }
                ret = get_option_input(com, 0, 4,  helpMSG, false);

                     if ( ret == 32000 )                                            { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.use_cosine_smoothing )       { term_writes_crlf(com, "No Change\n"); } 
                else    {
                    mouse_data.persistent.use_cosine_smoothing = ret;
                    term_save_settings(com); 
                    }
                break;
        }

        break;

    // 2: Mouse Buttons
    case 2:

        switch ( level2 )
        {
            // 2: Swap Left and Right
            case 2:
                memset(helpMSG, 0, 250);  

                if ( mouse_data.persistent.swap_left_right ){
                    strcpy(helpMSG, "Swap Left and Right Clicks. 0 -> Disabled | 1 -> Enabled | Currently: Enabled");
                } else {
                    strcpy(helpMSG, "Swap Left and Right Clicks. 0 -> Disabled | 1 -> Enabled | Currently: Disabled");
                }

                ret = get_option_input(com, 0, 1,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.swap_left_right)         { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.swap_left_right = ret;
                    term_save_settings(com); 
                    }
                break;
            
            // 3: Use Forward and Backward
            case 3:
                memset(helpMSG, 0, 250);  
                
                if ( mouse_data.persistent.use_forward_backward ){
                    strcpy(helpMSG, "Use Forward and Backward buttons as alternative Left and Right buttons.\n0 -> Disabled | 1 -> Enabled | Currently: Enabled");
                } else {
                    strcpy(helpMSG, "Use Forward and Backward buttons as alternative Left and Right buttons.\n0 -> Disabled | 1 -> Enabled | Currently: Disabled");
                }

                ret = get_option_input(com, 0, 1,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.use_forward_backward )   { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.use_forward_backward = ret;
                    term_save_settings(com); 
                    }
                break;
            
            // 4: Swap Forward and Backward
            case 4:
                memset(helpMSG, 0, 250);  

                if ( mouse_data.persistent.swap_forward_backward ){
                    strcpy(helpMSG, "Swap Forward and Backward Buttons\n0 -> Disabled | 1 -> Enabled | Currently: Enabled");
                } else {
                    strcpy(helpMSG, "Swap Forward and Backward Buttons\n0 -> Disabled | 1 -> Enabled | Currently: Disabled");
                }

                ret = get_option_input(com, 0, 1,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.swap_forward_backward )  { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.swap_forward_backward = ret;
                    term_save_settings(com); 
                    }
                break;
        }

        break;

    // 3: Serial Settings
    case 3:

        switch ( level2 )
        {
            // 2: Format
            case 2:
                memset(helpMSG, 0, 250);  

                if ( mouse_data.persistent.doublestopbit ){
                    strcpy(helpMSG, "Set Serial Format | 0 -> 7N1 | 1 -> 7N2 | Currently: 7N2\nNote: Any change will happen upon exit of terminal");
                } else {
                    strcpy(helpMSG, "Set Serial Format | 0 -> 7N1 | 1 -> 7N2 | Currently: 7N1\nNote: Any change will happen upon exit of terminal");
                }

                ret = get_option_input(com, 0, 1,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.doublestopbit )          { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.doublestopbit = ret;
                    uartSettingsUpdated = true;
                    term_save_settings(com); 
                    }
                break;

            // 3: Baud Rate
            case 3:
                memset(helpMSG, 0, 250); 
                memset(tmp, 0, 20); 

                strcpy(helpMSG, "Set Serial Baud Rate | Options: 1200 2400 4800 9600 | Current value:\x20");
                itoa(mouse_data.persistent.baudrate , tmp);
                strcat(helpMSG, tmp);
                strcat(helpMSG, "\nNote: Any change will happen upon exit of terminal");
                strcat(helpMSG, "\nImportant: Modified drivers required for baud rates higher than 1200!");
                
                ret = get_option_input(com, 0, 0,  helpMSG, true);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.baudrate )               { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.baudrate = ret;
                    uartSettingsUpdated = true;
                    term_save_settings(com); 
                    }
                break;
            
            // 4: Mouse Type
            case 4:
                memset(helpMSG, 0, 250);  
                memset(tmp, 0, 20); 

                strcpy(helpMSG, "Set Serial Mouse Type | Currently:\x20");

                // TWOBTN = 0 | THREEBTN = 1 | WHEELBTN = 2 
                     if ( mouse_data.persistent.mousetype == 0 )    { strcat(helpMSG, "Microsoft Two Button"); }
                else if ( mouse_data.persistent.mousetype == 1 )    { strcat(helpMSG, "Logitech Three Button"); }
                else if ( mouse_data.persistent.mousetype == 2 )    { strcat(helpMSG, "Microsoft Wheel"); }

                strcat(helpMSG, "\nOptions: 0 -> MS Two Button | 1 -> Logitech Three Button | 2 -> MS Wheel");
                strcat(helpMSG, "\nImportant: Refresh of mouse driver will be required!");

                ret = get_option_input(com, 0, 2,  helpMSG, false);

                     if ( ret == 32000 )                                        { post_invalid(com); } 
                else if ( ret == mouse_data.persistent.mousetype )              { term_writes_crlf(com, "No Change\n"); }   
                else    { 
                    mouse_data.persistent.mousetype = ret;
                    uartSettingsUpdated = true;
                    //rebootNeeded = true;
                    term_save_settings(com); 
                    }
                break;
        }

        break;

    // 4: Firmware
    case 4:

        switch ( level2 )
        {
        // 2: Reset
        case 2:
            memset(helpMSG, 0, 250);  

            strcpy(helpMSG, "Reset all stored settings to default? | 0 -> No | 1 -> Yes\n");
            strcat(helpMSG, "Note: Doing this will cause the adapter to reboot on exit\n");
            strcat(helpMSG, "Important: Different Firmware images may have different default settings\n");

            ret = get_option_input(com, 0, 1,  helpMSG, false);

                 if ( ret == 32000 )                { post_invalid(com); } 
            else if ( ret == 0 )                    { term_writes_crlf(com, "No Change\n"); }   
            else    { 
                
                loadPersistentSetDefaults();
                term_writes_crlf(com, "Defaults settings loaded\n");
                term_writes_crlf(com, "Settings will be saved on exit\n");
                settingsUpdated = true;
                uartSettingsUpdated = true;
                rebootNeeded = true;
                }
            break;

        // 3: List Default Settings
        case 3:
            memset(tmp, 0, 20); 
            memset(helpMSG, 0, 250);  
            
            // Mouse Travel Settings
            term_writes_crlf(com, "===== Mouse Travel =====\n");
            term_writes_crlf(com, "XY Travel:\x20");
            itoa(default_xytravel_percentage, tmp);
            term_writes_crlf(com, tmp); memset(tmp, 0, 20); 
            term_writes_crlf(com, "\x20| X Travel:\x20");
            itoa(default_xtravel_percentage, tmp);
            term_writes_crlf(com, tmp); memset(tmp, 0, 20); 
            term_writes_crlf(com, "\x20| Y Travel:\x20");
            itoa(default_ytravel_percentage, tmp);
            term_writes_crlf(com, tmp); memset(tmp, 0, 20); 
            term_writes_crlf(com, "\nInverted X Axis:\x20");

            if ( default_invert_x ) { term_writes_crlf(com, "Enabled"); }
            else                    { term_writes_crlf(com, "Disabled"); }

            term_writes_crlf(com, "\x20| Inverted Y Axis:\x20");

            if ( default_invert_y ) { term_writes_crlf(com, "Enabled"); }
            else                    { term_writes_crlf(com, "Disabled"); }
            
            sleep_us(500000);

            // Mouse Buttons Settings
            term_writes_crlf( com, "\n\n===== Mouse Buttons =====\n"  );

            term_writes_crlf( com, "Swap Left and Right:\x20" );
            
            if ( default_swap_left_right )          { term_writes_crlf(com, "Enabled"); }
            else                                    { term_writes_crlf(com, "Disabled"); }

            term_writes_crlf( com, "\nUse Forward and Backward:\x20" );

            if ( default_use_forward_backward )     { term_writes_crlf(com, "Enabled"); }
            else                                    { term_writes_crlf(com, "Disabled"); }

            term_writes_crlf( com, "\nSwap Forward and Backward:\x20" );

            if ( default_swap_forward_backward )    { term_writes_crlf(com, "Enabled"); }
            else                                    { term_writes_crlf(com, "Disabled"); }

            sleep_us(500000);

            // Serial Settings 
            term_writes_crlf( com, "\n\n===== Serial Settings =====\n" );
            term_writes_crlf( com, "Serial Format:\x20" );
            
            if ( default_doublestopbit )            { term_writes_crlf(com, "7N2"); }
            else                                    { term_writes_crlf(com, "7N1"); }

            term_writes_crlf( com, "\nBaud Rate:\x20" );

                 if ( default_baudrate  == 1200 )   { term_writes_crlf(com, "1200"); }
            else if ( default_baudrate  == 2400 )   { term_writes_crlf(com, "2400"); }
            else if ( default_baudrate  == 4800 )   { term_writes_crlf(com, "4800"); }
            else if ( default_baudrate  == 9600 )   { term_writes_crlf(com, "9600"); }
            else                                    { term_writes_crlf(com, "1200"); }

            term_writes_crlf( com, "\nMouse Type:\x20");

                 if  ( default_mousetype == 0 )     { term_writes_crlf(com, "Microsoft Two Button"); }
            else if  ( default_mousetype == 1 )     { term_writes_crlf(com, "Logitech Three Button"); }
            else if  ( default_mousetype == 2 )     { term_writes_crlf(com, "Microsoft Wheel"); }

            term_writec_crlf( com, '\n');
            sleep_us(500000);
            break;
        }
        break;

    default:
        break;
    }

    term_writec_crlf(com, '\n');            // New Line

    return;
}


void serial_terminal(uart_inst_t * com, uint64_t ddd) {

    consoleDelay = ddd;
    
    // ===== Variables
    char tmp_char[20];
    ushort buff_spot = 0;
    uint term_bufferi = 0;                              // Term Buffer Int
    ushort menu_level = 0;
    uint8_t raw_input_buffer[RAW_TERM_BUFFER] = {0};    // Our raw user input
    uint8_t term_buffer[TERM_BUFFER] = {0};             // Filtered uart data
    bool runLoop = true;                                // Keep our while loop runnng

    // ===== Prep
    memset(raw_input_buffer, 0, RAW_TERM_BUFFER);   // Nuke Raw user input
    memset(term_buffer, 0, TERM_BUFFER);            // Nuke Filtered uart data
    settingsUpdated = false;                        // Set our flags to false
    rebootNeeded = false;                           // Set our flags to false
    uartSettingsUpdated = false;                    // Set our flags to false

    // ===== Start Printing
    term_writec_crlf(com, '\n');                    // New Line
    post_header(com);                               // Print the Header for the serial mouse
    term_writes_crlf(com, "\n\n");                  // New Line
    term_writes_crlf(com, terminal_guide);          // Print the little guide
    term_writec_crlf(com, '\n');                    // New Line
    post_menu(com, menu_level);                     // Print the terminal menu
    post_prompt(com);                               // Cursor prompt

    while(runLoop) {
        
        term_getc(com, raw_input_buffer, RAW_TERM_BUFFER);  // Read our data from com

        switch (raw_input_buffer[0]) {

        case 0x08: case 0x7F: // Delete and backspace keys
            if ( buff_spot > 0 ) {   
                
                term_writes_crlf(com, "\b\x20\b");      // Issue a back space
                buff_spot--;                            // Step back the write pos
                term_buffer[buff_spot] = 0;             // Zero out the last spot
            }
            break;

        case 0x0c: // Handle Clear Screen | CTRL + L

            term_writec_crlf(com, 0x0c);                // Clear Screen
            post_header(com);                           // Print the Header for the serial mouse
            term_writes_crlf(com, "\n\n");              // New Line
            term_writes_crlf(com, terminal_guide);      // Print the little guide.

        case 0x05: // Handle Help | CTRL + E

            buff_spot = 0;                              // Reset buffer spot
            memset(term_buffer, 0, TERM_BUFFER);        // Our filtered uart data   
            term_writec_crlf(com, '\n');                // New Line
            post_menu(com, menu_level);                 // Reprint menu
            post_prompt(com);                           // Print the prompt

            break;

        case 0x0D:  // Handle the enter

            term_bufferi = atoi(term_buffer);           // Convert input to int

            if ( buff_spot > 0 )
            {   
                term_writec_crlf(com, '\n');            // New Line

                // Handle our dedicated back button
                if ( term_bufferi == 0 ) {

                    // Return to mouse mode
                    if ( menu_level == 0 ){
                        runLoop = false;                // Break our while loop
                        break;                          // Return to mouse

                    // Root of a sub menu -> return to main menu
                    } else {
                        menu_level = 0;
                    }           

                // Handle List Config | Number one button
                } else if ( term_bufferi == 1 && menu_level != 0) {

                    // Mouse Travel settings.
                    if ( menu_level == 1 ) {
                        
                        memset(tmp_char, 0, 20);
                        itoa(mouse_data.persistent.xytravel_percentage, tmp_char);

                        term_writes_crlf(com, "XY Travel:\x20");
                        term_writes_crlf(com, tmp_char);
                        term_writec_crlf(com, 0x25);
                        term_writec_crlf(com, 0x0A);

                        memset(tmp_char, 0, 20);
                        itoa(mouse_data.persistent.xtravel_percentage, tmp_char);

                        term_writes_crlf(com, "X Travel:\x20");
                        term_writes_crlf(com, tmp_char);
                        term_writec_crlf(com, 0x25);
                        term_writec_crlf(com, 0x0A);


                        memset(tmp_char, 0, 20);
                        itoa(mouse_data.persistent.ytravel_percentage, tmp_char);

                        term_writes_crlf(com, "Y Travel:\x20");
                        term_writes_crlf(com, tmp_char);
                        term_writec_crlf(com, 0x25);
                        term_writec_crlf(com, 0x0A);


                        if ( mouse_data.persistent.invert_x )   { term_writes_crlf(com, "Invert X: True\n" ); }
                        else                                    { term_writes_crlf(com, "Invert X: False\n" ); }

                        if ( mouse_data.persistent.invert_y )   { term_writes_crlf(com, "Invert Y: True\n" ); }
                        else                                    { term_writes_crlf(com, "Invert Y: False\n" ); }
                        
                        switch ( mouse_data.persistent.mouse_movt_type)
                        {      
                            case MO_MVT_ADDITIVE:
                                term_writes_crlf(com, "Movement Type: Additive\n" );
                            break;
                            case MO_MVT_AVERAGE:
                                term_writes_crlf(com, "Movement Type: Average\n" );
                            break;
                            case MO_MVT_COAST:
                                term_writes_crlf(com, "Movement Type: Coast\n" );
                            break;
                        }

                        switch ( mouse_data.persistent.use_cosine_smoothing )
                        {
                            case MO_MVT_COS_DISABLED:
                                term_writes_crlf(com, "Cosine Smoothing: False\n" );
                            break;
                            case MO_MVT_COS_VERYHIGH:
                                term_writes_crlf(com, "Cosine Smoothing: Very High\n" );
                            break;
                            case MO_MVT_COS_HIGH:
                                term_writes_crlf(com, "Cosine Smoothing: High\n" );
                            break;
                            case MO_MVT_COS_MEDIUM:
                                term_writes_crlf(com, "Cosine Smoothing: Medium\n" );
                            break;
                            case MO_MVT_COS_LOW:
                                term_writes_crlf(com, "Cosine Smoothing: Low\n" );
                            break;

                        }

                        // New Line
                        term_writec_crlf(com, '\n');
                    } 

                    // Mouse Buttons settings.
                    else if ( menu_level == 2 ) {

                        if ( mouse_data.persistent.swap_left_right )        { term_writes_crlf(com, "Swap Left and Right: True\n"); }
                        else                                                { term_writes_crlf(com, "Swap Left and Right: False\n"); }
                        
                        if ( mouse_data.persistent.use_forward_backward )   { term_writes_crlf(com, "Use Forward and Backward: True\n"); }
                        else                                                { term_writes_crlf(com, "Use Forward and Backward: False\n"); }

                        if ( mouse_data.persistent.swap_forward_backward )  { term_writes_crlf(com, "Swap Forward and Backward: True\n"); }
                        else                                                { term_writes_crlf(com, "Swap Forward and Backward: False\n"); }

                        // New Line
                        term_writec_crlf(com, '\n');
                    } 
                    
                    // Serial Settings
                    else if ( menu_level == 3 ) { 
                        if ( mouse_data.persistent.doublestopbit )          { term_writes_crlf(com, "Format: 7N2\n"); }
                        else                                                { term_writes_crlf(com, "Format: 7N1\n"); }

                        memset(tmp_char, 0, 20);
                        itoa(mouse_data.persistent.baudrate, tmp_char);
                        term_writes_crlf(com, "Baud Rate:\x20");
                        term_writes_crlf(com, tmp_char);
                        term_writec_crlf(com, '\n');

                        term_writes_crlf(com, "Mouse Type:\x20");
                             if ( mouse_data.persistent.mousetype == 0 )    { term_writes_crlf(com, "Microsoft Two Button\n"); }
                        else if ( mouse_data.persistent.mousetype == 1 )    { term_writes_crlf(com, "Logitech Three Button\n"); }
                        else if ( mouse_data.persistent.mousetype == 2 )    { term_writes_crlf(com, "Microsoft Wheel\n"); }
                        
                        term_writec_crlf(com, '\n');                        // New Line
                    } 
                    
                    // Firmware Settings
                    else if ( menu_level == 4 ) {
                        post_header(com);                                   // Print the Header for the serial mouse
                        term_writes_crlf(com, "\n\n");                      // New line and line break
                        term_writes_crlf(com, "Thank you for using my firmware, I hope you like it.\n-Lime");
                        term_writes_crlf(com, "\n\n");
                    }
                } 
                
                // Handle any number other than zero
                else {
                    bool valid = term_menu_option_valid(term_bufferi, menu_level);        // Is entered menu option valid

                    if ( valid ) {
                        if ( menu_level == 0 )  { menu_level = term_bufferi; } 
                        else                    { term_handle_options(com, menu_level, term_bufferi); }

                    }
                    
                    else { post_invalid(com); }     // If not valid, post invalid MSG
                }
                
                post_menu(com, menu_level);         // Reprint menu
            }

            else  { term_writec_crlf(com, '\n'); }

            buff_spot = 0;                          // Reset buffer spot
            memset(term_buffer, 0, TERM_BUFFER);    // Nuke filtered uart data
            post_prompt(com);                       // Print the prompt
            //

            break;

        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34:
        case 0x35: case 0x36: case 0x37: case 0x38: case 0x39:
            
            // Prevent an overflow
            if ( buff_spot == (TERM_BUFFER - 1) ) {
                term_writes_crlf(com, "\nBuffer Overflow Error... Press enter to continue\n");
                break;
            }
            term_buffer[buff_spot] = raw_input_buffer[0];   // Store raw buffer value
            buff_spot++;                                    // Inc buffer spot
            term_writec_crlf(com, term_buffer[0]);          // Write input back to console

            break;

        case 0x00: default: // user didn't type anything or non number
            
            sleep_us(1000);                         // Sleep for a bit
            continue;                               // Continue loop
            break;                                  // Just in case

        }   // End switch

        memset(raw_input_buffer, 0, RAW_TERM_BUFFER);   // Nuke Raw input

    } // End While


    term_writec_crlf(com, '\n');

    if ( settingsUpdated ) {
        savePersistentSet();
        term_writes_crlf(com, "Changes saved to flash memory");
        term_writec_crlf(com, '\n');
    }
    
    term_writes_crlf(com, terminal_footer);

    if ( uartSettingsUpdated ) {
        refresh_serial_uart();      // Reinit serial with updated settings
        calcSerialDelay();          // Recalulate serial delay used by the main mouse timer 
    }

    if (rebootNeeded ) {
        machine_reboot();
    }

    return;
}