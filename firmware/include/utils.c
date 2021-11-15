#include "utils.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"


/*---------------------------------------*/
//                 UTILS                 //
/*---------------------------------------*/

int constrain(int value, int8_t min, int8_t max) {
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


/* ---------------------------------------------------------- */
/*
Yes this is messy I know BUT, the pi pico lacks a floating point unit so decent division is slow.
I decided to make a look up table instead because it could be faster. Math was precalulated using python
*/
/* ---------------------------------------------------------- */

//__INT8_TYPE__ travel_limit75(__INT8_TYPE__ val)
int8_t travel_limit75(int8_t val)
{
  bool neg = false;
  int8_t newval = 0;

  // If val is negative, flag it as negative and make it positive.
  if ( val < 0 )  { val *= -1; neg = true; }

  switch (val)
  {
    case 0: newval = 0; break;
    case 1: newval = 1; break;
    case 2: newval = 2; break;
    case 3: newval = 2; break;
    case 4: newval = 3; break;
    case 5: newval = 4; break;
    case 6: newval = 4; break;
    case 7: newval = 5; break;
    case 8: newval = 6; break;
    case 9: newval = 7; break;
    case 10: newval = 8; break;
    case 11: newval = 8; break;
    case 12: newval = 9; break;
    case 13: newval = 10; break;
    case 14: newval = 11; break;
    case 15: newval = 11; break;
    case 16: newval = 12; break;
    case 17: newval = 13; break;
    case 18: newval = 14; break;
    case 19: newval = 14; break;
    case 20: newval = 15; break;
    case 21: newval = 16; break;
    case 22: newval = 16; break;
    case 23: newval = 17; break;
    case 24: newval = 18; break;
    case 25: newval = 19; break;
    case 26: newval = 20; break;
    case 27: newval = 20; break;
    case 28: newval = 21; break;
    case 29: newval = 22; break;
    case 30: newval = 22; break;
    case 31: newval = 23; break;
    case 32: newval = 24; break;
    case 33: newval = 25; break;
    case 34: newval = 26; break;
    case 35: newval = 26; break;
    case 36: newval = 27; break;
    case 37: newval = 28; break;
    case 38: newval = 28; break;
    case 39: newval = 29; break;
    case 40: newval = 30; break;
    case 41: newval = 31; break;
    case 42: newval = 32; break;
    case 43: newval = 32; break;
    case 44: newval = 33; break;
    case 45: newval = 34; break;
    case 46: newval = 34; break;
    case 47: newval = 35; break;
    case 48: newval = 36; break;
    case 49: newval = 37; break;
    case 50: newval = 38; break;
    case 51: newval = 38; break;
    case 52: newval = 39; break;
    case 53: newval = 40; break;
    case 54: newval = 40; break;
    case 55: newval = 41; break;
    case 56: newval = 42; break;
    case 57: newval = 43; break;
    case 58: newval = 44; break;
    case 59: newval = 44; break;
    case 60: newval = 45; break;
    case 61: newval = 46; break;
    case 62: newval = 46; break;
    case 63: newval = 47; break;
    case 64: newval = 48; break;
    case 65: newval = 49; break;
    case 66: newval = 50; break;
    case 67: newval = 50; break;
    case 68: newval = 51; break;
    case 69: newval = 52; break;
    case 70: newval = 52; break;
    case 71: newval = 53; break;
    case 72: newval = 54; break;
    case 73: newval = 55; break;
    case 74: newval = 56; break;
    case 75: newval = 56; break;
    case 76: newval = 57; break;
    case 77: newval = 58; break;
    case 78: newval = 58; break;
    case 79: newval = 59; break;
    case 80: newval = 60; break;
    case 81: newval = 61; break;
    case 82: newval = 61; break;
    case 83: newval = 62; break;
    case 84: newval = 63; break;
    case 85: newval = 64; break;
    case 86: newval = 64; break;
    case 87: newval = 65; break;
    case 88: newval = 66; break;
    case 89: newval = 67; break;
    case 90: newval = 68; break;
    case 91: newval = 68; break;
    case 92: newval = 69; break;
    case 93: newval = 70; break;
    case 94: newval = 70; break;
    case 95: newval = 71; break;
    case 96: newval = 72; break;
    case 97: newval = 73; break;
    case 98: newval = 74; break;
    case 99: newval = 74; break;
    case 100: newval = 75; break;
    case 101: newval = 76; break;
    case 102: newval = 76; break;
    case 103: newval = 77; break;
    case 104: newval = 78; break;
    case 105: newval = 79; break;
    case 106: newval = 80; break;
    case 107: newval = 80; break;
    case 108: newval = 81; break;
    case 109: newval = 82; break;
    case 110: newval = 82; break;
    case 111: newval = 83; break;
    case 112: newval = 84; break;
    case 113: newval = 85; break;
    case 114: newval = 85; break;
    case 115: newval = 86; break;
    case 116: newval = 87; break;
    case 117: newval = 88; break;
    case 118: newval = 88; break;
    case 119: newval = 89; break;
    case 120: newval = 90; break;
    case 121: newval = 91; break;
    case 122: newval = 92; break;
    case 123: newval = 92; break;
    case 124: newval = 93; break;
    case 125: newval = 94; break;
    case 126: newval = 94; break;
    case 127: newval = 95; break; 

    default:
      newval = 0; break;
  }

  // If val was flagged as a negative, make it negative.
  if ( neg )  { newval *= -1;}

  return newval;
}


int8_t travel_limit50(int8_t val)
{
  bool neg = false;
  int8_t newval = 0;

  // If val is negative, flag it as negative and make it positive.
  if ( val < 0 )  { val *= -1; neg = true; }

  switch (val)
  {
    case 0: newval = 0; break;
    case 1: newval = 0; break;
    case 2: newval = 1; break;
    case 3: newval = 2; break;
    case 4: newval = 2; break;
    case 5: newval = 2; break;
    case 6: newval = 3; break;
    case 7: newval = 4; break;
    case 8: newval = 4; break;
    case 9: newval = 4; break;
    case 10: newval = 5; break;
    case 11: newval = 6; break;
    case 12: newval = 6; break;
    case 13: newval = 6; break;
    case 14: newval = 7; break;
    case 15: newval = 8; break;
    case 16: newval = 8; break;
    case 17: newval = 8; break;
    case 18: newval = 9; break;
    case 19: newval = 10; break;
    case 20: newval = 10; break;
    case 21: newval = 10; break;
    case 22: newval = 11; break;
    case 23: newval = 12; break;
    case 24: newval = 12; break;
    case 25: newval = 12; break;
    case 26: newval = 13; break;
    case 27: newval = 14; break;
    case 28: newval = 14; break;
    case 29: newval = 14; break;
    case 30: newval = 15; break;
    case 31: newval = 16; break;
    case 32: newval = 16; break;
    case 33: newval = 16; break;
    case 34: newval = 17; break;
    case 35: newval = 18; break;
    case 36: newval = 18; break;
    case 37: newval = 18; break;
    case 38: newval = 19; break;
    case 39: newval = 20; break;
    case 40: newval = 20; break;
    case 41: newval = 20; break;
    case 42: newval = 21; break;
    case 43: newval = 22; break;
    case 44: newval = 22; break;
    case 45: newval = 22; break;
    case 46: newval = 23; break;
    case 47: newval = 24; break;
    case 48: newval = 24; break;
    case 49: newval = 24; break;
    case 50: newval = 25; break;
    case 51: newval = 26; break;
    case 52: newval = 26; break;
    case 53: newval = 26; break;
    case 54: newval = 27; break;
    case 55: newval = 28; break;
    case 56: newval = 28; break;
    case 57: newval = 28; break;
    case 58: newval = 29; break;
    case 59: newval = 30; break;
    case 60: newval = 30; break;
    case 61: newval = 30; break;
    case 62: newval = 31; break;
    case 63: newval = 32; break;
    case 64: newval = 32; break;
    case 65: newval = 32; break;
    case 66: newval = 33; break;
    case 67: newval = 34; break;
    case 68: newval = 34; break;
    case 69: newval = 34; break;
    case 70: newval = 35; break;
    case 71: newval = 36; break;
    case 72: newval = 36; break;
    case 73: newval = 36; break;
    case 74: newval = 37; break;
    case 75: newval = 38; break;
    case 76: newval = 38; break;
    case 77: newval = 38; break;
    case 78: newval = 39; break;
    case 79: newval = 40; break;
    case 80: newval = 40; break;
    case 81: newval = 40; break;
    case 82: newval = 41; break;
    case 83: newval = 42; break;
    case 84: newval = 42; break;
    case 85: newval = 42; break;
    case 86: newval = 43; break;
    case 87: newval = 44; break;
    case 88: newval = 44; break;
    case 89: newval = 44; break;
    case 90: newval = 45; break;
    case 91: newval = 46; break;
    case 92: newval = 46; break;
    case 93: newval = 46; break;
    case 94: newval = 47; break;
    case 95: newval = 48; break;
    case 96: newval = 48; break;
    case 97: newval = 48; break;
    case 98: newval = 49; break;
    case 99: newval = 50; break;
    case 100: newval = 50; break;
    case 101: newval = 50; break;
    case 102: newval = 51; break;
    case 103: newval = 52; break;
    case 104: newval = 52; break;
    case 105: newval = 52; break;
    case 106: newval = 53; break;
    case 107: newval = 54; break;
    case 108: newval = 54; break;
    case 109: newval = 55; break;
    case 110: newval = 55; break;
    case 111: newval = 56; break;
    case 112: newval = 56; break;
    case 113: newval = 56; break;
    case 114: newval = 57; break;
    case 115: newval = 57; break;
    case 116: newval = 58; break;
    case 117: newval = 58; break;
    case 118: newval = 59; break;
    case 119: newval = 60; break;
    case 120: newval = 60; break;
    case 121: newval = 60; break;
    case 122: newval = 61; break;
    case 123: newval = 62; break;
    case 124: newval = 62; break;
    case 125: newval = 62; break;
    case 126: newval = 63; break;
    case 127: newval = 64; break;
    default:
      newval = 0; break;
  }

  // If val was flagged as a negative, make it negative.
  if ( neg )  { newval *= -1; }

  return newval;
}


int8_t travel_limit25(int8_t val)
{
  bool neg = false;
  int8_t newval = 0;

  // If val is negative, flag it as negative and make it positive.
  if ( val < 0 )  { val *= -1; neg = true; }

  switch (val)
  {
    case 0: newval = 0; break;
    case 1: newval = 0; break;
    case 2: newval = 1; break;
    case 3: newval = 1; break;
    case 4: newval = 1; break;
    case 5: newval = 1; break;
    case 6: newval = 2; break;
    case 7: newval = 2; break;
    case 8: newval = 2; break;
    case 9: newval = 2; break;
    case 10: newval = 2; break;
    case 11: newval = 3; break;
    case 12: newval = 3; break;
    case 13: newval = 3; break;
    case 14: newval = 4; break;
    case 15: newval = 4; break;
    case 16: newval = 4; break;
    case 17: newval = 4; break;
    case 18: newval = 4; break;
    case 19: newval = 5; break;
    case 20: newval = 5; break;
    case 21: newval = 5; break;
    case 22: newval = 6; break;
    case 23: newval = 6; break;
    case 24: newval = 6; break;
    case 25: newval = 6; break;
    case 26: newval = 6; break;
    case 27: newval = 7; break;
    case 28: newval = 7; break;
    case 29: newval = 7; break;
    case 30: newval = 8; break;
    case 31: newval = 8; break;
    case 32: newval = 8; break;
    case 33: newval = 8; break;
    case 34: newval = 8; break;
    case 35: newval = 9; break;
    case 36: newval = 9; break;
    case 37: newval = 9; break;
    case 38: newval = 10; break;
    case 39: newval = 10; break;
    case 40: newval = 10; break;
    case 41: newval = 10; break;
    case 42: newval = 10; break;
    case 43: newval = 11; break;
    case 44: newval = 11; break;
    case 45: newval = 11; break;
    case 46: newval = 12; break;
    case 47: newval = 12; break;
    case 48: newval = 12; break;
    case 49: newval = 12; break;
    case 50: newval = 12; break;
    case 51: newval = 13; break;
    case 52: newval = 13; break;
    case 53: newval = 13; break;
    case 54: newval = 14; break;
    case 55: newval = 14; break;
    case 56: newval = 14; break;
    case 57: newval = 14; break;
    case 58: newval = 14; break;
    case 59: newval = 15; break;
    case 60: newval = 15; break;
    case 61: newval = 15; break;
    case 62: newval = 16; break;
    case 63: newval = 16; break;
    case 64: newval = 16; break;
    case 65: newval = 16; break;
    case 66: newval = 16; break;
    case 67: newval = 17; break;
    case 68: newval = 17; break;
    case 69: newval = 17; break;
    case 70: newval = 18; break;
    case 71: newval = 18; break;
    case 72: newval = 18; break;
    case 73: newval = 18; break;
    case 74: newval = 18; break;
    case 75: newval = 19; break;
    case 76: newval = 19; break;
    case 77: newval = 19; break;
    case 78: newval = 20; break;
    case 79: newval = 20; break;
    case 80: newval = 20; break;
    case 81: newval = 20; break;
    case 82: newval = 20; break;
    case 83: newval = 21; break;
    case 84: newval = 21; break;
    case 85: newval = 21; break;
    case 86: newval = 22; break;
    case 87: newval = 22; break;
    case 88: newval = 22; break;
    case 89: newval = 22; break;
    case 90: newval = 22; break;
    case 91: newval = 23; break;
    case 92: newval = 23; break;
    case 93: newval = 23; break;
    case 94: newval = 24; break;
    case 95: newval = 24; break;
    case 96: newval = 24; break;
    case 97: newval = 24; break;
    case 98: newval = 24; break;
    case 99: newval = 25; break;
    case 100: newval = 25; break;
    case 101: newval = 25; break;
    case 102: newval = 26; break;
    case 103: newval = 26; break;
    case 104: newval = 26; break;
    case 105: newval = 26; break;
    case 106: newval = 26; break;
    case 107: newval = 27; break;
    case 108: newval = 27; break;
    case 109: newval = 27; break;
    case 110: newval = 28; break;
    case 111: newval = 28; break;
    case 112: newval = 28; break;
    case 113: newval = 28; break;
    case 114: newval = 28; break;
    case 115: newval = 29; break;
    case 116: newval = 29; break;
    case 117: newval = 29; break;
    case 118: newval = 30; break;
    case 119: newval = 30; break;
    case 120: newval = 30; break;
    case 121: newval = 30; break;
    case 122: newval = 30; break;
    case 123: newval = 31; break;
    case 124: newval = 31; break;
    case 125: newval = 31; break;
    case 126: newval = 32; break;
    case 127: newval = 32; break;
    default:
      newval = 0; break;
  }

  // If val was flagged as a negative, make it negative.
  if ( neg )  { newval *= -1; }

  return newval;
}