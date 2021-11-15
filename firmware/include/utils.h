#include "pico.h"

#ifndef UTILS_H_ 
#define UTILS_H_

int constrain(int value, int8_t min, int8_t max);

void init_pinheader(uint pin);

void init_led(uint pin);

void machine_reboot();

int8_t travel_limit75(int8_t val);

int8_t travel_limit50(int8_t val);

int8_t travel_limit25(int8_t val);
#endif