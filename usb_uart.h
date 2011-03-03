#ifndef _USB_UART_H_
#define _USB_UART_H_

void usb_uart_init(void);
void usb_uart_tick(void);
int usb_uart_serial_port_open(void);
#endif
