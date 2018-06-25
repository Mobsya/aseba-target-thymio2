/********************************************************************
 FileName:     	usb_descriptors.c
 Dependencies:	See INCLUDES section
 Processor:		PIC18 or PIC24 USB Microcontrollers
 Hardware:		The code is natively intended to be used on the following
 				hardware platforms: PICDEM™ FS USB Demo Board, 
 				PIC18F87J50 FS USB Plug-In Module, or
 				Explorer 16 + PIC24 USB PIM.  The firmware may be
 				modified for use on other USB platforms by editing the
 				HardwareProfile.h file.
 Complier:  	Microchip C18 (for PIC18) or C30 (for PIC24)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the “Company”) for its PIC® Microcontroller is intended and
 supplied to you, the Company’s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

*********************************************************************
-usb_descriptors.c-
-------------------------------------------------------------------
Filling in the descriptor values in the usb_descriptors.c file:
-------------------------------------------------------------------

[Device Descriptors]
The device descriptor is defined as a USB_DEVICE_DESCRIPTOR type.  
This type is defined in usb_ch9.h  Each entry into this structure
needs to be the correct length for the data type of the entry.

[Configuration Descriptors]
The configuration descriptor was changed in v2.x from a structure
to a BYTE array.  Given that the configuration is now a byte array
each byte of multi-byte fields must be listed individually.  This
means that for fields like the total size of the configuration where
the field is a 16-bit value "64,0," is the correct entry for a
configuration that is only 64 bytes long and not "64," which is one
too few bytes.

The configuration attribute must always have the _DEFAULT
definition at the minimum. Additional options can be ORed
to the _DEFAULT attribute. Available options are _SELF and _RWU.
These definitions are defined in the usb_device.h file. The
_SELF tells the USB host that this device is self-powered. The
_RWU tells the USB host that this device supports Remote Wakeup.

[Endpoint Descriptors]
Like the configuration descriptor, the endpoint descriptors were 
changed in v2.x of the stack from a structure to a BYTE array.  As
endpoint descriptors also has a field that are multi-byte entities,
please be sure to specify both bytes of the field.  For example, for
the endpoint size an endpoint that is 64 bytes needs to have the size
defined as "64,0," instead of "64,"

Take the following example:
    // Endpoint Descriptor //
    0x07,                       //the size of this descriptor //
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,                   //EndpointAddress
    _INT,                       //Attributes
    0x08,0x00,                  //size (note: 2 bytes)
    0x02,                       //Interval

The first two parameters are self-explanatory. They specify the
length of this endpoint descriptor (7) and the descriptor type.
The next parameter identifies the endpoint, the definitions are
defined in usb_device.h and has the following naming
convention:
_EP<##>_<dir>
where ## is the endpoint number and dir is the direction of
transfer. The dir has the value of either 'OUT' or 'IN'.
The next parameter identifies the type of the endpoint. Available
options are _BULK, _INT, _ISO, and _CTRL. The _CTRL is not
typically used because the default control transfer endpoint is
not defined in the USB descriptors. When _ISO option is used,
addition options can be ORed to _ISO. Example:
_ISO|_AD|_FE
This describes the endpoint as an isochronous pipe with adaptive
and feedback attributes. See usb_device.h and the USB
specification for details. The next parameter defines the size of
the endpoint. The last parameter in the polling interval.

-------------------------------------------------------------------
Adding a USB String
-------------------------------------------------------------------
A string descriptor array should have the following format:

rom struct{byte bLength;byte bDscType;word string[size];}sdxxx={
sizeof(sdxxx),DSC_STR,<text>};

The above structure provides a means for the C compiler to
calculate the length of string descriptor sdxxx, where xxx is the
index number. The first two bytes of the descriptor are descriptor
length and type. The rest <text> are string texts which must be
in the unicode format. The unicode format is achieved by declaring
each character as a word type. The whole text string is declared
as a word array with the number of characters equals to <size>.
<size> has to be manually counted and entered into the array
declaration. Let's study this through an example:
if the string is "USB" , then the string descriptor should be:
(Using index 02)
rom struct{byte bLength;byte bDscType;word string[3];}sd002={
sizeof(sd002),DSC_STR,'U','S','B'};

A USB project may have multiple strings and the firmware supports
the management of multiple strings through a look-up table.
The look-up table is defined as:
rom const unsigned char *rom USB_SD_Ptr[]={&sd000,&sd001,&sd002};

The above declaration has 3 strings, sd000, sd001, and sd002.
Strings can be removed or added. sd000 is a specialized string
descriptor. It defines the language code, usually this is
US English (0x0409). The index of the string must match the index
position of the USB_SD_Ptr array, &sd000 must be in position
USB_SD_Ptr[0], &sd001 must be in position USB_SD_Ptr[1] and so on.
The look-up table USB_SD_Ptr is used by the get string handler
function.

-------------------------------------------------------------------

The look-up table scheme also applies to the configuration
descriptor. A USB device may have multiple configuration
descriptors, i.e. CFG01, CFG02, etc. To add a configuration
descriptor, user must implement a structure similar to CFG01.
The next step is to add the configuration descriptor name, i.e.
cfg01, cfg02,.., to the look-up table USB_CD_Ptr. USB_CD_Ptr[0]
is a dummy place holder since configuration 0 is the un-configured
state according to the definition in the USB specification.

********************************************************************/
 
