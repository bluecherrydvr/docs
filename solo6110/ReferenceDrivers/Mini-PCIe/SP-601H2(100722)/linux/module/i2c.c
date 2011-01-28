#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>

#include "solo6x10.h"

#define I2C_CLOCK_SCALE			7

enum SOLO6x10_I2C_STAT {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};

struct SOLO6x10_I2C
{
	struct SOLO6x10 *solo6x10;

	unsigned int channel;
	struct semaphore *mutex;

	struct i2c_adapter adap;

	enum SOLO6x10_I2C_STAT state;
	wait_queue_head_t wait;

	struct i2c_msg *msg;
	unsigned int msg_num;
	unsigned int msg_ptr;
};

static inline void solo6x10_i2c_start(struct SOLO6x10_I2C *solo6x10_i2c)
{
	u32 address;
	u32 ctrl;

	address = solo6x10_i2c->msg->addr << 1;
	if(solo6x10_i2c->msg->flags & I2C_M_RD)
		address |= 1;

	solo6x10_i2c->state = STATE_START;

	solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_TXD, address);

	ctrl = IIC_CH(solo6x10_i2c->channel) | IIC_START | IIC_WRITE;
	if(solo6x10_i2c->msg_ptr == solo6x10_i2c->msg->len)
		ctrl |= IIC_STOP;

	solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_CTRL, ctrl);
}

static inline void solo6x10_i2c_stop(struct SOLO6x10_I2C *solo6x10_i2c)
{
	solo6x10_i2c->state = STATE_STOP;
	wake_up(&solo6x10_i2c->wait);
}

int solo6x10_i2c_interrupt(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_I2C *solo6x10_i2c;
	u32 status;

	solo6x10_i2c = solo6x10->i2c_current;
	if(!solo6x10_i2c)
	{
		solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_IIC);
		dbg_msg("i2c wrong interrupt\n");
		return 0;
	}

	status = solo6x10_reg_read(solo6x10, REG_IIC_CTRL);
	solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_CTRL, IIC_CH(solo6x10_i2c->channel));

	solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_IIC);

	if((IIC_STAT_TRNS | IIC_STAT_SIG_ERR) & status)
	{
		solo6x10_i2c_stop(solo6x10_i2c);
		dbg_msg("i2c status : %08x\n", status);
		return 0;
	}

	switch(solo6x10_i2c->state)
	{
		case STATE_START:
			if(solo6x10_i2c->msg->flags & I2C_M_RD)
			{
				solo6x10_i2c->state = STATE_READ;
				goto prepare_read;
			}

			solo6x10_i2c->state = STATE_WRITE;

		case STATE_WRITE:

		retry_write:

			if(solo6x10_i2c->msg_ptr != solo6x10_i2c->msg->len)
			{
				u32 ctrl;

				solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_TXD, solo6x10_i2c->msg->buf[solo6x10_i2c->msg_ptr]);
				solo6x10_i2c->msg_ptr++;

				ctrl = IIC_CH(solo6x10_i2c->channel) | IIC_WRITE;

				if(solo6x10_i2c->msg_ptr == solo6x10_i2c->msg->len)
					ctrl |= IIC_STOP;

				solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_CTRL, ctrl);
			}
			else
			{
				solo6x10_i2c->msg_ptr = 0;
				solo6x10_i2c->msg++;
				solo6x10_i2c->msg_num--;
				if(solo6x10_i2c->msg_num == 0)
				{
					solo6x10_i2c_stop(solo6x10_i2c);
					return 0;
				}

				if(!(solo6x10_i2c->msg->flags & I2C_M_NOSTART))
					solo6x10_i2c_start(solo6x10_i2c);
				else
				{
					if(solo6x10_i2c->msg->flags & I2C_M_RD)
						solo6x10_i2c_stop(solo6x10_i2c);
					else
						goto retry_write;
				}
			}
			break;

		case STATE_READ:

			solo6x10_i2c->msg->buf[solo6x10_i2c->msg_ptr] = solo6x10_reg_read(solo6x10, REG_IIC_RXD);
			solo6x10_i2c->msg_ptr++;

		prepare_read:

			if(solo6x10_i2c->msg_ptr != solo6x10_i2c->msg->len)
			{
				u32 ctrl;

				ctrl = IIC_CH(solo6x10_i2c->channel) | IIC_READ;
				if((solo6x10_i2c->msg_ptr+1) == solo6x10_i2c->msg->len)
					ctrl |= IIC_NACK | IIC_STOP;

				solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_CTRL, ctrl);
			}
			else
			{
				solo6x10_i2c->msg_ptr = 0;
				solo6x10_i2c->msg++;
				solo6x10_i2c->msg_num--;
				if(solo6x10_i2c->msg_num == 0)
				{
					solo6x10_i2c_stop(solo6x10_i2c);
					return 0;
				}

				if(!(solo6x10_i2c->msg->flags & I2C_M_NOSTART))
					solo6x10_i2c_start(solo6x10_i2c);
				else
				{
					if(solo6x10_i2c->msg->flags & I2C_M_RD)
						goto prepare_read;
					else
						solo6x10_i2c_stop(solo6x10_i2c);
				}
			}
			break;

		default:
			dbg_msg("i2c error! (%08x)\n", status);
			solo6x10_i2c_stop(solo6x10_i2c);
			return -1;
	}

	return 0;
}

