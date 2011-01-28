#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>

#include "solo6x10.h"

#define PORT_SOLO6x10	6010

struct SOLO6x10_UART
{
	struct SOLO6x10 *solo6x10;
	unsigned int channel;
	unsigned int line;
	u32 ctrl;
	int tx_enable;
	int rx_enable;
	struct uart_port port;
};

static inline unsigned char solo6x10_uart_get_rx_data(struct SOLO6x10_UART *uart)
{
	u32 data;

	data = solo6x10_reg_read(uart->solo6x10, REG_UART_RX_DATA(uart->channel));
	solo6x10_reg_write(uart->solo6x10, REG_UART_RX_DATA(uart->channel), UART_RX_DATA_POP);
	solo6x10_reg_write(uart->solo6x10, REG_UART_RX_DATA(uart->channel), 0);

	return (u8)(data & 0xff);
}

static inline void solo6x10_uart_put_tx_data(struct SOLO6x10_UART *uart, unsigned char data)
{
	solo6x10_reg_write(uart->solo6x10, REG_UART_TX_DATA(uart->channel), UART_TX_DATA_PUSH | (u32)data);
	solo6x10_reg_write(uart->solo6x10, REG_UART_TX_DATA(uart->channel), (u32)data);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
static void solo6x10_uart_stop_tx(struct uart_port *port)
#else
static void solo6x10_uart_stop_tx(struct uart_port *port, unsigned int tty_stop)
#endif
{
	struct SOLO6x10_UART *uart = container_of(port, struct SOLO6x10_UART, port);

	if(uart->tx_enable)
		uart->tx_enable = 0;
}

static int solo6x10_uart_rx_chars(struct SOLO6x10_UART *uart)
{
	struct uart_port *port = &uart->port;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	struct tty_struct *tty = port->info->port.tty;
#else
	struct tty_struct *tty = port->info->tty;
#endif

	while(1)
	{
		u32 status;

		status = solo6x10_reg_read(uart->solo6x10, REG_UART_STATUS(uart->channel));
		if(UART_RX_BUFF_CNT(status) == 0)
			break;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
		tty_insert_flip_char(port->info->port.tty, solo6x10_uart_get_rx_data(uart), 0);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 10))
		tty_insert_flip_char(port->info->tty, solo6x10_uart_get_rx_data(uart), 0);
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

		*tty->flip.char_buf_ptr++ = solo6x10_uart_get_rx_data(uart);
		*tty->flip.flag_buf_ptr++ = TTY_NORMAL;
		port->icount.rx++;
		tty->flip.count++;
#endif
	}

	tty_flip_buffer_push(tty);

	return 0;
}

static int solo6x10_uart_tx_chars(struct SOLO6x10_UART *uart)
{
	struct uart_port *port = &uart->port;
	struct circ_buf *xmit = &port->info->xmit;

	if(port->x_char)
	{
		solo6x10_uart_put_tx_data(uart, port->x_char);
		port->icount.tx++;
		port->x_char = 0;

		return 0;
	}

	if(uart_circ_empty(xmit) || uart_tx_stopped(port))
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
		solo6x10_uart_stop_tx(port);
#else
		solo6x10_uart_stop_tx(port, 0);
#endif
		return 0;
	}

	while (!uart_circ_empty(xmit))
	{
		u32 status;

		status = solo6x10_reg_read(uart->solo6x10, REG_UART_STATUS(uart->channel));
		if(UART_TX_BUFF_CNT(status) == 0)
			break;

		solo6x10_uart_put_tx_data(uart, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
	}

	if(uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (uart_circ_empty(xmit))
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
		solo6x10_uart_stop_tx(port);
#else
		solo6x10_uart_stop_tx(port, 0);
#endif
	}

	return 0;
}

static unsigned int solo6x10_uart_tx_empty(struct uart_port *port)
{
	struct SOLO6x10_UART *uart = container_of(port, struct SOLO6x10_UART, port);
	u32 status;

	status = solo6x10_reg_read(uart->solo6x10, REG_UART_STATUS(uart->channel));
	return (UART_TX_BUFF_CNT(status) != 8) ? 0 : TIOCSER_TEMT;
}

