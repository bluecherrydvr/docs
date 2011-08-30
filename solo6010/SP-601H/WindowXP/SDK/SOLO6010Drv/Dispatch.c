#include "Dispatch.h"
#include "Interrupts.h"
#include "Solo6010_Ioctl.h"
#include "SupportFunc.h"
#include "Driver.h"

#include "Solo6010.h"
#include "Solo6010_MP4Enc.h"
#include "Solo6010_Display.h"
#include "Solo6010_TW2815.h"
#include "Solo6010_I2C.h"
#include "Solo6010_VideoIn.h"
#include "Solo6010_G723.h"
#include "Solo6010_MP4Dec.h"
#include "Solo6010_GPIO.h"
#include "Solo6010_UART.h"


/******************************************************************************
 *
 * Function   :  DispatchIoControl
 *
 * Description:  Processes the IOCTL IRPs sent to this device.
 *
 * Note       :  The Fast Mutex used in this routine is left here because
 *               some multi-CPU systems hang if it is removed.  As of this time
 *               the cause for this is unknown.  Once the cause is determined,
 *               the Fast Mutex will most likely be removed in a future release.
 *               Performance may improve slightly once it is removed since the
 *               DPC_LEVEL won't continuously need to go between PASSIVE and APC.
 *
 ******************************************************************************/