static int master_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct SOLO6x10_I2C *solo6x10_i2c;
	unsigned long timeout;
	int retval;

	solo6x10_i2c = adap->algo_data;

	down(solo6x10_i2c->mutex);
	solo6x10_i2c->solo6x10->i2c_current = solo6x10_i2c;
	solo6x10_i2c->msg = msgs;
	solo6x10_i2c->msg_num = num;
	solo6x10_i2c->msg_ptr = 0;

	solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_CTRL, IIC_CH(solo6x10_i2c->channel));
	solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_CFG, IIC_PRESCALE(I2C_CLOCK_SCALE) | IIC_ENABLE);
	solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_INTR_STAT, INTR_IIC);
	solo6x10_enable_interrupt(solo6x10_i2c->solo6x10, INTR_IIC);

	solo6x10_i2c_start(solo6x10_i2c);

	timeout = HZ/2;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
{
	DEFINE_WAIT(wait);

	for(;;)
	{
		prepare_to_wait(&solo6x10_i2c->wait, &wait, TASK_INTERRUPTIBLE);

		if(solo6x10_i2c->state == STATE_STOP)
			break;

		timeout = schedule_timeout(timeout);
		if(!timeout)
		{
			dbg_msg("timeout\n");
			break;
		}

		if(signal_pending(current))
		{
			dbg_msg("signal_pending\n");
			break;
		}
	}

	finish_wait(&solo6x10_i2c->wait, &wait);
}
#else
{
	spin_lock_irq(&solo6x10_i2c->wait.lock);
	if (solo6x10_i2c->state != STATE_STOP) {
		DECLARE_WAITQUEUE(wait, current);

		wait.flags |= WQ_FLAG_EXCLUSIVE;
		__add_wait_queue_tail(&solo6x10_i2c->wait, &wait);
		do {
			__set_current_state(TASK_UNINTERRUPTIBLE);
			spin_unlock_irq(&solo6x10_i2c->wait.lock);
			timeout = schedule_timeout(timeout);
			spin_lock_irq(&solo6x10_i2c->wait.lock);
			if (!timeout) {
				__remove_wait_queue(&solo6x10_i2c->wait, &wait);
				dbg_msg("i2c timeout (%08x)\n", solo6x10_reg_read(solo6x10_i2c->solo6x10, REG_IIC_CTRL));
				goto out;
			}
		} while (solo6x10_i2c->state != STATE_STOP);
		__remove_wait_queue(&solo6x10_i2c->wait, &wait);
	}
out:
	spin_unlock_irq(&solo6x10_i2c->wait.lock);
}
#endif

	retval = num - solo6x10_i2c->msg_num;

	solo6x10_i2c->state = STATE_IDLE;

	solo6x10_disable_interrupt(solo6x10_i2c->solo6x10, INTR_IIC);
	solo6x10_reg_write(solo6x10_i2c->solo6x10, REG_IIC_CFG, IIC_PRESCALE(I2C_CLOCK_SCALE));

	solo6x10_i2c->solo6x10->i2c_current = NULL;
	up(solo6x10_i2c->mutex);

	return retval;
}

static u32 functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C;
}

static struct i2c_algorithm solo6x10_i2c_algorithm = {
	.master_xfer	= master_xfer,
	.functionality	= functionality,
};

