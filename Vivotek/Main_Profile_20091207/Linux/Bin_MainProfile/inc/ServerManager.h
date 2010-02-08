/*
 *******************************************************************************
 *
 *  Copyright (c) 2006-2010 Vivotek Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VIVOTEK INC.                                                    |
 *  +-----------------------------------------------------------------+
*/
/*!
 *******************************************************************************
 * Copyright 2006-2010 Vivotek, Inc. All rights reserved.
 *
 * \file
 * ServerManager.h
 *
 * \brief
 * This file defines the interface for ServerManager. 
 * \p 
 *
 *notice:
 * \n 1. We will create a TSVRMNGR_INIT_INFO ServerManager_Initial. It will store the ServerManager object infroamtion.
 * \n 2. There are two interface to access two different lower module.  We need to load library and wrap the function in initializaion.
 * \n		- ServerController part : LoadServerControllerLibrary -> WrapServerControllerFunctions
 * \n      - Server Utility part : LoadServerUtilityLibrary -> WrapServerUtilityFunctions
 * \n		If it is success to do it, we will set it's flag as true.
 * \n 3. We will create a TSVRMNGR_DEV_CREATE_INFO for every CreateDeice. It will store the SeverManager device object information.
 * \n
 * \n 4. When the user call ServerManager_OpenDevice or ServerManager_OpenDeviceBlock, the module will find out
 *     which inner interface (new or old , Server Controller or ServerUtl) the device should apply  to operate it.
 * \n  
 * \n 3. In TSVRMNGR_DEV_CREATE_INFO, We will fill the hDevice, phParent and pDVCFns base on  the interface has been choosen. After that, all  the function working is need to call the corresponding functin in pDVCFns.

 * \date
 * 2006/11/01
 *
 * \author
 * Bruce Wu
 *
 *
 *******************************************************************************
 */

#ifndef _SERVER_MANAGER_H_
#define	_SERVER_MANAGER_H_

#include "typedef.h"
#include "common.h"
#include "ControlCommon.h"

#define SERVERMANAGER_VERSION   MAKEFOURCC(3, 3, 0, 2)
//	The ServerManager Version information
#define SRVMNGR_VERSION_MAJOR		3
#define SRVMNGR_VERSION_MINOR		3
#define SRVMNGR_VERSION_BUILD		0
#define SRVMNGR_VERSION_REVISION	2

/** 
  * \brief The structure that is used to hold the property of target host and the priority interface to connect the server.
  */
typedef struct
{
	/// the property of target host. All field have to be filled except the http port. The server authentication could fail without user 
    ///  name and password.
	TSRVCTRL_DEV_PROPERTY tServerProperty;
	/// the boolean value first to use ServerUtl module or not. We will use ServerUtl module to connect first, if fail, we will automatically roll to 
	/// Server Manager to do it.
	///If the user does not know which module to use first, just set it false. It will use ServerManager to connect it fisrt.
	/// If fail, we will still automatically roll to ServerUtl to connect it.
	BOOL												bServerUtlPriority;
}
TSRVMNGR_DEV_PROPERTY;

#if defined _WIN32 || defined _WIN32_WCE
/** 
  * \brief The structure is wide character version of TSRVMNGR_DEV_PROPERTY.
  */
typedef struct
{
	/// the property of target host. All field have to be filled except the http port. The server authentication could fail without user 
    ///  name and password.
	TSRVCTRL_DEV_PROPERTYW tServerProperty;
	/// the boolean value first to use ServerUtl module or not. We will use ServerUtl module to connect first, if fail, we will automatically roll to 
	/// Server Manager to do it.
	///If the user does not know which module to use first, just set it false. It will use ServerManager to connect it fisrt.
	/// If fail, we will still automatically roll to ServerUtl to connect it.
	BOOL												bServerUtlPriority;
}
TSRVMNGR_DEV_PROPERTYW;

typedef TSRVCTRL_DEV_PROXYW TSRVMNGR_DEV_PROXYW;
#endif //_WIN32 || _WIN32_WCE

typedef TSRVCTRL_DEV_PROXY TSRVMNGR_DEV_PROXY;


