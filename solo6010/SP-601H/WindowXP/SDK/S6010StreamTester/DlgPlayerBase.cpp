// DlgPlayerBase.cpp : implementation file
//

#include "stdafx.h"
#include "S6010StreamTester.h"
#include "DlgPlayerBase.h"

#include "MPEG4\\xvid.h"
#include "LibDeIntFilter.h"
#include "LibMPEG4ToAVI\DooMPEG4ToAVI.h"
#include <Mmsystem.h>
#include "DlgMainControl.h"
#include "DlgG723PlayCtrl.h"
#include "G723Coder.h"
#include "Solo6010.h"
#include "Solo6010_ExRecFileInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgPlayerBase *glPDlgPB;
CDlgMainControl glDlgMC;
CDlgG723PlayCtrl glDlgG723PC;

// Global Variables[S]...
BYTE glBufStrh[30] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x20, 0x02, 0x48, 0x0d, 0xc0, 0x00, 0x40, 0x00, 0x40,
					  0x00, 0x40, 0x00, 0x80, 0x00, 0x97, 0x53, 0x0a, 0x2c, 0x08, 0x3c, 0x28, 0x8c, 0x1f};
int glSzH_byScale[NUM_VIDEO_MODE][MAX_NUM_SCALE];
int glSzV_byScale[NUM_VIDEO_MODE][MAX_NUM_SCALE];
// Global Variables[E]...

// For Audio[S]...
struct AUDIO_BUFFER
{
	BYTE *bufData;
	int szData;
	AUDIO_BUFFER *pNext;
};

CG723Coder glCoderG723;
AUDIO_BUFFER *pBufAudio, *pLastBufAudio;
CCriticalSection glCrtG723PlayerAccNumBuf;	// glNumBufAudio access lock
int glNumBufAudio;
#define MAX_AUDIO_RCV_BUF	120
#define SZ_G723_SRC_1_FRM	48
#define SZ_G723_DEC_1_FRM	(SZ_G723_SRC_1_FRM *8 /3)
#define NUM_G723_REQ_READ	20
#define NUM_G723_READ	60

int GetDataToSoundOut (CBuffer* buffer, void* Owner);

HEADER_RECORD_FILE glHdrFileG723;
DWORD glSzG723BufLine;
DWORD glBufIdxLinePosFromCh[NUM_G723_CH];
// For Audio[E]...

#pragma warning( disable : 4005 )

#define GET_FRM_POS(idxCh, idxFrm)	(pDlgPB->m_infoStreamIdx.GetFramePos (idxCh, idxFrm))
#define GET_SCALE(idxCh, idxFrm)	(pDlgPB->m_infoStreamIdx.GetScale (idxCh, idxFrm))
#define GET_INTERVAL(idxCh, idxFrm)	(pDlgPB->m_infoStreamIdx.GetInterval (idxCh, idxFrm))
#define IS_KEY_FRM(idxCh, idxFrm)	(pDlgPB->m_infoStreamIdx.IsKeyFrame (idxCh, idxFrm))
#define GET_FRM_SIZE(idxCh, idxFrm)	(pDlgPB->m_infoStreamIdx.GetFrameSize (idxCh, idxFrm))
#define GET_MAX_FRM_NUM(idxCh)	(pDlgPB->m_infoStreamIdx.GetMaxFrameNum (idxCh))
#define IS_MOTION_EXIST(idxCh, idxFrm)	(pDlgPB->m_infoStreamIdx.IsMotionExist (idxCh, idxFrm))
#define GET_VIDEO_TYPE(idxCh)	(pDlgPB->m_infoStreamIdx.GetVideoType (idxCh))

#pragma warning( default : 4005 )

