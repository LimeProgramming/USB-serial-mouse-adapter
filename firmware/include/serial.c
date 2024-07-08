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

void init_serial_uart(uint data_bits)
{   
    // Set up the UART with the required speed.
    mouse_data.realbaudrate = uart_init(UART_ID, mouse_data.persistent.baudrate);        

    // Set UART flow control CTS/RTS
    uart_set_hw_flow(UART_ID, false, false);   

    // Turn off crlf conversion
    uart_set_translate_crlf(UART_ID, false); 

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);      

    // Set the serial Format
    set_serial_data(data_bits);

    #if DEBUG >0
    // Set up the UART with the required speed.    
    printf("Wanted Baud: %d, Real Baud: %d\n", mouse_data.persistent.baudrate, mouse_data.realbaudrate);  
    #endif
}

void set_serial_data(uint data_bits)
{
    // Set Double Stop Toggle
  if  ( mouse_data.persistent.doublestopbit ) { uart_set_format(UART_ID, 7, 2, UART_PARITY_NONE); } // 7N2
  else                                        { uart_set_format(UART_ID, 7, 1, UART_PARITY_NONE); } // 7N1
}
// Called when serial settings are updated. 
void refresh_serial_uart()
{
    uart_deinit(UART_ID);   

    busy_wait_us(250000);

    init_serial_uart(7);

    busy_wait_us(250000);
}

/*---------------------------------------*/
//           Serial Mouse Funcs          //
/*---------------------------------------*/

/* ---------------------------------------------------------- */
// Mouse ID Strings

// Basic MS Mouse
uint8_t ID_Microsoft[4] = { 0x01, 0x4D, 0x00, 0x00 };

// Logitech mouse 'M3' + PNP
uint8_t ID_Logitech[2][8] = {
  { 0x0D, 0x4D, 0x33, 0x08, 0x01, 0x24, 0x2C, 0x27 },
  { 0x29, 0x18, 0x10, 0x10, 0x11, 0x09, 0x00, 0x00 } };

//{ 0x52, 0x4D, 0x5A, 0x40, 0x00, 0x00, 0x00, 0x08 },

// MS Wheel mouse 'MZ@' + PNP
uint8_t ID_Wheelmouse[11][8] = {
	{ 0x52, 0x4D, 0x5A, 0x40, 0x00, 0x00, 0x00, 0x00 },
	{ 0x01, 0x24, 0x2D, 0x33, 0x28, 0x10, 0x10, 0x10 },
	{ 0x11, 0x3C, 0x10, 0x10, 0x10, 0x14, 0x10, 0x12 },
	{ 0x10, 0x10, 0x3C, 0x2D, 0x2F, 0x35, 0x33, 0x25 },
	{ 0x3C, 0x30, 0x2E, 0x30, 0x10, 0x26, 0x10, 0x21 },
	{ 0x3C, 0x2D, 0x29, 0x23, 0x32, 0x2F, 0x33, 0x2F },
	{ 0x26, 0x34, 0x00, 0x29, 0x2E, 0x34, 0x25, 0x2C },
	{ 0x2C, 0x29, 0x2D, 0x2F, 0x35, 0x33, 0x25, 0x00 },
	{ 0x0D, 0x00, 0x33, 0x25, 0x32, 0x29, 0x21, 0x2C },
	{ 0x00, 0x36, 0x25, 0x32, 0x33, 0x29, 0x2F, 0x2E },
	{ 0x15, 0x16, 0x09, 0x00 } };

// Post mouse data over UART. 
void serial_putc(uint8_t *buffer, int size){ 
    for( uint8_t i=0; i <= size; i++ )  { 
        uart_putc_raw( UART_ID, buffer[i] ); 
    } 
}

void serialMouseNego_old() {

    /*---------------------------------------*/
    //        Serial Mouse negotiation       //
    /*---------------------------------------*/
    // Byte1:Always M                        //
    // Byte2:[None]=MS 3=Logitech Z=MSWheel  // 
    /*---------------------------------------*/

    // Wait for UART to be writable
    while ( !uart_is_writable(UART_ID) ) { sleep_us(mouse_data.serialdelay_1B); }   

    switch ( mouse_data.persistent.mousetype )
    {
      case TWOBTN:
        uart_putc_raw( UART_ID, 0x4D);
      break;

      case THREEBTN:
        // Set Serial data bits to 8 for the pnp info
        set_serial_data(8);

        for( uint8_t i=0; i < 2; i++ ) { 
          serial_putc(ID_Logitech[i], 7); 
          sleep_us(mouse_data.serialdelay_1B); 
        }

        // Set back to 7 bits for mouse movement
        set_serial_data(7);
      break;

      case WHEELBTN:
        // Set Serial data bits to 8 for the pnp info
        set_serial_data(8);

        for( uint8_t i=0; i < 1; i++ )
        {
          for( uint8_t j=0; j < 8; j++ ) { 
            uart_putc_raw( UART_ID, ID_Wheelmouse[i][j] ); 
            sleep_us(mouse_data.serialdelay_1B);
            }

          sleep_us(mouse_data.serialdelay_1B);
        }

        // Send the last 4 characters
        //for( uint8_t i=0; i < 4; i++ ) { uart_putc_raw( UART_ID, ID_Wheelmouse[10][i] ); }

        // Set back to 7 bits for mouse movement
        set_serial_data(7);
        
      break;
    }

    sleep_us(mouse_data.serialdelay_1B);
}


