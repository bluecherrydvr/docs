#ifndef SOLO6x10_UART_H
#define SOLO6x10_UART_H

int solo6x10_uart_interrupt(struct SOLO6x10 *solo6x10, int channel);
int solo6x10_uart_init(struct SOLO6x10 *solo6x10);
void solo6x10_uart_exit(struct SOLO6x10 *solo6x10);
int solo6x10_uart_register_driver(void);
void solo6x10_uart_unregister_driver(void);

#endif

