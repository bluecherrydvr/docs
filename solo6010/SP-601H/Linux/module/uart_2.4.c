#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/tty.h>

#include "solo6010.h"

int solo6010_uart_interrupt(struct SOLO6010 *solo6010, int channel)
{
	solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_UART(channel));

	return 0;
}

int solo6010_uart_init(struct SOLO6010 *solo6010)
{
	return 0;
}

void solo6010_uart_exit(struct SOLO6010 *solo6010)
{
}

int solo6010_uart_register_driver(void)
{
	return 0;
}

void solo6010_uart_unregister_driver(void)
{
}