NTSTATUS DispatchIoControl (PDEVICE_OBJECT fdo, PIRP pIrp)
{
    DEVICE_EXTENSION *pdx;
    PIO_STACK_LOCATION pStack;

	U8 *pUserBuf;
	INFO_REG_RW *pRegRW;
	INFO_S6010_INIT *pS6010Init;
	INFO_REC *pS6010Rec;
	INFO_LIVE *pInfoLive;
	int i, j;

    pdx = fdo->DeviceExtension;

    pStack = IoGetCurrentIrpStackLocation (pIrp);

//	KdPrint(("DispatchIoControl_S\n"));
    ExAcquireFastMutex (&pdx->bufFMutex[FMUX_DISPATCH]);

    // Handle the SOLO6010 specific message
    switch (pStack->Parameters.DeviceIoControl.IoControlCode)
    {
	case S6010_IOCTL_REG_READ:
	case S6010_IOCTL_REG_WRITE:
		KdPrint(("S6010_IOCTL_REG_READ or WRITE\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		if (pUserBuf != NULL)
		{
			pRegRW = (INFO_REG_RW *)pUserBuf;

			if (pStack->Parameters.DeviceIoControl.IoControlCode == S6010_IOCTL_REG_READ)
			{
				pRegRW->nVal = S6010_RegRead (pdx, pRegRW->nAddr);
			}
			else
			{
				S6010_RegWrite (pdx, pRegRW->nAddr, pRegRW->nVal);
			}
			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_OPEN_S6010:
		KdPrint(("S6010_IOCTL_OPEN_S6010\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		for (i=0, j=0; i<NUM_P2M_BUF; i++)
		{
			if (pdx->bufInfoP2M[i].szBufP2M != 0 &&
				pdx->bufInfoP2M[i].bufP2M == NULL)
			{
				j = 1;
				break;
			}
		}

		if (pdx->iS6010.bOpen == 0 && pUserBuf != NULL &&
			pdx->pIShare != NULL && j == 0)
		{
			pS6010Init = (INFO_S6010_INIT *)pUserBuf;

			// Common event connect[S]
			for (i=0; i<NUM_COMMON_EVT; i++)
			{
				if (ObReferenceObjectByHandle (pS6010Init->bufHEvt[i],
					EVENT_ALL_ACCESS, *ExEventObjectType, UserMode,
					&pdx->iS6010.bufPKEvt[i], NULL) != STATUS_SUCCESS)
				{
					pdx->iS6010.bufPKEvt[i] = NULL;
					break;
				}
			}
			if (i != NUM_COMMON_EVT)
			{
				for (j=0; j<i; i++)
				{
					ObDereferenceObject (pdx->iS6010.bufPKEvt[j]);
					pdx->iS6010.bufPKEvt[j] = NULL;
				}
				pIrp->IoStatus.Information = 0;
				break;
			}
			// Common event connect[E]

			// Shared buffer init[S]
			if (pdx->iS6010.bLockBuffer == 0)
			{
				pS6010Init->pIShare = MmMapLockedPagesSpecifyCache (pdx->pMDL_iShare,
					UserMode, MmCached, NULL, FALSE, NormalPagePriority);

				pS6010Init->pIShare->pBufMP4Rec = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_MP4_ENC].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_MP4_ENC].pLockedBuf = pS6010Init->pIShare->pBufMP4Rec;
				pS6010Init->pIShare->iMP4Buf.szBuf = pdx->bufInfoP2M[P2M_BUF_MP4_ENC].szBufP2M;

				pS6010Init->pIShare->pBufJPEGRec = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_JPEG_ENC].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_JPEG_ENC].pLockedBuf = pS6010Init->pIShare->pBufJPEGRec;
				pS6010Init->pIShare->iJPEGBuf.szBuf = pdx->bufInfoP2M[P2M_BUF_JPEG_ENC].szBufP2M;

				pS6010Init->pIShare->pBufG723Rec = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_G723_ENC].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_G723_ENC].pLockedBuf = pS6010Init->pIShare->pBufG723Rec;
				pS6010Init->pIShare->iG723Buf.szBuf = pdx->bufInfoP2M[P2M_BUF_G723_ENC].szBufP2M;

				pS6010Init->pIShare->pBufMP4Dec = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_MP4_DEC].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_MP4_DEC].pLockedBuf = pS6010Init->pIShare->pBufMP4Dec;
				pS6010Init->pIShare->iMP4DecBuf.szBuf = pdx->bufInfoP2M[P2M_BUF_MP4_DEC].szBufP2M;

				pS6010Init->pIShare->pBufG723Dec = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_G723_DEC].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_G723_DEC].pLockedBuf = pS6010Init->pIShare->pBufG723Dec;
				pS6010Init->pIShare->iG723DecBuf.szBuf = pdx->bufInfoP2M[P2M_BUF_G723_DEC].szBufP2M;

				pS6010Init->pIShare->pBufEncOSD = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_ENC_OSD].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_ENC_OSD].pLockedBuf = pS6010Init->pIShare->pBufEncOSD;
				pS6010Init->pIShare->iEncOSD.szBuf = pdx->bufInfoP2M[P2M_BUF_ENC_OSD].szBufP2M;

				pS6010Init->pIShare->pBufVidOSG = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_VID_OSG].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_VID_OSG].pLockedBuf = pS6010Init->pIShare->pBufVidOSG;
				pS6010Init->pIShare->iVidOSG.szBuf = pdx->bufInfoP2M[P2M_BUF_VID_OSG].szBufP2M;

				pS6010Init->pIShare->pBufCamBlkDet = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_CAM_BLK_DET].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_CAM_BLK_DET].pLockedBuf = pS6010Init->pIShare->pBufCamBlkDet;

				pS6010Init->pIShare->pBufAdvVM = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_ADV_MOTION].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_ADV_MOTION].pLockedBuf = pS6010Init->pIShare->pBufAdvVM;

				pS6010Init->pIShare->pBufLiveImg = MmMapLockedPagesSpecifyCache (pdx->bufInfoP2M[P2M_BUF_SHR_MEM_FOR_LIVE].pMDL,
					UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
				pdx->bufInfoP2M[P2M_BUF_SHR_MEM_FOR_LIVE].pLockedBuf = pS6010Init->pIShare->pBufLiveImg;
				pS6010Init->pIShare->szBufLiveImg = pdx->bufInfoP2M[P2M_BUF_SHR_MEM_FOR_LIVE].szBufP2M;

				pdx->iS6010.pLockedInfoShareData = pS6010Init->pIShare;

				pdx->iS6010.bLockBuffer = 1;
			}
			// Shared buffer init[E]

			pdx->iS6010.bOpen = 1;

			// S6010 init[S]
			S6010_InitInfoS6010 (pdx, pS6010Init);
			if (S6010_Init (pdx) == -1)
			{
				pdx->iS6010.bInit = 0;
				pIrp->IoStatus.Information = 0;
			}
			else
			{
				pdx->iS6010.bInit = 1;
				pIrp->IoStatus.Information = 1;
			}
			// S6010 init[E]
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_SET_REC_PROP:
		KdPrint(("S6010_IOCTL_SET_REC_PROP\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		if (pUserBuf != NULL)
		{
			pS6010Rec = (INFO_REC *)pUserBuf;
			S6010_SetInfoMP4JPEGRec (pdx, pS6010Rec);
			S6010_SetInfoG723Rec (pdx, pS6010Rec);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_START_REC:
		KdPrint(("S6010_IOCTL_START_REC\n"));

		if (pdx->bufInfoP2M[P2M_BUF_MP4_ENC].bufP2M != NULL &&
			pdx->bufInfoP2M[P2M_BUF_JPEG_ENC].bufP2M != NULL)
		{
			S6010_StartMP4JPEGEnc (pdx);
		}
		if (pdx->bufInfoP2M[P2M_BUF_G723_ENC].bufP2M != NULL)
		{
			S6010_StartG723Enc (pdx);
		}

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_END_REC:
		KdPrint(("S6010_IOCTL_END_REC\n"));

		S6010_EndG723Enc (pdx);
		S6010_EndMP4JPEGEnc (pdx);
		KeSetEvent (&pdx->bufEvent[IDX_EVT_TH_VID_ENC_DATA_READ], EVENT_INCREMENT, FALSE);
		KeSetEvent (&pdx->bufEvent[IDX_EVT_TH_G723_ENC_READ_DEC_WRITE], EVENT_INCREMENT, FALSE);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_CHANGE_REC_PROP:
		KdPrint(("S6010_IOCTL_CHANGE_REC_PROP\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		if (pUserBuf != NULL)
		{
			pS6010Rec = (INFO_REC *)pUserBuf;
			S6010_ChangeInfoMP4JPEGRec (pdx, pS6010Rec);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_SET_LIVE_PROP:
		KdPrint(("S6010_IOCTL_SET_LIVE_PROP\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		if (pUserBuf != NULL)
		{
			pInfoLive = (INFO_LIVE *)pUserBuf;
			S6010_SetInfoLive (pdx, pInfoLive);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_START_LIVE:
		KdPrint(("S6010_IOCTL_START_LIVE\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
			INFO_DDRAW_SURF *pIDDrawSurf;
			pIDDrawSurf = (INFO_DDRAW_SURF *)pUserBuf;

			// INFO_DDRAW Init from INFO_DDRAW_SURF[S]
			memset (&pdx->pIShare->curStatDDrawBuf, 0, sizeof(CUR_STAT_DDRAW_BUF));

			pdx->pIShare->curStatDDrawBuf.numBuf = pIDDrawSurf->numSurf;

			for (i=0; i<pIDDrawSurf->numSurf; i++)
			{
				pdx->iDDraw.bufPMdl[i] = IoAllocateMdl (pIDDrawSurf->lpSurf[i], pIDDrawSurf->nPitch *pIDDrawSurf->szV, FALSE, TRUE, NULL);
				MmProbeAndLockPages (pdx->iDDraw.bufPMdl[i], KernelMode, IoModifyAccess);
				pdx->iDDraw.bufPhyAddr[i].LowPart = (MmGetMdlPfnArray (pdx->iDDraw.bufPMdl[i])[0]) <<PAGE_SHIFT;
				pdx->iDDraw.bufPhyAddr[i].HighPart = 0;

//				pdx->iDDraw.bufPhyAddr[i] = MmGetPhysicalAddress ((PVOID)pIDDrawSurf->lpSurf[i]);
			}

			pdx->iDDraw.bField = pIDDrawSurf->bField;
			pdx->iDDraw.bFlip = pIDDrawSurf->bFlip;
			pdx->iDDraw.bUse1FieldInFrmMode = pIDDrawSurf->bUse1FieldInFrmMode;

			pdx->iDDraw.szH = pIDDrawSurf->szH;
			pdx->iDDraw.szV = pIDDrawSurf->szV;
			if (pdx->iDDraw.szH > (unsigned long)pdx->iS6010.szVideoH)
			{
				pdx->iDDraw.szH = pdx->iS6010.szVideoH;
			}
			if (pdx->iDDraw.szV > (unsigned long)pdx->iS6010.szVideoV *2)
			{
				pdx->iDDraw.szV = pdx->iS6010.szVideoV *2;
			}

			if (pdx->iS6010.typeVideo == VIDEO_TYPE_NTSC)
			{
				pdx->iDDraw.szLiveExt1Pic = 0xf0000;
			}
			else
			{
				pdx->iDDraw.szLiveExt1Pic = 0x120000;
			}

			pdx->iDDraw.nPitch = pIDDrawSurf->nPitch;
			pdx->iDDraw.sz1LineCopy = pdx->iDDraw.szH *2;
			if (pdx->iDDraw.nPitch < pdx->iDDraw.sz1LineCopy)
			{
				pdx->iDDraw.sz1LineCopy = pdx->iDDraw.nPitch;
			}
			// INFO_DDRAW Init from INFO_DDRAW_SURF[E]

			pdx->iDDraw.bLiveUpdateOn = 1;
			S6010_EnableInterrupt (pdx, INTR_VIDEO_IN);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_END_LIVE:
		KdPrint(("S6010_IOCTL_END_LIVE\n"));

		pdx->iDDraw.bLiveUpdateOn = 0;
		S6010_DisableInterrupt (pdx, INTR_VIDEO_IN);

		for (i=0; i<pdx->pIShare->curStatDDrawBuf.numBuf; i++)
		{
			MmUnlockPages (pdx->iDDraw.bufPMdl[i]);
		}

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_GET_COLOR:
		KdPrint(("S6010_IOCTL_GET_COLOR\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		S6010_GetColor (pdx, (INFO_COLOR *)pUserBuf);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_SET_COLOR:
		KdPrint(("S6010_IOCTL_SET_COLOR\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		S6010_SetColor (pdx, (INFO_COLOR *)pUserBuf);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_GET_VLOSS_STATE:
//		KdPrint(("S6010_IOCTL_GET_VLOSS_STATE\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
			S6010_GetVLoss (pdx, (INFO_VLOSS *)pUserBuf);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_SET_MOSAIC_PROP:
		KdPrint(("S6010_IOCTL_SET_MOSAIC_PROP\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		S6010_SetInfoMosaic (pdx, (INFO_MOSAIC *)pUserBuf);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_SET_V_MOTION_PROP:
//		KdPrint(("SET_V_M\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		S6010_SetInfoVMotion (pdx, (INFO_V_MOTION *)pUserBuf);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_SET_CAM_TO_WND_PROP:
		KdPrint(("S6010_IOCTL_SET_CAM_TO_WND_PROP\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		S6010_SetInfoCamToCh (pdx, (INFO_VIDEO_MATRIX *)pUserBuf);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_GET_CAM_BLK_STATE:
//		KdPrint(("S6010_IOCTL_GET_CAM_BLK_STATE\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		S6010_GetCamBlkDetState (pdx, (INFO_CAM_BLK_DETECTION *)pUserBuf);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_OPEN_HW_MP4_DEC:
		KdPrint(("S6010_IOCTL_OPEN_HW_MP4_DEC\n"));

		S6010_OpenMP4Dec (pdx);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_CLOSE_HW_MP4_DEC:
		KdPrint(("S6010_IOCTL_CLOSE_HW_MP4_DEC\n"));

		S6010_CloseMP4Dec (pdx);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_OPEN_HW_G723_DEC:
		KdPrint(("S6010_IOCTL_OPEN_HW_G723_DEC\n"));

		S6010_OpenG723Dec (pdx, pdx->pIShare->iG723DecStat.idxSamplingRate);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_CLOSE_HW_G723_DEC:
		KdPrint(("S6010_IOCTL_CLOSE_HW_G723_DEC\n"));

		S6010_EndG723Dec (pdx);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_PLAY_HW_G723_DEC:
		KdPrint(("S6010_IOCTL_PLAY_HW_G723_DEC\n"));

		S6010_StartG723Dec (pdx);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_STOP_HW_G723_DEC:
		KdPrint(("S6010_IOCTL_STOP_HW_G723_DEC\n"));

		S6010_EndG723Dec (pdx);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_GPIO_CTRL:
		KdPrint(("S6010_IOCTL_GPIO_CTRL\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);
		if (pUserBuf != NULL)
		{
			S6010_GPIO_Operation (pdx, (INFO_GPIO *)pUserBuf);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_SET_ENC_OSD_PROP:
//		KdPrint(("S6010_IOCTL_SET_ENC_OSD_PROP\n"));

		S6010_SetEncoderOSD_Prop (pdx, (INFO_ENCODER_OSD *)&pdx->pIShare->iEncOSD);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_GET_ENC_OSD:
		KdPrint(("S6010_IOCTL_GET_ENC_OSD\n"));

		pdx->pIShare->iEncOSD.phyAddrBufOSD = pdx->bufInfoP2M[P2M_BUF_ENC_OSD].nPhyAddr.LowPart;

		S6010_GetEncoderOSD (pdx, (INFO_ENCODER_OSD *)&pdx->pIShare->iEncOSD);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_SET_ENC_OSD:
//		KdPrint(("S6010_IOCTL_SET_ENC_OSD\n"));

		pdx->pIShare->iEncOSD.phyAddrBufOSD = pdx->bufInfoP2M[P2M_BUF_ENC_OSD].nPhyAddr.LowPart;

		S6010_SetEncoderOSD (pdx, (INFO_ENCODER_OSD *)&pdx->pIShare->iEncOSD);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_GET_VID_OSG:
		KdPrint(("S6010_IOCTL_GET_VID_OSG\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
		}

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_SET_VID_OSG:
		KdPrint(("S6010_IOCTL_SET_VID_OSG\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
		}

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_SETUP_UART:
		KdPrint(("S6010_IOCTL_SETUP_UART\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
			INFO_UART_SETUP *pI_UART_Setup = (INFO_UART_SETUP *)pUserBuf;
			S6010_UART_Setup (pdx, pI_UART_Setup);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_READ_UART:
		KdPrint(("S6010_IOCTL_READ_UART\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
			INFO_UART_RW *pI_UART_RW = (INFO_UART_RW *)pUserBuf;
			S6010_UART_Read (pdx, pI_UART_RW->idxPort, pI_UART_RW->szBufData, pI_UART_RW->pBufData, &pI_UART_RW->szActualRW);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_WRITE_UART:
		KdPrint(("S6010_IOCTL_WRITE_UART\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
			INFO_UART_RW *pI_UART_RW = (INFO_UART_RW *)pUserBuf;
			S6010_UART_Write (pdx, pI_UART_RW->idxPort, pI_UART_RW->szBufData, pI_UART_RW->pBufData, &pI_UART_RW->szActualRW);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_START_ADV_MOTION_DATA_GET:
		KdPrint(("S6010_IOCTL_START_ADV_MOTION_DATA_GET\n"));

		S6010_StartAdvMotionDataGet (pdx);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_END_ADV_MOTION_DATA_GET:
		KdPrint(("S6010_IOCTL_END_ADV_MOTION_DATA_GET\n"));

		S6010_EndAdvMotionDataGet (pdx);

		pIrp->IoStatus.Information = 1;
		break;
	case S6010_IOCTL_I2C_READ:
		KdPrint(("S6010_IOCTL_I2C_READ\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
			INFO_I2C_RW *pI_I2C_RW = (INFO_I2C_RW *)pUserBuf;
			pI_I2C_RW->nData = S6010_I2C_Read (pdx, pI_I2C_RW->nChannel, pI_I2C_RW->nAddrSlave, pI_I2C_RW->nAddrSlave +1, pI_I2C_RW->nAddrSub);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;
	case S6010_IOCTL_I2C_WRITE:
		KdPrint(("S6010_IOCTL_I2C_WRITE\n"));

		pUserBuf = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, NormalPagePriority);

		if (pUserBuf != NULL)
		{
			INFO_I2C_RW *pI_I2C_RW = (INFO_I2C_RW *)pUserBuf;
			pI_I2C_RW->nData = S6010_I2C_Write (pdx, pI_I2C_RW->nChannel, pI_I2C_RW->nAddrSlave, pI_I2C_RW->nAddrSub, pI_I2C_RW->nData);

			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		break;

	/*****************************
	 *   Unsupported Messages
	 ****************************/
	default:
		KdPrint(("Unsupported S6010_IOCTL_Xxx (0x%08x)\n",
			pStack->Parameters.DeviceIoControl.IoControlCode));

		// Set all possible return codes
		break;
	}

//	KdPrint(("DispatchIoControl_E\n"));
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_DISPATCH]);

	return S6010_CompleteIrp (pIrp, STATUS_SUCCESS);
}

/******************************************************************************
 *
 * Function   :  DispatchCreate
 *
 * Description:  Handle IRP_MJ_CREATE, which allows applications to open handles
 *               to our device
 *
 ******************************************************************************/
NTSTATUS DispatchCreate (PDEVICE_OBJECT fdo, PIRP pIrp)
{
	DEVICE_EXTENSION *pdx;
	
#if __NUM_SOLO_CHIPS > 1
#define SET_ADDR_OTHER_BASE_REG(idxID, idxOtherID)	\
	glIAboutRegIO.bufAddrOtherBaseReg[idxID] = (U32)MmMapIoSpace (glIAboutRegIO.bufAddrPhyBaseAddr[idxOtherID], sizeof(U32), FALSE)

	const int GL_MMAP_CHIP_ID_TO_REG_IO_ID[NUM_MAX_S6010 -1][NUM_MAX_S6010] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 0,
		0, 0, 1, 1, 2, 2,
	};
	const int GL_MMAP_CHIP_ID_TO_OTHER_CHIP_ID[NUM_MAX_S6010 -1][NUM_MAX_S6010] = {
		1, 0, 0, 0, 0, 0,
		1, 2, 0, 0, 0, 0,
		1, 0, 3, 2, 0, 0,
		1, 0, 3, 4, 2, 0,
		1, 0, 3, 2, 5, 4,
	};
#endif

	KdPrint(("\n"));
	KdPrint((DBG_NAME "Received message (IRP=0x%08x) ===> IRP_MJ_CREATE\n", pIrp));

	pdx = fdo->DeviceExtension;

#if __NUM_SOLO_CHIPS > 1
	pdx->iS6010.idxID_RegIO = GL_MMAP_CHIP_ID_TO_REG_IO_ID[__NUM_SOLO_CHIPS -2][pdx->iS6010.idxID];
	SET_ADDR_OTHER_BASE_REG(pdx->iS6010.idxID, GL_MMAP_CHIP_ID_TO_OTHER_CHIP_ID[__NUM_SOLO_CHIPS -2][pdx->iS6010.idxID]);
#endif

	return S6010_CompleteIrpWithInformation (pIrp, STATUS_SUCCESS, 0);
}

/******************************************************************************
 *
 * Function   :  DispatchCleanup
 *
 * Description:  Handle the IRP_MJ_CLEANUP IRP.
 *
 ******************************************************************************/
NTSTATUS DispatchCleanup (PDEVICE_OBJECT fdo, PIRP pIrp)
{
	PIO_STACK_LOCATION pStack;

	KdPrint(("\n"));
	KdPrint((DBG_NAME "Received message (IRP=0x%08x) ===> IRP_MJ_CLEANUP\n", pIrp));

	pStack = IoGetCurrentIrpStackLocation (pIrp);

	return S6010_CompleteIrpWithInformation (pIrp, STATUS_SUCCESS, 0);
}

/******************************************************************************
 *
 * Function   :  DispatchClose
 *
 * Description:  Handle IRP_MJ_CLOSE, which allows applications to close handles
 *               to our device
 *
 ******************************************************************************/
NTSTATUS DispatchClose (PDEVICE_OBJECT fdo, PIRP pIrp)
{
	DEVICE_EXTENSION *pdx;
	int i;
	LARGE_INTEGER nTimeout;

	KdPrint(("\n"));
	KdPrint((DBG_NAME "Received message (IRP=0x%08x) ===> IRP_MJ_CLOSE\n", pIrp));

	pdx = fdo->DeviceExtension;

	if (pdx->iS6010.bOpen == 1)
	{
		S6010_Remove (pdx);

		nTimeout.QuadPart = 0;
		for (i=IDX_EVT_TH_VID_ENC_DATA_READ; i<NUM_EVENT; i++)
		{
			while (1)
			{
				if (KeWaitForSingleObject (&pdx->bufEvent[i], Executive, KernelMode, FALSE, &nTimeout) == STATUS_TIMEOUT)
				{
					break;
				}
			}
		}

		for (i=0; i<NUM_COMMON_EVT; i++)
		{
			ObDereferenceObject (pdx->iS6010.bufPKEvt[i]);
			pdx->iS6010.bufPKEvt[i] = NULL;
		}
		
		pdx->iS6010.bOpen = 0;
	}

	// Unlock Pages[S]
	KdPrint((DBG_NAME "Unlock Pages\n"));
	if (pdx->iS6010.bLockBuffer == 1)
	{
		MmUnmapLockedPages (pdx->iS6010.pLockedInfoShareData, pdx->pMDL_iShare);
		for (i=0; i<NUM_P2M_BUF; i++)
		{
			if (pdx->bufInfoP2M[i].szBufP2M != 0 && (i != P2M_BUF_TEMP && i != P2M_BUF_TEST))
			{
				MmUnmapLockedPages (pdx->bufInfoP2M[i].pLockedBuf, pdx->bufInfoP2M[i].pMDL);
			}
		}

		pdx->iS6010.bLockBuffer = 0;
	}
	// Unlock Pages[E]

	pdx->iS6010.bInit = 0;

#if __NUM_SOLO_CHIPS > 1
	MmUnmapIoSpace ((PVOID)glIAboutRegIO.bufAddrOtherBaseReg[pdx->iS6010.idxID], sizeof(U32));
#endif

	KdPrint((DBG_NAME "S6010_CompleteIrpWithInformation\n"));
	return S6010_CompleteIrpWithInformation (pIrp, STATUS_SUCCESS, 0);
}

/******************************************************************************
 *
 * Function   :  DispatchRead
 *
 * Description:  Handle IRP_MJ_READ, which is not implemented by this driver
 *
 ******************************************************************************/
NTSTATUS DispatchRead (PDEVICE_OBJECT fdo, PIRP pIrp)
{
	KdPrint(("\n"));
	KdPrint((DBG_NAME "Received message (IRP=0x%08x) ===> IRP_MJ_READ\n", pIrp));

	// Just complete the request
	return S6010_CompleteIrpWithInformation (pIrp, STATUS_NOT_IMPLEMENTED, 0);
}

/******************************************************************************
 *
 * Function   :  DispatchWrite
 *
 * Description:  Handle the IRP_MJ_WRITE, which is not implemented by this driver
 *
 ******************************************************************************/
NTSTATUS DispatchWrite (PDEVICE_OBJECT fdo, PIRP pIrp)
{
	KdPrint(("\n"));
	KdPrint((DBG_NAME "Received message (IRP=0x%08x) ===> IRP_MJ_WRITE\n", pIrp));

	// Just complete the request
	return S6010_CompleteIrpWithInformation (pIrp, STATUS_NOT_IMPLEMENTED, 0);
}

#if defined(WDM_DRIVER)
/******************************************************************************
 *
 * Function   :  DispatchSystemControl
 *
 * Description:  The dispatch routine for WMI IRPs.  It does nothing except
 *               forward the IRP to the next device in the stack.
 *
 ******************************************************************************/
NTSTATUS DispatchSystemControl (PDEVICE_OBJECT fdo, PIRP pIrp)
{
	KdPrint(("\n"));
	KdPrint((DBG_NAME "Received message (IRP=0x%08x) ===> IRP_MJ_SYSTEM_CONTROL\n", pIrp));

	IoSkipCurrentIrpStackLocation (pIrp);

	return IoCallDriver (((DEVICE_EXTENSION *)fdo->DeviceExtension)->pLowerDeviceObject, pIrp);
}
#endif




