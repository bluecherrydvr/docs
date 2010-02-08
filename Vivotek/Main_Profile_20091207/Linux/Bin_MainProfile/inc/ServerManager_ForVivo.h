#ifndef _SERVER_MANAGER_FORVIVO_H_
#define	_SERVER_MANAGER_FORVIVO_H_
#include "ControlCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// hObject is create from ServerManager_Initial
/**
  * \brief This function is used for Installation Wizard 2 (not open to user). 
  * 
  */
SCODE DEFEXPAPI ServerManager_SendHttpCommandReadBlockEx(HANDLE hObject, TSRVCTRL_DEV_PROPERTY *ptDevProperty, const char *pszCommand, 
														  BOOL bPost, BYTE *pbyData, DWORD *pdwBufLen, DWORD dwReadTimeOut); // sam 2007/03/14

/**
  * \brief This function is used to initial ServerManager without initializing NetScheduler 
  * 
  */
SCODE DEFEXPAPI ServerManager_InitialEX(HANDLE *phObject, HANDLE hScheAgent, DWORD dwMaxDevice, DWORD dwFlag, TVersionInfo *ptVersion);

/**
  * \brief This function is used to release ServerManager without release NetScheduler
  * 
  */
SCODE DEFEXPAPI ServerManager_ReleaseEX(HANDLE *phObject);

/**
  * \brief This function is used to set device property. 
  * 
  */
SCODE DEFEXPAPI ServerManager_SetDevicePropertyEX(HANDLE hDevice, TSRVMNGR_DEV_PROPERTY *ptDevProperty, const char *pszGetParamName);

/**
  * \brief This function is used to open device. It will try connection with dwConnTimeOut, and if it can connect to server it will realy to do open device.
  * 
  */
SCODE DEFEXPAPI ServerManager_OpenDeviceEX(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify, DWORD dwConnTimeOut);

/**
  * \brief This function is used to send server the camera movement command.
  */
SCODE DEFEXPAPI ServerManager_MoveCameraBlockEX(HANDLE hDevice, DWORD dwCamera, DWORD dwX, DWORD dwY, DWORD dwResolutionWidth, DWORD dwResolutionHeight,
												 DWORD dwDisplayWidth, DWORD dwDisplayHeight, BOOL bWaitRes);

/**
  * \brief This function is used to send server the camera movement command. If ptNotify is null, the response will not
  * be notified.
  */
SCODE DEFEXPAPI ServerManager_MoveCameraEX(HANDLE hDevice, DWORD dwCamera, DWORD dwX, DWORD dwY, DWORD dwResolutionWidth, DWORD dwResolutionHeight,
											DWORD dwDisplayWidth, DWORD dwDisplayHeight, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);


SCODE DEFEXPAPI ServerManager_SetProxySetting(HANDLE hDevice, TSRVMNGR_DEV_PROXY *ptProxyOpt);

/**
  * \brief This function is used to reboot server. 
  * 
  */
SCODE DEFEXPAPI ServerManager_RebootBlock(HANDLE hDevice);

/**
  * \brief This function is used to set root's password. 
  * 
  */
SCODE DEFEXPAPI ServerManager_SetSystemPasswdBlock(HANDLE hDevice, const char *pszPasswd);

/**
  * \brief This function is used to restore server. 
  * 
  */
SCODE DEFEXPAPI ServerManager_RestoreServerBlock(HANDLE hDevice);

/**
  * \brief This function is used set system time. 
  * 
  */
SCODE DEFEXPAPI ServerManager_SetSystemDateTimeBlock(HANDLE hDevice, const char *pszDate, const char *pszTime);

/**
  * \brief This function is used get system time. 
  * 
  */
SCODE DEFEXPAPI ServerManager_GetSystemDateTimeBlock(HANDLE hDevice, char *pszDate, char *pszTime);


/**
  * \brief This function is used to check the server is new model or not. 
  * 
  */
SCODE DEFEXPAPI ServerManager_IsNewModel(HANDLE hDevice, BOOL *pbRet);


#if defined _WIN32 || defined _WIN32_WCE
/**
  * \brief This function is used to set device property. 
  * 
  */
SCODE DEFEXPAPI ServerManager_SetDevicePropertyEXW(HANDLE hDevice, TSRVMNGR_DEV_PROPERTYW *ptDevPropertyW, const wchar_t *pszGetParamNameW);

SCODE DEFEXPAPI ServerManager_SetProxySettingW(HANDLE hDevice, TSRVMNGR_DEV_PROXYW *ptProxyOptW);

#endif //_WIN32 || _WIN32_WCE

#ifdef __cplusplus
}
#endif

#endif 