UINT ThFn_MultiPlayer (LPVOID pParam)
{
	CDlgPlayerBase *pDlgPB = glPDlgPB;

	int i, j, nNumCurDispWnd, nScale, nInterval, idxCh;

	BOOL bUpdateDisplay, bufBNullFile[MAX_DISP_WND], bufBEncryptedFile[MAX_DISP_WND],
		bufBWMFile[MAX_DISP_WND], bufBStartPlay[MAX_DISP_WND], bInterlace;
	DWORD nNumRead, bufTimeOldDec[MAX_DISP_WND];
	int bufTimeDiffOld[MAX_DISP_WND], bufIdxCurFrame[MAX_DISP_WND];
	BYTE *bufMP4Stream, *pBufMP4Stream;
	int nPitch, nZoom[2], numDiv;
	HANDLE hS6010Stream;
	RECT bufRcSrc[MAX_DISP_WND], bufRcDest[MAX_DISP_WND];
	BYTE *bufDec, *bufDisp, *pBufMotion;
	char strInfo[256];
	SIZE szImgZoom, szImg[MAX_DISP_WND];
	char charEncrypted, charWM;
	DWORD timeCur;
	SIZE szDispByMode;
	POINT posDispByIdx;

	hS6010Stream = INVALID_HANDLE_VALUE;
	for (i=0; i<MAX_DISP_WND; i++)
	{
		bufBStartPlay[i] = FALSE;
		bufBNullFile[i] = TRUE;
	}
	memset (bufIdxCurFrame, 0, sizeof(int) *MAX_DISP_WND);
	bufDec = new BYTE[SZ_MAX_DISP_BUF];

	// for XviD [S]...
	xvid_dec_stats_t xvid_dec_stats;
	xvid_dec_frame_t xvid_dec_frame;
	DWORD nUsedBytes;

	xvid_dec_frame.version = XVID_VERSION;
	xvid_dec_stats.version = XVID_VERSION;
	xvid_dec_frame.general = XVID_LOWDELAY;
	xvid_dec_frame.output.csp = XVID_CSP_I420;
	xvid_dec_frame.brightness = 0;
	// for XviD [E]...

	bufMP4Stream = new BYTE[SZ_MAX_MP4_STREAM];
	pBufMotion = NULL;

	while (1)
	{
		pDlgPB->m_bufEvent[EVT_MP4_PLAYER].Lock ();
		if (pDlgPB->m_bEndThread[IDX_TH_MP4_PLAYER] == TRUE)
		{
			break;
		}

		nNumCurDispWnd = GL_DISP_MODE_TO_NUM_DISP[pDlgPB->m_idxCurDispMode];

		bUpdateDisplay = FALSE;
		for (i=0; i<nNumCurDispWnd; i++)
		{
			idxCh = pDlgPB->m_idxCurDispCh[i];

			// Time & Player Mode Check[S]...
			pDlgPB->m_crtPlayer.Lock ();

			if (pDlgPB->m_bReqSnapshot == TRUE)
			{
				if (bufBNullFile[i] == FALSE)
				{
/*					pDlgPB->m_manSnapshot.WritePlayerSnapshotToFile (bufDec,
						bufSmp4FileHdr[i].szImgH, bufSmp4FileHdr[i].szImgV,
						LPCTSTR(pDlgPB->m_strSnapshotFilePath));
*/				}
				pDlgPB->m_bReqSnapshot = FALSE;
			}

			if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_REQ_INIT)
			{
				if (i == 0)
				{
					hS6010Stream = CreateFile (LPCTSTR(pDlgPB->m_strStreamFilePath),
						GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL |FILE_FLAG_RANDOM_ACCESS,
						NULL);

					for (j=0; j<MAX_DISP_WND; j++)
					{
						if (hS6010Stream == INVALID_HANDLE_VALUE ||
							pDlgPB->m_infoStreamIdx.GetMaxFrameNum (pDlgPB->m_idxCurDispCh[j]) <= 0)
						{
							bufBNullFile[j] = TRUE;
						}
						else
						{
							bufBNullFile[j] = FALSE;
							
							AddReport ("Disp:%d, Ch:%d Stream Exist", j, idxCh);
						}
					}
				}

				if (bufBNullFile[i] != TRUE)	// File Exist...
				{
/*					if (bufSmp4FileHdr[i].nEnc_Poly != 0 ||
						bufSmp4FileHdr[i].nEnc_InitialKey != 0)
					{
						bufBEncryptedFile[i] = TRUE;
						bufEncryptPoly[i] = bufSmp4FileHdr[i].nEnc_Poly;
					}
					else
					{
						bufBEncryptedFile[i] = FALSE;
					}
*/
					bufBEncryptedFile[i] = FALSE;

/*					if (bufSmp4FileHdr[i].nWM_Strength != 0)
					{
						bufBWMFile[i] = TRUE;
						bufWMPoly[i] = bufSmp4FileHdr[i].nWM_Poly;
					}
					else
					{
						bufBWMFile[i] = FALSE;
					}
*/
					bufBWMFile[i] = FALSE;

					nScale = GET_SCALE(idxCh, bufIdxCurFrame[i]);
					szImg[i].cx = glSzH_byScale[GET_VIDEO_TYPE(idxCh)][nScale];
					szImg[i].cy = glSzV_byScale[GET_VIDEO_TYPE(idxCh)][nScale];
					bInterlace = (nScale > 8) ? 1 : 0;

					AddReport ("Disp:%d, Ch:%d : nScale:%d, szH:%d, szV:%d, Interlace:%d",
						i, idxCh, nScale, szImg[i].cx, szImg[i].cy, bInterlace);

					MakeMP4SeqHdr (szImg[i].cx, szImg[i].cy, bInterlace);

					pBufMP4Stream = &glBufStrh[0];

					xvid_dec_frame.bitstream = pBufMP4Stream;
					xvid_dec_frame.length = sizeof(glBufStrh);
					xvid_dec_frame.output.plane[0] = 0;
					xvid_dec_frame.output.stride[0] = szImg[i].cx;

					xvid_dec_frame.general = XVID_LOWDELAY;
					nUsedBytes = xvid_decore (pDlgPB->m_hXvidDec[i], XVID_DEC_DECODE,
						&xvid_dec_frame, &xvid_dec_stats);
				}
				else
				{
					szImg[i].cx = 1;
					szImg[i].cy = 1;

					pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;
					pDlgPB->m_crtPlayer.Unlock ();
					goto ZOOM_AND_DISP;
				}

				bufTimeDiffOld[i] = 0;
				bufTimeOldDec[i] = timeGetTime ();

				pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;
			}
			else if (bufBNullFile[i] == TRUE)
			{
				pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;
				pDlgPB->m_crtPlayer.Unlock ();
				goto ZOOM_AND_DISP;
			}
			else if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_REQ_PLAY)
			{
				pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PLAYING;
			}
			else if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_REQ_STOP)
			{
				bufIdxCurFrame[i] = 0;
				pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;
			}
			else if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_PLAYING)
			{
				// Time Check[S]...
				timeCur = timeGetTime ();
				if (bufTimeOldDec[i] +bufTimeDiffOld[i] <= timeCur)
				{
					if (bufBStartPlay[i] == FALSE)
					{
						bufTimeDiffOld[i] = 0;
						bufTimeOldDec[i] = timeCur;

						bufBStartPlay[i] = TRUE;
					}
					else
					{
						bufTimeDiffOld[i] = bufTimeOldDec[i] +bufTimeDiffOld[i] -timeCur;
						bufTimeOldDec[i] = timeCur;
					}
					nScale = GET_SCALE(idxCh, bufIdxCurFrame[i]);
					nInterval = GET_INTERVAL(idxCh, bufIdxCurFrame[i]);
					if (nScale < 8)	// Field...
					{
						if (nInterval == 0)
						{
							bufTimeDiffOld[i] +=
								(1000 *(nInterval +1) *(bufIdxCurFrame[i] +1) /60)
								-(1000 *(nInterval +1) *bufIdxCurFrame[i] /60);
						}
						else
						{
							bufTimeDiffOld[i] +=
								(10000 *nInterval *(bufIdxCurFrame[i] +1) /300)
								-(10000 *nInterval *bufIdxCurFrame[i] /300);
						}
					}
					else	// Frame...
					{
						bufTimeDiffOld[i] +=
							(1000 *(nInterval +1) *(bufIdxCurFrame[i] +1) /30)
							-(1000 *(nInterval +1) *bufIdxCurFrame[i] /30);
					}
				}
				else
				{
					pDlgPB->m_crtPlayer.Unlock ();
					continue;
				}
				// Time Check[E]...
			}
			else if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_PAUSE)
			{
				bufBStartPlay[i] = FALSE;

				pDlgPB->m_crtPlayer.Unlock ();
				continue;
			}
			else if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_BACK_GOP)
			{
				bufIdxCurFrame[i]--;
				do
				{
					bufIdxCurFrame[i]--;
					if (bufIdxCurFrame[i] < 0)
					{
						bufIdxCurFrame[i] = 0;
						break;
					}
				} while (IS_KEY_FRM(idxCh, bufIdxCurFrame[i]) == FALSE);

				pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;
			}
			else if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_FW_ONE_FRM)
			{
				pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;
			}
			else if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_FW_GOP)
			{
				int idxTmpFrmPos = bufIdxCurFrame[i];
				
				while (IS_KEY_FRM(idxCh, bufIdxCurFrame[i]) == FALSE)
				{
					bufIdxCurFrame[i]++;
					if (bufIdxCurFrame[i] >= GET_MAX_FRM_NUM(idxCh))
					{
						break;
					}
				}

				pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;

				if (bufIdxCurFrame[i] >= GET_MAX_FRM_NUM(idxCh))
				{
					bufIdxCurFrame[i] = idxTmpFrmPos;
					pDlgPB->m_crtPlayer.Unlock ();
					continue;
				}
			}
			else if (pDlgPB->m_idxCurPlayerMode[i] == IDX_PLAYER_SET_POS)
			{
				bufIdxCurFrame[i] = pDlgPB->m_posSetFrm[i];

				while (IS_KEY_FRM(idxCh, bufIdxCurFrame[i]) == FALSE)
				{
					bufIdxCurFrame[i]--;

					if (bufIdxCurFrame[i] < 0)
					{
						bufIdxCurFrame[i] = 0;
						break;
					}
				}
				
				pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;
			}
			pDlgPB->m_crtPlayer.Unlock ();
			// Time & Player Mode Check[E]...

			// Decode[S]...
			/// Frame data read from file[S]...
			if (IS_MOTION_EXIST(idxCh, bufIdxCurFrame[i]) == TRUE)
			{
				SetFilePointer (hS6010Stream, GET_FRM_POS(idxCh, bufIdxCurFrame[i]) -SZ_MAX_MOTION_BUF,
					NULL, FILE_BEGIN);
				ReadFile (hS6010Stream, bufMP4Stream,
					GET_FRM_SIZE(idxCh, bufIdxCurFrame[i]) +SZ_MAX_MOTION_BUF, &nNumRead, NULL);

				pBufMotion = bufMP4Stream;
				pBufMP4Stream = bufMP4Stream +SZ_MAX_MOTION_BUF;
			}
			else
			{
				SetFilePointer (hS6010Stream, GET_FRM_POS(idxCh, bufIdxCurFrame[i]),
					NULL, FILE_BEGIN);
				ReadFile (hS6010Stream, bufMP4Stream,
					GET_FRM_SIZE(idxCh, bufIdxCurFrame[i]), &nNumRead, NULL);

				pBufMotion = NULL;
				pBufMP4Stream = bufMP4Stream;
			}

/*			AddReport ("Disp:%d, Ch:%d, Read:%d(%d), pt:%08x, idxCurFrm:%d, FrmPos:%d",
				i, idxCh, GET_FRM_SIZE(idxCh, bufIdxCurFrame[i]), nNumRead,
				pBufMP4Stream, bufIdxCurFrame[i],
				GET_FRM_POS(idxCh, bufIdxCurFrame[i]));*/

