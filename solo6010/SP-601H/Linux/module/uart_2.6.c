#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>

#include "solo6010.h"

#define PORT_SOLO6010	6010

struct SOLO6010_UART
{
	struct SOLO6010 *solo6010;
	unsigned int channel;
	unsigned int line;
	u32 ctrl;
	u32 old_gpio_config;
	int tx_enable;
	int rx_enable;
	struct uart_port port;
};

static inline unsigned char solo6010_uart_get_rx_data(struct SOLO6010_UART *uart)
{
	u32 data;

	data = solo6010_reg_read(uart->solo6010, REG_UART_RX_DATA(uart->channel));
	solo6010_reg_write(uart->solo6010, REG_UART_RX_DATA(uart->channel), UART_RX_DATA_POP);
	solo6010_reg_write(uart->solo6010, REG_UART_RX_DATA(uart->channel), 0);

	return (u8)(data & 0xff);
}

static inline void solo6010_uart_put_tx_data(struct SOLO6010_UART *uart, unsigned char data)
{
	solo6010_reg_write(uart->solo6010, REG_UART_TX_DATA(uart->channel), UART_TX_DATA_PUSH | (u32)data);
	solo6010_reg_write(uart->solo6010, REG_UART_TX_DATA(uart->channel), (u32)data);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
static void solo6010_uart_stop_tx(struct uart_port *port)
#else
static void solo6010_uart_stop_tx(struct uart_port *port, unsigned int tty_stop)
#endif
{
	struct SOLO6010_UART *uart = container_of(port, struct SOLO6010_UART, port);

	if(uart->tx_enable)
		uart->tx_enable = 0;
}

static int solo6010_uart_rx_chars(struct SOLO6010_UART *uart)
{
	struct uart_port *port = &uart->port;
	struct tty_struct *tty = port->info->tty;

	while(1)
	{
		u32 status;

		status = solo6010_reg_read(uart->solo6010, REG_UART_STATUS(uart->channel));
		if(UART_RX_BUFF_CNT(status) == 0)
			break;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 10))
		tty_insert_flip_char(port->info->tty, solo6010_uart_get_rx_data(uart), 0);
#else
		if(tty->flip.count >= TTY_FLIPBUF_SIZE)
		{
			tty->flip.work.func((void *)tty);
			if(tty->flip.count >= TTY_FLIPBUF_SIZE)
			{
				dbg_msg("TTY_DONT_FLIP set\n");
				return -1;
			}
		}

		*tty->flip.char_buf_ptr++ = solo6010_uart_get_rx_data(uart);
		*tty->flip.flag_buf_ptr++ = TTY_NORMAL;
		port->icount.rx++;
		tty->flip.count++;
#endif
	}

	tty_flip_buffer_push(tty);

	return 0;
}

static int solo6010_uart_tx_chars(struct SOLO6010_UART *uart)
{
	struct uart_port *port = &uart->port;
	struct circ_buf *xmit = &port->info->xmit;

	if(port->x_char)
	{
		solo6010_uart_put_tx_data(uart, port->x_char);
		port->icount.tx++;
		port->x_char = 0;

		return 0;
	}

	if(uart_circ_empty(xmit) || uart_tx_stopped(port))
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
		solo6010_uart_stop_tx(port);
#else
		solo6010_uart_stop_tx(port, 0);
#endif
		return 0;
	}

	while (!uart_circ_empty(xmit))
	{
		u32 status;

		status = solo6010_reg_read(uart->solo6010, REG_UART_STATUS(uart->channel));
		if(UART_TX_BUFF_CNT(status) == 0)
			break;

		solo6010_uart_put_tx_data(uart, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
	}

	if(uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (uart_circ_empty(xmit))
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
		solo6010_uart_stop_tx(port);
#else
		solo6010_uart_stop_tx(port, 0);
#endif
	}

	return 0;
}

