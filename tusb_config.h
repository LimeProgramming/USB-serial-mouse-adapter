#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_
#endif

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif


#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_HOST

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_NONE
#endif

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif


#define CFG_TUH_HUB                 1
#define CFG_TUH_CDC                 0
#define CFG_TUH_HID                 1
//#define CFG_TUH_HID                 4
//#define CFG_TUH_HID_MOUSE           1
#define CFG_TUH_MSC                 0
#define CFG_TUH_VENDOR              0
#define CFG_TUSB_HOST_HID_GENERIC   0 // (not yet supported)

//#define CFG_TUH_HID_KEYBOARD        0
//#define CFG_TUH_HID_MOUSE           1


#define CFG_TUSB_HOST_DEVICE_MAX    (CFG_TUH_HUB ? 5 : 1) // normal hub has 4 ports