/*			if (bufBEncryptedFile[i] == TRUE)	// Decryption...
			{
				pDlgPB->m_manWmEncrypt[0].SetEncryptPoly (bufEncryptPoly[i]);
				pDlgPB->m_manWmEncrypt[0].SetEncryptKey (frmHdr.vidHdr.nStat[3]);
				pDlgPB->m_manWmEncrypt[0].DecryptStream (bufMP4Stream, GetTrueSizeFromVH (&frmHdr.vidHdr));
			}
*/
			/// Frame data read from file[E]...

			xvid_dec_frame.bitstream = pBufMP4Stream;
			xvid_dec_frame.length = GET_FRM_SIZE(idxCh, bufIdxCurFrame[i]);
			xvid_dec_frame.output.plane[0] = bufDec;
			xvid_dec_frame.output.stride[0] = szImg[i].cx;

			xvid_dec_frame.general = XVID_LOWDELAY;
			if (pDlgPB->m_bDeblocking == TRUE)
			{
				xvid_dec_frame.general |= XVID_DEBLOCKY |XVID_DEBLOCKUV;

				if (pDlgPB->m_bDeringing == TRUE)
				{
					xvid_dec_frame.general |= XVID_DERINGY |XVID_DERINGUV;
				}
			}
			if (glPDlgPB->m_bIBasedPred == TRUE)
			{
				xvid_dec_frame.general |= XVID_DOO_MPEG4PLUS;
			}

			nUsedBytes = xvid_decore (pDlgPB->m_hXvidDec[i], XVID_DEC_DECODE,
				&xvid_dec_frame, &xvid_dec_stats);

/*			if (pDlgPB->m_bWMInfo == TRUE && bufBWMFile[i] == TRUE &&
				GetPicTypeFromVH (&frmHdr.vidHdr) == 0x00)	// Watermark Decode & Detect...
			{
				pDlgPB->m_manWmEncrypt[0].SetWMPoly (bufWMPoly[i]);
				pDlgPB->m_manWmEncrypt[0].SetWMKey (frmHdr.vidHdr.nStat[2]);

				pDlgPB->m_manWmEncrypt[0].CalcCorrelation (
					bufSmp4FileHdr[i].szImgH, bufSmp4FileHdr[i].szImgV, bufDec,
					pDlgPB->m_numWMThCorr[int(pDlgPB->m_idxCurDispCh[i] /8)]);
			}
*/

			if (pDlgPB->m_bDeinterlace == TRUE)	// Apply deinterlace...
			{
				ExecDeIntFilter (szImg[i].cx, szImg[i].cy, bufDec);
			}

			bufDisp = (BYTE *)pDlgPB->m_wndS6010Disp.LockAndGetTempSurface ();
			nPitch = pDlgPB->m_wndS6010Disp.GetTempPitch ();

			pDlgPB->m_wndS6010Disp.YV12toBGRA (bufDec, bufDisp,
				szImg[i].cx, szImg[i].cy, nPitch);	// Colorspace converison

			if (IS_KEY_FRM(idxCh, bufIdxCurFrame[i]) == FALSE)	// P Picture...
			{
				if (pDlgPB->m_bMotionInfo == TRUE && pBufMotion != NULL)	// Motion info
				{
					pDlgPB->m_wndS6010Disp.DrawMotionData (nPitch,
						szImg[i].cx, szImg[i].cy, bufDisp, pBufMotion);
				}
			}
			else	// I Picture...
			{
/*				if (pDlgPB->m_bWMInfo == TRUE && bufBWMFile[i] == TRUE)	// Watermark Grid Draw...
				{
					pDlgPB->m_wndS6010Disp.DrawWMData (nPitch,
						szImg[i].cx, szImg[i].cy, bufDisp,
						pDlgPB->m_manWmEncrypt[0].GetWMResult ());
				}
*/			}
			pDlgPB->m_wndS6010Disp.UnlockTempSurface ();
			// Decode[E]...

ZOOM_AND_DISP:
			// Zoom & Disp Img Copy to Offscr Surf[S]...
			if (pDlgPB->m_bUpdateDispWndPt[i] == TRUE)
			{
				if (pDlgPB->m_idxCurDispMode == IDX_DISP_MODE_32)
				{
					szDispByMode.cx = pDlgPB->m_szDispWnd.cx /8;
					szDispByMode.cy = pDlgPB->m_szDispWnd.cy /4;

					posDispByIdx.x = szDispByMode.cx *(i %8);
					posDispByIdx.y = szDispByMode.cy *(i /8);
				}
				else
				{
					numDiv = GL_DISP_MODE_TO_NUM_DIV[pDlgPB->m_idxCurDispMode];

					szDispByMode.cx = pDlgPB->m_szDispWnd.cx /numDiv;
					szDispByMode.cy = pDlgPB->m_szDispWnd.cy /numDiv;

					posDispByIdx.x = szDispByMode.cx *(i %numDiv);
					posDispByIdx.y = szDispByMode.cy *(i /numDiv);
				}

				switch (pDlgPB->m_idxCurZoom[0])
				{
				case IDX_ZOOM_X1:
				case IDX_ZOOM_X2:
				case IDX_ZOOM_X3:
					szImgZoom.cx = szImg[i].cx *(pDlgPB->m_idxCurZoom[0] +1);
					nZoom[0] = pDlgPB->m_idxCurZoom[0] +1;
					break;
				}
				switch (pDlgPB->m_idxCurZoom[1])
				{
				case IDX_ZOOM_X1:
				case IDX_ZOOM_X2:
				case IDX_ZOOM_X3:
					szImgZoom.cy = szImg[i].cy *(pDlgPB->m_idxCurZoom[1] +1);
					nZoom[1] = pDlgPB->m_idxCurZoom[1] +1;
					break;
				}

				if (pDlgPB->m_idxCurZoom[0] == IDX_ZOOM_FIT)
				{
					bufRcSrc[i].left = 0;
					bufRcSrc[i].right = szImg[i].cx;

					bufRcDest[i].left = posDispByIdx.x;
					bufRcDest[i].right = bufRcDest[i].left +szDispByMode.cx;
				}
				else
				{
					if (szDispByMode.cx < szImgZoom.cx)
					{
						bufRcSrc[i].left = (szImg[i].cx -szDispByMode.cx /nZoom[0]) /2;
						bufRcSrc[i].right = bufRcSrc[i].left +szDispByMode.cx /nZoom[0];

						bufRcDest[i].left = posDispByIdx.x;
						bufRcDest[i].right = bufRcDest[i].left +szDispByMode.cx;
					}
					else
					{
						bufRcSrc[i].left = 0;
						bufRcSrc[i].right = szImg[i].cx;

						bufRcDest[i].left = posDispByIdx.x +(szDispByMode.cx -szImgZoom.cx) /2;
						bufRcDest[i].right = bufRcDest[i].left +szImgZoom.cx;
					}
				}
				if (pDlgPB->m_idxCurZoom[1] == IDX_ZOOM_FIT)
				{
					bufRcSrc[i].top = 0;
					bufRcSrc[i].bottom = szImg[i].cy;

					bufRcDest[i].top = posDispByIdx.y;
					bufRcDest[i].bottom = bufRcDest[i].top +szDispByMode.cy;
				}
				else
				{
					if (szDispByMode.cy < szImgZoom.cy)
					{
						bufRcSrc[i].top = (szImg[i].cy -szDispByMode.cy /nZoom[1]) /2;
						bufRcSrc[i].bottom = bufRcSrc[i].top +szDispByMode.cy /nZoom[1];

						bufRcDest[i].top = posDispByIdx.y;
						bufRcDest[i].bottom = bufRcDest[i].top +szDispByMode.cy;
					}
					else
					{
						bufRcSrc[i].top = 0;
						bufRcSrc[i].bottom = szImg[i].cy;

						bufRcDest[i].top = posDispByIdx.y +(szDispByMode.cy -szImgZoom.cy) /2;
						bufRcDest[i].bottom = bufRcDest[i].top +szImgZoom.cy;
					}
				}

				pDlgPB->m_bUpdateDispWndPt[i] = FALSE;
			}

			if (bufBNullFile[i] == TRUE)
			{
				if (pDlgPB->m_bFillDispWndBlack[i] == TRUE)
				{
					pDlgPB->m_wndS6010Disp.FillTempBlack ();

					bufRcSrc[i].left = bufRcSrc[i].top = 0;
					bufRcSrc[i].right = bufRcSrc[i].bottom = 1;

					bufRcDest[i].left = posDispByIdx.x;
					bufRcDest[i].top = posDispByIdx.y;
					bufRcDest[i].right = bufRcDest[i].left +szDispByMode.cx;
					bufRcDest[i].bottom = bufRcDest[i].top +szDispByMode.cy;

					pDlgPB->m_wndS6010Disp.BltTempToBack (bufRcDest[i], bufRcSrc[i],
						FALSE, NULL);

					pDlgPB->m_bFillDispWndBlack[i] = FALSE;
				}
			}
			else
			{
				if (pDlgPB->m_bChannelInfo == TRUE)	// Info String Make...
				{
					if (bufBEncryptedFile[i] == TRUE)
					{
						charEncrypted = 'E';
					}
					else
					{
						charEncrypted = ' ';
					}
					if (bufBWMFile[i] == TRUE)
					{
						charWM = 'W';
					}
					else
					{
						charWM = ' ';
					}
					sprintf (strInfo, "%2d F:%d/%d %c %c %d",
						pDlgPB->m_idxCurDispCh[i] +1, bufIdxCurFrame[i] +1,
						pDlgPB->m_infoStreamIdx.GetMaxFrameNum (idxCh),
						charEncrypted, charWM,
						pDlgPB->m_infoStreamIdx.GetTimeCode (idxCh, bufIdxCurFrame[i]));
				}

				pDlgPB->m_wndS6010Disp.BltTempToBack (bufRcDest[i], bufRcSrc[i],
					pDlgPB->m_bChannelInfo, strInfo);

				bufIdxCurFrame[i]++;

				if (bufIdxCurFrame[i] >= GET_MAX_FRM_NUM(idxCh))
				{
					bufIdxCurFrame[i] = 0;
					pDlgPB->m_idxCurPlayerMode[i] = IDX_PLAYER_PAUSE;
				}
			}

			bUpdateDisplay = TRUE;
			// Zoom & Disp Img Copy to Offscr Surf[E]...
		}

		if (pDlgPB->m_bEndThread[IDX_TH_MP4_PLAYER] == TRUE)
		{
			break;
		}
		
		// Display Img[S]...
		if (bUpdateDisplay == TRUE)
		{
			pDlgPB->m_wndS6010Disp.BltBackToPrimaryWithDrawingBorder ();
		}
		// Display Img[E]...
	}

	if (hS6010Stream != INVALID_HANDLE_VALUE)
	{
		CloseHandle (hS6010Stream);
	}
	delete[] bufMP4Stream;
	delete[] bufDec;

	pDlgPB->m_bEndThread[IDX_TH_MP4_PLAYER] = FALSE;

	return 0;
}

