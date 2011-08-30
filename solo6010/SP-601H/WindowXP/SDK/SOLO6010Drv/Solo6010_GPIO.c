#include "Solo6010_GPIO.h"


int S6010_InitGPIO (DEVICE_EXTENSION *pdx)
{
	S6010_RegWrite (pdx, REG_GPIO_CONFIG_0, 0);
	S6010_RegWrite (pdx, REG_GPIO_CONFIG_1, 0);

	S6010_RegWrite (pdx, REG_GPIO_CONFIG_1, 0xffff0000);	// GPIO[16] ~ GPIO[31] On

	S6010_RegWrite (pdx, REG_GPIO_INT_ENA, 0);
	S6010_EnableInterrupt (pdx, INTR_GPIO);

	pdx->iS6010.curStatGPIO_Out = 0;
	S6010_RegWrite (pdx, REG_GPIO_DATA_OUT, pdx->iS6010.curStatGPIO_Out);

	return 0;
}

void S6010_DestroyGPIO (DEVICE_EXTENSION *pdx)
{
	S6010_RegWrite (pdx, REG_GPIO_INT_ENA, 0);
	S6010_DisableInterrupt (pdx, INTR_GPIO);

	S6010_RegWrite (pdx, REG_GPIO_CONFIG_0, 0);
	S6010_RegWrite (pdx, REG_GPIO_CONFIG_1, 0);
}

void S6010_GPIO_Operation (DEVICE_EXTENSION *pdx, INFO_GPIO *pIGPIO)
{
	int i;
	ULONG maskPort, maskData;

	switch (pIGPIO->idxOp)
	{
	case GPIO_OP_SET_MODE_IN:
	case GPIO_OP_SET_MODE_OUT:
	case GPIO_OP_SET_MODE_I2C:
	case GPIO_OP_SET_MODE_ETC:
		S6010_GPIO_A_Mode (pdx, (pIGPIO->maskPort &GPIO_PORT_A) >>0,  pIGPIO->idxOp);
		S6010_GPIO_B_Mode (pdx, (pIGPIO->maskPort &GPIO_PORT_B) >>16,  pIGPIO->idxOp);
		break;
	case GPIO_OP_WRITE:
		S6010_GPIO_Set (pdx, pIGPIO->maskPort, pIGPIO->nData);
		break;
	case GPIO_OP_READ:
		pIGPIO->nData = S6010_GPIO_Get (pdx);
		break;
	case GPIO_OP_SET_ENABLE_INT:
		if (pIGPIO->nData == GPIO_INT_CFG_DISABLE_INTERRUPT)
		{
			break;
		}

		maskData = S6010_RegRead (pdx, REG_GPIO_INT_CFG_0);
		for (i=0; i<NUM_GPIO_PORT_A; i++)
		{
			if (pIGPIO->maskPort &GPIO_PORT(i))
			{
				maskData &= ~(0x3 <<(i *2));
				maskData |= (pIGPIO->nData <<(i *2));
			}
		}
		S6010_RegWrite (pdx, REG_GPIO_INT_CFG_0, maskData);

		maskData = S6010_RegRead (pdx, REG_GPIO_INT_CFG_1);
		for (i=0; i<NUM_GPIO_PORT_B; i++)
		{
			if (pIGPIO->maskPort &GPIO_PORT(NUM_GPIO_PORT_A +i))
			{
				maskData &= ~(0x3 <<(i *2));
				maskData |= (pIGPIO->nData <<(i *2));
			}
		}
		S6010_RegWrite (pdx, REG_GPIO_INT_CFG_1, maskData);

		maskPort = S6010_RegRead (pdx, REG_GPIO_INT_ENA);
		maskPort |= pIGPIO->maskPort;
		S6010_RegWrite (pdx, REG_GPIO_INT_ENA, maskPort);
		break;
	case GPIO_OP_SET_DISABLE_INT:
		maskPort = S6010_RegRead (pdx, REG_GPIO_INT_ENA);
		maskPort &= ~pIGPIO->maskPort;
		S6010_RegWrite (pdx, REG_GPIO_INT_ENA, maskPort);
		break;
	}
}

void S6010_GPIO_A_Mode (DEVICE_EXTENSION *pdx, ULONG port_mask, ULONG mode)
{
	int port;
	unsigned int ret;

	if (port_mask == 0)
	{
		return;
	}

	ret = S6010_RegRead (pdx, REG_GPIO_CONFIG_0);

	for (port=0; port<16; port++)
	{
		if (!((1 <<port) &port_mask))
		{
			continue;
		}

		ret &= (~(3 <<(port <<1)));
		ret |= ((mode &3) <<(port <<1));
	}

	S6010_RegWrite (pdx, REG_GPIO_CONFIG_0, ret);
}

void S6010_GPIO_B_Mode (DEVICE_EXTENSION *pdx, ULONG port_mask, ULONG mode)
{
	int port;
	unsigned int ret;

	if (port_mask == 0 || mode > GPIO_OP_SET_MODE_OUT)
	{
		return;
	}

	ret = S6010_RegRead (pdx, REG_GPIO_CONFIG_1);

	for (port=0; port<16; port++)
	{
		if (!((1 <<port) &port_mask))
		{
			continue;
		}

		ret &= (~(1 <<port));
		ret |= ((mode &1) <<port);
	}

	S6010_RegWrite (pdx, REG_GPIO_CONFIG_1, ret);
}

ULONG S6010_GPIO_Get (DEVICE_EXTENSION *pdx)
{
	return S6010_RegRead (pdx, REG_GPIO_DATA_IN);
}

void S6010_GPIO_Set (DEVICE_EXTENSION *pdx, ULONG maskPort, ULONG nValue)
{
	int i;
	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		if (maskPort &GPIO_PORT(i))
		{
			pdx->iS6010.curStatGPIO_Out &= ~GPIO_PORT(i);
			pdx->iS6010.curStatGPIO_Out |= nValue &GPIO_PORT(i);
		}
	}

	S6010_RegWrite (pdx, REG_GPIO_DATA_OUT, pdx->iS6010.curStatGPIO_Out);
}

void S6010_GPIO_Clear (DEVICE_EXTENSION *pdx, unsigned int nValue)
{
	pdx->iS6010.curStatGPIO_Out &= ~nValue;

	S6010_RegWrite (pdx, REG_GPIO_DATA_OUT, pdx->iS6010.curStatGPIO_Out);
}
