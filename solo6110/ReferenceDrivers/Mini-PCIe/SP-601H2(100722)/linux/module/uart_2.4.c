#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/tty.h>

#include "solo6x10.h"

int solo6x10_uart_interrupt(struct SOLO6x10 *solo6x10, int channel)
{
	solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_UART(channel));

	return 0;
}

int solo6x10_uart_init(struct SOLO6x10 *solo6x10)
{
	return 0;
}

void solo6x10_uart_exit(struct SOLO6x10 *solo6x10)
{
}

int solo6x10_uart_register_driver(void)
{
	return 0;
}

void solo6x10_uart_unregister_driver(void)
{
}