void CALLBACK TIMER_PROC (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CDlgPlayerBase *pDlg = (CDlgPlayerBase *)dwUser;

	pDlg->m_bufEvent[EVT_MP4_PLAYER].PulseEvent ();
}

UINT ThFn_G723Player (LPVOID pParam)
{
	CDlgPlayerBase *pDlgPB = glPDlgPB;

	BYTE *bufTmp, *bufG723;
	CFile fpAudio;
	short *bufAudioDec;
	DWORD i, nNumRead, tmpCurG723PlayCh;
	BOOL bFileOpen, bAudioPlaying, tmpNumBufAudio;
	AUDIO_BUFFER *pNextBuf;
	HEADER_RECORD_FILE hdrG723File;

	bFileOpen = FALSE;
	bAudioPlaying = FALSE;
	tmpNumBufAudio = 0;
	bufTmp = new BYTE[SZ_G723_1_ITEM *NUM_G723_READ];
	bufG723 = new BYTE[SZ_G723_SRC_1_FRM *NUM_G723_READ];
	bufAudioDec = new short[SZ_G723_DEC_1_FRM];
	
	pDlgPB->m_curG723PlayerMode = G723_PLAYER_PAUSE;
	while (1)
	{
		if (pDlgPB->m_bEndThread[IDX_TH_G723_PLAYER] == TRUE)
		{
			break;
		}

		pDlgPB->m_crtG723PlayerMode.Lock ();
		if (pDlgPB->m_curG723PlayerMode == G723_PLAYER_REQ_PLAY)
		{
			if (bFileOpen == FALSE)
			{
				AddReport ("FILE OPEN");

				glCrtG723PlayerAccNumBuf.Lock ();
				glNumBufAudio = 0;
				while (pBufAudio != NULL)
				{
					pNextBuf = pBufAudio->pNext;
					delete[] pBufAudio->bufData;
					delete pBufAudio;
					pBufAudio = pNextBuf;
				}
				glCrtG723PlayerAccNumBuf.Unlock ();

				AddReport ("Open G723 File: %s", pDlgPB->m_strG723FilePath);
				fpAudio.Abort ();
				if (fpAudio.Open (pDlgPB->m_strG723FilePath, CFile::modeRead) == FALSE)
				{
					bFileOpen = FALSE;

					pDlgPB->m_curG723PlayerMode = G723_PLAYER_PAUSE;

					AddReport ("FILE OPEN ERROR");
				}
				else
				{
					bFileOpen = TRUE;

					pDlgPB->m_posCurG723Frm = 0;

					fpAudio.Read (&hdrG723File, sizeof(HEADER_RECORD_FILE));

					if (hdrG723File.G723.setRec.idxSampRate == G723_SAMPLING_RATE_8K)
					{
						pDlgPB->m_soundOut.SetSamplesPerSecond (8000);
						AddReport ("Sampling Rate 8KHz");
					}
					else	// if (hdrG723File.iG723Rec.idxSampRate == G723_SAMPLING_RATE_16K)
					{
						pDlgPB->m_soundOut.SetSamplesPerSecond (16000);
						AddReport ("Sampling Rate 16KHz");
					}

					pDlgPB->m_curG723PlayerMode = G723_PLAYER_PLAYING;
					AddReport ("Num Frame: %d", pDlgPB->m_numG723Frm);
				}
			}
			else
			{
				if (tmpNumBufAudio != 0 && glNumBufAudio == 0)
				{
					glCrtG723PlayerAccNumBuf.Lock ();
					glNumBufAudio = tmpNumBufAudio;
					tmpNumBufAudio = 0;
					glCrtG723PlayerAccNumBuf.Unlock ();
				}
				
				pDlgPB->m_curG723PlayerMode = G723_PLAYER_PLAYING;
			}
		}
		else if (pDlgPB->m_curG723PlayerMode == G723_PLAYER_REQ_STOP)
		{
			if (bAudioPlaying == TRUE)
			{
				pDlgPB->m_soundOut.Stop ();
				bAudioPlaying = FALSE;
			}
			
			if (bFileOpen == TRUE)
			{
				fpAudio.Close ();
				bFileOpen = FALSE;
			}

			glCrtG723PlayerAccNumBuf.Lock ();
			tmpNumBufAudio = 0;
			glNumBufAudio = 0;
			while (pBufAudio != NULL)
			{
				pNextBuf = pBufAudio->pNext;
				delete[] pBufAudio->bufData;
				delete pBufAudio;
				pBufAudio = pNextBuf;
			}
			glCrtG723PlayerAccNumBuf.Unlock ();
			
			pDlgPB->m_curG723PlayerMode = G723_PLAYER_PAUSE;
		}
		else if (pDlgPB->m_curG723PlayerMode == G723_PLAYER_PLAYING)
		{
			tmpCurG723PlayCh = pDlgPB->m_curG723PlayCh;
			glCrtG723PlayerAccNumBuf.Lock ();
			
			if (glNumBufAudio < NUM_G723_REQ_READ)
			{
				glCrtG723PlayerAccNumBuf.Unlock ();

				nNumRead = fpAudio.Read (bufTmp, glSzG723BufLine *NUM_G723_READ);

				if (nNumRead == 0)
				{	// End of File (STOP)
					if (glNumBufAudio > 0)
					{	// need playing
					}
					else
					{
						AddReport ("EOF");

						fpAudio.Close ();					
						bFileOpen = FALSE;

						pDlgPB->m_curG723PlayerMode = G723_PLAYER_PAUSE;
					}
				}
				else
				{	// Playing
					for (i=0; i<nNumRead /glSzG723BufLine; i++)
					{
						memcpy (&bufG723[i *SZ_G723_SRC_1_FRM],
							&bufTmp[i *glSzG723BufLine +glBufIdxLinePosFromCh[tmpCurG723PlayCh]],
							SZ_G723_SRC_1_FRM);
					}
					for (i=0; i<nNumRead /glSzG723BufLine; i++)
					{
						glCoderG723.Decode (&bufG723[i *SZ_G723_SRC_1_FRM], SZ_G723_SRC_1_FRM,
							bufAudioDec, SZ_G723_DEC_1_FRM);

						glCrtG723PlayerAccNumBuf.Lock ();
						if (glNumBufAudio == 0)
						{
							pBufAudio = new AUDIO_BUFFER;
							pBufAudio->pNext = NULL;
							pBufAudio->szData = SZ_G723_DEC_1_FRM *2;
							pBufAudio->bufData = new BYTE[pBufAudio->szData];
							memcpy (pBufAudio->bufData, (BYTE *)bufAudioDec, pBufAudio->szData);
							pLastBufAudio = pBufAudio;
						}
						else
						{
							pLastBufAudio->pNext = new AUDIO_BUFFER;
							pLastBufAudio->pNext->pNext = NULL;
							pLastBufAudio->pNext->szData = SZ_G723_DEC_1_FRM *2;
							pLastBufAudio->pNext->bufData = new BYTE[pLastBufAudio->pNext->szData];
							memcpy (pLastBufAudio->pNext->bufData,
								(BYTE *)bufAudioDec, pLastBufAudio->pNext->szData);
							pLastBufAudio = pLastBufAudio->pNext;
						}
						glNumBufAudio++;
						glCrtG723PlayerAccNumBuf.Unlock ();
					}
				}
			}
			else
			{
				glCrtG723PlayerAccNumBuf.Unlock ();

				Sleep (50);
			}

			if (bAudioPlaying == FALSE)
			{
				pDlgPB->m_soundOut.Start ();
				bAudioPlaying = TRUE;
			}
		}
		else if (pDlgPB->m_curG723PlayerMode == G723_PLAYER_PAUSE)
		{
			glCrtG723PlayerAccNumBuf.Lock ();
			if (glNumBufAudio != 0)
			{
				tmpNumBufAudio = glNumBufAudio;
				glNumBufAudio = 0;
			}
			glCrtG723PlayerAccNumBuf.Unlock ();

			Sleep (50);
		}
		else if (pDlgPB->m_curG723PlayerMode == G723_PLAYER_SET_POS)
		{
			pDlgPB->m_curG723PlayerMode = G723_PLAYER_REQ_PLAY;
		}
		pDlgPB->m_crtG723PlayerMode.Unlock ();
	}

	if (bAudioPlaying == TRUE)
	{
		pDlgPB->m_soundOut.Stop ();
	}

	glCrtG723PlayerAccNumBuf.Lock ();
	glNumBufAudio = 0;
	glCrtG723PlayerAccNumBuf.Unlock ();

	delete[] bufTmp;
	delete[] bufG723;
	delete[] bufAudioDec;

	while (pBufAudio != NULL)
	{
		pNextBuf = pBufAudio->pNext;
		delete[] pBufAudio->bufData;
		delete pBufAudio;
		pBufAudio = pNextBuf;
	}
	Sleep (100);
	
	pDlgPB->m_bEndThread[IDX_TH_G723_PLAYER] = FALSE;

	return 0;
}

