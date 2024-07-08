#ifndef HID_APP_H_
#define HID_APP_H_


#include "tusb.h"

// Needed to account for update in tinyUSB
#if __has_include("bsp/board_api.h")
#include "bsp/board_api.h"
#else
#include "bsp/board.h"
#endif

#define MAX_HID_REPORT  4

static struct
{
  uint8_t report_count;
  tuh_hid_report_info_t report_info[MAX_HID_REPORT];
}hid_info[CFG_TUH_HID];

//void tuh_cdc_xfer_isr(uint8_t dev_addr, xfer_result_t event, cdc_pipeid_t pipe_id, uint32_t xferred_bytes);

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance);

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);

void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);

#endif // HID_APP_H_