static unsigned int solo6010_uart_tx_empty(struct uart_port *port)
{
	struct SOLO6010_UART *uart = container_of(port, struct SOLO6010_UART, port);
	u32 status;

	status = solo6010_reg_read(uart->solo6010, REG_UART_STATUS(uart->channel));
	return (UART_TX_BUFF_CNT(status) != 8) ? 0 : TIOCSER_TEMT;
}

static void solo6010_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static unsigned int solo6010_uart_get_mctrl(struct uart_port *port)
{
	struct SOLO6010_UART *uart = container_of(port, struct SOLO6010_UART, port);
	u32 status;

	status = solo6010_reg_read(uart->solo6010, REG_UART_STATUS(uart->channel));

	if(status & UART_CTS)
		return TIOCM_CAR | TIOCM_DSR | TIOCM_CTS;
	else
		return TIOCM_CAR | TIOCM_DSR;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
static void solo6010_uart_start_tx(struct uart_port *port)
#else
static void solo6010_uart_start_tx(struct uart_port *port, unsigned int tty_start)
#endif
{
	struct SOLO6010_UART *uart = container_of(port, struct SOLO6010_UART, port);

	if(!uart->tx_enable)
		uart->tx_enable = 1;

	solo6010_uart_tx_chars(uart);
}

static void solo6010_uart_stop_rx(struct uart_port *port)
{
	struct SOLO6010_UART *uart = container_of(port, struct SOLO6010_UART, port);

	if(uart->rx_enable)
		uart->rx_enable = 0;
}

static void solo6010_uart_enable_ms(struct uart_port *port)
{
}

static void solo6010_uart_break_ctl(struct uart_port *port, int break_state)
{
	dbg_msg("break_state - %08x\n", break_state);
}

static int solo6010_uart_startup(struct uart_port *port)
{
	struct SOLO6010_UART *uart = container_of(port, struct SOLO6010_UART, port);
	u32 gpio_config;

	gpio_config = solo6010_reg_read(uart->solo6010, REG_GPIO_CONFIG_0);
	uart->old_gpio_config = 0xff & (gpio_config>>(16 + (uart->channel*8)));
	gpio_config |= 0xff<<(16 + (uart->channel*8));
	solo6010_reg_write(uart->solo6010, REG_GPIO_CONFIG_0, gpio_config);

	uart->rx_enable = 1;
	uart->tx_enable = 1;

	solo6010_enable_interrupt(uart->solo6010, INTR_UART(uart->channel));

	return 0;
}

static void solo6010_uart_shutdown(struct uart_port *port)
{
	struct SOLO6010_UART *uart = container_of(port, struct SOLO6010_UART, port);
	u32 gpio_config;

	gpio_config = solo6010_reg_read(uart->solo6010, REG_GPIO_CONFIG_0);
	gpio_config &= ~(0xff<<(16 + (uart->channel*8)));
	gpio_config |= uart->old_gpio_config<<(16 + (uart->channel*8));
	solo6010_reg_write(uart->solo6010, REG_GPIO_CONFIG_0, gpio_config);

	uart->rx_enable = 0;
	uart->tx_enable = 0;

	solo6010_disable_interrupt(uart->solo6010, INTR_UART(uart->channel));
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20))
static void solo6010_uart_set_termios(struct uart_port *port, struct ktermios *termios, struct ktermios *old)
#else
static void solo6010_uart_set_termios(struct uart_port *port, struct termios *termios, struct termios *old)
#endif
{
	struct SOLO6010_UART *uart = container_of(port, struct SOLO6010_UART, port);
	unsigned long flags;
	unsigned int baud;

	uart->ctrl &= 0xffeff800;

	if(termios->c_cflag & CRTSCTS)
		uart->ctrl |= MODEM_CTRL_EN;

	baud = uart_get_baud_rate(port, termios, old, 0, 230400);

	switch(baud)
	{
		case 300:		uart->ctrl |= BAUDRATE_300;		break;
		case 1200:		uart->ctrl |= BAUDRATE_1200;	break;
		case 2400:		uart->ctrl |= BAUDRATE_2400;	break;
		case 4800:		uart->ctrl |= BAUDRATE_4800;	break;
		case 9600:		uart->ctrl |= BAUDRATE_9600;	break;
		case 19200:		uart->ctrl |= BAUDRATE_19200;	break;
		case 38400:		uart->ctrl |= BAUDRATE_38400;	break;
		case 57600:		uart->ctrl |= BAUDRATE_57600;	break;
		case 115200:	uart->ctrl |= BAUDRATE_115200;	break;
		case 230400:	uart->ctrl |= BAUDRATE_230400;	break;
		default:
			uart->ctrl |= BAUDRATE_9600;
			dbg_msg("unknown baud rate : %d\n", baud);
	}

	switch(termios->c_cflag & CSIZE)
	{
		case CS5:	uart->ctrl |= UART_DATA_BIT_5;	break;
		case CS6:	uart->ctrl |= UART_DATA_BIT_6;	break;
		case CS7:	uart->ctrl |= UART_DATA_BIT_7;	break;
		case CS8:	uart->ctrl |= UART_DATA_BIT_8;	break;
		default:
			uart->ctrl |= UART_DATA_BIT_8;
			dbg_msg("unknown CSIZE\n");
	}

	if(termios->c_cflag & CSTOPB)
		uart->ctrl |= UART_STOP_BIT_2;

	if(termios->c_cflag & PARENB)
		uart->ctrl |= (termios->c_cflag & PARODD) ? UART_PARITY_ODD : UART_PARITY_EVEN;

	spin_lock_irqsave(&port->lock, flags);

	solo6010_reg_write(uart->solo6010, REG_UART_CONTROL(uart->channel), uart->ctrl);

	uart_update_timeout(port, termios->c_cflag, baud);

	port->read_status_mask = UART_OVERRUN;
	if(termios->c_iflag & INPCK)
		port->read_status_mask |= UART_FRAME_ERR | UART_PARITY_ERR;

	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		port->ignore_status_mask |= UART_OVERRUN;
	if (termios->c_iflag & IGNBRK && termios->c_iflag & IGNPAR)
		port->ignore_status_mask |= UART_FRAME_ERR;

	spin_unlock_irqrestore(&port->lock, flags);
 }