int GetDataToSoundOut(CBuffer* buffer, void* Owner)
{
	BOOL bRetVal;

	glCrtG723PlayerAccNumBuf.Lock ();

	if (glNumBufAudio == 0)
	{
		buffer->ByteLen = 256;
		memset (buffer->ptr.b, 0, 256);
		
		bRetVal = 1;
	}
	else
	{
		buffer->ByteLen = pBufAudio->szData;
		memcpy (buffer->ptr.b, pBufAudio->bufData, buffer->ByteLen);

		AUDIO_BUFFER *pNext = pBufAudio->pNext;
		delete[] pBufAudio->bufData;
		delete pBufAudio;
		pBufAudio = pNext;

		glNumBufAudio--;
		glPDlgPB->m_posCurG723Frm++;

		if (glDlgG723PC.GetSafeHwnd () != NULL)
		{
			glDlgG723PC.SendMessage (WM_DOO_UPDATE_G723_SLIDER_POS, 0, 0);
		}

		bRetVal = 1;
	}

	glCrtG723PlayerAccNumBuf.Unlock ();

	return bRetVal;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayerBase dialog


CDlgPlayerBase::CDlgPlayerBase(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPlayerBase::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPlayerBase)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	int i;

	CS6010StreamTesterApp *pApp = (CS6010StreamTesterApp *)AfxGetApp ();

	m_bFileOpen = FALSE;
	m_idxCurVideoMode = pApp->GetProfileInt ("SETTINGS", "VIDEO_MODE", IDX_VIDEO_NTSC);
	m_idxCurVideoMode = IDX_VIDEO_NTSC;
	memset (glSzH_byScale, 0, sizeof(glSzH_byScale));
	memset (glSzV_byScale, 0, sizeof(glSzV_byScale));

	glSzH_byScale[IDX_VIDEO_NTSC][1] = VIDEO_IN_H_SIZE;
	glSzV_byScale[IDX_VIDEO_NTSC][1] = 240;
	glSzH_byScale[IDX_VIDEO_NTSC][2] = VIDEO_IN_H_SIZE /2;
	glSzV_byScale[IDX_VIDEO_NTSC][2] = 240;
	glSzH_byScale[IDX_VIDEO_NTSC][5] = VIDEO_IN_H_SIZE /4;
	glSzV_byScale[IDX_VIDEO_NTSC][5] = 112;
	glSzH_byScale[IDX_VIDEO_NTSC][9] = VIDEO_IN_H_SIZE;
	glSzV_byScale[IDX_VIDEO_NTSC][9] = 480;
	glSzH_byScale[IDX_VIDEO_NTSC][10] = VIDEO_IN_H_SIZE /2;
	glSzV_byScale[IDX_VIDEO_NTSC][10] = 480;

	glSzH_byScale[IDX_VIDEO_PAL][1] = VIDEO_IN_H_SIZE;
	glSzV_byScale[IDX_VIDEO_PAL][1] = 288;
	glSzH_byScale[IDX_VIDEO_PAL][2] = VIDEO_IN_H_SIZE /2;
	glSzV_byScale[IDX_VIDEO_PAL][2] = 288;
	glSzH_byScale[IDX_VIDEO_PAL][5] = VIDEO_IN_H_SIZE /4;
	glSzV_byScale[IDX_VIDEO_PAL][5] = 144;
	glSzH_byScale[IDX_VIDEO_PAL][9] = VIDEO_IN_H_SIZE;
	glSzV_byScale[IDX_VIDEO_PAL][9] = 576;
	glSzH_byScale[IDX_VIDEO_PAL][10] = VIDEO_IN_H_SIZE /2;
	glSzV_byScale[IDX_VIDEO_PAL][10] = 576;

	m_idxCurDispMode = IDX_DISP_MODE_1;
	m_idxCurSelDisp = 0;
	UpdatePlayerWndPt (FALSE);
	m_idxCurZoom[IDX_ZOOM_HOR] = m_idxCurZoom[IDX_ZOOM_VER] = IDX_ZOOM_FIT;

	for (i=0; i<MAX_DISP_WND; i++)
	{
		m_bUpdateDispWndPt[i] = TRUE;
		m_bFillDispWndBlack[i] = TRUE;
	}
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		m_idxCurDispCh[i] = i;
	}
	for (; i<MAX_DISP_WND; i++)
	{
		m_idxCurDispCh[i] = 0;
	}

	m_bDeblocking = FALSE;
	m_bDeringing = FALSE;
	m_bDeinterlace = FALSE;
	m_bMotionInfo = FALSE;
	m_bChannelInfo = TRUE;
	m_bWMInfo = FALSE;
	m_bIBasedPred = FALSE;

	InitDeIntFilter (IDX_DEFLT_CS_YV12);	// Deinterlace Filter Init...

	// XviD Init[S]...
	xvid_gbl_init_t xvid_gbl_init;
	xvid_dec_create_t xvid_dec_create;

	xvid_gbl_init.version = XVID_VERSION;
	xvid_gbl_init.cpu_flags = 0;
	xvid_global (NULL, 0, &xvid_gbl_init, NULL);

	xvid_dec_create.version = XVID_VERSION;

	xvid_dec_create.width = 0;
	xvid_dec_create.height = 0;

	for (i=0; i<MAX_DISP_WND; i++)
	{
		if (xvid_decore(NULL, XVID_DEC_CREATE, &xvid_dec_create, NULL) < 0)
		{
			AfxMessageBox ("Decoder Initialization Failed");
		}

		m_hXvidDec[i] = xvid_dec_create.handle;
	}
	// XviD Init[E]...

	// Multimedia Timer Init[S]...
	TIMECAPS tc;
	if (timeGetDevCaps (&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
	{
		AfxMessageBox ("Multimedia Timer Init. Fail");
	}
	m_nPlayerTimerRes = min(max(tc.wPeriodMin, PLAYER_TIMER_RES), tc.wPeriodMax);
	timeBeginPeriod (m_nPlayerTimerRes);

	m_nPlayerTimerID = timeSetEvent (m_nPlayerTimerRes, m_nPlayerTimerRes,
		TIMER_PROC, (DWORD)this, TIME_PERIODIC);
	// Multimedia Timer Init[E]...

	// For Snapshot[S]...
	m_bReqSnapshot = FALSE;
	// For Snapshot[E]...

	// For G723 Play[S]
	m_curG723PlayCh = 0;
	m_numG723Frm = 0;
	m_posCurG723Frm = 0;

	pBufAudio = NULL;
	pLastBufAudio = NULL;
	glNumBufAudio = 0;

	m_soundOut.m_bAutoDelete = TRUE;
	m_soundOut.m_pOwner = this;
	m_soundOut.SetBitsPerSample (16);
	m_soundOut.SetNumberOfChannels (1);
	m_soundOut.SetSamplesPerSecond (8000);
	m_soundOut.SetBufferSize (8000 *10);
	m_soundOut.GetDataToSoundOut = GetDataToSoundOut;  // assign pointer to callback function
	// For G723 Play[E]
}


void CDlgPlayerBase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPlayerBase)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgPlayerBase, CDialog)
	ON_MESSAGE (WM_DOO_CTRL_DLG_CHANGE, OnCtrlDlgChange)
	ON_MESSAGE (WM_DOO_DISP_WND_L_CLICK, OnDispWndLButClick)
	ON_MESSAGE (WM_DOO_DISP_WND_L_DBLCLK, OnDispWndLButDblClk)
	ON_MESSAGE (WM_DOO_DISP_WND_MOUSE_DRAG, OnDispWndMouseDrag)
	//{{AFX_MSG_MAP(CDlgPlayerBase)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayerBase message handlers