static void solo6x10_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static unsigned int solo6x10_uart_get_mctrl(struct uart_port *port)
{
	struct SOLO6x10_UART *uart = container_of(port, struct SOLO6x10_UART, port);
	u32 status;

	status = solo6x10_reg_read(uart->solo6x10, REG_UART_STATUS(uart->channel));

	if(status & UART_CTS)
		return TIOCM_CAR | TIOCM_DSR | TIOCM_CTS;
	else
		return TIOCM_CAR | TIOCM_DSR;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
static void solo6x10_uart_start_tx(struct uart_port *port)
#else
static void solo6x10_uart_start_tx(struct uart_port *port, unsigned int tty_start)
#endif
{
	struct SOLO6x10_UART *uart = container_of(port, struct SOLO6x10_UART, port);

	if(!uart->tx_enable)
		uart->tx_enable = 1;

	solo6x10_uart_tx_chars(uart);
}

static void solo6x10_uart_stop_rx(struct uart_port *port)
{
	struct SOLO6x10_UART *uart = container_of(port, struct SOLO6x10_UART, port);

	if(uart->rx_enable)
	{
		uart->ctrl &= ~RX_EN;
		solo6x10_reg_write(uart->solo6x10, REG_UART_CONTROL(uart->channel), uart->ctrl);
		uart->rx_enable = 0;
	}
}

static void solo6x10_uart_enable_ms(struct uart_port *port)
{
}

static void solo6x10_uart_break_ctl(struct uart_port *port, int break_state)
{
	dbg_msg("break_state - %08x\n", break_state);
}

static int solo6x10_uart_startup(struct uart_port *port)
{
	struct SOLO6x10_UART *uart = container_of(port, struct SOLO6x10_UART, port);
	u32 gpio_config;

	gpio_config = solo6x10_reg_read(uart->solo6x10, REG_GPIO_CONFIG_0);
	gpio_config |= 0xff<<(16 + (uart->channel*8));
	solo6x10_reg_write(uart->solo6x10, REG_GPIO_CONFIG_0, gpio_config);

	uart->rx_enable = 1;
	uart->tx_enable = 1;

	uart->ctrl |= RX_EN;
	solo6x10_reg_write(uart->solo6x10, REG_UART_CONTROL(uart->channel), uart->ctrl);

	return 0;
}

static void solo6x10_uart_shutdown(struct uart_port *port)
{
	struct SOLO6x10_UART *uart = container_of(port, struct SOLO6x10_UART, port);

	if(uart->rx_enable)
	{
		uart->ctrl &= ~RX_EN;
		solo6x10_reg_write(uart->solo6x10, REG_UART_CONTROL(uart->channel), uart->ctrl);
		uart->rx_enable = 0;
	}

	if(uart->tx_enable)
		uart->tx_enable = 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20))