#ifdef __cplusplus
extern "C" {
#endif

/**
  * \brief This function is used to initialize the ServerManager module.
  */
SCODE DEFEXPAPI ServerManager_Initial(HANDLE *phObject, DWORD dwMaxDevice, DWORD dwFlag, TVersionInfo *ptVersion);

/**
  * \brief This function is used to release the ServerManager module. All device handles must be released before
  * calling this function.
  */
SCODE DEFEXPAPI ServerManager_Release(HANDLE *phObject);

/**
  * \brief This function is used to retrieve the version information of the current module.
  */
SCODE DEFEXPAPI ServerManager_GetVersionInfo(TVersionInfo *ptVersion);

/**
  * \brief This function is used to create a ServerManager device object instance.
  */
SCODE DEFEXPAPI ServerManager_CreateDevice(HANDLE hObject, HANDLE *phDevice);

/**
  * \brief This function is used to delete a device. All pending or unfinished request will be removed when this
  * function is called. But it is recommend to call ServerManager_StopRequest before calling this function.
  */
SCODE DEFEXPAPI ServerManager_DeleteDevice(HANDLE *phDevice);

/**
  * \brief This function is used to stop all pending or unfinished request for a device handle. If you are calling
  * this before calling ServerManager_DeleteDevice, set the bBlocking flag to FALSE.
  */
SCODE DEFEXPAPI ServerManager_StopRequest(HANDLE hDevice, BOOL bBlocking);

/**
  * \brief This function is used to set new property associated with the device. When new property is set,
  * the ServerManager_StopRequest is called to stop all pending or unfinished request. Any operation should
  * be called after this function is called.
  */
SCODE DEFEXPAPI ServerManager_SetDeviceProperty(HANDLE hDevice, TSRVMNGR_DEV_PROPERTY *ptDevProperty);

/****** blocking version *******/
/**
  * \brief This function is used connect to server, and retrieve the server model and config data.
  */
SCODE DEFEXPAPI ServerManager_OpenDeviceBlock(HANDLE hDevice, char *pszModelName, DWORD dwMaxSize);

/**
  * \brief This function is used connect to server, and retrieve the server model. If want to retrieve config data, 
  * must use ServerManager_UpdateLocalConfig.
  */
SCODE DEFEXPAPI ServerManager_OpenDeviceLiteBlock(HANDLE hDevice, char *pszModelName, DWORD dwMaxSize);

/**
  * \brief This function is used to retrieve the server site configuration to local buffer. Any previous modified setting
  * will be overwritten.
  */
SCODE DEFEXPAPI ServerManager_UpdateLocalConfigBlock(HANDLE hDevice);

/**
  * \brief This function is used to update the server site configuration from local buffer.
  */
SCODE DEFEXPAPI ServerManager_UpdateRemoteConfigBlock(HANDLE hDevice);

/**
  * \brief This function is used to send server the PTZ command. 
  */
SCODE DEFEXPAPI ServerManager_SendPTZCommandBlock(HANDLE hDevice, DWORD dwCamera, ESRVCTRL_PTZ_COMMAND ePtzCommand,
													 const char *pszExtraCmd, BOOL bWaitRes);

/**
  * \brief This function is used to send server the camera movement command.
  */
// sam 2007/08/21 modify for supporting ServerController, ServerUtility do not concern dwStream, dwDisplaySizeX and dwDisplaySizeY
SCODE DEFEXPAPI ServerManager_MoveCameraBlock(HANDLE hDevice, DWORD dwCamera, DWORD dwStream, DWORD dwX, DWORD dwY, 
												 DWORD dwDisplayWidth, DWORD dwDisplayHeight, BOOL bWaitRes);
/**
  * \brief This function is used to send UART (COM port) command to server.
  */
SCODE DEFEXPAPI ServerManager_UartWriteBlock(HANDLE hDevice, DWORD dwPort, 
												const BYTE *pbyCommand, DWORD dwLen,
												BOOL bWaitRes);

/**
  * \brief This function is used to receive UART (COM port) command from server. 
  */
SCODE DEFEXPAPI ServerManager_UartReadBlock(HANDLE hDevice, DWORD dwPort, BYTE *pszRespone, 
											   DWORD *pdwReadLen, BOOL bFlush, DWORD dwWaitTime);

/**
  * \brief This function is used to receive UART (COM port) data after send a command to server. 
  */
SCODE DEFEXPAPI ServerManager_UartReadAFWriteBlock(HANDLE hDevice, DWORD dwPort, const BYTE *pbyCommand,		 
													DWORD dwLen, BYTE *pszRespone, DWORD *pdwReadLen, BOOL bFlush, DWORD dwWaitTime);

/**
  * \brief This function is used to get current DI information from server.
  */
SCODE DEFEXPAPI ServerManager_GetDIStatusBlock(HANDLE hDevice, ESrvCtrlDiDoLevel *peDIStatus,
											   DWORD *pdwMaxDINum);

/**
  * \brief This function is used to get current DO information from server.
  */
SCODE DEFEXPAPI ServerManager_GetDOStatusBlock(HANDLE hDevice, ESrvCtrlDiDoLevel *peDOStatus, DWORD *pdwMaxDONum);

/**
  * \brief This function is used to set current DO information into server. For DO that is not to be set,
  * put eSrvCtrlDiDoNone in the corresponding position of the array
  */
SCODE DEFEXPAPI ServerManager_SetDOStatusBlock(HANDLE hDevice, ESrvCtrlDiDoLevel *peDOStatus, DWORD dwMaxDOStatus,
												  BOOL bWaitRes);

/**
  * \brief This function is used to send a http command to server. For POST, the parameter is also
  * put after the question mark as GET does. The result of the commadn is ignored by this function.
  */
SCODE DEFEXPAPI ServerManager_SendHttpCommandBlock(HANDLE hDevice, const char *pszCommand, BOOL bPost);

/**
  * \brief This function is used to send a http command to server. For POST, the parameter is also
  * put after the question mark as GET does. The result is read into pbyData until reaches the buffer length
  * or connection is closed. If there are more data, special success code is returned and the phOper
  * could hold the operation handle. Use ServerManager_ReadDataBlock to read futher data.
  */
SCODE DEFEXPAPI ServerManager_SendHttpCommandReadBlock(HANDLE hDevice, const char *pszCommand, BOOL bPost,
														  BYTE *pbyData, DWORD *pdwBufLen, HANDLE *phOper);

/**
  * \brief This function is used to read the data from a http connection. User must ensure that the connection
  * is already connected. Or error will be returned. If the data is still not enough, the same success code is returned.
  * Use thsi function again to read futher data.
  */
SCODE DEFEXPAPI ServerManager_ReadDataBlock(HANDLE hDevice, HANDLE hOper, BYTE *pbyBuffer, DWORD *pdwBufLen);

/**
  * \brief This function is used to get the server's motion detection setting. The setting is retrieved from server
  * directly, not from the local cache.
  */
SCODE DEFEXPAPI ServerManager_GetMotionDetectionInfoBlock(HANDLE hDevice, DWORD dwCamera, 
															 TSRVCTRL_MODETECT_INFO *ptMDInfo);

/**
  * \brief This function is used to set the server's motion detection setting. The local cache for motion detection
  * setting is not changed by calling this function.
  */
SCODE DEFEXPAPI ServerManager_SetMotionDetectionInfoBlock(HANDLE hDevice, DWORD dwCamera, 
															 const TSRVCTRL_MODETECT_INFO *ptMDInfo,
															 BOOL bWaitRes);

/**
  * \brief This function is used to get the server's motion detection setting. The setting is retrieved from server
  * directly, not from the local cache.
  */
SCODE DEFEXPAPI ServerManager_GetPrivateMaskInfoBlock(HANDLE hDevice, DWORD dwCamera, 
														 TSRVCTRL_PRIVATEMASK_INFO *ptPMInfo);

/**
  * \brief This function is used to set the server's motion detection setting. The local cache for motion detection
  * setting is not changed by calling this function.
  */
SCODE DEFEXPAPI ServerManager_SetPrivateMaskInfoBlock(HANDLE hDevice, DWORD dwCamera, 
														 const TSRVCTRL_PRIVATEMASK_INFO *ptPMInfo,
														 BOOL bWaitRes);
/****** blocking version *******/

/****** non-blocking version *******/
/**
  * \brief This function is used to connect to server, and retrieve the server model. The function usually will return
  * soon. The result of the model name will be notified by callback.
  */
SCODE DEFEXPAPI ServerManager_OpenDevice(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify);

/**
  * \brief This function is used to retrieve the server site configuration to local buffer. Any previous modified setting
  * will be overwritten. The function usually will return soon. When done or failed, notification function will be called.
  */
SCODE DEFEXPAPI ServerManager_UpdateLocalConfig(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify);

/**
  * \brief This function is used to update the server site configuration from local buffer. 
  * The function usually will return soon. When done or failed, notification function will be called.
  */
SCODE DEFEXPAPI ServerManager_UpdateRemoteConfig(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify);

/**
  * \brief This function is used to send server the PTZ command. If ptNotify is null, the response will not
  * be notified.
  */
SCODE DEFEXPAPI ServerManager_SendPTZCommand(HANDLE hDevice, DWORD dwCamera, ESRVCTRL_PTZ_COMMAND ePtzCommand,
												const char *pszExtraCmd, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to send server the camera movement command. If ptNotify is null, the response will not
  * be notified.
  */
// sam 2007/08/21 modify for supporting ServerController, ServerUtility do not concern dwStream, dwDisplaySizeX and dwDisplaySizeY
SCODE DEFEXPAPI ServerManager_MoveCamera(HANDLE hDevice, DWORD dwCamera, DWORD dwStream, DWORD dwX, DWORD dwY,
											DWORD dwDisplayWidth, DWORD dwDisplayHeight, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to send UART (COM port) command to server. If ptNotify is null, the response will not
  * be notified.
  */
SCODE DEFEXPAPI ServerManager_UartWrite(HANDLE hDevice, DWORD dwPort, 
										   const BYTE *pbyCommand, DWORD dwLen,
										   TSVRCTRL_NOTIFY *ptNotify,
										   HANDLE *phProc);

/**
  * \brief This function is used to receive UART (COM port) command to server. 
  */
SCODE DEFEXPAPI ServerManager_UartRead(HANDLE hDevice, DWORD dwPort, DWORD dwReadLen, 
										  BOOL bFlush, DWORD dwWaitTime, 
										  TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to receive UART (COM port) after send write command to server. 
  */
SCODE DEFEXPAPI ServerManager_UartReadAFWrite(HANDLE hDevice, DWORD dwPort, const BYTE *pbyCommand, 
											  DWORD dwLen, DWORD dwReadLen,  BOOL bFlush, DWORD dwWaitTime, 
										     TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to get current DI information from server.
  */
SCODE DEFEXPAPI ServerManager_GetDIStatus(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to get current DO information from server.
  */
SCODE DEFEXPAPI ServerManager_GetDOStatus(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to set current DO information into server. For DO that is not to be set,
  * put eSrvCtrlDiDoNone in the corresponding position of the array
  */
SCODE DEFEXPAPI ServerManager_SetDOStatus(HANDLE hDevice, ESrvCtrlDiDoLevel *peDOStatus,
										  DWORD dwMaxDOStatus, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to send a http command to server. For POST, the parameter is also
  * put after the question mark as GET does. If ptNotify is NULL, the post result will not be notified. 
  * After notified. User could use ServerManager_ReadData to read the returned page, or he should call 
  */
SCODE DEFEXPAPI ServerManager_SendHttpCommand(HANDLE hDevice, const char *pszCommand, BOOL bPost,
											  TSVRCTRL_NOTIFY *ptNotify, HANDLE *phOper);

/**
  * \brief This function is used to read the data from a http connection. User must ensure that the connection
  * is already connected. Or error will be returned.
  */
SCODE DEFEXPAPI ServerManager_ReadData(HANDLE hDevice, HANDLE hOper, BYTE *pbyBuffer, DWORD *pdwBufLen,
										  TSVRCTRL_NOTIFY *ptNotify);

/**
  * \brief This function is used to send HTTP command and read the data from a http connection. sam 2007/05/23 add for ServerUtility
  */
SCODE DEFEXPAPI ServerManager_SendHttpCommandRead(HANDLE hDevice, const char *pszCommand, BOOL bPost, BYTE *pbyBuffer, DWORD *pdwBufLen,
											  TSVRCTRL_NOTIFY *ptNotify);
/**
  * \brief This function is used to get the server's motion detection setting. 
  */
SCODE DEFEXPAPI ServerManager_GetMotionDetectionInfo(HANDLE hDevice, DWORD dwCamera, 
														TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to set the server's motion detection setting. The local cache for motion detection
  * setting is not changed by calling this function.
  */
SCODE DEFEXPAPI ServerManager_SetMotionDetectionInfo(HANDLE hDevice, DWORD dwCamera, 
														const TSRVCTRL_MODETECT_INFO *ptMDInfo, 
														TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to get the server's private mask setting. The setting is retrieved from server
  * directly, not from the local cache.
  */
SCODE DEFEXPAPI ServerManager_GetPrivateMaskInfo(HANDLE hDevice, DWORD dwCamera, 
													TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to set the server's private mask setting. The local cache for private mask
  * setting is not changed by calling this function.
  */
SCODE DEFEXPAPI ServerManager_SetPrivateMaskInfo(HANDLE hDevice, DWORD dwCamera, 
													const TSRVCTRL_PRIVATEMASK_INFO *ptPMInfo, 
													TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);
/****** non-blocking version *******/

/**
  * \brief This function is used to return the operater to hDevice. 
  */
SCODE DEFEXPAPI ServerManager_FreeOperator(HANDLE hDevice, HANDLE hOper);

/**
  * \brief This function is used to abort the operation 
  */
SCODE DEFEXPAPI ServerManager_AbortProcess(HANDLE hDevice, HANDLE hProc);

/**
  * \brief This function is used to set the root key of configuration. 
  */
SCODE DEFEXPAPI ServerManager_SetRooKey(HANDLE hDevice, const char *pszKey);

/**
  * \brief This function is used get a value from local buffer. 
  * 
  */
SCODE DEFEXPAPI ServerManager_GetValueByName(HANDLE hDevice, const char *pszName, 
												char *pszValue, DWORD *pdwValueSize);

/**
  * \brief This function is used set a value into local buffer. 
  * 
  */
SCODE DEFEXPAPI ServerManager_SetValueByName(HANDLE hDevice, const char *pszName, 
												const char *pszValue, BOOL bMustExist);

/**
  * \brief This function is used retrieve the value entries under the specified name. 
  * 
  */
SCODE DEFEXPAPI ServerManager_GetNameList(HANDLE hDevice, const char *pszName, char *pszNameList, DWORD *pdwValueSize);

///**
//  * \brief This function is SSL version of ServerManager_SendHttpCommand. 
//  * 
//  */
//SCODE DEFEXPAPI ServerManager_SendHttpsCommand(HANDLE hDevice, const char *pszCommand, BOOL bPost,
//											  TSVRCTRL_NOTIFY *ptNotify, HANDLE *phOper);
//
///**
//  * \brief This function is wide cheracter version of ServerManager_SendHttpsCommand.  
//  * 
//  */
//SCODE DEFEXPAPI ServerManager_SendHttpsCommandW(HANDLE hDevice, const wchar_t *pszCommand, BOOL bPost,
//											  TSVRCTRL_NOTIFY *ptNotify, HANDLE *phOper);
//
///**
//  * \brief This function is SSL version of ServerManager_SendHttpCommandBlock. 
//  * 
//  */
//SCODE DEFEXPAPI ServerManager_SendHttpsCommandBlock(HANDLE hDevice, const char *pszCommand, BOOL bPost);
//
///**
//  * \brief This function is wide cheracter version of ServerManager_SendHttpsCommandBlock.  
//  * 
//  */
//SCODE DEFEXPAPI ServerManager_SendHttpsCommandBlockW(HANDLE hDevice, const wchar_t *pszCommand, BOOL bPost);
//
///**
//  * \brief This function is SSL version of ServerManager_SendHttpCommandReadBlock. 
//  * 
//  */
//SCODE DEFEXPAPI ServerManager_SendHttpsCommandReadBlock(HANDLE hDevice, const char *pszCommand, BOOL bPost,
//														  BYTE *pbyData, DWORD *pdwBufLen, HANDLE *phOper);
//
///**
//  * \brief This function is wide cheracter version of ServerManager_SendHttpsCommandReadBlock. 
//  * 
//  */
//SCODE DEFEXPAPI ServerManager_SendHttpsCommandReadBlockW(HANDLE hDevice, const wchar_t *pszCommand, BOOL bPost,
//														  BYTE *pbyData, DWORD *pdwBufLen, HANDLE *phOper);


// sam 2008/12/12
SCODE DEFEXPAPI ServerManager_SendJoystickPTCommandBlock(HANDLE hDevice, DWORD dwCamera, int nX, int nY, int nSpeed, BOOL bWaitRes);
SCODE DEFEXPAPI ServerManager_SendJoystickZoomCommandBlock(HANDLE hDevice, DWORD dwCamera, EJOYSTICK_ZOOMING_COMMAND eZoomType, int nSpeed, BOOL bWaitRes);
SCODE DEFEXPAPI ServerManager_SendJoystickPTCommand(HANDLE hDevice, DWORD dwCamera, int nX, int nY, int nSpeed, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);
SCODE DEFEXPAPI ServerManager_SendJoystickZoomCommand(HANDLE hDevice, DWORD dwCamera, EJOYSTICK_ZOOMING_COMMAND eZoomType, int nSpeed, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/// sam 2009/04/15
SCODE DEFEXPAPI ServerManager_SetPTZType(HANDLE hDevice, ESrvCtrlPTZType ePTZType);


#if defined _WIN32 || defined _WIN32_WCE
/**
  * \brief This function is used to set new property associated with the device which is used for wide-character version. 
  * When new property is set, the ServerManager_StopRequest is called to stop all pending or unfinished request. 
  * Any operation should be called after this function is called.
  */
SCODE DEFEXPAPI ServerManager_SetDevicePropertyW(HANDLE hDevice, TSRVMNGR_DEV_PROPERTYW *ptDevPropertyW);

/**
  * \brief This function is used connect to server, and retrieve the server model and config data.
  * This is wide-character version of ServerManager_OpenDeviceBlock.
  */
SCODE DEFEXPAPI ServerManager_OpenDeviceBlockW(HANDLE hDevice, wchar_t *pszModelName, DWORD dwMaxSize);

/**
  * \brief This function is used connect to server, and retrieve the server model. If want to retrieve config data, 
  * must use ServerManager_UpdateLocalConfig. This is wide-character version of ServerManager_OpenDeviceLiteBlock.
  */
SCODE DEFEXPAPI ServerManager_OpenDeviceLiteBlockW(HANDLE hDevice, wchar_t *pszModelName, DWORD dwMaxSize);

/**
  * \brief This function is used to send a http command to server. For POST, the parameter is also
  * put after the question mark as GET does. The result of the commadn is ignored by this function.
  * This is wide-character version of ServerManager_SendHttpCommandBlock.
  */
SCODE DEFEXPAPI ServerManager_SendHttpCommandBlockW(HANDLE hDevice, const wchar_t *pszCommand, BOOL bPost);

/**
  * \brief This function is used to send a http command to server. For POST, the parameter is also
  * put after the question mark as GET does. The result is read into pbyData until reaches the buffer length
  * or connection is closed. If there are more data, special success code is returned and the phOper
  * could hold the operation handle. Use ServerManager_ReadDataBlock to read futher data.
  * This is wide-character version of ServerManager_SendHttpCommandBlock.
  */
SCODE DEFEXPAPI ServerManager_SendHttpCommandReadBlockW(HANDLE hDevice, const wchar_t *pszCommand, BOOL bPost,
														  BYTE *pbyData, DWORD *pdwBufLen, HANDLE *phOper);

#endif //_WIN32 || _WIN32_WCE

#ifdef __cplusplus
}
#endif
#endif //_SERVER_MANAGER_H_

