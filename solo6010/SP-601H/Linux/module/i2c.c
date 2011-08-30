#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>

#include "solo6010.h"

enum SOLO6010_I2C_STAT {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};

struct SOLO6010_I2C
{
	struct SOLO6010 *solo6010;

	unsigned int channel;
	struct semaphore *mutex;

	struct i2c_adapter adap;

	enum SOLO6010_I2C_STAT state;
	wait_queue_head_t wait;

	struct i2c_msg *msg;
	unsigned int msg_num;
	unsigned int msg_ptr;
};

static inline void solo6010_i2c_flush(struct SOLO6010_I2C *solo6010_i2c, int wr)
{
	u32 ctrl;

	ctrl = IIC_CH(solo6010_i2c->channel);

	if(solo6010_i2c->state == STATE_START)
		ctrl |= IIC_START;

	if(wr)
		ctrl |= IIC_WRITE;
	else
	{
		ctrl |= IIC_READ;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
		if(!(solo6010_i2c->msg->flags & I2C_M_NO_RD_ACK))
			ctrl |= IIC_ACK_EN;
#else
		ctrl |= IIC_ACK_EN;
#endif
	}

	if(solo6010_i2c->msg_ptr == solo6010_i2c->msg->len)
		ctrl |= IIC_STOP;

	solo6010_reg_write(solo6010_i2c->solo6010, REG_IIC_CTRL, ctrl);
}

static inline void solo6010_i2c_start(struct SOLO6010_I2C *solo6010_i2c)
{
	unsigned int address;

	address = solo6010_i2c->msg->addr << 1;
	if(solo6010_i2c->msg->flags & I2C_M_RD)
		address |= 1;

	solo6010_i2c->state = STATE_START;

	solo6010_reg_write(solo6010_i2c->solo6010, REG_IIC_TXD, address);

	solo6010_i2c_flush(solo6010_i2c, 1);
}

static inline void solo6010_i2c_stop(struct SOLO6010_I2C *solo6010_i2c)
{
	solo6010_disable_interrupt(solo6010_i2c->solo6010, INTR_IIC);
	solo6010_reg_write(solo6010_i2c->solo6010, REG_IIC_CTRL, IIC_CH(solo6010_i2c->channel));
	solo6010_i2c->state = STATE_STOP;
	wake_up(&solo6010_i2c->wait);
}

int solo6010_i2c_interrupt(struct SOLO6010 *solo6010)
{
	struct SOLO6010_I2C *solo6010_i2c;
	u32 status;

	solo6010_i2c = solo6010->i2c_current;

	status = solo6010_reg_read(solo6010, REG_IIC_CTRL);

	solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_IIC);

	if((IIC_STAT_TRNS | IIC_STAT_SIG_ERR) & status)
	{
		solo6010_i2c_stop(solo6010_i2c);
		return -1;
	}

	switch(solo6010_i2c->state)
	{
		case STATE_START:
			if(solo6010_i2c->msg->flags & I2C_M_RD)
			{
				solo6010_i2c->state = STATE_READ;
				goto prepare_read;
			}

			solo6010_i2c->state = STATE_WRITE;

		case STATE_WRITE:

		retry_write:

			if(solo6010_i2c->msg_ptr != solo6010_i2c->msg->len)
			{
				solo6010_reg_write(solo6010_i2c->solo6010, REG_IIC_TXD, solo6010_i2c->msg->buf[solo6010_i2c->msg_ptr]);
				solo6010_i2c->msg_ptr++;

				solo6010_i2c_flush(solo6010_i2c, 1);
			}
			else
			{
				solo6010_i2c->msg_ptr = 0;
				solo6010_i2c->msg++;
				solo6010_i2c->msg_num--;
				if(solo6010_i2c->msg_num == 0)
				{
					solo6010_i2c_stop(solo6010_i2c);
					return 0;
				}

				if(!(solo6010_i2c->msg->flags & I2C_M_NOSTART))
					solo6010_i2c_start(solo6010_i2c);
				else
				{
					if(solo6010_i2c->msg->flags & I2C_M_RD)
						solo6010_i2c_stop(solo6010_i2c);
					else
						goto retry_write;
				}
			}
			break;

		case STATE_READ:

			solo6010_i2c->msg->buf[solo6010_i2c->msg_ptr] = solo6010_reg_read(solo6010, REG_IIC_RXD);
			solo6010_i2c->msg_ptr++;

		prepare_read:

			if(solo6010_i2c->msg_ptr != solo6010_i2c->msg->len)
			{
				solo6010_i2c_flush(solo6010_i2c, 0);
			}
			else
			{
				solo6010_i2c->msg_ptr = 0;
				solo6010_i2c->msg++;
				solo6010_i2c->msg_num--;
				if(solo6010_i2c->msg_num == 0)
				{
					solo6010_i2c_stop(solo6010_i2c);
					return 0;
				}

				if(!(solo6010_i2c->msg->flags & I2C_M_NOSTART))
					solo6010_i2c_start(solo6010_i2c);
				else
				{
					if(solo6010_i2c->msg->flags & I2C_M_RD)
						goto prepare_read;
					else
						solo6010_i2c_stop(solo6010_i2c);
				}
			}
			break;

		default:
			dbg_msg("i2c error! (%08x)\n", status);
			solo6010_i2c_stop(solo6010_i2c);
			return -1;
	}

	return 0;
}