static void solo6x10_uart_set_termios(struct uart_port *port, struct ktermios *termios, struct ktermios *old)
#else
static void solo6x10_uart_set_termios(struct uart_port *port, struct termios *termios, struct termios *old)
#endif
{
	struct SOLO6x10_UART *uart = container_of(port, struct SOLO6x10_UART, port);
	struct SOLO6x10 *solo6x10;
	unsigned long flags;
	unsigned int baud;

	solo6x10 = uart->solo6x10;

	uart->ctrl &= 0xffeff800;

	if(termios->c_cflag & CRTSCTS)
		uart->ctrl |= MODEM_CTRL_EN;

	baud = uart_get_baud_rate(port, termios, old, 0, 230400);

	if(solo6x10->pdev->device == 0x6110)
	{
		unsigned int clk_div;

		switch(baud)
		{
			case 1200:
				uart->ctrl |= BAUDRATE_300;
				clk_div = ((((solo6x10->system_clock / 4) / 337500) + 5) / 10) - 1;
				break;
			case 2400:
				uart->ctrl |= BAUDRATE_1200;
				clk_div = ((((solo6x10->system_clock / 2) / 337500) + 5) / 10) - 1;
				break;
			case 4800:
				uart->ctrl |= BAUDRATE_2400;
				clk_div = ((((solo6x10->system_clock / 2) / 337500) + 5) / 10) - 1;
				break;
			case 9600:
				uart->ctrl |= BAUDRATE_4800;
				clk_div = ((((solo6x10->system_clock / 2) / 337500) + 5) / 10) - 1;
				break;
			case 19200:
				uart->ctrl |= BAUDRATE_9600;
				clk_div = ((((solo6x10->system_clock / 2) / 337500) + 5) / 10) - 1;
				break;
			case 38400:
				uart->ctrl |= BAUDRATE_19200;
				clk_div = ((((solo6x10->system_clock / 2) / 337500) + 5) / 10) - 1;
				break;
			case 57600:
				uart->ctrl |= BAUDRATE_19200;
				clk_div = ((((solo6x10->system_clock / 3) / 337500) + 5) / 10) - 1;
				break;
			case 115200:
				uart->ctrl |= BAUDRATE_57600;
				clk_div = ((((solo6x10->system_clock / 2) / 337500) + 5) / 10) - 1;
				break;
			case 230400:
				uart->ctrl |= BAUDRATE_115200;
				clk_div = ((((solo6x10->system_clock / 2) / 337500) + 5) / 10) - 1;
				break;
			default:
				uart->ctrl |= BAUDRATE_4800;
				clk_div = ((((solo6x10->system_clock / 2) / 337500) + 5) / 10) - 1;
				dbg_msg("unknown baud rate : %d\n", baud);
		}
		uart->ctrl &= ~0x1f000000;
		uart->ctrl |= UART_CLK_DIV(clk_div);
		//dbg_msg("uart clk_div:%d\n", clk_div);
	}
	else
	{
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

	solo6x10_reg_write(solo6x10, REG_UART_CONTROL(uart->channel), uart->ctrl);

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

static const char *solo6x10_uart_type(struct uart_port *port)
{
	return "SOLO6x10";
}

static void solo6x10_uart_release_port(struct uart_port *port)
{
}

static int solo6x10_uart_request_port(struct uart_port *port)
{
 	return 0;
}

static void solo6x10_uart_config_port(struct uart_port *port, int flags)
{
}

static int solo6x10_uart_verify_port(struct uart_port *port, struct serial_struct *serial)
{
 	return 0;
}

static struct uart_ops solo6x10_uart_ops = {
	.tx_empty = solo6x10_uart_tx_empty,
	.set_mctrl = solo6x10_uart_set_mctrl,
	.get_mctrl = solo6x10_uart_get_mctrl,
	.stop_tx = solo6x10_uart_stop_tx,
	.start_tx = solo6x10_uart_start_tx,
	.stop_rx = solo6x10_uart_stop_rx,
	.enable_ms = solo6x10_uart_enable_ms,
	.break_ctl = solo6x10_uart_break_ctl,
	.startup = solo6x10_uart_startup,
	.shutdown = solo6x10_uart_shutdown,
	.set_termios = solo6x10_uart_set_termios,
	.type = solo6x10_uart_type,
	.release_port = solo6x10_uart_release_port,
	.request_port = solo6x10_uart_request_port,
	.config_port = solo6x10_uart_config_port,
	.verify_port = solo6x10_uart_verify_port,
};

static struct uart_driver solo6x10_uart_driver = {
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

int solo6x10_uart_interrupt(struct SOLO6x10 *solo6x10, int channel)
{
	struct SOLO6x10_UART *uart;
	u32 status;

	uart = (struct SOLO6x10_UART *)solo6x10->uart;
	uart += channel;

	status = solo6x10_reg_read(solo6x10, REG_UART_STATUS(channel));
	solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_UART(channel));

	if(status & (UART_OVERRUN | UART_FRAME_ERR | UART_PARITY_ERR))
	{
		struct uart_port *port = &uart->port;
		char flag = 0;

		if(status & UART_OVERRUN)
		{
			solo6x10_reg_write(uart->solo6x10, REG_UART_CONTROL(uart->channel), uart->ctrl & ~RX_EN);
			solo6x10_reg_write(uart->solo6x10, REG_UART_CONTROL(uart->channel), uart->ctrl);
			if(port->read_status_mask & ~port->ignore_status_mask & UART_OVERRUN)
				flag |= TTY_OVERRUN;
		}

		if(status & port->read_status_mask & ~port->ignore_status_mask & UART_FRAME_ERR)
			flag |= TTY_FRAME;

		if(status & port->read_status_mask & ~port->ignore_status_mask & UART_PARITY_ERR)
			flag |= TTY_PARITY;

		if(flag)
		{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
			tty_insert_flip_char(port->info->port.tty, 0, flag);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 10))
			tty_insert_flip_char(port->info->tty, 0, flag);
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
			*tty->flip.flag_buf_ptr++ = flag;
			port->icount.rx++;
			tty->flip.count++;
#endif
		}
	}

	if(UART_RX_BUFF_CNT(status))
	{
		if(uart->rx_enable)
			solo6x10_uart_rx_chars(uart);
		else
		{
			while(UART_RX_BUFF_CNT(status))
			{
				solo6x10_uart_get_rx_data(uart);
				status = solo6x10_reg_read(uart->solo6x10, REG_UART_STATUS(uart->channel));
			}
		}
		status = solo6x10_reg_read(uart->solo6x10, REG_UART_STATUS(uart->channel));
	}

	if(uart->tx_enable && UART_TX_BUFF_CNT(status))
		solo6x10_uart_tx_chars(uart);

	return 0;
}