void serialMouseNego() {

    /*---------------------------------------*/
    //        Serial Mouse negotiation       //
    /*---------------------------------------*/
    // Byte1:Always M                        //
    // Byte2:[None]=MS 3=Logitech Z=MSWheel  // 
    /*---------------------------------------*/

    // Wait for UART to be writable
    while ( !uart_is_writable(UART_ID) ) { tight_loop_contents(); }   

    switch ( mouse_data.persistent.mousetype ) {
      // Basic MS Mouse
      case TWOBTN:
        serial_putc( "\x01\x4D\x00\x00\x00\x00\x00\x00", 7); 

      break;

      // Logitech mouse 'M3' + PNP
      case THREEBTN:
        serial_putc( "\x0D\x4D\x33\x08\x01\x24\x2C\x27", 7); 
        serial_putc( "\x29\x18\x10\x10\x11\x09\x00\x00", 7);

      break;

      // MS Wheel mouse 'MZ@' + PNP
      case WHEELBTN:
        serial_putc("\x52\x4D\x5A\x40\x00\x00\x00\x00", 7);
        //serial_putc("\x01\x24\x2D\x33\x28\x10\x10\x10", 7);
        //serial_putc("\x11\x3C\x10\x10\x10\x14\x10\x12", 7);
        //serial_putc("\x10\x10\x3C\x2D\x2F\x35\x33\x25", 7);
        //serial_putc("\x3C\x30\x2E\x30\x10\x26\x10\x21", 7);
        //serial_putc("\x3C\x2D\x29\x23\x32\x2F\x33\x2F", 7);
        //serial_putc("\x26\x34\x00\x29\x2E\x34\x25\x2C", 7);
        //serial_putc("\x2C\x29\x2D\x2F\x35\x33\x25\x00", 7);
        //serial_putc("\x0D\x00\x33\x25\x32\x29\x21\x2C", 7);
        //serial_putc("\x00\x36\x25\x32\x33\x29\x2F\x2E", 7);
        //serial_putc("\x15\x16\x09\x00\x00\x00\x00\x00", 7);

      break;
    }

    busy_wait_us(mouse_data.serialdelay_1B);
}

void printfMousePacket() {
  printf("Mouse: (%d %d %d)", mouse_data.mpkt.x, mouse_data.mpkt.y, mouse_data.mpkt.wheel);
  fflush(stdout);

  printf(" %c%c%c\n",
      mouse_data.mpkt.left   ? 'L' : '-',
      mouse_data.mpkt.middle ? 'M' : '-',
      mouse_data.mpkt.right  ? 'R' : '-');
  fflush(stdout);

  return;
}

