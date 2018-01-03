/********************************************************************************
  File Information:
    FileName:       usb_function_cdc.c
    Dependencies:   See INCLUDES section
    Processor:      PIC18 or PIC24 USB Microcontrollers
    Hardware:       The code is natively intended to be used on the following
                    hardware platforms: PICDEM™ FS USB Demo Board,
                    PIC18F87J50 FS USB Plug-In Module, or
                    Explorer 16 + PIC24 USB PIM.  The firmware may be
                    modified for use on other USB platforms by editing the
                    HardwareProfile.h file.
    Complier:   Microchip C18 (for PIC18) or C30 (for PIC24)
    Company:        Microchip Technology, Inc.

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
    
  Summary:
    This file contains all of functions, macros, definitions, variables,
    datatypes, etc. that are required for usage with the CDC function
    driver. This file should be included in projects that use the CDC
    \function driver.
    
    
    
    This file is located in the "\<Install Directory\>\\Microchip\\USB\\CDC
    Device Driver" directory.
  Description:
    USB CDC Function Driver File
    
    This file contains all of functions, macros, definitions, variables,
    datatypes, etc. that are required for usage with the CDC function
    driver. This file should be included in projects that use the CDC
    \function driver.
    
    This file is located in the "\<Install Directory\>\\Microchip\\USB\\CDC
    Device Driver" directory.
    
    When including this file in a new project, this file can either be
    referenced from the directory in which it was installed or copied
    directly into the user application folder. If the first method is
    chosen to keep the file located in the folder in which it is installed
    then include paths need to be added so that the library and the
    application both know where to reference each others files. If the
    application folder is located in the same folder as the Microchip
    folder (like the current demo folders), then the following include
    paths need to be added to the application's project:
    
    ..\\Include
    
    .
    
    If a different directory structure is used, modify the paths as
    required. An example using absolute paths instead of relative paths
    would be the following:
    
    C:\\Microchip Solutions\\Microchip\\Include
    
    C:\\Microchip Solutions\\My Demo Application                                 
  ********************************************************************************/

/********************************************************************
 Change History:
  Rev    Description
  ----   -----------
  2.3    Decricated the mUSBUSARTIsTxTrfReady() macro.  It is 
         replaced by the USBUSARTIsTxTrfReady() function.

  2.6    Minor definition changes

  2.6a   No Changes

  2.7    Fixed error in the part support list of the variables section
         where the address of the CDC variables are defined.  The 
         PIC18F2553 was incorrectly named PIC18F2453 and the PIC18F4558
         was incorrectly named PIC18F4458.

         http://www.microchip.com/forums/fb.aspx?m=487397

  2.8    Minor change to CDCInitEP() to enhance ruggedness in
         multithreaded usage scenarios.

********************************************************************/

/** I N C L U D E S **********************************************************/
#include "USB/usb.h"
#include "usb_function_cdc.h"
#include <transport/microchip_usb/usb-buffer.h>

#ifdef USB_USE_CDC

#include <usb_global.h>



/**************************************************************************
  SEND_ENCAPSULATED_COMMAND and GET_ENCAPSULATED_RESPONSE are required
  requests according to the CDC specification.
  However, it is not really being used here, therefore a dummy buffer is
  used for conformance.
 **************************************************************************/


#if defined(USB_CDC_SET_LINE_CODING_HANDLER)
CTRL_TRF_RETURN USB_SECTION USB_CDC_SET_LINE_CODING_HANDLER(CTRL_TRF_PARAMS);
#endif



/** C L A S S  S P E C I F I C  R E Q ****************************************/
/******************************************************************************
 	Function:
 		void USBCheckCDCRequest(void)
 
 	Description:
 		This routine checks the setup data packet to see if it
 		knows how to handle it
 		
 	PreCondition:
 		None

	Parameters:
		None
		
	Return Values:
		None
		
	Remarks:
		None
		 
  *****************************************************************************/
void USBCheckCDCRequest(void)
{
    /*
     * If request recipient is not an interface then return
     */
    if(SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) return;

    /*
     * If request type is not class-specific then return
     */
    if(SetupPkt.RequestType != USB_SETUP_TYPE_CLASS_BITFIELD) return;

    /*
     * Interface ID must match interface numbers associated with
     * CDC class, else return
     */
    if((SetupPkt.bIntfID != CDC_COMM_INTF_ID)&&
       (SetupPkt.bIntfID != CDC_DATA_INTF_ID)) return;
    
    switch(SetupPkt.bRequest)
    {
        //****** These commands are required ******//
        case SEND_ENCAPSULATED_COMMAND:
         //send the packet
            inPipes[0].pSrc.bRam = (BYTE*)&dummy_encapsulated_cmd_response;
            inPipes[0].wCount.Val = dummy_length;
            inPipes[0].info.bits.ctrl_trf_mem = USB_EP0_RAM;
            inPipes[0].info.bits.busy = 1;
            break;
        case GET_ENCAPSULATED_RESPONSE:
            // Populate dummy_encapsulated_cmd_response first.
            inPipes[0].pSrc.bRam = (BYTE*)&dummy_encapsulated_cmd_response;
            inPipes[0].info.bits.busy = 1;
            break;
        //****** End of required commands ******//

        #if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1)
        case SET_LINE_CODING:
            outPipes[0].wCount.Val = SetupPkt.wLength;
            outPipes[0].pDst.bRam = (BYTE*)LINE_CODING_TARGET;
            outPipes[0].pFunc = LINE_CODING_PFUNC;
            outPipes[0].info.bits.busy = 1;
            break;
            
        case GET_LINE_CODING:
            USBEP0SendRAMPtr(
                (BYTE*)&line_coding,
                LINE_CODING_LENGTH,
                USB_EP0_INCLUDE_ZERO);
            break;

        case SET_CONTROL_LINE_STATE:
            control_signal_bitmap._byte = (BYTE)SetupPkt.W_Value.v[0];
            CONFIGURE_RTS(control_signal_bitmap.CARRIER_CONTROL);
            CONFIGURE_DTR(control_signal_bitmap.DTE_PRESENT);
            inPipes[0].info.bits.busy = 1;
            break;
        #endif

        #if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2)
        case SEND_BREAK:                        // Optional
            inPipes[0].info.bits.busy = 1;
			if (SetupPkt.wValue == 0xFFFF)
			{
				UART_ENABLE = 0;  // turn off USART
				UART_TRISTx = 0;   // Make TX pin an output
				UART_Tx = 0;   // make it low
			}
			else if (SetupPkt.wValue == 0x0000)
			{
				UART_ENABLE = 1;  // turn on USART
				UART_TRISTx = 1;   // Make TX pin an input
			}
			else
			{
                UART_SEND_BREAK();
			}
            break;
        #endif
        default:
            break;
    }//end switch(SetupPkt.bRequest)

}//end USBCheckCDCRequest