/*********************************************************************
 * Descriptor specific type definitions are defined in:
 * usb_device.h
 *
 * Configuration options are defined in:
 * usb_config.h
 ********************************************************************/
#ifndef __USB_DESCRIPTORS_C
#define __USB_DESCRIPTORS_C
 
/** INCLUDES *******************************************************/
#include <usb/usb.h>
#include "usb_function_cdc.h"
#include <usb/usb_device_local.h>
#include <usb/usb_ch9.h>

/*                          _
__      ____ _ _ __ _ __ (_)_ __   __ _
\ \ /\ / / _` | '__| '_ \| | '_ \ / _` |
 \ V  V / (_| | |  | | | | | | | | (_| |
  \_/\_/ \__,_|_|  |_| |_|_|_| |_|\__, |
                                  |___/

All the variables here are shared with the usb bootloader
if you change the position of any variables then you need
to reflash the bootloader ... 

*/


/********************************************************************
 * Section A: Buffer Descriptor Table
 * - 256 bytes max.  Actual size depends on number of endpoints enabled and 
 *   the ping pong buffering mode.
 * - USB_MAX_EP_NUMBER is defined in usb_config.h
 *******************************************************************/
#if (USB_PING_PONG_MODE == USB_PING_PONG__NO_PING_PONG)
    volatile BDT_ENTRY BDT[(USB_MAX_EP_NUMBER + 1) * 2] __attribute__ ((address(0x1000),noload));
#elif (USB_PING_PONG_MODE == USB_PING_PONG__EP0_OUT_ONLY)
    volatile BDT_ENTRY BDT[((USB_MAX_EP_NUMBER + 1) * 2)+1] __attribute__ ((address(0x1000),noload));
#elif (USB_PING_PONG_MODE == USB_PING_PONG__FULL_PING_PONG)
    volatile BDT_ENTRY BDT[(USB_MAX_EP_NUMBER + 1) * 4] __attribute__ ((address(0x1000),noload));
#elif (USB_PING_PONG_MODE == USB_PING_PONG__ALL_BUT_EP0)
    volatile BDT_ENTRY BDT[((USB_MAX_EP_NUMBER + 1) * 4)-2] __attribute__ ((address(0x1000),noload));
#else
    #error "No ping pong mode defined."
#endif

/********************************************************************
 * Section B: EP0 Buffer Space
 *******************************************************************/
volatile  __attribute((address(0x1000 + sizeof(BDT)),noload)) CTRL_TRF_SETUP  SetupPkt;           // 8-byte only
volatile __attribute((address(0x1000 + sizeof(BDT) + 0x8),noload)) BYTE  CtrlTrfData[USB_EP0_BUFF_SIZE] ;