static int master_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct SOLO6010_I2C *solo6010_i2c;
	unsigned long timeout;
	int retval;

	solo6010_i2c = adap->algo_data;

	down(solo6010_i2c->mutex);
	solo6010_i2c->solo6010->i2c_current = solo6010_i2c;
	solo6010_i2c->msg = msgs;
	solo6010_i2c->msg_num = num;
	solo6010_i2c->msg_ptr = 0;

	solo6010_reg_write(solo6010_i2c->solo6010, REG_IIC_CTRL, IIC_CH(solo6010_i2c->channel));

	solo6010_reg_write(solo6010_i2c->solo6010, REG_IIC_CFG, IIC_PRESCALE(7) | IIC_ENABLE);
	solo6010_enable_interrupt(solo6010_i2c->solo6010, INTR_IIC);

	solo6010_i2c_start(solo6010_i2c);

	timeout = HZ/2;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
{
	DEFINE_WAIT(wait);

	for(;;)
	{
		prepare_to_wait(&solo6010_i2c->wait, &wait, TASK_INTERRUPTIBLE);

		if(solo6010_i2c->state == STATE_STOP)
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

	finish_wait(&solo6010_i2c->wait, &wait);
}
#else
{
	spin_lock_irq(&solo6010_i2c->wait.lock);
	if (solo6010_i2c->state != STATE_STOP) {
		DECLARE_WAITQUEUE(wait, current);

		wait.flags |= WQ_FLAG_EXCLUSIVE;
		__add_wait_queue_tail(&solo6010_i2c->wait, &wait);
		do {
			__set_current_state(TASK_UNINTERRUPTIBLE);
			spin_unlock_irq(&solo6010_i2c->wait.lock);
			timeout = schedule_timeout(timeout);
			spin_lock_irq(&solo6010_i2c->wait.lock);
			if (!timeout) {
				__remove_wait_queue(&solo6010_i2c->wait, &wait);
				dbg_msg("i2c timeout (%08x)\n", solo6010_reg_read(solo6010_i2c->solo6010, REG_IIC_CTRL));
				goto out;
			}
		} while (solo6010_i2c->state != STATE_STOP);
		__remove_wait_queue(&solo6010_i2c->wait, &wait);
	}
out:
	spin_unlock_irq(&solo6010_i2c->wait.lock);
}
#endif

	retval = num - solo6010_i2c->msg_num;

	solo6010_i2c->state = STATE_IDLE;

	solo6010_reg_write(solo6010_i2c->solo6010, REG_IIC_CFG, IIC_PRESCALE(7));

	up(solo6010_i2c->mutex);

	return retval;
}