// Update stored mouse data and post
void postSerialMouse() {
    uint8_t packet[4];                          // Create packet array
    if ( !mouse_data.mpkt.update ) { return; }  // Skip if no update.

    packet[0] = ( 0x40 | (mouse_data.mpkt.left ? 0x20 : 0) | (mouse_data.mpkt.right ? 0x10 : 0) | ((mouse_data.mpkt.y >> 4) & 0xC) | ((mouse_data.mpkt.x >> 6) & 0x3));
    packet[1] = ( 0x00 | (mouse_data.mpkt.x & 0x3F)); 
    packet[2] = ( 0x00 | (mouse_data.mpkt.y & 0x3F));

    // according to: https://sourceforge.net/p/cutemouse/trunk/ci/master/tree/cutemouse/PROTOCOL.TXT
    // for a wheel mouse, the middle button should be reported in bit 0x10
    // for a 3-button mouse, the middle button should be reported in bit 0x20
    if ( mouse_data.persistent.mousetype == WHEELBTN ){         // Add Wheel Data + Third Button
        packet[3] = (0x00 | (mouse_data.mpkt.middle ? 0x10 : 0) | (-mouse_data.mpkt.wheel & 0x0f));
        serial_putc(packet,  3);
    }
    else if ( mouse_data.persistent.mousetype == THREEBTN ){    // Add Third Button
        packet[3] = (0x00 | (mouse_data.mpkt.middle ? 0x20 : 0));
        serial_putc(packet,  3);
    }
    else{
        serial_putc(packet,  2);
    }

    /* ---------------------------------------- */
    // Debug Print Out
    #if DEBUG > 0
    printfMousePacket();
    #endif
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

const char terminal_menu_headings[2][5][36] = { 
{ // English
  { "===== Main Menu =====\n" }, { "===== Mouse Travel =====\n" }, { "===== Mouse Buttons =====\n" }, { "===== Serial Settings =====\n" }, { "===== Firmware =====\n" }
},
{ // German
  { "===== Hauptmenü =====\n" }, { "===== Mausbewegung =====\n" }, { "===== Maustasten =====\n" }, { "===== Seriell-Einstellungen =====\n" }, { "===== Firmware =====\n" }
}
};

const char terminal_menus[2][5][248] = {
{ // English
  { "1: Mouse Travel\n2: Mouse Buttons\n3: Serial Settings\n4: Firmware\n0: Exit\n" },
  { "1: List Config\n2: XY Travel\n3: X Travel\n4: Y Travel\n5: Invert X\n6: Invert Y\n7: Movement Type\n8: Cosine Smoothing\n0: Back\n" },
  { "1: List Config\n2: Swap Left and Right\n3: Use Forward and Backward\n4: Swap Forward and Backward\n0: Back\n" },
  { "1: List Config\n2: Format\n3: Baud Rate\n4: Mouse Type\n0: Back\n" },
  { "1: Information\n2: Reset\n3: Language\n4: List Default Settings\n0: Back\n"}
},

{ // German
  { "1: Mausbewegung\n2: Maustasten\n3: Seriell-Einstellungen\n4: Firmware\n0: Beenden\n" },
  { "1: Konfiguration anzeigen\n2: Zeigergeschwindigkeit in XY Richtung\n3: Zeigergeschwindigkeit in X Richtung\n4: Zeigergeschwindigkeit in Y Richtung\n5: X Invertieren\n6: Y Invertieren\n7: Bewegungsmodus\n8: Dynamische Empfindlichkeit\n0: Zurück\n" },
  { "1: Konfiguration anzeigen\n2: Links & Rechts tauschen\n3: Seitliche Knöpfe für Links und Rechts verwenden\n4: Seitliche Knöpfe tauschen\n0: Zurück\n" },
  { "1: Konfiguration anzeigen\n2: Stop-bit Format\n3: BaudRate\n4: Maustyp\n0: Zurück\n" },
  { "1: Information\n2: Zurücksetzen\n3: Sprache\n4: Standardeinstellungen anzeigen\n0: Zurück\n"}
}
};

const uint8_t terminal_valids[5][8] = {
  { 1, 2, 3, 4},
  { 1, 2, 3, 4, 5, 6, 7, 8},
  { 1, 2, 3, 4},
  { 1, 2, 3, 4},
  { 1, 2, 3, 4}
};

const char terminal_guide[2][158] = {
{ // English
  "Use the numbers to navigate menu. Clear screen: CTRL + L | Menu: CTRL + E\nReminder: Changes saved on exit"
},
{ // German
  "Nummer des Eintrags tippen, um im Menü zu navigieren.\nBildschirm löschen: STRG + L | Menü: STRG + E\nHinweis: Änderungen werden beim Beenden gespeichert\n"
}
};

const char terminal_prompt[] = ">>> ";

const char terminal_list_options[2][4][7][50] = {
{ // English
  { // Mouse Travel
    {"XY Travel"},
    {"X Travel"},
    {"Y Travel"},
    {"Inverted X Axis"},
    {"Inverted Y Axis"},
    {"Movement Type"},
    {"Cosine Smoothing"}
  },
  { // Mouse Buttons
    {"Swap Left and Right"},
    {"Use Forward and Backward"},
    {"Swap Forward and Backward"}
  },
  { // Serial Settings
    {"Serial Format"},
    {"Baud Rate"},
    {"Mouse Type"}
  },
  { // Firmware
    {"Language"}
  }
},
{ // German
  { // Mouse Travel
    {"Zeigergeschwindigkeit in XY Richtung"},
    {"Zeigergeschwindigkeit in X Richtung"},
    {"Zeigergeschwindigkeit in Y Richtung"},
    {"X Invertieren"},
    {"Y Invertieren"},
    {"Bewegungsmodus"},
    {"Dynamische Empfindlichkeit"}
  },
  { // Mouse Buttons
    {"Links & Rechts tauschen"},
    {"Seitliche Knöpfe für Links und Rechts verwenden"},
    {"Seitliche Knöpfe tauschen"}
  },
  { // Serial Settings
    {"Serielle Stop-bit Format"},
    {"BaudRate"},
    {"Maustyp"}
  },
  { // Firmware
    {"Sprache"}
  }
}
};


const char terminal_handle_options[2][4][10][185] = {
{ // English
  { // Mouse Travel
    { "Set both X & Y axis mouse travel. Range: 1 -> 200\x25 | Current value:\x20" },
    { "Set X axis mouse travel. Range: 1 -> 200\x25 | Current value:\x20" },
    { "Set Y axis mouse travel. Range: 1 -> 200\x25 | Current value:\x20" },
    { "Set Invert X. 0 -> Disabled | 1 -> Enabled | Currently:\x20" },
    { "Set Invert Y. 0 -> Disabled | 1 -> Enabled | Currently:\x20" },
    { "USB mice talk faster than serial mice,\nmovement type decides what to do with the buffered mouse updates.\n" },
    { "Additive: Sum the mouse movement (Sensitive)\nAverage: Avg the mouse movement (Insensitive)\nCoast: Send the mouse movement incrementally (Slippy)\n"},
    { "0 -> Additive | 1 -> Average | 2 -> Coast | Currently:\x20" },
    { "Set Cosine Smoothing. Makes the cursor proportionally less sensitive at high speeds, leaving the movement mostly one to one at low speeds.\n" },
    { "0 -> Disabled | 1 -> Low | 2 -> Medium | 3 -> High | 4 -> Very High\nCurrently:\x20" }
  },

  { // Mouse Buttons
    { "Swap Left and Right Clicks. 0 -> Disabled | 1 -> Enabled | Currently:\x20" },
    { "Use Forward and Backward buttons as alternative Left and Right buttons.\n0 -> Disabled | 1 -> Enabled | Currently:\x20" },
    { "Swap Forward and Backward Buttons\n0 -> Disabled | 1 -> Enabled | Currently:\x20" }
  },

  { // Serial Settings
    { "Set Serial Format | 0 -> 7N1 | 1 -> 7N2 | Currently:\x20" },
    { "Set Serial Baud Rate | Options: 1200 2400 4800 9600 19200 | Current value:\x20" },
    { "Set Serial Mouse Type | Currently:\x20" },
    { "\nOptions: 0 -> MS Two Button | 1 -> Logitech Three Button | 2 -> MS Wheel" }
  },

  { // Firmware
    { "Reset all stored settings to default? | 0 -> No | 1 -> Yes\n" },
    { "Set terminal language | 0 -> English | 1 -> German\n" }
  }
},
{ // German
  {
    { "Setzt die Zeigergeschwindigkeit auf der X & Y-Achse.\nMöglich: 1 -> 200\x25 | Aktueller Wert:\x20" },
    { "Setzt die Zeigergeschwindigkeit auf der X-Achse.\nMöglich: 1 -> 200\x25 | Aktueller Wert:\x20" },
    { "Setzt die Zeigergeschwindigkeit auf der Y-Achse.\nMöglich: 1 -> 200\x25 | Aktueller Wert:\x20" },
    { "X-Achse Invertieren.\n0 -> Deaktiviert | 1-> Aktiviert | Momentan:\x20" },
    { "Y-Achse invertieren.\n0 -> Deaktiviert | 1-> Aktiviert | Momentan:\x20" },
    { "USB-Mäuse kommunizieren schneller als Serielle Mäuse.\nDer Bewegungsmodus entscheidet, was mit den gepufferten Mausaktualisierungen passieren soll.\n" },
    { "Additiv: Summe der Mausbewegung (Sensitiv)\nMittelwert: Durchschnitt der Mausbewegung (Insensitiv)\nGleiten: Mausbewegung inkrementell senden (Slippy)\n" },
    { "0 -> Additiv | 2 -> Mittelwert | 3 -> Gleiten | Momentan:\x20" },
    { "Dynamische Mausgeschwindigkeit aktivieren:\nMacht die Maus bei hohen Geschwindigkeiten weniger sensitiv,\nlässt sie die Maus bei niedrigen Geschwindigkeiten empfindlicher.\n" },
    { "0 -> Deaktiviert | 1 -> Niedrig | 2 -> Mittel | 3 -> Hoch | 4 -> Sehr Hoch | Momentan:\x20" }
  },

  { // Mouse Buttons
    { "Tasten für Links & Rechts vertauschen.\n0 -> Deaktiviert | 1-> Aktiviert | Momentan:\x20" },
    { "Seitliche Knöpfe für Links & Rechts verwenden\n0 -> Deaktiviert | 1-> Aktiviert | Momentan:\x20" },
    { "Seitliche Knöpfe tauschen\n0 -> Deaktiviert | 1-> Aktiviert | Momentan:\x20" }
  },

  { // Serial Settings
    { "Setze das Serielle Stop-Bit Format | 0 -> 7N1 | 1 -> 7N2 | Momentan:\x20" },
    { "Serieller BaudRate:\nOptionen: 1200 2400 4800 9600 19200 | Aktueller Wert:\x20" },
    { "Setze den Seriellen Maustyp | Momentan:\x20" },
    { "\nOptionen: 0 -> MS Maus | 1 -> Logitech Maus | 2 -> MS Wheel Maus" }
  },

  { // Firmware
    { "Alle gespeicherten Änderungen zurücksetzen? 0-> Nein | 1-> Ja\n" },
    { "Terminalsprache: 0 -> Englisch | 1 -> Deutsch\n" }
  }
}
};

const char terminal_warn_msg[2][11][80] = {
{ // English
  { "No Change\n" },
  { "Note: Any change will happen upon exit of terminal" },
  { "Important: Modified drivers required for baud rates higher than 1200!" },
  { "Important: Refresh of mouse driver will be required!" },
  { "Note: Doing this will cause the adapter to reboot on exit"},
  { "Important: Different Firmware images may have different default settings"},
  { "Default settings loaded" },
  { "Settings will be saved on exit" },
  { "Invalid Input"}, {"\nBuffer Overflow Error... Press enter to continue\n"},
  { "Changes saved to flash memory\n"}
},
{ // German
  { "Keine Änderung\n" },
  { "Hinweis: Änderungen werden beim Verlassen des Terminals gesetzt" },
  { "Wichtig: Baudraten über 1200 benötigen angepasste Maustreiber!" },
  { "Wichtig: Der Maustreiber muss im Anschluß neu geladen werden!" },
  { "Hinweis: Diese Änderung wird den Adapter beim Verlassen neu starten"},
  { "Wichtig: Unterschiedliche Firmware kann abweichende Default-Einstellungen haben"},
  { "Defaults settings loaded" },
  { "Änderungen werden beim Verlassen gespeichert" },
  { "Ungültige Eingabe"}, {"\nPufferüberlauf... Weiter mit der Eingabetaste / Enter\n"},
  { "Änderungen im Flash gespeichert\n"}
}
};

const char terminal_shorties[2][15][26] = {
{ // English
  {"Enabled"}, {"Disabled"}, {"Low"}, {"Medium"}, {"High"}, {"Very High"}, 
  {"Additive"}, {"Average"}, {"Coast"}, 
  {"Microsoft Two Button"}, {"Logitech Three Button"}, {"Microsoft Wheel"},
  {"Returning to mouse mode\n"}, {"German"}, {"English"} 
},
{ // German
  {"Aktiviert"}, {"Deaktiviert"}, {"Niedrig"}, {"Mittel"}, {"Hoch"}, {"Sehr Hoch"}, 
  {"Additiv"}, {"Mittlewert"}, {"Gleiten"}, 
  {"Microsoft Maus"}, {"Logitech Maus"}, {"Microsoft Wheel Maus"},
  {"Verbindung beendet\n"}, {"Deutsch"}, {"Englisch"} 
}
};

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
    term_writes_crlf(com, terminal_menu_headings[mouse_data.persistent.language][i]);   // Print the menu heading
    term_writes_crlf(com, terminal_menus[mouse_data.persistent.language][i]);           // Print the terminal menu
    term_writec_crlf(com, '\n');                        // New Line
}

