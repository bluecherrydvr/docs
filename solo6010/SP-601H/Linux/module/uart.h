#ifndef SOLO6010_UART_H
#define SOLO6010_UART_H

int solo6010_uart_interrupt(struct SOLO6010 *solo6010, int channel);
int solo6010_uart_init(struct SOLO6010 *solo6010);
void solo6010_uart_exit(struct SOLO6010 *solo6010);
int solo6010_uart_register_driver(void);
void solo6010_uart_unregister_driver(void);

#endif