BOOL CDlgPlayerBase::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CDlgPlayerBase::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here

	SetWindowText (STR_APPLICATION_TITLE);
	
	int hRes = GetSystemMetrics (SM_CXSCREEN);
	int vRes = GetSystemMetrics (SM_CYSCREEN);
	int szFrameX = GetSystemMetrics (SM_CXDLGFRAME);
	int szFrameY = GetSystemMetrics (SM_CYDLGFRAME);

	if (m_idxCurVideoMode == IDX_VIDEO_NTSC)
	{
		m_szDispWnd.cx = SZ_INIT_DISP_WND_NTSC_H;
		m_szDispWnd.cy = SZ_INIT_DISP_WND_NTSC_V;
	}
	else	// IDX_VIDEO_PAL
	{
		m_szDispWnd.cx = SZ_INIT_DISP_WND_PAL_H;
		m_szDispWnd.cy = SZ_INIT_DISP_WND_PAL_V;
	}

	SetWindowPos (NULL,
		(hRes -(m_szDispWnd.cx +szFrameX *2)) /2,
		0,
		m_szDispWnd.cx +szFrameX *2,
		m_szDispWnd.cy +szFrameY *2,
		SWP_SHOWWINDOW |SWP_NOZORDER);

	CRect rcDispWnd;
	rcDispWnd.SetRect (POS_DISP_WND_X,
		POS_DISP_WND_Y,
		POS_DISP_WND_X +m_szDispWnd.cx,
		POS_DISP_WND_Y +m_szDispWnd.cy);

	m_wndS6010Disp.Create (NULL, NULL, WS_CHILD |WS_VISIBLE,
		rcDispWnd,
		this, ID_S6010_DISP_WND);
	m_wndS6010Disp.InitDDraw (m_szDispWnd.cx, m_szDispWnd.cy);

	glDlgMC.Create (IDD_DLG_MAIN_CONTROL, this);
	glDlgMC.ShowWindow (SW_SHOW);

	glDlgG723PC.Create (IDD_DLG_G723_PLAY_CTRL, this);
	glDlgG723PC.ShowWindow (SW_HIDE);

	glPDlgPB = this;

	StartMP4Player ();	// must call this function after "glPDlgPB = this;"...
	StartG723Player ();
	
	return 0;
}

void CDlgPlayerBase::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
	EndMP4Player ();
	EndG723Player ();

	// Multimedia Timer Destroy[S]...
	timeKillEvent (m_nPlayerTimerID);
	// Multimedia Timer Destroy[E]...

	CS6010StreamTesterApp *pApp = (CS6010StreamTesterApp *)AfxGetApp ();

	pApp->WriteProfileInt ("SETTINGS", "VIDEO_MODE", m_idxCurVideoMode);
}

UINT CDlgPlayerBase::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
//	return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}

void CDlgPlayerBase::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

void CDlgPlayerBase::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	if (glPWndDisp != NULL)
	{
		CRect rcClient;

		GetClientRect (&rcClient);

		m_szDispWnd.cx = rcClient.Width ();
		m_szDispWnd.cy = rcClient.Height ();
		
		FillDispWndBlack ();

		UpdateDisplay ();
		
		glPWndDisp->MoveWindow (&rcClient);
	}
}

void CDlgPlayerBase::StartMP4Player ()
{
	int i;
	for (i=0; i<MAX_DISP_WND; i++)
	{
		ChangeCurPlayerMode (i, IDX_PLAYER_REQ_INIT);
	}
	
	FillDispWndBlack ();
	UpdatePlayerWndPt (TRUE);
	
	m_bEndThread[IDX_TH_MP4_PLAYER] = FALSE;
	AfxBeginThread (ThFn_MultiPlayer, this);
}

void CDlgPlayerBase::EndMP4Player ()
{
	m_bEndThread[IDX_TH_MP4_PLAYER] = TRUE;
	m_bufEvent[EVT_MP4_PLAYER].PulseEvent ();
	while (m_bEndThread[IDX_TH_MP4_PLAYER] == TRUE)
	{
		Sleep (10);
	}
}

void CDlgPlayerBase::StartG723Player ()
{
	FillDispWndBlack ();

	m_bEndThread[IDX_TH_G723_PLAYER] = FALSE;
	AfxBeginThread (ThFn_G723Player, this);
}

void CDlgPlayerBase::EndG723Player ()
{
	m_bEndThread[IDX_TH_G723_PLAYER] = TRUE;
	while (m_bEndThread[IDX_TH_G723_PLAYER] == TRUE)
	{
		Sleep (10);
	}
}

void CDlgPlayerBase::ChangeCurPlayerMode (int idxMode)
{
	m_crtPlayer.Lock ();
	m_idxCurPlayerMode[m_idxCurSelDisp] = idxMode;
	m_crtPlayer.Unlock ();
}

void CDlgPlayerBase::ChangeCurPlayerMode (int idxDisp, int idxMode)
{
	m_crtPlayer.Lock ();
	m_idxCurPlayerMode[idxDisp] = idxMode;
	m_crtPlayer.Unlock ();
}

void CDlgPlayerBase::UpdatePlayerWndPt (BOOL bUpdate)
{
	int i;
	for (i=0; i<MAX_DISP_WND; i++)
	{
		m_bUpdateDispWndPt[i] = bUpdate;
	}
}
	
void CDlgPlayerBase::FillDispWndBlack ()
{
	int i;
	for (i=0; i<MAX_DISP_WND; i++)
	{
		m_bFillDispWndBlack[i] = TRUE;
	}
}
	