static const char *solo6010_uart_type(struct uart_port *port)
{
	return "SOLO6010";
}

static void solo6010_uart_release_port(struct uart_port *port)
{
}

static int solo6010_uart_request_port(struct uart_port *port)
{
 	return 0;
}

static void solo6010_uart_config_port(struct uart_port *port, int flags)
{
}

static int solo6010_uart_verify_port(struct uart_port *port, struct serial_struct *serial)
{
 	return 0;
}

static struct uart_ops solo6010_uart_ops = {
	.tx_empty = solo6010_uart_tx_empty,
	.set_mctrl = solo6010_uart_set_mctrl,
	.get_mctrl = solo6010_uart_get_mctrl,
	.stop_tx = solo6010_uart_stop_tx,
	.start_tx = solo6010_uart_start_tx,
	.stop_rx = solo6010_uart_stop_rx,
	.enable_ms = solo6010_uart_enable_ms,
	.break_ctl = solo6010_uart_break_ctl,
	.startup = solo6010_uart_startup,
	.shutdown = solo6010_uart_shutdown,
	.set_termios = solo6010_uart_set_termios,
	.type = solo6010_uart_type,
	.release_port = solo6010_uart_release_port,
	.request_port = solo6010_uart_request_port,
	.config_port = solo6010_uart_config_port,
	.verify_port = solo6010_uart_verify_port,
};

static struct uart_driver solo6010_uart_driver = {
	.owner = THIS_MODULE,
	.driver_name = "ttySL",
	.dev_name = "ttySL",
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 18))
	.devfs_name = "ttySL",
