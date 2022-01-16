#include "tusb.h"
#include "pico.h"

#ifndef UTILS_H_ 
#define UTILS_H_

/*---------------------------------------*/
//                Utilites               //
/*---------------------------------------*/

int constraini(int16_t value, int8_t min, int8_t max);

void init_pinheader(uint pin);

void init_led(uint pin);

void machine_reboot();

/*---------------------------------------*/
//            Repeating Timers           //
/*---------------------------------------*/

void startTerminalTimer();

void stopTerminalTimer();

void startMouseTimer();

void stopMouseTimer();

void calcSerialDelay();

/*---------------------------------------*/
//          Mouse Settings Stuff         //
/*---------------------------------------*/

void setDipSerialBaud();

void setDipMouseType();

void setDipMouseSpeed();

/*---------------------------------------*/
//             Dip Switch IRQ            //
/*---------------------------------------*/

void dipswGPIOCallback(uint gpio, uint32_t events);

/*---------------------------------------*/
//           Persistent Settings         //
/*---------------------------------------*/

void initPersistentSet();

void savePersistentSet();

void loadPersistentSet();

void loadPersistentSetDefaults();

void updateStoredDipswitchs();

/*---------------------------------------*/
//              DEBUG Tools              //
/*---------------------------------------*/

void blink_led_task(void);

/*---------------------------------------*/
//             Travel Limits             //
/*---------------------------------------*/

int8_t travel_limit(int16_t val, int8_t percentage, uint8_t constainval);

/*---------------------------------------*/
//            Mouse Processing           //
/*---------------------------------------*/

void set_mouseclick(uint8_t spot, bool value);

void process_mouse_report(hid_mouse_report_t const * report);

void reset_cycle();

void update_mousepacket();

#endif