void CDlgPlayerBase::ChangeChannel (int idxCh)
{
	if (m_idxCurDispCh[m_idxCurSelDisp] == idxCh)
	{
		return;
	}

	m_idxCurDispCh[m_idxCurSelDisp] = idxCh;
	
	EndMP4Player ();
	StartMP4Player ();
}

void CDlgPlayerBase::ChangeDispMode (int idxDispMode)
{
	if (m_idxCurDispMode == idxDispMode)
	{
		return;
	}

	EndMP4Player ();

	m_idxCurDispMode = idxDispMode;
	m_idxCurSelDisp = 0;

	int i;
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		m_idxCurDispCh[i] = i;
	}
	for (; i<MAX_DISP_WND; i++)
	{
		m_idxCurDispCh[i] = 0;
	}

	StartMP4Player ();
}

void CDlgPlayerBase::MakeAVIFile (CString strAVIFilePath, int idxCh)
{
	EndMP4Player ();

	CFile fpSrc;
	if (fpSrc.Open (m_strStreamFilePath, CFile::modeRead) == FALSE)
	{
		AfxMessageBox ("ERROR: Cannot open SMP4 file");
		return;
	}

	CDooMPEG4ToAVI clMPEG4ToAVI;
	DWORD szH, szV, typeVideo, idxScale, bInterlace, idxFrm;
	BYTE *buf1Frm = new BYTE[2 *1024 *1024];

	typeVideo = m_infoStreamIdx.GetVideoType (idxCh);
	idxScale = m_infoStreamIdx.GetScale (idxCh, 0);
	bInterlace = m_infoStreamIdx.IsInterlace (idxCh);
	szH = glSzH_byScale[typeVideo][idxScale];
	szV = glSzV_byScale[typeVideo][idxScale];
	MakeMP4SeqHdr (szH, szV, bInterlace);

	idxFrm = 0;
	memcpy (buf1Frm, glBufStrh, sizeof(glBufStrh));	// Sequence header copy to frame buffer

	fpSrc.Seek (m_infoStreamIdx.GetFramePos (idxCh, idxFrm), CFile::begin);
	fpSrc.Read (&buf1Frm[sizeof(glBufStrh)], m_infoStreamIdx.GetFrameSize (idxCh, idxFrm));
	clMPEG4ToAVI.StartConvert (szH, szV, buf1Frm,
		sizeof(glBufStrh) +m_infoStreamIdx.GetFrameSize (idxCh, idxFrm));

	idxFrm++;
	while (1)
	{
		fpSrc.Seek (m_infoStreamIdx.GetFramePos (idxCh, idxFrm), CFile::begin);
		fpSrc.Read (buf1Frm, m_infoStreamIdx.GetFrameSize (idxCh, idxFrm));
		clMPEG4ToAVI.AddFrame (buf1Frm, m_infoStreamIdx.GetFrameSize (idxCh, idxFrm));

		idxFrm++;

		if (idxFrm >= m_infoStreamIdx.GetMaxFrameNum (idxCh))
		{
			break;
		}
	}
	fpSrc.Close ();
	delete[] buf1Frm;

	clMPEG4ToAVI.EndConvertMSPF (strAVIFilePath, m_infoStreamIdx.GetMSPF (idxCh));

	StartMP4Player ();
	
	AfxMessageBox ("AVI file successfully have made.");
}

void CDlgPlayerBase::MakeRAWStream (CString strFilePath, int idxCh, BOOL bInsertVOLH)
{
	EndMP4Player ();

	CFile fpSrc, fpDest;
	if (fpSrc.Open (m_strStreamFilePath, CFile::modeRead) == FALSE)
	{
		AfxMessageBox ("ERROR: Cannot open SMP4 file");
		return;
	}
	if (fpDest.Open (strFilePath, CFile::modeCreate |CFile::modeWrite) == FALSE)
	{
		AfxMessageBox ("ERROR: Cannot open RAW stream file");
		fpSrc.Close ();
		return;
	}

	DWORD szH, szV, typeVideo, idxScale, bInterlace, idxFrm;
	BYTE *buf1Frm = new BYTE[2 *1024 *1024];

	typeVideo = m_infoStreamIdx.GetVideoType (idxCh);
	idxScale = m_infoStreamIdx.GetScale (idxCh, 0);
	bInterlace = m_infoStreamIdx.IsInterlace (idxCh);
	szH = glSzH_byScale[typeVideo][idxScale];
	szV = glSzV_byScale[typeVideo][idxScale];

	if (bInsertVOLH == TRUE)
	{
		MakeMP4SeqHdr (szH, szV, bInterlace);
		fpDest.Write (glBufStrh, sizeof(glBufStrh));
	}

	idxFrm = 0;
	while (1)
	{
		fpSrc.Seek (m_infoStreamIdx.GetFramePos (idxCh, idxFrm), CFile::begin);
		fpSrc.Read (buf1Frm, m_infoStreamIdx.GetFrameSize (idxCh, idxFrm));
		fpDest.Write (buf1Frm, m_infoStreamIdx.GetFrameSize (idxCh, idxFrm));

		idxFrm++;

		if (idxFrm >= m_infoStreamIdx.GetMaxFrameNum (idxCh))
		{
			break;
		}
	}
	fpSrc.Close ();
	fpDest.Close ();
	delete[] buf1Frm;

	StartMP4Player ();
	
	AfxMessageBox ("RAW MPEG4 file successfully have made.");
}

LRESULT CDlgPlayerBase::OnDispWndLButClick (WPARAM wParam, LPARAM lParam)
{
	m_idxCurSelDisp = wParam;
	m_wndS6010Disp.Invalidate (FALSE);

	glPDlgMC->ChangeChannelButState (m_idxCurDispCh[m_idxCurSelDisp]);

	return 0;
}

LRESULT CDlgPlayerBase::OnDispWndLButDblClk (WPARAM wParam, LPARAM lParam)
{
	glPDlgMC->ShowWindow (SW_SHOW);

	return 0;
}

LRESULT CDlgPlayerBase::OnDispWndMouseDrag (WPARAM wParam, LPARAM lParam)
{
	CRect rcWnd, rcClient;
	GetWindowRect (&rcWnd);
	GetClientRect (&rcClient);

	rcWnd.OffsetRect (wParam, lParam);

	MoveWindow (rcWnd);

	rcClient.OffsetRect (wParam, lParam);

	FillDispWndBlack ();
	UpdateDisplay ();
	glPWndDisp->MoveWindow (&rcClient);

	return 0;
}

void CDlgPlayerBase::ChangeZoom (int idxHV, int idxZoom)
{
	m_idxCurZoom[idxHV] = idxZoom;
	
	glPWndDisp->FillBackBlack ();
	UpdateDisplay ();
}

void CDlgPlayerBase::ChangeDispSize (int idxDispSz)
{
	int hRes = GetSystemMetrics (SM_CXSCREEN);
	int vRes = GetSystemMetrics (SM_CYSCREEN);
	int szFrameX = GetSystemMetrics (SM_CXDLGFRAME);
	int szFrameY = GetSystemMetrics (SM_CYDLGFRAME);

	if (m_idxCurVideoMode == IDX_VIDEO_NTSC)
	{
		m_szDispWnd.cx = SZ_INIT_DISP_WND_NTSC_H;
		m_szDispWnd.cy = SZ_INIT_DISP_WND_NTSC_V;
	}
	else	// IDX_VIDEO_PAL
	{
		m_szDispWnd.cx = SZ_INIT_DISP_WND_PAL_H;
		m_szDispWnd.cy = SZ_INIT_DISP_WND_PAL_V;
	}

	switch (idxDispSz)
	{
	case IDX_DISP_SZ_X2:
		m_szDispWnd.cx *= 2;
		m_szDispWnd.cy *= 2;
		break;
	case IDX_DISP_SZ_X3:
		m_szDispWnd.cx *= 3;
		m_szDispWnd.cy *= 3;
		break;
	}

	if (m_szDispWnd.cx > hRes -(2 *szFrameX))
	{
		m_szDispWnd.cx = hRes -(2 *szFrameX);
	}
	if (m_szDispWnd.cy > vRes -(2 *szFrameY))
	{
		m_szDispWnd.cy = vRes -(2 *szFrameY);
	}

	if (idxDispSz == IDX_DISP_SZ_FULL)
	{
		SetWindowPos (NULL, -szFrameX, -szFrameY,
			hRes +(2 *szFrameX), vRes +(2 *szFrameY),
			SWP_SHOWWINDOW |SWP_NOZORDER);
	}
	else
	{
		SetWindowPos (NULL,
			(hRes -(m_szDispWnd.cx +szFrameX *2)) /2,
			0,
			m_szDispWnd.cx +szFrameX *2,
			m_szDispWnd.cy +szFrameY *2,
			SWP_SHOWWINDOW |SWP_NOZORDER);
	}
	
	glPWndDisp->FillBackBlack ();
	UpdateDisplay ();
}