void handle_rx_urb(USB_HANDLE urb) {
	unsigned char len;
	/* Got some data from the PC */
	/* TODO: Call a callback to aseba, check if data is consumed.
	   if yes: Rearm the URB
	   if not: Do not rearm the URB let it as is. wait until aseba consume the previous one
	*/
	if(urb == NULL) 
		urb = CDCDataOutHandle;
	
	len = USBHandleGetLength(urb);
	
	
	if(len > 0 && AsebaUsbBulkRecv(cdc_data_rx, len)) {
		rx_blocked = 1;
		// Don't rearm, wait until Aseba call USBCDCKickRx()
		return;
	}
	
	// Mark the packet as processed.
	((volatile BDT_ENTRY*)urb)->CNT = 0;
	
	// Rearm
	CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(BYTE*)cdc_data_rx,sizeof(cdc_data_rx));
}

void handle_tx_urb(void) {
	unsigned char size = AsebaTxReady(cdc_data_tx);
	
	if(size) {
		CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,(BYTE *)cdc_data_tx,size);
	} else {
		if(CDCDataInHandle && (USBHandleGetLength(CDCDataInHandle) == 64)) {
			// Send ZLP
			CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,NULL,0);
		}
	}
}

void USBCBTransfertDoneHandler(USB_HANDLE urb) {
	if(CDCDataOutHandle == urb)
		handle_rx_urb(urb);
	else if(CDCDataInHandle == urb)
		handle_tx_urb();
}

int USBTXBusy(void) {
	return USBHandleBusy(CDCDataInHandle);
}

/* Must be called with usb interrupt masked */
void USBCDCKickTx(void) {
	if(USBHandleBusy(CDCDataInHandle))
		return;
		
	handle_tx_urb();
}	

void USBCDCKickRx(void) {
	if(!rx_blocked)
		return;
	
	if(USBHandleBusy(CDCDataOutHandle))
		return;

	handle_rx_urb(CDCDataOutHandle);
}

void usbInterrupt(void) {
	if(!USBHandleBusy(CDCDataOutHandle))
		handle_rx_urb(CDCDataOutHandle);
	if(!USBHandleBusy(CDCDataInHandle))
		handle_tx_urb();
}

/** U S E R  A P I ***********************************************************/

/**************************************************************************
  Function:
        void CDCInitEP(void)
    
  Summary:
    This function initializes the CDC function driver. This function should
    be called after the SET_CONFIGURATION command.
  Description:
    This function initializes the CDC function driver. This function sets
    the default line coding (baud rate, bit parity, number of data bits,
    and format). This function also enables the endpoints and prepares for
    the first transfer from the host.
    
    This function should be called after the SET_CONFIGURATION command.
    This is most simply done by calling this function from the
    USBCBInitEP() function.
    
    Typical Usage:
    <code>
        void USBCBInitEP(void)
        {
            CDCInitEP();
        }
    </code>
  Conditions:
    None
  Remarks:
    None                                                                   
  **************************************************************************/
void CDCInitEP(void)
{
   	//Abstract line coding information
   	line_coding.dwDTERate.Val = 19200;      // baud rate
   	line_coding.bCharFormat = 0x00;             // 1 stop bit
   	line_coding.bParityType = 0x00;             // None
   	line_coding.bDataBits = 0x08;               // 5,6,7,8, or 16

    /*
     * Do not have to init Cnt of IN pipes here.
     * Reason:  Number of BYTEs to send to the host
     *          varies from one transaction to
     *          another. Cnt should equal the exact
     *          number of BYTEs to transmit for
     *          a given IN transaction.
     *          This number of BYTEs will only
     *          be known right before the data is
     *          sent.
     */
    USBEnableEndpoint(CDC_COMM_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    USBEnableEndpoint(CDC_DATA_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(BYTE*)&cdc_data_rx,sizeof(cdc_data_rx));
    CDCDataInHandle = NULL;
    
}//end CDCInitEP




#endif //USB_USE_CDC

/** EOF cdc.c ****************************************************************/