u8 solo6x10_i2c_readbyte(struct SOLO6x10 *solo6x10, int i2c_channel, u8 dev_addr, u8 sub_addr)
{
	struct SOLO6x10_I2C *solo6x10_i2c;
	struct i2c_msg msgs[2];
	u8 data = 0;

	solo6x10_i2c = solo6x10->i2c;
	solo6x10_i2c = &solo6x10_i2c[i2c_channel];

	msgs[0].flags = 0;
	msgs[0].addr = dev_addr;
	msgs[0].len = 1;
	msgs[0].buf = &sub_addr;

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr = dev_addr;
	msgs[1].len = 1;
	msgs[1].buf = &data;

	i2c_transfer(&solo6x10_i2c->adap, msgs, 2);

	return data;
}

void solo6x10_i2c_readbytes(struct SOLO6x10 *solo6x10, int i2c_channel, u8 dev_addr, u8 sub_addr, u8 *data, int len)
{
	struct SOLO6x10_I2C *solo6x10_i2c;
	struct i2c_msg msgs[2];

	solo6x10_i2c = solo6x10->i2c;
	solo6x10_i2c = &solo6x10_i2c[i2c_channel];

	msgs[0].flags = 0;
	msgs[0].addr = dev_addr;
	msgs[0].len = 1;
	msgs[0].buf = &sub_addr;

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr = dev_addr;
	msgs[1].len = len;
	msgs[1].buf = data;

	i2c_transfer(&solo6x10_i2c->adap, msgs, 2);
}

void solo6x10_i2c_writebyte(struct SOLO6x10 *solo6x10, int i2c_channel, u8 dev_addr, u8 sub_addr, u8 data)
{
	struct SOLO6x10_I2C *solo6x10_i2c;
	struct i2c_msg msgs;
	u8 buf[2];

	solo6x10_i2c = solo6x10->i2c;
	solo6x10_i2c = &solo6x10_i2c[i2c_channel];

	buf[0] = sub_addr;
	buf[1] = data;
	msgs.flags = 0;
	msgs.addr = dev_addr;
	msgs.len = 2;
	msgs.buf = buf;

	i2c_transfer(&solo6x10_i2c->adap, &msgs, 1);
}

int solo6x10_i2c_init(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_I2C *solo6x10_i2c;
	int channel;

	solo6x10_reg_write(solo6x10, REG_IIC_CFG, IIC_PRESCALE(I2C_CLOCK_SCALE));

	solo6x10_i2c = kmalloc(sizeof(struct SOLO6x10_I2C) * 8, GFP_KERNEL);
	if(solo6x10_i2c == NULL)
		return -ENOMEM;

	memset(solo6x10_i2c, 0, sizeof(struct SOLO6x10_I2C) * 8);
	init_MUTEX(&solo6x10->i2c_mutex);

	solo6x10->i2c = solo6x10_i2c;
	for(channel=0; channel<8; channel++)
	{
		solo6x10_i2c[channel].solo6x10 = solo6x10;
		solo6x10_i2c[channel].channel = channel;
		solo6x10_i2c[channel].mutex = &solo6x10->i2c_mutex;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
		snprintf(solo6x10_i2c[channel].adap.name, I2C_NAME_SIZE, "SOLO6x10 I2C %d Adapter", channel);
#else
		snprintf(solo6x10_i2c[channel].adap.name, 32, "SOLO6x10 I2C %d Adapter", channel);
#endif
		solo6x10_i2c[channel].adap.algo = &solo6x10_i2c_algorithm;
		solo6x10_i2c[channel].adap.algo_data = &solo6x10_i2c[channel];
		solo6x10_i2c[channel].adap.retries = 1;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
		solo6x10_i2c[channel].adap.dev.parent = &solo6x10->pdev->dev;
#else
#endif
		solo6x10_i2c[channel].state = STATE_IDLE;
		init_waitqueue_head(&solo6x10_i2c[channel].wait);

		i2c_add_adapter(&solo6x10_i2c[channel].adap);
	}

	return 0;
}

void solo6x10_i2c_exit(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_I2C *solo6x10_i2c;
	int channel;

	solo6x10_i2c = solo6x10->i2c;
	for(channel=0; channel<8; channel++)
	{
		i2c_del_adapter(&solo6x10_i2c[channel].adap);
	}
	kfree(solo6x10_i2c);
}