void CDlgPlayerBase::TakeSnapshot (CString strSnapshotFilePath)
{
	m_strSnapshotFilePath = strSnapshotFilePath;
	m_bReqSnapshot = TRUE;
}

BOOL CDlgPlayerBase::OpenStreamFile (CString strStreamFilePath)
{
	m_strStreamFilePath = strStreamFilePath;
	
	if (m_infoStreamIdx.MakeIndex (m_strStreamFilePath) == TRUE)
	{
		EndMP4Player ();
		StartMP4Player ();
	}
	else
	{
		m_bFileOpen = FALSE;
		return FALSE;
	}

	m_bFileOpen = TRUE;
	return TRUE;
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDlgPlayerBase::OnQueryDragIcon() 
{
	return (HCURSOR) m_hIcon;
}

void CDlgPlayerBase::S6010StreamPlayerQuit ()
{
	OnOK ();
}

void CDlgPlayerBase::UpdateDisplay ()
{
	UpdatePlayerWndPt (TRUE);

	int i;
	for (i=0; i<GL_DISP_MODE_TO_NUM_DISP[m_idxCurDispMode]; i++)
	{
		if (m_idxCurPlayerMode[i] == IDX_PLAYER_PAUSE)
		{
			m_idxCurPlayerMode[i] = IDX_PLAYER_REQ_STOP;
		}
	}
}

void CDlgPlayerBase::OnKeyDownFromWndDisp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int i;

	switch (nChar)
	{
	case '1':
		glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_REQ_PLAY);
		break;
	case '2':
		glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_PAUSE);
		break;
	case '3':
		glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_REQ_STOP);
		break;
	case 'Q':
		for (i=0; i<GL_DISP_MODE_TO_NUM_DISP[m_idxCurDispMode]; i++)
		{
			glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_REQ_PLAY);
		}
		break;
	case 'W':
		for (i=0; i<GL_DISP_MODE_TO_NUM_DISP[m_idxCurDispMode]; i++)
		{
			glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_PAUSE);
		}
		break;
	case 'E':
		for (i=0; i<GL_DISP_MODE_TO_NUM_DISP[m_idxCurDispMode]; i++)
		{
			glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_REQ_STOP);
		}
		break;
	}
}

int CDlgPlayerBase::GetFrameNum (int idxDisp)
{
	return m_infoStreamIdx.GetMaxFrameNum (m_idxCurDispCh[idxDisp]);
}

void CDlgPlayerBase::SetFramePos (int idxDisp, int idxPos)
{
	m_posSetFrm[idxDisp] = idxPos;
	
	glPDlgPB->ChangeCurPlayerMode (idxDisp, IDX_PLAYER_SET_POS);
}

void CDlgPlayerBase::SetFramePosInPercent (int idxDisp, int idxPosPercent)
{
	m_posSetFrm[idxDisp] = idxPosPercent
		*(m_infoStreamIdx.GetMaxFrameNum (m_idxCurDispCh[idxDisp]) -1) /100;
	
	glPDlgPB->ChangeCurPlayerMode (idxDisp, IDX_PLAYER_SET_POS);
}

LRESULT CDlgPlayerBase::OnCtrlDlgChange (WPARAM wParam, LPARAM lParam)
{
	if (wParam == CTRL_DLG_MP4)
	{
		glDlgG723PC.ShowWindow (SW_HIDE);
		glDlgMC.ShowWindow (SW_SHOW);

		glDlgMC.SendMessage (WM_DOO_UPDATE_FILE_PATH, 0, lParam);
	}
	else if (wParam == CTRL_DLG_G723)
	{
		glDlgG723PC.ShowWindow (SW_SHOW);
		glDlgMC.ShowWindow (SW_HIDE);

		glDlgG723PC.SendMessage (WM_DOO_UPDATE_FILE_PATH, 0, lParam);
	}

	return 0;
}

BOOL CDlgPlayerBase::OpenG723File (CString strFilePath)
{
	
	EndG723Player ();

	m_strG723FilePath = strFilePath;

	CFile fp;
	if (fp.Open (m_strG723FilePath, CFile::modeRead) == TRUE)
	{
		fp.Read (&glHdrFileG723, sizeof(HEADER_RECORD_FILE));

		m_posCurG723Frm = 0;

		glSzG723BufLine = 0;
		int i;

		memset (glBufIdxLinePosFromCh, 0, sizeof(DWORD) *NUM_G723_CH);
		for (i=0; i<NUM_G723_CH; i++)
		{
			if (glHdrFileG723.G723.setRec.bufBEnc[i] == TRUE)
			{
				glBufIdxLinePosFromCh[i] = SZ_G723_FRM_HEADER +glSzG723BufLine *SZ_G723_1_FRM;

				glSzG723BufLine++;
			}
		}
		glSzG723BufLine *= SZ_G723_1_FRM;
		glSzG723BufLine += SZ_G723_FRM_HEADER;
		
		m_numG723Frm = glHdrFileG723.G723.numFrm;
		fp.Close ();

	}

	StartG723Player ();

	return TRUE;
}

void CDlgPlayerBase::ChangeCurG723PlayerMode (int idxMode)
{
	m_crtG723PlayerMode.Lock ();
	m_curG723PlayerMode = idxMode;
	m_crtG723PlayerMode.Unlock ();
}

void CDlgPlayerBase::ChangeCurG723PlayCh (int idxCh)
{
	m_curG723PlayCh = idxCh;
}

BOOL CDlgPlayerBase::WriteG723RAW (int idxCh, const char *strFilePath)
{
	if (m_strG723FilePath.IsEmpty () == TRUE)
	{
		return FALSE;
	}

	ChangeCurG723PlayerMode (G723_PLAYER_REQ_STOP);
	while (m_curG723PlayerMode == G723_PLAYER_REQ_STOP)
	{
		Sleep (0);
	}

	CFile fpSrc ,fpDest;
	if (fpSrc.Open (m_strG723FilePath, CFile::modeRead) == FALSE)
	{
		return FALSE;
	}
	if (fpDest.Open (strFilePath, CFile::modeCreate |CFile::modeWrite) == FALSE)
	{
		fpSrc.Close ();
		return FALSE;
	}

	int i;
	HEADER_RECORD_FILE hdrG723File;
	ITEM_AUD_REC_FILE_INDEX itemAudRecFileIndex;
	BYTE bufTmp[SZ_G723_1_ITEM +sizeof(HEADER_RECORD_FILE)];

	fpSrc.Read (&hdrG723File, sizeof(HEADER_RECORD_FILE));
	for (i=0; i<m_numG723Frm; i++)
	{
		fpSrc.Read (bufTmp, glSzG723BufLine);
		fpDest.Write (&bufTmp[glBufIdxLinePosFromCh[idxCh]], SZ_G723_SRC_1_FRM);
	}
	fpDest.Close ();

	// Write index[S]
	FILE *fp;
	fp = fopen ("C:\\G723RawIndex.txt", "wt");

	fpSrc.Seek (hdrG723File.G723.posIndex, CFile::begin);

	for (i=0; i<m_numG723Frm; i++)
	{
		fpSrc.Read (&itemAudRecFileIndex, sizeof(ITEM_AUD_REC_FILE_INDEX));

		fprintf (fp, "%8d [%10d:%06d]\n", itemAudRecFileIndex.posFrm, itemAudRecFileIndex.timeSec, itemAudRecFileIndex.timeUSec);
	}
	fclose (fp);
	// Write index[E]

	fpSrc.Close ();

	return TRUE;
}
