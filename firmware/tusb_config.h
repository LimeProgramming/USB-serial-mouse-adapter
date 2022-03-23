#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
  #error CFG_TUSB_MCU must be defined
#endif


// RHPort max operational speed can defined by board.mk
// Default to Highspeed for MCU with internal HighSpeed PHY (can be port specific), otherwise FullSpeed
#ifndef BOARD_HOST_RHPORT_SPEED
  #if TU_CHECK_MCU(OPT_MCU_LPC18XX, OPT_MCU_LPC43XX, OPT_MCU_MIMXRT10XX, OPT_MCU_NUC505) ||\
      TU_CHECK_MCU(OPT_MCU_CXD56, OPT_MCU_SAMX7X, OPT_MCU_BCM2711) ||\
      TU_CHECK_MCU(OPT_MCU_FT90X, OPT_MCU_FT93X)
    #define BOARD_HOST_RHPORT_SPEED   OPT_MODE_HIGH_SPEED
  #else
    #define BOARD_HOST_RHPORT_SPEED   OPT_MODE_FULL_SPEED
  #endif
#endif

// Device mode with rhport and speed defined by board.mk
#if   BOARD_HOST_RHPORT_NUM == 0
  #define CFG_TUSB_RHPORT0_MODE     (OPT_MODE_HOST | BOARD_HOST_RHPORT_SPEED)
#elif BOARD_HOST_RHPORT_NUM == 1
  #define CFG_TUSB_RHPORT1_MODE     (OPT_MODE_HOST | BOARD_HOST_RHPORT_SPEED)
#else
  #error "Incorrect RHPort configuration"
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_NONE
#endif

// CFG_TUSB_DEBUG is defined by compiler in DEBUG build
// #define CFG_TUSB_DEBUG           0

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(32)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

//--------------------------------------------------------------------
// HOST CONFIGURATION
//--------------------------------------------------------------------

// Size of buffer to hold descriptors and other data used for enumeration
#define CFG_TUH_ENUMERATION_BUFSIZE 256

#define CFG_TUH_HUB                 4 // Number of Hubs chained
#define CFG_TUH_CDC                 0 // Virtual Com I think
#define CFG_TUH_MSC                 0 // Mass Storage
#define CFG_TUH_VENDOR              0 // Just fucking broken

#define CFG_TUSB_HOST_DEVICE_MAX    (CFG_TUH_HUB ? 4 : 1) // normal hub has 4 ports

//------------- HID -------------//
#define CFG_TUH_HID                 4
#define CFG_TUH_HID_EPIN_BUFSIZE    64
#define CFG_TUH_HID_EPOUT_BUFSIZE   64

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */