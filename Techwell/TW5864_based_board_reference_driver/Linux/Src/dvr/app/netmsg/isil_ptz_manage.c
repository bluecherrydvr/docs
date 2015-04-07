/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_ptz_manage.c
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:handle ptz information.


	Other:
	Function list:
	History:

*******************************************************************
*/

#include "isil_ptz_manage.h"
#include "isil_debug.h"
#include "isil_cfg_file_manage.h"
//#include "isil_pel_com.h"

static S32 PTZAddr[ISIL_PTZ_PROTOCOL_MAX]={-1,};


static S32 ISIL_SetPtzAddrByType(S32 iPTZTYPE, S32 iAddr)
{
	if(iPTZTYPE > PTZ_PROTOCOL_NONE && iPTZTYPE < ISIL_PTZ_PROTOCOL_MAX)
	{
		PTZAddr[iPTZTYPE] = iAddr;
		return ISIL_SUCCESS;
	}

	return ISIL_FAIL;
}
static S32 ISIL_GetPtzAddrByType(S32 iPTZTYPE)
{
	if(iPTZTYPE > PTZ_PROTOCOL_NONE && iPTZTYPE < ISIL_PTZ_PROTOCOL_MAX)
	{
		return PTZAddr[iPTZTYPE];
	}

	return ISIL_FAIL;
}


/******************************PTZ PROTOCOL PELCOD**********************************/

static S32 ISIL_SetPelcodCmd(S08 *cSendData,U08 cComNo/*, S08 cSpeed_Pre*/)
{
	S32 iRet = -1;
	S32 iFd = -1;
	S32 iAddr;
	S32 iSendLen;
	if(cSendData == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "%s point null", __FUNCTION__);
		return ISIL_FAIL;
	}

	iAddr = ISIL_GetPtzAddrByType(PTZ_PROTOCOL_PELCOD);
	ISIL_TRACE(BUG_NORMAL, "%s, addr %d", __FUNCTION__, iAddr);
	if(iAddr != ISIL_FAIL)
	{
		cSendData[1] = iAddr;
		/*
		if(cSpeed_Pre >= 0)
		{
			cSendData[5] = cSpeed_Pre;
		}
		*/
		cSendData[6] = (cSendData[1]+cSendData[2]+cSendData[3]+cSendData[4]+cSendData[5])%0x100;
		iSendLen = 7;
		//iFd = ISIL_GetCOMFdByNo(COM_RS485, cComNo);
		ISIL_TRACE(BUG_NORMAL, "%s fd %d", __FUNCTION__, iFd);
		if(iFd != ISIL_FAIL)
		{
			//iRet = ISIL_COMWrite(iFd, cSendData, iSendLen);
			return iRet;
		}
	}
	return ISIL_FAIL;
}

static S32 ISIL_PelcodUp(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x08,0x00,0xff,0x08};

	ISIL_TRACE(BUG_NORMAL, "enter %s", __FUNCTION__);
	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
	return iRet;

}


static S32 ISIL_PelcodDown(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x10,0x00,0xff,0x10};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
	return iRet;
}

static S32 ISIL_PelcodLeft(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x04,0xff,0x00,0x04};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
	return iRet;
}

static S32 ISIL_PelcodRight(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x02,0xff,0x00,0x02};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
	return iRet;
}

static S32 ISIL_PelcodFocusAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x80,0x00,0x00,0x81};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcodFocusSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x01,0x00,0x00,0x00,0x02};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcodIrisAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x04,0x00,0x00,0x00,0x05};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcodIrisSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x04,0x00,0x00,0x00,0x05};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcodZoomAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x40,0x00,0x00,0x41};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcodZoomSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x20,0x00,0x00,0x21};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;
}

static S32 ISIL_PelcodAutoOpen(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x98,0x00,0x00,0x20,0xB7};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcodAutoClose(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x00,0x00,0x00,0x01};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;
}


static S32 ISIL_PelcodSetPreset(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x03,0x00,0x01,0x05};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcodGoPreset(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x07,0x00,0x01,0x09};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;
}

static S32 ISIL_PelcodClearPreset(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x05,0x00,0x01,0x07};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;
}

static S32 ISIL_PelcodStop(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[7] = {0xff,0x01,0x00,0x00,0x00,0x00,0x01};

	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;
}



/******************************PTZ PROTOCOL PELCOP**********************************/

/*need set send data the 1,5,7 byte*/
static S32 ISIL_SetPelcopCmd(S08 *cSendData,U08 cComNo)
{
	S32 iRet = -1;
	S32 iFd = -1;
	S32 iAddr;
	S32 iSendLen;
	if(cSendData == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "%s point null", __FUNCTION__);
		return ISIL_FAIL;
	}

	iAddr = ISIL_GetPtzAddrByType(PTZ_PROTOCOL_PELCOP);
	if(iAddr != ISIL_FAIL)
	{
		cSendData[1] = iAddr;
		cSendData[7] = (cSendData[1]^cSendData[2]^cSendData[3]^cSendData[4]^cSendData[5]);
		iSendLen = 8;
		//iFd = ISIL_GetCOMFdByNo(COM_RS485, cComNo);
		if(iFd != ISIL_FAIL)
		{
			//iRet = ISIL_COMWrite(iFd, cSendData, iSendLen);
			return iRet;
		}
	}
	return ISIL_FAIL;
}


static S32 ISIL_PelcopUp(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x08,0x00,0x30,0xaf,0x39};
	/*cSendData[5] = speed;*/
	iRet = ISIL_SetPelcopCmd(cSendData, ComNo);
	return iRet;

}


static S32 ISIL_PelcopDown(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x10,0x00,0x30,0xaf,0x21};
	/*cSendData[5] = speed;*/
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
	return iRet;
}

static S32 ISIL_PelcopLeft(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x04,0x10,0x00,0xaf,0x15};
	/*cSendData[4] = speed;*/
	iRet = ISIL_SetPelcopCmd(cSendData, ComNo);
	return iRet;
}

static S32 ISIL_PelcopRight(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x02,0x10,0x00,0xaf,0x13};
	/*cSendData[4] = speed;*/
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
	return iRet;
}

static S32 ISIL_PelcopFocusAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x02,0x00,0x00,0x00,0xaf,0x03};
	iRet = ISIL_SetPelcopCmd(cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcopFocusSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x01,0x00,0x00,0x00,0xaf,0x00};
	iRet = ISIL_SetPelcopCmd(cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcopIrisAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x04,0x00,0x00,0x00,0xaf,0x05};
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcopIrisSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x08,0x00,0x00,0x00,0xaf,0x09};
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcopZoomAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x20,0x00,0x00,0xaf,0x21};
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcopZoomSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x40,0x00,0x00,0xaf,0x41};
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
		return iRet;
}

static S32 ISIL_PelcopAutoOpen(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x96,0x00,0x20,0xaf,0xb7};
	iRet = ISIL_SetPelcopCmd(cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcopAutoClose(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x99,0x00,0x20,0xaf,0xb8};
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
		return iRet;
}


static S32 ISIL_PelcopSetPreset(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x03,0x00,0x01,0xaf,0x03};
	/*cSendData[5] = preset num;*/
	iRet = ISIL_SetPelcodCmd (cSendData, ComNo);
		return iRet;

}

static S32 ISIL_PelcopGoPreset(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x07,0x00,0x01,0xaf,0x07};
	/*cSendData[5] = preset num;*/
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
		return iRet;
}

static S32 ISIL_PelcopClearPreset(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x05,0x00,0x01,0xaf,0x01};
	/*cSendData[5] = speed;*/
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
		return iRet;
}

static S32 ISIL_PelcopStop(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0xa0,0x01,0x00,0x00,0x00,0x00,0xaf,0x01};
	/*cSendData[5] = preset num;*/
	iRet = ISIL_SetPelcopCmd(cSendData, ComNo);
		return iRet;
}

/******************************PTZ PROTOCOL SONY**********************************/
/*
	need set send data the 0 byte and send len is 9
	cLenFlag: 1:9; 0, 6
*/
static S32 ISIL_SetSonyCmd(S08 *cSendData,U08 cComNo, U08 cLen)
{
	S32 iRet = -1;
	S32 iFd = -1;
	S32 iAddr;
	if(cSendData == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "%s point null", __FUNCTION__);
		return ISIL_FAIL;
	}

	iAddr = ISIL_GetPtzAddrByType(PTZ_PROTOCOL_PELCOP);
	if(iAddr != ISIL_FAIL)
	{
		cSendData[0] |= iAddr;
		///iFd = ISIL_GetCOMFdByNo(COM_RS485, cComNo);
		if(iFd != ISIL_FAIL)
		{
			//iRet = ISIL_COMWrite(iFd, cSendData, cLen);
			return iRet;
		}
	}
	return ISIL_FAIL;
}


static S32 ISIL_SonyUp(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[9] = {0x80,0x01,0x06,0x01,0x00,0x00,0x03,0x01,0xff};
	/*
	speed = 23*(camera->speed + 1)/64;
	cSendData[4] = speed;
	cSendData[5] = speed;
	*/
	iRet = ISIL_SetSonyCmd(cSendData, ComNo, 9);
	return iRet;

}


static S32 ISIL_SonyDown(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[9] = {0x80,0x01,0x06,0x01,0x00,0x00,0x03,0x02,0xff};
	/*
	speed = 23*(camera->speed + 1)/64;
	cSendData[4] = speed;
	cSendData[5] = speed;
	*/

	iRet = ISIL_SetSonyCmd (cSendData, ComNo, 9);
	return iRet;
}

static S32 ISIL_SonyLeft(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[9] = {0xa0,0x01,0x00,0x04,0x10,0x00,0xaf,0x15};
	/*
	speed = 24*(camera->speed + 1)/64;
	cSendData[4] = speed;
	cSendData[5] = speed;
	*/
	iRet = ISIL_SetSonyCmd(cSendData, ComNo, 9);
	return iRet;
}

static S32 ISIL_SonyRight(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[9] = {0xa0,0x01,0x00,0x02,0x10,0x00,0xaf,0x13};
	/*
	speed = 23*(camera->speed + 1)/64;
	cSendData[4] = speed;
	cSendData[5] = speed;
	*/
	iRet = ISIL_SetSonyCmd(cSendData, ComNo, 9);
	return iRet;
}

static S32 ISIL_SonyFocusAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[6] = {0x80,0x01,0x04,0x08,0x02,0xff};

	iRet = ISIL_SetSonyCmd(cSendData, ComNo, 6);
		return iRet;

}

static S32 ISIL_SonyFocusSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[6] = {0x80,0x01,0x04,0x08,0x03,0xff};

	iRet = ISIL_SetSonyCmd(cSendData, ComNo, 6);
		return iRet;

}

static S32 ISIL_SonyIrisAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[6] = {0x80,0x01,0x04,0x0b,0x02,0xff};

	iRet = ISIL_SetSonyCmd(cSendData, ComNo, 6);
		return iRet;

}

static S32 ISIL_SonyIrisSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[6] = {0x80,0x01,0x04,0x0b,0x03,0xff};

	iRet = ISIL_SetSonyCmd (cSendData, ComNo, 6);
		return iRet;

}

static S32 ISIL_SonyZoomAdd(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[6] = {0x80,0x01,0x04,0x07,0x03,0xff};
	/*
	speed = (camera->speed +1)/4;
	send_data[4] |= (speed<9)?0x03:(0x30|(speed-9));
	*/
	iRet = ISIL_SetSonyCmd (cSendData, ComNo, 6);
		return iRet;

}

static S32 ISIL_SonyZoomSub(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[8] = {0x80,0x01,0x04,0x07,0x02,0xff};
	/*
	speed = (camera->speed +1)/4;
	send_data[4] |= (speed<9)?0x03:(0x30|(speed-9));
	*/
	iRet = ISIL_SetPelcopCmd (cSendData, ComNo);
		return iRet;
}

static S32 ISIL_SonyAutoOpen(U08 ComNo)
{
    ISIL_TRACE(BUG_NORMAL, "%d", ComNo);
	return ISIL_FAIL;
}

static S32 ISIL_SonyAutoClose(U08 ComNo)
{
    ISIL_TRACE(BUG_NORMAL, "%d", ComNo);
	return ISIL_FAIL;
}



static S32 ISIL_SonySetPreset(U08 ComNo)
{
    ISIL_TRACE(BUG_NORMAL, "%d", ComNo);
	return ISIL_FAIL;
}


static S32 ISIL_SonyGoPreset(U08 ComNo)
{
    ISIL_TRACE(BUG_NORMAL, "%d", ComNo);
	return ISIL_FAIL;
}


static S32 ISIL_SonyClearPreset(U08 ComNo)
{
    ISIL_TRACE(BUG_NORMAL, "%d", ComNo);
	return ISIL_FAIL;
}


