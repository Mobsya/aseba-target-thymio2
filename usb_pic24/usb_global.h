#ifndef _USB_GLOBAL_H_
#define _USB_GLOBAL_H_

#include <usb/usb.h>
#include <HardwareProfile.h>
#include "usb_function_cdc.h"
#include <usb/usb_device_local.h>
#include <usb/usb_ch9.h>


#if (USB_PING_PONG_MODE == USB_PING_PONG__NO_PING_PONG)
    extern volatile BDT_ENTRY BDT[(USB_MAX_EP_NUMBER + 1) * 2];
#elif (USB_PING_PONG_MODE == USB_PING_PONG__EP0_OUT_ONLY)
    extern volatile BDT_ENTRY BDT[((USB_MAX_EP_NUMBER + 1) * 2)+1];
#elif (USB_PING_PONG_MODE == USB_PING_PONG__FULL_PING_PONG)
    extern volatile BDT_ENTRY BDT[(USB_MAX_EP_NUMBER + 1) * 4];
#elif (USB_PING_PONG_MODE == USB_PING_PONG__ALL_BUT_EP0)
    extern volatile BDT_ENTRY BDT[((USB_MAX_EP_NUMBER + 1) * 4)-2];
#else
    #error "No ping pong mode defined."
#endif


extern volatile CTRL_TRF_SETUP SetupPkt;           // 8-byte only
extern volatile BYTE CtrlTrfData[USB_EP0_BUFF_SIZE];

extern USB_VOLATILE USB_DEVICE_STATE USBDeviceState;
extern USB_VOLATILE BYTE USBActiveConfiguration;
extern USB_VOLATILE BYTE USBAlternateInterface[USB_MAX_NUM_INT];
extern volatile BDT_ENTRY *pBDTEntryEP0OutCurrent;
extern volatile BDT_ENTRY *pBDTEntryEP0OutNext;
extern volatile BDT_ENTRY *pBDTEntryOut[USB_MAX_EP_NUMBER+1];
extern volatile BDT_ENTRY *pBDTEntryIn[USB_MAX_EP_NUMBER+1];
extern USB_VOLATILE BYTE shortPacketStatus;
extern USB_VOLATILE BYTE controlTransferState;
extern USB_VOLATILE IN_PIPE inPipes[1];
extern USB_VOLATILE OUT_PIPE outPipes[1];
extern USB_VOLATILE BYTE *pDst;
extern USB_VOLATILE BOOL RemoteWakeup;
extern USB_VOLATILE BOOL USBBusIsSuspended;
extern USB_VOLATILE USTAT_FIELDS USTATcopy;
extern USB_VOLATILE BYTE endpoint_number;
extern USB_VOLATILE BOOL BothEP0OutUOWNsSet;
extern USB_VOLATILE EP_STATUS ep_data_in[USB_MAX_EP_NUMBER+1];
extern USB_VOLATILE EP_STATUS ep_data_out[USB_MAX_EP_NUMBER+1];
extern USB_VOLATILE BYTE USBStatusStageTimeoutCounter;
extern volatile BOOL USBDeferStatusStagePacket;
extern volatile BOOL USBStatusStageEnabledFlag1;
extern volatile BOOL USBStatusStageEnabledFlag2;
extern volatile BOOL USBDeferINDataStagePackets;
extern volatile BOOL USBDeferOUTDataStagePackets;


extern unsigned char cdc_data_rx[CDC_DATA_OUT_EP_SIZE];
extern unsigned char cdc_data_tx[CDC_DATA_IN_EP_SIZE];
extern LINE_CODING line_coding;    // Buffer to store line coding information

extern USB_HANDLE CDCDataOutHandle;
extern USB_HANDLE CDCDataInHandle;

extern CONTROL_SIGNAL_BITMAP control_signal_bitmap;

#define dummy_length    0x08
extern BYTE_VAL dummy_encapsulated_cmd_response[dummy_length];

extern int rx_blocked;



#endif