USB_VOLATILE USB_DEVICE_STATE  USBDeviceState __attribute((address(0x1000 + sizeof(BDT) + 0x10),noload));
USB_VOLATILE BYTE  USBActiveConfiguration __attribute((address(0x1000 + sizeof(BDT) + 0x12),noload));
USB_VOLATILE BYTE  USBAlternateInterface[USB_MAX_NUM_INT] __attribute((address(0x1000 + sizeof(BDT) + 0x14),noload));
volatile BDT_ENTRY  *pBDTEntryEP0OutCurrent __attribute((address(0x1000 + sizeof(BDT) + 0x16),noload));
volatile BDT_ENTRY  *pBDTEntryEP0OutNext __attribute((address(0x1000 + sizeof(BDT) + 0x18),noload));
volatile BDT_ENTRY  *pBDTEntryOut[USB_MAX_EP_NUMBER+1] __attribute((address(0x1000 + sizeof(BDT) + 0x1a),noload));
volatile BDT_ENTRY  *pBDTEntryIn[USB_MAX_EP_NUMBER+1] __attribute((address(0x1000 + sizeof(BDT) + 0x1c + 2*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE BYTE  shortPacketStatus __attribute((address(0x1000 + sizeof(BDT) + 0x1e + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE BYTE  controlTransferState __attribute((address(0x1000 + sizeof(BDT) + 0x20 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE IN_PIPE  inPipes[1] __attribute((address(0x1000 + sizeof(BDT) + 0x22 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE OUT_PIPE  outPipes[1] __attribute((address(0x1000 + sizeof(BDT) + 0x28 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE BYTE  *pDst __attribute((address(0x1000 + sizeof(BDT) + 0x30 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE BYTE  endpoint_number __attribute((address(0x1000 + sizeof(BDT) + 0x32 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE USTAT_FIELDS  USTATcopy __attribute((address(0x1000 + sizeof(BDT) + 0x34 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE EP_STATUS  ep_data_in[USB_MAX_EP_NUMBER+1] __attribute((address(0x1000 + sizeof(BDT) + 0x36 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE EP_STATUS  ep_data_out[USB_MAX_EP_NUMBER+1] __attribute((address(0x1000 + sizeof(BDT) + 0x3a + 4*USB_MAX_EP_NUMBER),noload));
volatile BOOL  USBDeferStatusStagePacket __attribute((address(0x1000 + sizeof(BDT) + 0x3e + 4*USB_MAX_EP_NUMBER),noload));
volatile BOOL  USBStatusStageEnabledFlag1 __attribute((address(0x1000 + sizeof(BDT) + 0x40 + 4*USB_MAX_EP_NUMBER),noload));
volatile BOOL  USBStatusStageEnabledFlag2 __attribute((address(0x1000 + sizeof(BDT) + 0x42 + 4*USB_MAX_EP_NUMBER),noload));
volatile BOOL  USBDeferINDataStagePackets __attribute((address(0x1000 + sizeof(BDT) + 0x44 + 4*USB_MAX_EP_NUMBER),noload));
volatile BOOL  USBDeferOUTDataStagePackets __attribute((address(0x1000 + sizeof(BDT) + 0x46 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE BOOL  BothEP0OutUOWNsSet __attribute((address(0x1000 + sizeof(BDT) + 0x48 + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE BOOL  RemoteWakeup __attribute((address(0x1000 + sizeof(BDT) + 0x4a + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE BOOL  USBBusIsSuspended __attribute((address(0x1000 + sizeof(BDT) + 0x4c + 4*USB_MAX_EP_NUMBER),noload));
USB_VOLATILE BYTE  USBStatusStageTimeoutCounter __attribute((address(0x1000 + sizeof(BDT) + 0x4e + 4*USB_MAX_EP_NUMBER),noload));

unsigned char  cdc_data_rx[CDC_DATA_OUT_EP_SIZE] __attribute((address(0x1000 + sizeof(BDT) + 0x50 + 4*USB_MAX_EP_NUMBER),noload));
unsigned char  cdc_data_tx[CDC_DATA_IN_EP_SIZE] __attribute((address(0x1000 + sizeof(BDT) + 0x90 + 4*USB_MAX_EP_NUMBER),noload));
LINE_CODING  line_coding __attribute((address(0x1000 + sizeof(BDT) + 0xd0 + 4*USB_MAX_EP_NUMBER),noload));    // Buffer to store line coding information

USB_HANDLE  CDCDataOutHandle __attribute((address(0x1000 + sizeof(BDT) + 0xd8 + 4*USB_MAX_EP_NUMBER),noload));
USB_HANDLE  CDCDataInHandle __attribute((address(0x1000 + sizeof(BDT) + 0xda + 4*USB_MAX_EP_NUMBER),noload));

CONTROL_SIGNAL_BITMAP  control_signal_bitmap __attribute((address(0x1000 + sizeof(BDT) + 0xdc + 4*USB_MAX_EP_NUMBER),noload));

#define dummy_length    0x08
BYTE_VAL  dummy_encapsulated_cmd_response[dummy_length] __attribute((address(0x1000 + sizeof(BDT) + 0xde + 4*USB_MAX_EP_NUMBER)));


int  rx_blocked __attribute((address(0x1000 + sizeof(BDT) + 0xe6 + 4*USB_MAX_EP_NUMBER)));

/* Device Descriptor */
USB_DEVICE_DESCRIPTOR __attribute((address(0x1000+sizeof(BDT)+0xe8 + 4*USB_MAX_EP_NUMBER)))  device_dsc=
{
    0x12,                   // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    CDC_DEVICE,             // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usb_config.h
    0x0617,                 // Vendor ID ( EPFL VID )
    0x000A,                 // Product ID: Thymio II ( Assigned ... )
    0x0000,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
BYTE __attribute((address(0x1000+sizeof(BDT)+0xfa + 4*USB_MAX_EP_NUMBER))) configDescriptor1[]={
    /* Configuration Descriptor */
    0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
    67,0,                   // Total length of data for this cfg
    2,                      // Number of interfaces in this cfg
    1,                      // Index value of this configuration
    0,                      // Configuration string index
    _DEFAULT,               // Attributes, see usb_device.h
    250,                     // Max power consumption (2X mA)
							
    /* Interface Descriptor */
    9,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
    0,                      // Interface Number
    0,                      // Alternate Setting Number
    1,                      // Number of endpoints in this intf
    COMM_INTF,              // Class code
    ABSTRACT_CONTROL_MODEL, // Subclass code
    V25TER,                 // Protocol code
    0,                      // Interface string index

    /* CDC Class-Specific Descriptors */
    sizeof(USB_CDC_HEADER_FN_DSC),
    CS_INTERFACE,
    DSC_FN_HEADER,
    0x10,0x01,

    sizeof(USB_CDC_ACM_FN_DSC),
    CS_INTERFACE,
    DSC_FN_ACM,
    USB_CDC_ACM_FN_DSC_VAL,

    sizeof(USB_CDC_UNION_FN_DSC),
    CS_INTERFACE,
    DSC_FN_UNION,
    CDC_COMM_INTF_ID,
    CDC_DATA_INTF_ID,

    sizeof(USB_CDC_CALL_MGT_FN_DSC),
    CS_INTERFACE,
    DSC_FN_CALL_MGT,
    0x00,
    CDC_DATA_INTF_ID,

    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP02_IN,_INT,CDC_INT_EP_SIZE,0x02,
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP01_IN,            //EndpointAddress
    _INTERRUPT,                       //Attributes
    0x08,0x00,                  //size
    0x02,                       //Interval

    /* Interface Descriptor */
    9,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
    1,                      // Interface Number
    0,                      // Alternate Setting Number
    2,                      // Number of endpoints in this intf
    DATA_INTF,              // Class code
    0,                      // Subclass code
    NO_PROTOCOL,            // Protocol code
    0,                      // Interface string index
    
    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP03_OUT,_BULK,CDC_BULK_OUT_EP_SIZE,0x00,
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_OUT,            //EndpointAddress
    _BULK,                       //Attributes
    0x40,0x00,                  //size
    0x00,                       //Interval

    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP03_IN,_BULK,CDC_BULK_IN_EP_SIZE,0x00
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,            //EndpointAddress
    _BULK,                       //Attributes
    0x40,0x00,                  //size
    0x00,                       //Interval
};


//Language code string descriptor
__attribute((address(0x1000+sizeof(BDT)+0x13e + 4*USB_MAX_EP_NUMBER))) struct{BYTE bLength;BYTE bDscType;WORD string[1];}sd000={
sizeof(sd000),USB_DESCRIPTOR_STRING,{0x0409}};

//Manufacturer string descriptor
__attribute((address(0x1000+sizeof(BDT)+0x142 + 4*USB_MAX_EP_NUMBER))) struct{BYTE bLength;BYTE bDscType;WORD string[10];}sd001={
sizeof(sd001),USB_DESCRIPTOR_STRING,
{'M','o','b','s','y','a','.','o','r','g'
}};

//Product string descriptor
__attribute((address(0x1000+sizeof(BDT)+0x158 + 4*USB_MAX_EP_NUMBER))) struct{BYTE bLength;BYTE bDscType;WORD string[22];}sd002={
sizeof(sd002),USB_DESCRIPTOR_STRING,
{'T','h','y','m','i','o','-','I','I',' ',' ',
' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
};

//Array of configuration descriptors
__attribute((address(0x1000+sizeof(BDT)+0x186 + 4*USB_MAX_EP_NUMBER))) BYTE * USB_CD_Ptr[]=
{
    (BYTE *)&configDescriptor1
};
//Array of string descriptors
__attribute((address(0x1000+sizeof(BDT)+0x188 + 4*USB_MAX_EP_NUMBER))) BYTE * USB_SD_Ptr[USB_NUM_STRING_DESCRIPTORS]=
{
    (BYTE *)&sd000,
    (BYTE *)&sd001,
    (BYTE *)&sd002
};


#endif
/** EOF usb_descriptors.c ****************************************************/
