#ifndef _VIDEO_PLAYER_H_
#define _VIDEO_PLAYER_H_

#define ID_PP_NONE                      32782
#define ID_PP_SLIGHT                    32783
#define ID_PP_MEDIUM                    32784
#define ID_PP_HEAVY                     32785
#define ID_PP_SUPERHEAVY                32786
#define ID_SNAPSHOT                     32787

int		playerCreate(CWnd *pParentWnd, int windowStyle = WS_POPUPWINDOW|WS_CAPTION|WS_THICKFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX);

void	playerDestroy(int hdlPlayer);

int		playerInit(int hdlPlayer, int VideoWidth, int VideoHeight, int preferredMode = DDRAW_YUY2);

void	playerClose(int hdlPlayer, BOOL bShowWindow = FALSE);

int		playerGetVideoMode(int hdlPlayer);

int		playerGetPitch(int hdlPlayer); 

BYTE	*playerBeginDraw(int hdlPlayer);

void	playerEndDraw(int hdlPlayer, BOOL draw = FALSE);

void	playerSetFullScreen(int hdlPlayer, BOOL bFullScreen);

void	playerZoom(int hdlPlayer, double zoomX, double zoomY);

HWND	playerGetWindow(int hdlPlayer);

void	playerSetWindowStyle(int hdlPlayer, int windowStyle);

void	playerEnableDefaultMenu(int hdlPlayer, BOOL bEnable = TRUE);

void	playerSetCallback(int hdlPlayer, int callback, int param);

#pragma comment (lib, "VideoWnd.lib")				  

#endif