static S32 ISIL_SonyStop(U08 ComNo)
{
	S32 iRet;
	S08 cSendData[9] = {0x80,0x01,0x06,0x01,0x00,0x00,0x03,0x03,0xff};

	iRet = ISIL_SetSonyCmd(cSendData, ComNo, 9);
		return iRet;
}


/*PTZ control function*/
static pFun ISIL_PTZ_F_UP[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodUp,
	ISIL_PelcopUp,
	ISIL_SonyUp,
	NULL
};
static pFun ISIL_PTZ_F_DOWN[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodDown,
	ISIL_PelcopDown,
	ISIL_SonyDown,
	NULL
};

static pFun ISIL_PTZ_F_LEFT[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodLeft,
	ISIL_PelcopLeft,
	ISIL_SonyLeft,
	NULL

};
static pFun ISIL_PTZ_F_RIGHT[ISIL_PTZ_PROTOCOL_MAX]=
{
	NULL,
	ISIL_PelcodRight,
	ISIL_PelcopRight,
	ISIL_SonyRight,
	NULL
};

static pFun ISIL_PTZ_F_FOCUSADD[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodFocusAdd,
	ISIL_PelcopFocusAdd,
	ISIL_SonyFocusAdd,
	NULL
};
static pFun ISIL_PTZ_F_FOCUSSUB[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodFocusSub,
	ISIL_PelcopFocusSub,
	ISIL_SonyFocusSub,
	NULL
};
static pFun ISIL_PTZ_F_IRISADD[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodIrisAdd,
	ISIL_PelcopIrisAdd,
	ISIL_SonyIrisAdd,
	NULL
};
static pFun ISIL_PTZ_F_IRISSUB[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodIrisSub,
	ISIL_PelcopIrisSub,
	ISIL_SonyIrisSub,
	NULL
};
static pFun ISIL_PTZ_F_ZOOMADD[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodZoomAdd,
	ISIL_PelcopZoomAdd,
	ISIL_SonyZoomAdd,
	NULL
};
static pFun ISIL_PTZ_F_ZOOMSUB[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodZoomSub,
	ISIL_PelcopZoomSub,
	ISIL_SonyZoomSub,
	NULL
};
static pFun ISIL_PTZ_F_AUTOOPEN[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodAutoOpen,
	ISIL_PelcopAutoOpen,
	ISIL_SonyAutoOpen,
	NULL
};
static pFun ISIL_PTZ_F_AUTOCLOSE[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodAutoOpen,
	ISIL_PelcopAutoOpen,
	ISIL_SonyAutoOpen,
	NULL
};
static pFun ISIL_PTZ_F_LAMPOPEN[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
};
static pFun ISIL_PTZ_F_LAMPCLOSE[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
};
static pFun ISIL_PTZ_F_BRUSHOPEN[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
};
static pFun ISIL_PTZ_F_BRUSHCLOSE[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
};
static pFun ISIL_PTZ_F_WATEROPEN[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
};
static pFun ISIL_PTZ_F_WATERCLOSE[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
};
static pFun ISIL_PTZ_F_PRESET[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
};
static pFun ISIL_PTZ_F_CALL[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
};
static pFun ISIL_PTZ_F_STOP[ISIL_PTZ_PROTOCOL_MAX] =
{
	NULL,
	ISIL_PelcodStop,
	ISIL_PelcopStop,
	ISIL_SonyStop,
	NULL
};