static u32 functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C;
}

static struct i2c_algorithm solo6010_i2c_algorithm = {
	.master_xfer	= master_xfer,
	.functionality	= functionality,
};

u8 solo6010_i2c_readbyte(struct SOLO6010 *solo6010, int i2c_channel, u8 dev_addr, u8 sub_addr)
{
	struct SOLO6010_I2C *solo6010_i2c;
	struct i2c_msg msgs[2];
	u8 data;

	solo6010_i2c = solo6010->i2c;
	solo6010_i2c = &solo6010_i2c[i2c_channel];

	msgs[0].flags = 0;
	msgs[0].addr = dev_addr;
	msgs[0].len = 1;
	msgs[0].buf = &sub_addr;

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr = dev_addr;
	msgs[1].len = 1;
	msgs[1].buf = &data;

	i2c_transfer(&solo6010_i2c->adap, msgs, 2);

	return data;
}

void solo6010_i2c_writebyte(struct SOLO6010 *solo6010, int i2c_channel, u8 dev_addr, u8 sub_addr, u8 data)
{
	struct SOLO6010_I2C *solo6010_i2c;
	struct i2c_msg msgs;
	u8 buf[2];

	solo6010_i2c = solo6010->i2c;
	solo6010_i2c = &solo6010_i2c[i2c_channel];

	buf[0] = sub_addr;
	buf[1] = data;
	msgs.flags = 0;
	msgs.addr = dev_addr;
	msgs.len = 2;
	msgs.buf = buf;

	i2c_transfer(&solo6010_i2c->adap, &msgs, 1);
}

int solo6010_i2c_init(struct SOLO6010 *solo6010)
{
	struct SOLO6010_I2C *solo6010_i2c;
	int channel;

	solo6010_reg_write(solo6010, REG_IIC_CFG, IIC_PRESCALE(7));

	solo6010_i2c = kmalloc(sizeof(struct SOLO6010_I2C) * 8, GFP_KERNEL);
	if(solo6010_i2c == NULL)
		return -ENOMEM;

	memset(solo6010_i2c, 0, sizeof(struct SOLO6010_I2C) * 8);
	init_MUTEX(&solo6010->i2c_mutex);

	solo6010->i2c = solo6010_i2c;
	for(channel=0; channel<8; channel++)
	{
		solo6010_i2c[channel].solo6010 = solo6010;
		solo6010_i2c[channel].channel = channel;
		solo6010_i2c[channel].mutex = &solo6010->i2c_mutex;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
		snprintf(solo6010_i2c[channel].adap.name, I2C_NAME_SIZE, "SOLO6010 I2C %d Adapter", channel);
#else
		snprintf(solo6010_i2c[channel].adap.name, 32, "SOLO6010 I2C %d Adapter", channel);
#endif
		solo6010_i2c[channel].adap.algo = &solo6010_i2c_algorithm;
		solo6010_i2c[channel].adap.algo_data = &solo6010_i2c[channel];
		solo6010_i2c[channel].adap.retries = 1;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
		solo6010_i2c[channel].adap.dev.parent = &solo6010->pdev->dev;
#else
#endif
		solo6010_i2c[channel].state = STATE_IDLE;
		init_waitqueue_head(&solo6010_i2c[channel].wait);

		i2c_add_adapter(&solo6010_i2c[channel].adap);
	}

	return 0;
}

void solo6010_i2c_exit(struct SOLO6010 *solo6010)
{
	struct SOLO6010_I2C *solo6010_i2c;
	int channel;

	solo6010_i2c = solo6010->i2c;
	for(channel=0; channel<8; channel++)
	{
		i2c_del_adapter(&solo6010_i2c[channel].adap);
	}
	kfree(solo6010_i2c);
}