int solo6x10_uart_init(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_UART *uart;
	unsigned int clk_div;
	int i;

	uart = kmalloc(sizeof(struct SOLO6x10_UART) * 2, GFP_KERNEL);
	if(uart == NULL)
		return -1;

	memset(uart, 0, sizeof(struct SOLO6x10_UART) * 2);

	clk_div = (((solo6x10->system_clock / 337500) + 5) / 10) - 1;
	dbg_msg("uart clk_div:%d\n", clk_div);

	solo6x10->uart = uart;

	for(i=0; i<2; i++)
	{
		uart[i].solo6x10 = solo6x10;
		uart[i].channel = i;
		uart[i].port.lock = SPIN_LOCK_UNLOCKED;
		uart[i].port.iotype = UPIO_MEM;
		uart[i].port.irq = solo6x10->irq;
		uart[i].port.uartclk = 0;
		uart[i].port.fifosize = 8;
		uart[i].port.type = PORT_SOLO6x10;
		uart[i].port.ops = &solo6x10_uart_ops;
		uart[i].port.flags = 0;
		uart[i].port.line = (solo6x10->chip.id * 2) + i;
		uart[i].port.dev = &solo6x10->pdev->dev;

		uart[i].ctrl = UART_CLK_DIV(clk_div) | INTR_ERR_EN | INTR_RX_EN | INTR_TX_EN | TX_EN;
		uart_add_one_port(&solo6x10_uart_driver, &uart[i].port);
		solo6x10_enable_interrupt(uart->solo6x10, INTR_UART(i));
	}

	return 0;
}

void solo6x10_uart_exit(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_UART *uart;
	int i;

	uart = solo6x10->uart;

	for(i=0; i<2; i++)
	{
		uart[i].ctrl &= ~(RX_EN | TX_EN);
		solo6x10_reg_write(uart->solo6x10, REG_UART_CONTROL(i), uart[i].ctrl);
		solo6x10_disable_interrupt(solo6x10, INTR_UART(i));
		uart_remove_one_port(&solo6x10_uart_driver, &uart[i].port);
	}

	kfree(uart);
}

int solo6x10_uart_register_driver(void)
{
	return uart_register_driver(&solo6x10_uart_driver);

}

void solo6x10_uart_unregister_driver(void)
{
	uart_unregister_driver(&solo6x10_uart_driver);
}