void post_prompt(uart_inst_t * com) {
    sleep_ms(50);                                   // Wait a bit for fun
    term_writes_crlf(com, terminal_prompt);         // Cursor prompt
}

void post_invalid(uart_inst_t * com) {   
    term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][8]);        // Print Invalid input MSG
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
          term_writec_crlf(com, '\n');            // New Line
          term_writes_crlf(com, msg);             // Repost MSG
          term_writec_crlf(com, '\n');            // New Line
          post_prompt(com);                       // Print the prompt

          break;

        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34:
        case 0x35: case 0x36: case 0x37: case 0x38: case 0x39:

            // Prevent an overflow
            if ( buff_spot == (TERM_BUFFER - 1) ) {
                term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][9]);
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
                    if ( filter_buffi > 0 && filter_buffi <= 19200 && filter_buffi % 1200 == 0 ) { 
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
    char helpMSG[450] = {0};                // Make our Help MSG string    
    char tmp[30] = {0};                     // TMP string for things                  
    term_writec_crlf(com, '\n');            // New Line

    switch (level1)
    {
    // 1: Mouse Travel   
    case 1:
      memset(tmp, 0, 30);   memset(helpMSG, 0, 450);   

      switch ( level2 ) {
        case 2: // 2: XY Travel
          
          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][0]);
          itoa(mouse_data.persistent.xytravel_percentage, tmp);
          strcat(helpMSG, tmp );
          strcat(helpMSG, "\x25");

          ret = get_option_input(com, 1, 200,  helpMSG, false);

               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.xytravel_percentage )    { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.xytravel_percentage = ret;
              term_save_settings(com); 
              }
        break;
        
        case 3: // 3: X Travel

          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][1]);
          itoa(mouse_data.persistent.xtravel_percentage, tmp);
          strcat(helpMSG, tmp);
          strcat(helpMSG, "\x25");

          ret = get_option_input(com, 1, 200,  helpMSG, false);

               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.xtravel_percentage )     { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.xtravel_percentage = ret;
              term_save_settings(com); 
              }
        break;
        
        case 4: // 4: Y Travel

          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][2]);
          itoa(mouse_data.persistent.ytravel_percentage, tmp);
          strcat(helpMSG, tmp);
          strcat(helpMSG, "\x25");

          ret = get_option_input(com, 1, 200,  helpMSG, false);

               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.ytravel_percentage )     { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.ytravel_percentage = ret;
              term_save_settings(com); 
              }
        break;

        case 5: // 5: Invert X

          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][3]);
          switch ( mouse_data.persistent.invert_x )
          {
            case 0: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][1]);  break;
            case 1: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][0]);  break;
          }

          ret = get_option_input(com, 0, 1,  helpMSG, false);

               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.invert_x )               { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.invert_x = ret;
              term_save_settings(com); 
              }
        break;
        
        case 6: //6: Invert Y

          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][4]);
          switch ( mouse_data.persistent.invert_y )
          {
            case 0: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][1]); break;
            case 1: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][0]); break;
          }

          ret = get_option_input(com, 0, 1,  helpMSG, false);

               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.invert_y )               { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.invert_y = ret;
              term_save_settings(com); 
              }
        break;

        case 7: // 7: Movement Type

          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][5] );
          strcat(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][6] );
          strcat(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][7] );

          switch ( mouse_data.persistent.mouse_movt_type )
          {
            case MO_MVT_ADDITIVE: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][6]);  break;
            case MO_MVT_AVERAGE:  strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][7]);  break;
            case MO_MVT_COAST:    strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][8]);  break;
          }

          ret = get_option_input(com, 0, 2,  helpMSG, false);

               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.mouse_movt_type )        { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); } 
          else    {
              mouse_data.persistent.mouse_movt_type = ret;
              term_save_settings(com); 
              }
        break;

        case 8: // Cosine smoothing

          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][8]);
          strcat(helpMSG, terminal_handle_options[mouse_data.persistent.language][0][9] );

          switch ( mouse_data.persistent.use_cosine_smoothing )
          {
            case MO_MVT_COS_DISABLED: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][1]);  break;
            case MO_MVT_COS_LOW:      strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][2]);  break;
            case MO_MVT_COS_MEDIUM:   strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][3]);  break;
            case MO_MVT_COS_HIGH:     strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][4]);  break;
            case MO_MVT_COS_VERYHIGH: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][5]);  break;
          }

          ret = get_option_input(com, 0, 4,  helpMSG, false);

               if ( ret == 32000 )                                            { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.use_cosine_smoothing )       { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); } 
          else    {
              mouse_data.persistent.use_cosine_smoothing = ret;
              term_save_settings(com); 
              }
          break;
      }
  
    break;

    // 2: Mouse Buttons
    case 2:
      memset(tmp, 0, 30);   memset(helpMSG, 0, 450);   

      switch ( level2 )
      {
        // 2: Swap Left and Right
        case 2:
          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][1][0]);
          switch ( mouse_data.persistent.swap_left_right )
          {
            case 0: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][1]); break;
            case 1: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][0]); break;
          }

          ret = get_option_input(com, 0, 1,  helpMSG, false);
               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.swap_left_right)         { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.swap_left_right = ret;
              term_save_settings(com); 
              }
        break;
        
        // 3: Use Forward and Backward
        case 3:
          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][1][1]);
          switch ( mouse_data.persistent.use_forward_backward )
          {
            case 0: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][1]);  break;
            case 1: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][0]);  break;
          }

          ret = get_option_input(com, 0, 1,  helpMSG, false);
               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.use_forward_backward )   { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.use_forward_backward = ret;
              term_save_settings(com); 
              }
        break;
        
        // 4: Swap Forward and Backward
        case 4:
          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][1][2]);
          switch ( mouse_data.persistent.use_forward_backward )
          {
            case 0: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][1]); break;
            case 1: strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][0]); break;
          }

          ret = get_option_input(com, 0, 1,  helpMSG, false);
               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.swap_forward_backward )  { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.swap_forward_backward = ret;
              term_save_settings(com); 
              }
        break;
      }

    break;

    // 3: Serial Settings
    case 3:
      memset(tmp, 0, 30);   memset(helpMSG, 0, 450);   

      switch ( level2 )
      {
        // 2: Format
        case 2: 
          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][2][0]);
          switch ( mouse_data.persistent.doublestopbit )
          {
            case 0: strcat(helpMSG, "7N1\n"); break;
            case 1: strcat(helpMSG, "7N2\n"); break;
          }
          strcat(helpMSG, terminal_warn_msg[mouse_data.persistent.language][1]);

          ret = get_option_input(com, 0, 1,  helpMSG, false);
               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.doublestopbit )          { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.doublestopbit = ret;
              uartSettingsUpdated = true;
              term_save_settings(com); 
              }
        break;

        // 3: Baud Rate
        case 3:
          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][2][1]);
          itoa(mouse_data.persistent.baudrate, tmp);
          strcat(helpMSG, tmp);
          strcat(helpMSG, "\n");
          strcat(helpMSG, terminal_warn_msg[mouse_data.persistent.language][1]);
          strcat(helpMSG, "\n");
          strcat(helpMSG, terminal_warn_msg[mouse_data.persistent.language][2]);

          ret = get_option_input(com, 0, 0,  helpMSG, true);

               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.baudrate )               { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.baudrate = ret;
              uartSettingsUpdated = true;
              term_save_settings(com); 
              }
        break;
        
        // 4: Mouse Type
        case 4:
          strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][2][2]);
          switch ( mouse_data.persistent.mousetype )
          {
            case TWOBTN:    strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][9] ); break;
            case THREEBTN:  strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][10]); break;
            case WHEELBTN:  strcat(helpMSG, terminal_shorties[mouse_data.persistent.language][11]); break;
          }
          strcat(helpMSG, terminal_handle_options[mouse_data.persistent.language][2][3]);
          strcat(helpMSG, "\n");
          strcat(helpMSG, terminal_warn_msg[mouse_data.persistent.language][3]);

          ret = get_option_input(com, 0, 2,  helpMSG, false);

               if ( ret == 32000 )                                        { post_invalid(com); } 
          else if ( ret == mouse_data.persistent.mousetype )              { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
          else    { 
              mouse_data.persistent.mousetype = ret;
              uartSettingsUpdated = true;
              term_save_settings(com); 
              }

        break;
        }

      break;

    // 4: Firmware
    case 4:
      memset(tmp, 0, 30); memset(helpMSG, 0, 250);

      switch ( level2 )
      {
      // 2: Reset
      case 2:
        strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][3][0]);
        strcat(helpMSG, terminal_warn_msg[mouse_data.persistent.language][4]);
        strcat(helpMSG, "\n");
        strcat(helpMSG, terminal_warn_msg[mouse_data.persistent.language][5]);
        strcat(helpMSG, "\n");

        ret = get_option_input(com, 0, 1,  helpMSG, false);

             if ( ret == 32000 )                { post_invalid(com); } 
        else if ( ret == 0 )                    { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
        else    { 
          loadPersistentSetDefaults();
          term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][6]);
          term_writes_crlf(com, "\n");
          term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][7]);
          term_writes_crlf(com, "\n");
          settingsUpdated = true;
          uartSettingsUpdated = true;
          rebootNeeded = true;
          }

      break;

      // 3: Language
      case 3:
        strcpy(helpMSG, terminal_handle_options[mouse_data.persistent.language][3][1]);

        ret = get_option_input(com, 0, 1,  helpMSG, false);

             if ( ret == 32000 )                            { post_invalid(com); } 
        else if ( ret == mouse_data.persistent.language )   { term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][0]); }   
        else    { 
          mouse_data.persistent.language = ret;
          term_save_settings(com); 
        }

      break;

      // 4: List Default Settings
      case 4: 
        // Mouse Travel Settings
        term_writes_crlf(com, terminal_menu_headings[mouse_data.persistent.language][1] );
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][0]);
        term_writes_crlf(com, "\x3a\x20");
        itoa(default_xytravel_percentage, tmp);
        term_writes_crlf(com, tmp); memset(tmp, 0, 30); 
        if ( mouse_data.persistent.language != 0 ){
          term_writes_crlf(com, "\n");
        } else {
          term_writes_crlf(com, "\x20\x7c\x20");
        }
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][1]);
        term_writes_crlf(com, "\x3a\x20");
        itoa(default_xtravel_percentage, tmp);
        term_writes_crlf(com, tmp); memset(tmp, 0, 30); 
        if ( mouse_data.persistent.language != 0 ){
          term_writes_crlf(com, "\n");
        } else {
          term_writes_crlf(com, "\x20\x7c\x20");
        }
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][2]);
        term_writes_crlf(com, "\x3a\x20");
        itoa(default_ytravel_percentage, tmp);
        term_writes_crlf(com, tmp); memset(tmp, 0, 30); 
        term_writes_crlf(com, "\x0a");
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][3]);
        term_writes_crlf(com, "\x3a\x20");

        if ( default_invert_x ) { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0]); }
        else                    { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1]); }

        term_writes_crlf(com, "\x20\x7c\x20");
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][4]);
        term_writes_crlf(com, "\x3a\x20");

        if ( default_invert_y ) { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0]); }
        else                    { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1]); }
        
        term_writes_crlf(com, "\x0a");
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][5]); 
        term_writes_crlf(com, "\x3a\x20");
        
        switch ( default_mouse_movt_type )
        {
          case MO_MVT_ADDITIVE:     term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][6]);  break;
          case MO_MVT_AVERAGE:      term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][7]);  break;
          case MO_MVT_COAST:        term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][8]);  break;
        }

        term_writes_crlf(com, "\x20\x7c\x20");
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][6]); 
        term_writes_crlf(com, "\x3a\x20");

        switch ( default_use_cosine_smoothing )
        {
          case MO_MVT_COS_DISABLED:   term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1]);  break;
          case MO_MVT_COS_LOW:        term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][2]);  break;
          case MO_MVT_COS_MEDIUM:     term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][3]);  break;
          case MO_MVT_COS_HIGH:       term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][4]);  break;
          case MO_MVT_COS_VERYHIGH:   term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][5]);  break;
        }

        sleep_us(500000);

        // Mouse Buttons Settings
        term_writes_crlf(com, "\x0a\x0a");
        term_writes_crlf(com, terminal_menu_headings[mouse_data.persistent.language][2] );
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][1][0]); 
        term_writes_crlf(com, "\x3a\x20" );
        
        if ( default_swap_left_right )          { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0]); }
        else                                    { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1]); }

        term_writes_crlf(com, "\x0a");
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][1][1]); 
        term_writes_crlf(com, "\x3a\x20" );

        if ( default_use_forward_backward )     { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0]); }
        else                                    { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1]); }

        term_writes_crlf(com, "\x0a");
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][1][2]); 
        term_writes_crlf(com, "\x3a\x20" );

        if ( default_swap_forward_backward )    { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0]); }
        else                                    { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1]); }

        sleep_us(500000);

        // Serial Settings 
        term_writes_crlf(com, "\x0a\x0a");
        term_writes_crlf(com, terminal_menu_headings[mouse_data.persistent.language][3] );
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][2][0]);
        term_writes_crlf(com, "\x3a\x20" );
        
        if ( default_doublestopbit )            { term_writes_crlf(com, "7N2"); }
        else                                    { term_writes_crlf(com, "7N1"); }

        term_writes_crlf(com, "\x0a");
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][2][1]);
        term_writes_crlf(com, "\x3a\x20" );

        switch ( default_baudrate ) 
        {
          case 1200:  term_writes_crlf(com, "1200");  break;
          case 2400:  term_writes_crlf(com, "2400");  break;
          case 4800:  term_writes_crlf(com, "4800");  break;
          case 9600:  term_writes_crlf(com, "9600");  break;
          case 19200: term_writes_crlf(com, "19200"); break;
          default:    term_writes_crlf(com, "1200");  break;
        }

        term_writes_crlf( com, "\x0a");
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][2][2]);
        term_writes_crlf(com, "\x3a\x20");

        switch ( default_mousetype )
        {
          case TWOBTN:    term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][9] ); break;
          case THREEBTN:  term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][10]); break;
          case WHEELBTN:  term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][11]); break;
        }

        sleep_us(500000);
        
        // Firmware Settings 
        term_writes_crlf(com, "\x0a\x0a");
        term_writes_crlf(com, terminal_menu_headings[mouse_data.persistent.language][4] );
        term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][3][0]);
        term_writes_crlf(com, "\x3a\x20" );

        switch ( default_language )
        {
          case 0:   { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][14]); }  break;
          case 1:   { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][13]); }  break;
          default:  { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][14]); }  break;
        }

        term_writec_crlf( com, '\x0a');
        term_writec_crlf( com, '\x0a');
        sleep_us(1000000);

      break;
      }

    break;

    default: break;
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
    term_writes_crlf(com, terminal_guide[mouse_data.persistent.language]);          // Print the little guide
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
          term_writes_crlf(com, terminal_guide[mouse_data.persistent.language]);      // Print the little guide.

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
                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][0]);
                term_writec_crlf(com, 0x3A);
                term_writec_crlf(com, 0x20);  
                term_writes_crlf(com, tmp_char);
                term_writec_crlf(com, 0x25);
                term_writec_crlf(com, 0x0A);


                memset(tmp_char, 0, 20);
                itoa(mouse_data.persistent.xtravel_percentage, tmp_char);
                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][1]);
                term_writec_crlf(com, 0x3A);
                term_writec_crlf(com, 0x20);
                term_writes_crlf(com, tmp_char);
                term_writec_crlf(com, 0x25);
                term_writec_crlf(com, 0x0A);


                memset(tmp_char, 0, 20);
                itoa(mouse_data.persistent.ytravel_percentage, tmp_char);
                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][2]);
                term_writec_crlf(com, 0x3A);
                term_writec_crlf(com, 0x20);
                term_writes_crlf(com, tmp_char);
                term_writec_crlf(com, 0x25);
                term_writec_crlf(com, 0x0A);
                

                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][3]);
                term_writes_crlf(com, "\x3A\x20");
                if ( mouse_data.persistent.invert_x )   { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0] ); }
                else                                    { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1] ); }
                term_writec_crlf(com, '\n');
                

                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][4]);
                term_writes_crlf(com, "\x3A\x20");
                if ( mouse_data.persistent.invert_y )   { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0] ); }
                else                                    { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1] ); }
                term_writec_crlf(com, '\n');
                

                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][5]);
                term_writes_crlf(com, "\x3A\x20");
                switch ( mouse_data.persistent.mouse_movt_type)
                {      
                  case MO_MVT_ADDITIVE: term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][6] );  break;
                  case MO_MVT_AVERAGE:  term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][7] );  break;
                  case MO_MVT_COAST:    term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][8] );  break;
                }
                term_writec_crlf(com, '\n');


                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][0][6]);
                term_writes_crlf(com, "\x3A\x20");
                switch ( mouse_data.persistent.use_cosine_smoothing )
                {
                  case MO_MVT_COS_DISABLED: term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1] );  break;
                  case MO_MVT_COS_LOW:      term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][2] );  break;
                  case MO_MVT_COS_MEDIUM:   term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][3] );  break;
                  case MO_MVT_COS_HIGH:     term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][4] );  break;
                  case MO_MVT_COS_VERYHIGH: term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][5] );  break;
                }

                term_writes_crlf(com, "\n\n");
              } 

              // Mouse Buttons settings.
              else if ( menu_level == 2 ) {

                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][1][0]);
                term_writes_crlf(com, "\x3A\x20");
                if ( mouse_data.persistent.swap_left_right )        { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0] ); }
                else                                                { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1] ); }
                term_writec_crlf(com, '\n');


                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][1][1]);
                term_writes_crlf(com, "\x3A\x20");
                if ( mouse_data.persistent.use_forward_backward )   { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0] ); }
                else                                                { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1] ); }
                term_writec_crlf(com, '\n');


                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][1][2]);
                term_writes_crlf(com, "\x3A\x20");
                if ( mouse_data.persistent.swap_forward_backward )  { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][0] ); }
                else                                                { term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][1] ); }
                
                term_writes_crlf(com, "\n\n");
              } 
              
              // Serial Settings
              else if ( menu_level == 3 ) { 

                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][2][0]);
                term_writes_crlf(com, "\x3A\x20");
                if ( mouse_data.persistent.doublestopbit )          { term_writes_crlf(com, "7N2"); }
                else                                                { term_writes_crlf(com, "7N1"); }
                term_writec_crlf(com, '\n');


                memset(tmp_char, 0, 20);
                itoa(mouse_data.persistent.baudrate, tmp_char);
                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][2][1]);
                term_writes_crlf(com, "\x3A\x20");
                term_writes_crlf(com, tmp_char);
                term_writec_crlf(com, '\n');


                term_writes_crlf(com, terminal_list_options[mouse_data.persistent.language][2][2]);
                term_writes_crlf(com, "\x3A\x20");
                switch ( mouse_data.persistent.mousetype ) 
                {
                  case TWOBTN:    term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][9] );  break;
                  case THREEBTN:  term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][10]);  break;
                  case WHEELBTN:  term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][11]);  break;
                }
                
                term_writes_crlf(com, "\n\n");
              } 
              
              // Firmware Settings
              else if ( menu_level == 4 ) {
                  post_header(com);                                   // Print the Header for the serial mouse
                  term_writes_crlf(com, "\n\n");                      // New line and line break
                  switch ( mouse_data.persistent.language ){
                    //german
                    case 1:
                      term_writes_crlf(com, "Thank you for using my firmware, I hope you like it.\n");
                      term_writes_crlf(com, "Special thanks to MJay99 for aiding in the German Translation.\n-Lime");
                    break;
                    
                    case 0:
                    default:
                      term_writes_crlf(com, "Thank you for using my firmware, I hope you like it.\n-Lime");
                    break;
                  }
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
                term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][9]);
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
      term_writes_crlf(com, terminal_warn_msg[mouse_data.persistent.language][10]);
    }
    
    term_writes_crlf(com, terminal_shorties[mouse_data.persistent.language][12]);

    if ( uartSettingsUpdated ) {
      refresh_serial_uart();      // Reinit serial with updated settings
      calcSerialDelay();          // Recalulate serial delay used by the main mouse timer 
    }

    if (rebootNeeded ) {
      machine_reboot();
    }

    return;
}