#endif
	.major = 0,
	.minor = 0,
	.nr = 64,
	.cons = NULL,
};

int solo6010_uart_interrupt(struct SOLO6010 *solo6010, int channel)
{
	struct SOLO6010_UART *uart;
	u32 status;

	uart = (struct SOLO6010_UART *)solo6010->uart;
	uart += channel;

	status = solo6010_reg_read(solo6010, REG_UART_STATUS(channel));
	solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_UART(channel));

	if(status & (UART_OVERRUN | UART_FRAME_ERR | UART_PARITY_ERR))
	{
		struct uart_port *port = &uart->port;

		dbg_msg("uart status - %08x\n", status);

		status &= port->read_status_mask;

		if (status & ~port->ignore_status_mask & UART_OVERRUN)
		{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 10))
			tty_insert_flip_char(port->info->tty, 0, TTY_OVERRUN);
#else
			struct tty_struct *tty = port->info->tty;

			if(tty->flip.count >= TTY_FLIPBUF_SIZE)
			{
				tty->flip.work.func((void *)tty);
				if(tty->flip.count >= TTY_FLIPBUF_SIZE)
				{
					dbg_msg("TTY_DONT_FLIP set\n");
					return -1;
				}
			}

			*tty->flip.char_buf_ptr++ = 0;
			*tty->flip.flag_buf_ptr++ = TTY_OVERRUN;
			port->icount.rx++;
			tty->flip.count++;
#endif
		}
	}

	if(uart->rx_enable && UART_RX_BUFF_CNT(status))
		solo6010_uart_rx_chars(uart);

	if(uart->tx_enable && UART_TX_BUFF_CNT(status))
		solo6010_uart_tx_chars(uart);

	return 0;
}

int solo6010_uart_init(struct SOLO6010 *solo6010)
{
	struct SOLO6010_UART *uart;
	unsigned int clk_div;
	int i;

	uart = kmalloc(sizeof(struct SOLO6010_UART) * 2, GFP_KERNEL);
	if(uart == NULL)
		return -1;

	memset(uart, 0, sizeof(struct SOLO6010_UART) * 2);

	clk_div = (((solo6010->system_clock / 337500) + 5) / 10) - 1;
	dbg_msg("uart clk_div:%d\n", clk_div);

	for(i=0; i<2; i++)
	{
		uart[i].solo6010 = solo6010;
		uart[i].channel = i;
		uart[i].port.lock = SPIN_LOCK_UNLOCKED;
		uart[i].port.iotype = UPIO_MEM;
		uart[i].port.irq = solo6010->irq;
		uart[i].port.uartclk = 0;
		uart[i].port.fifosize = 8;
		uart[i].port.type = PORT_SOLO6010;
		uart[i].port.ops = &solo6010_uart_ops;
		uart[i].port.flags = 0;
		uart[i].port.line = (solo6010->chip.id * 2) + i;
		uart[i].port.dev = &solo6010->pdev->dev;

		uart[i].ctrl = UART_CLK_DIV(clk_div) | INTR_ERR_EN | INTR_RX_EN | INTR_TX_EN | RX_EN | TX_EN;
		uart_add_one_port(&solo6010_uart_driver, &uart[i].port);
	}

	solo6010->uart = uart;

	return 0;
}

void solo6010_uart_exit(struct SOLO6010 *solo6010)
{
	struct SOLO6010_UART *uart;
	int i;

	uart = solo6010->uart;

	solo6010_disable_interrupt(solo6010, INTR_UART(0));
	solo6010_disable_interrupt(solo6010, INTR_UART(1));

	for(i=0; i<2; i++)
	{
		uart_remove_one_port(&solo6010_uart_driver, &uart[i].port);
	}

	kfree(uart);
}

int solo6010_uart_register_driver(void)
{
	return uart_register_driver(&solo6010_uart_driver);

}

void solo6010_uart_unregister_driver(void)
{
	uart_unregister_driver(&solo6010_uart_driver);
}