static S32 ISIL_PTZ_DispatchCmd(U08 ucPtzTye, U08 ucPtzCmd, U08 ComNo)
{
	S32 iRet = 0;
	ISIL_TRACE(BUG_NORMAL, "[%s] cmd %d", __FUNCTION__, ucPtzCmd);
	switch(ucPtzCmd)
	{

		case ISIL_PTZ_UP:
			iRet =  ISIL_PTZ_F_UP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_DOWN:
			iRet =  ISIL_PTZ_F_DOWN[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_LEFT:
			iRet =  ISIL_PTZ_F_LEFT[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_RIGHT:
			iRet =  ISIL_PTZ_F_RIGHT[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_FOCUSADD:
			iRet =  ISIL_PTZ_F_FOCUSADD[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_FOCUSSUB:
			iRet =  ISIL_PTZ_F_FOCUSSUB[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_IRISADD:
			iRet =  ISIL_PTZ_F_IRISADD[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_IRISSUB:
			iRet =  ISIL_PTZ_F_IRISSUB[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_ZOOMADD:
			iRet =  ISIL_PTZ_F_ZOOMADD[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_ZOOMSUB:
			iRet =  ISIL_PTZ_F_ZOOMSUB[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_AUTOOPEN:
			iRet =  ISIL_PTZ_F_AUTOOPEN[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_AUTOCLOSE:
			iRet =  ISIL_PTZ_F_AUTOCLOSE[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_LAMPOPEN:
			iRet =  ISIL_PTZ_F_LAMPOPEN[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_LAMPCLOSE:
			iRet =  ISIL_PTZ_F_LAMPCLOSE[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_BRUSHOPEN:
			iRet =  ISIL_PTZ_F_BRUSHOPEN[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_BRUSHCLOSE:
			iRet =  ISIL_PTZ_F_BRUSHCLOSE[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_WATEROPEN:
			iRet =  ISIL_PTZ_F_WATEROPEN[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_WATERCLOSE:
			iRet =  ISIL_PTZ_F_WATERCLOSE[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_PRESET:
			iRet =  ISIL_PTZ_F_LEFT[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_CALL:
			iRet =  ISIL_PTZ_F_CALL[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_UP_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_DOWN_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_LEFT_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_RIGHT_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_FOCUSADD_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_FOCUSSUB_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_IRISADD_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_IRISSUB_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_ZOOMADD_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		case ISIL_PTZ_ZOOMSUB_STOP:
			iRet =  ISIL_PTZ_F_STOP[ucPtzTye](ComNo);
			break;
		default:
			break;
	}

	return iRet;
}


static U32 ISIL_GetPTZTypeByChannel(U32 lChannnel)
{
	U32 lPTZtype = 0;
	switch(lChannnel)
	{
		case 1:
		break;

		default:
		break;
	}
	return lPTZtype;
}
S32 ISIL_GetPTZProtocol(U32 lChannel, ISIL_PTZ_PROTOCOL_S * ptzProtocol)
{
	U32 lPTZType = 0;
	if(ptzProtocol == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "%s, ptz protocol point is null", __FUNCTION__);
		return ISIL_FAIL;
	}
	lPTZType = ISIL_GetPTZTypeByChannel(lChannel);

	return ISIL_SUCCESS;
}

S32 ISIL_SetPTZContrl(ISIL_PTZ_CTRL_S* ptzCtrl)
{
	S32 iRet;
	if(ptzCtrl == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "%s ptzCtrl point is null", __FUNCTION__);
		return ISIL_FAIL;
	}
	iRet = ISIL_PTZ_DispatchCmd(ptzCtrl->cPTZType, ptzCtrl->cPTZCmd, ptzCtrl->cCOMMNo);

	return iRet;
}

S32 ISIL_SetPTZInfo(ISIL_PTZ_CFG_S* ptzCfg, S32 channel)
{
	S32 iRet;
	S32 iType;
	S08 cPath[ISIL_PATHNAME_LEN];

	if(ptzCfg == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "%s ptzCfg point is null", __FUNCTION__);
		return ISIL_FAIL;
	}

	//sprintf(cPath, "%s%s", RS485_CONFIG_FILE, ptzCfg->sName);
	sprintf(cPath, "%s%d", RS485_CONFIG_FILE, channel);
	iRet = ISIL_WriteConfigFile(cPath, 0, sizeof(ISIL_PTZ_CFG_S),
		&iType, ptzCfg);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], read ptz cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
	return ISIL_SUCCESS;
}

S32 ISIL_GetPTZInfo(ISIL_PTZ_CFG_S* ptzCfg, S32 channel)
{
	S32 iRet;
	S32 iType;
	S08 cPath[ISIL_PATHNAME_LEN];
	if(ptzCfg == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "%s ptzCfg point is null", __FUNCTION__);
		return ISIL_FAIL;
	}


	//sprintf(cPath, "%s%s", RS485_CONFIG_FILE, ptzCfg->sName);
	sprintf(cPath, "%s%d", RS485_CONFIG_FILE, channel);
	iRet = ISIL_ReadConfigFile(cPath, 0, sizeof(ISIL_PTZ_CFG_S),
		&iType, ptzCfg);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], read ptz cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
    strcpy(ptzCfg->sName, "PELCO_D.COD");
	ISIL_TRACE(BUG_NORMAL, "%s %d %d", ptzCfg->sName, ptzCfg->cCOMMNo, ptzCfg->cDecoderAddr);
	return ISIL_SUCCESS;
}



