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
 * ServerController.h
 *
 * \brief
 * This file defines the interface for server controller. The server control is the brand new server
 * configuration management module that could be used to control the latest 7000 models of Vivotek
 * product. It does not support the old fasion configuration management of Vivotek product. With the
 * new architecture, the module is light weight and could be reduce system overhead.
 *
 * \date
 * 2006/08/30
 *
 * \author
 * Perkins Chen
 *
 *
 *******************************************************************************
 */

#ifndef _SERVER_CONTROLLER_H_
#define	_SERVER_CONTROLLER_H_

#include "typedef.h"
#include "common.h"
#include "ControlCommon.h"


#ifndef MAX_PATH
#define	MAX_PATH		260
#endif // MAX_PATH

#define MODULE1_VERSION   MAKEFOURCC(2, 3, 0, 3)

///	The ServerController Version information
#define SRVCTRL_VERSION_MAJOR		2
#define SRVCTRL_VERSION_MINOR		3
#define SRVCTRL_VERSION_BUILD		0
#define SRVCTRL_VERSION_REVISION	3

#define SRVCTRL_MAX_TOKEN_NUM		10
#define SRVCTRL_MAX_VALUE_LEN		1024
#define SRVCTRL_MAX_ENCODE_VALUE_LEN 2048
#define SRVCTRL_MAX_MODELNAME_LEN	32
#define SRVCTRL_MAX_ROOTNAME_LEN	128
#define SRVCTRL_MAX_CMD_LEN			4096
#define SRVCTRL_MAX_MD_NUM			3
#define SRVCTRL_MAX_PM_NUM			5
#define SRVCTRL_MAX_CAM_NUM			4
#define SRVCTRL_MAX_PORT_NUM		2
#define SRVCTRL_MAX_IP_ADDRESS		40

#define	SRVCTRL_MAX_DI_NUM			8
#define	SRVCTRL_MAX_DO_NUM			4

/**
  * \brief The enumeration to define the usage flag for server controller. If the server supports individual
  * channel for controling each operation below, the device will try to establish when device is opened.
  * If the channel opens failed, URL will be used to communicate with ip camera. This mode would be slower, but
  * it still works. The open device will take longer if you turn on these channels. 
  */
typedef enum
{
	/// The user will use PTZ operation
	eSrvCtrlUsagePTZ = 1,

	/// The user will use UART operation
	eSrvCtrlUsageUART = (1 << 1),

	/// The user will use DI/DO operation
	eSrvCtrlUsageDIDO = (1 << 2),

	/// All defined operations are included
	eSrvCtrlUsageAll = (eSrvCtrlUsagePTZ | eSrvCtrlUsageUART | eSrvCtrlUsageDIDO),
} ESrvCtrlUsage;


#ifdef __cplusplus
extern "C" {
#endif


/**
  * \brief This function is used to initialize the server controller module.
  */
SCODE DEFEXPAPI ServerController_Initial(HANDLE *phObject, HANDLE hScheAgent, DWORD dwMaxDevice, DWORD dwFlag, TVersionInfo *ptVersion);

/**
  * \brief This function is used to release the server controller module. All device handles must be released before
  * calling this function.
  */
SCODE DEFEXPAPI ServerController_Release(HANDLE *phObject);

/**
  * \brief This function is used to retrieve the version information of the current module.
  */
SCODE DEFEXPAPI ServerController_GetVersionInfo(TVersionInfo *ptVersion);

/**
  * \brief This function is used to create a device.
  */
SCODE DEFEXPAPI ServerController_CreateDevice(HANDLE hObject, HANDLE *phDevice);

/**
  * \brief This function is used to delete a device. All pending or unfinished request will be removed when this
  * function is called. But it is recommend to call ServerController_StopRequest before calling this function.
  */
SCODE DEFEXPAPI ServerController_DeleteDevice(HANDLE *phDevice);

/**
  * \brief This function is used to stop all pending or unfinished request for a device handle. If you are calling
  * this before calling ServerController_DeleteDevice, set the bBlocking flag to FALSE.
  */
SCODE DEFEXPAPI ServerController_StopRequest(HANDLE hDevice, BOOL bBlocking);

/**
  * \brief This function is used to set new property associated with the device. When new property is set,
  * the ServerController_StopRequest is called to stop all pending or unfinished request. Any operation should
  * be called after this function is called.
  */
SCODE DEFEXPAPI ServerController_SetDeviceProperty(HANDLE hDevice, TSRVCTRL_DEV_PROPERTY *ptDevProperty);

/**
  * \brief This function is used to set new property associated with the device. When new property is set,
  * the ServerController_StopRequest is called to stop all pending or unfinished request. Any operation should
  * be called after this function is called.
  */
SCODE DEFEXPAPI ServerController_SetDevicePropertyEX(HANDLE hDevice, TSRVCTRL_DEV_PROPERTY *ptDevProperty, const char *pszGetParamName);

/****** blocking version *******/
/**
  * \brief This function is used connect to server, and retrieve the server model.
  */
SCODE DEFEXPAPI ServerController_OpenDeviceBlock(HANDLE hDevice, char *pszModelName, DWORD dwMaxSize);

/**
  * \brief This function is used connect to server, and retrieve the server model.
  */
SCODE DEFEXPAPI ServerController_OpenDeviceLiteBlock(HANDLE hDevice, char *pszModelName, DWORD dwMaxSize);

/**
  * \brief This function is used to retrieve the server site configuration to local buffer. Any previous modified setting
  * will be overwritten.
  */
SCODE DEFEXPAPI ServerController_UpdateLocalConfigBlock(HANDLE hDevice);

/**
  * \brief This function is used to update the server site configuration from local buffer.
  */
SCODE DEFEXPAPI ServerController_UpdateRemoteConfigBlock(HANDLE hDevice);

/**
  * \brief This function is used to send server the PTZ command. 
  */
SCODE DEFEXPAPI ServerController_SendPTZCommandBlock(HANDLE hDevice, DWORD dwChannel, DWORD dwCamera, ESRVCTRL_PTZ_COMMAND ePtzCommand,
													 const char *pszExtraCmd, BOOL bWaitRes);

/**
  * \brief This function is used to send server the camera movement command.
  */
SCODE DEFEXPAPI ServerController_MoveCameraBlock(HANDLE hDevice, DWORD dwChannel, DWORD dwStream, DWORD dwX, DWORD dwY, 
												 DWORD dwDisplaySizeX, DWORD dwDisplaySizeY, BOOL bWaitRes);

/**
  * \brief This function is used to send server the camera movement command.
  */
SCODE DEFEXPAPI ServerController_MoveCameraBlockEX(HANDLE hDevice, DWORD dwChannel, DWORD dwX, DWORD dwY, DWORD dwResolutionWidth, DWORD dwResolutionHeight, 
												 DWORD dwDisplaySizeX, DWORD dwDisplaySizeY, BOOL bWaitRes);

/**
  * \brief This function is used to send UART (COM port) command to server.
  */
SCODE DEFEXPAPI ServerController_UartWriteBlock(HANDLE hDevice, DWORD dwPort, 
												const BYTE *pbyCommand, DWORD dwLen,
												BOOL bWaitRes);

/**
  * \brief This function is used to receive UART (COM port) command to server. 
  */
SCODE DEFEXPAPI ServerController_UartReadBlock(HANDLE hDevice, DWORD dwPort, BYTE *pszRespone, 
											   DWORD *pdwReadLen, BOOL bFlush, DWORD dwWaitTime);

/**
  * \brief This function is used to get current DI information from server.
  */
SCODE DEFEXPAPI ServerController_GetDIStatusBlock(HANDLE hDevice, ESrvCtrlDiDoLevel *peDIStatus,
											   DWORD *pdwMaxDINum);

/**
  * \brief This function is used to get current DO information from server.
  */
SCODE DEFEXPAPI ServerController_GetDOStatusBlock(HANDLE hDevice, ESrvCtrlDiDoLevel *peDOStatus, DWORD *pdwMaxDONum);

/**
  * \brief This function is used to set current DO information into server. For DO that is not to be set,
  * put eSrvCtrlDiDoNone in the corresponding position of the array
  */
SCODE DEFEXPAPI ServerController_SetDOStatusBlock(HANDLE hDevice, ESrvCtrlDiDoLevel *peDOStatus, DWORD dwMaxDOStatus,
												  BOOL bWaitRes);

/**
  * \brief This function is used to send a http command to server. For POST, the parameter is also
  * put after the question mark as GET does. The result of the commadn is ignored by this function.
  */
SCODE DEFEXPAPI ServerController_SendHttpCommandBlock(HANDLE hDevice, const char *pszCommand, BOOL bPost);

/**
  * \brief This function is used to send a http command to server. For POST, the parameter is also
  * put after the question mark as GET does. The result is read into pbyData until reaches the buffer length
  * or connection is closed. If there are more data, special success code is returned and the phOper
  * could hold the operation handle. Use ServerController_ReadDataBlock to read futher data
  */
SCODE DEFEXPAPI ServerController_SendHttpCommandReadBlock(HANDLE hDevice, const char *pszCommand, BOOL bPost,
														  BYTE *pbyData, DWORD *pdwBufLen, HANDLE *phOper);

/**
  * \brief This function is used to read the data from a http connection. User must ensure that the connection
  * is already connected. Or error will be returned. If the data is still not enough, the same success code is returned.
  * Use thsi function again to read futher data.
  */
SCODE DEFEXPAPI ServerController_ReadDataBlock(HANDLE hOper, BYTE *pbyBuffer, DWORD *pdwBufLen);

/**
  * \brief This function is used to get the server's motion detection setting. The setting is retrieved from server
  * directly, not from the local cache.
  */
SCODE DEFEXPAPI ServerController_GetMotionDetectionInfoBlock(HANDLE hDevice, DWORD dwCamera, 
															 TSRVCTRL_MODETECT_INFO *ptMDInfo);

/**
  * \brief This function is used to set the server's motion detection setting. The local cache for motion detection
  * setting is not changed by calling this function.
  */
SCODE DEFEXPAPI ServerController_SetMotionDetectionInfoBlock(HANDLE hDevice, DWORD dwCamera, 
															 const TSRVCTRL_MODETECT_INFO *ptMDInfo,
															 BOOL bWaitRes);

/**
  * \brief This function is used to get the server's motion detection setting. The setting is retrieved from server
  * directly, not from the local cache.
  */
SCODE DEFEXPAPI ServerController_GetPrivateMaskInfoBlock(HANDLE hDevice, DWORD dwCamera, 
														 TSRVCTRL_PRIVATEMASK_INFO *ptPMInfo);

/**
  * \brief This function is used to set the server's motion detection setting. The local cache for motion detection
  * setting is not changed by calling this function.
  */
SCODE DEFEXPAPI ServerController_SetPrivateMaskInfoBlock(HANDLE hDevice, DWORD dwCamera, 
														 const TSRVCTRL_PRIVATEMASK_INFO *ptPMInfo,
														 BOOL bWaitRes);
/****** blocking version *******/

/****** non-blocking version *******/
/**
  * \brief This function is used to connect to server, and retrieve the server model. The function usually will return
  * soon. The result of the model name will be notified by callback.
  */
SCODE DEFEXPAPI ServerController_OpenDevice(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify);

/**
  * \brief This function is used to retrieve the server site configuration to local buffer. Any previous modified setting
  * will be overwritten. The function usually will return soon. When done or failed, notification function will be called.
  */
SCODE DEFEXPAPI ServerController_UpdateLocalConfig(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify);

/**
  * \brief This function is used to update the server site configuration from local buffer. 
  * The function usually will return soon. When done or failed, notification function will be called.
  */
SCODE DEFEXPAPI ServerController_UpdateRemoteConfig(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify);

/**
  * \brief This function is used to send server the PTZ command. If ptNotify is null, the response will not
  * be notified.
  */
SCODE DEFEXPAPI ServerController_SendPTZCommand(HANDLE hDevice, DWORD dwChannel, DWORD dwCamera, ESRVCTRL_PTZ_COMMAND ePtzCommand,
												const char *pszExtraCmd, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to send server the camera movement command. If ptNotify is null, the response will not
  * be notified.
  */
SCODE DEFEXPAPI ServerController_MoveCamera(HANDLE hDevice, DWORD dwChannel, DWORD dwStream, DWORD dwX, DWORD dwY,
											DWORD dwDisplaySizeX, DWORD dwDisplaySizeY, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to send server the camera movement command. If ptNotify is null, the response will not
  * be notified.
  */
SCODE DEFEXPAPI ServerController_MoveCameraEX(HANDLE hDevice, DWORD dwChannel, DWORD dwX, DWORD dwY, DWORD dwResolutionWidth, DWORD dwResolutionHeight,
											DWORD dwDisplaySizeX, DWORD dwDisplaySizeY, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to send UART (COM port) command to server. If ptNotify is null, the response will not
  * be notified.
  */
SCODE DEFEXPAPI ServerController_UartWrite(HANDLE hDevice, DWORD dwPort, 
										   const BYTE *pbyCommand, DWORD dwLen,
										   TSVRCTRL_NOTIFY *ptNotify,
										   HANDLE *phProc);

/**
  * \brief This function is used to receive UART (COM port) command to server. 
  */
SCODE DEFEXPAPI ServerController_UartRead(HANDLE hDevice, DWORD dwPort, DWORD dwReadLen, 
										  BOOL bFlush, DWORD dwWaitTime, 
										  TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to get current DI information from server.
  */
SCODE DEFEXPAPI ServerController_GetDIStatus(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to get current DO information from server.
  */
SCODE DEFEXPAPI ServerController_GetDOStatus(HANDLE hDevice, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to set current DO information into server. For DO that is not to be set,
  * put eSrvCtrlDiDoNone in the corresponding position of the array
  */
SCODE DEFEXPAPI ServerController_SetDOStatus(HANDLE hDevice, ESrvCtrlDiDoLevel *peDOStatus,
										  DWORD dwMaxDOStatus, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to send a http command to server. For POST, the parameter is also
  * put after the question mark as GET does. If ptNotify is NULL, the post result will not be notified. 
  * After notified. User could use ServerController_ReadData to read the returned page, or he should call 
  */
SCODE DEFEXPAPI ServerController_SendHttpCommand(HANDLE hDevice, const char *pszCommand, BOOL bPost,
											  TSVRCTRL_NOTIFY *ptNotify, HANDLE *phOper);

/**
  * \brief This function is used to read the data from a http connection. User must ensure that the connection
  * is already connected. Or error will be returned.
  */
SCODE DEFEXPAPI ServerController_ReadData(HANDLE hOper, BYTE *pbyBuffer, DWORD *pdwBufLen,
										  TSVRCTRL_NOTIFY *ptNotify);

/**
  * \brief This function is used to get the server's motion detection setting. 
  */
SCODE DEFEXPAPI ServerController_GetMotionDetectionInfo(HANDLE hDevice, DWORD dwCamera, 
														TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to set the server's motion detection setting. The local cache for motion detection
  * setting is not changed by calling this function.
  */
SCODE DEFEXPAPI ServerController_SetMotionDetectionInfo(HANDLE hDevice, DWORD dwCamera, 
														const TSRVCTRL_MODETECT_INFO *ptMDInfo, 
														TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to get the server's motion detection setting. The setting is retrieved from server
  * directly, not from the local cache.
  */
SCODE DEFEXPAPI ServerController_GetPrivateMaskInfo(HANDLE hDevice, DWORD dwCamera, 
													TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/**
  * \brief This function is used to set the server's motion detection setting. The local cache for motion detection
  * setting is not changed by calling this function.
  */
SCODE DEFEXPAPI ServerController_SetPrivateMaskInfo(HANDLE hDevice, DWORD dwCamera, 
													const TSRVCTRL_PRIVATEMASK_INFO *ptPMInfo, 
													TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);
/****** non-blocking version *******/

/**
  * \brief This function is used to return the operater to hDevice. 
  */
SCODE DEFEXPAPI ServerController_FreeOperator(HANDLE hOper);

/**
  * \brief This function is used to abort operation & return the operater to hDevice. 
  */
SCODE DEFEXPAPI ServerController_AbortProcess(HANDLE hProc);

/**
  * \brief This function is used to set the root key of configuration. 
  */
SCODE DEFEXPAPI ServerController_SetRooKey(HANDLE hDevice, const char *pszKey);

/**
  * \brief This function is used get a value from local buffer. 
  * 
  */
SCODE DEFEXPAPI ServerController_GetValueByName(HANDLE hDevice, const char *pszName, 
												char *pszValue, DWORD *pdwValueSize);

/**
  * \brief This function is used set a value into local buffer. 
  * 
  */
SCODE DEFEXPAPI ServerController_SetValueByName(HANDLE hDevice, const char *pszName, 
												const char *pszValue, BOOL bMustExist);

/**
  * \brief This function is used retrieve the value entries under the specified name. 
  * 
  */
SCODE DEFEXPAPI ServerController_GetNameList(HANDLE hDevice, const char *pszName, char *pszNameList, DWORD *pdwValueSize);

SCODE DEFEXPAPI ServerController_SendHttpCommandReadBlockEx(HANDLE hScheAgent, TSRVCTRL_DEV_PROPERTY *ptDevProperty, const char *pszCommand, 
														  BOOL bPost, BYTE *pbyData, DWORD *pdwBufLen, DWORD dwReadTimeOut);


// sam 2008/11/20 set proxy
SCODE DEFEXPAPI ServerController_SetProxySetting(HANDLE hDevice, TSRVCTRL_DEV_PROXY *ptProxyOpt);

// sam 2008/12/12 Joystick command
SCODE DEFEXPAPI ServerController_SendJoystickPTCommandBlock(HANDLE hDevice, DWORD dwChannel, DWORD dwCamera, int nX, int nY, int nSpeed, BOOL bWaitRes);
SCODE DEFEXPAPI ServerController_SendJoystickZoomCommandBlock(HANDLE hDevice, DWORD dwChannel, DWORD dwCamera, EJOYSTICK_ZOOMING_COMMAND eZoomType, int nSpeed, BOOL bWaitRes);

SCODE DEFEXPAPI ServerController_SendJoystickPTCommand(HANDLE hDevice, DWORD dwChannel, DWORD dwCamera,
												int nX, int nY, int nSpeed, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);
SCODE DEFEXPAPI ServerController_SendJoystickZoomCommand(HANDLE hDevice, DWORD dwChannel, DWORD dwCamera,
												EJOYSTICK_ZOOMING_COMMAND eZoomType, int nSpeed, TSVRCTRL_NOTIFY *ptNotify, HANDLE *phProc);

/// sam 2009/04/15 support ePTZ
SCODE DEFEXPAPI ServerController_SetPTZType(HANDLE hDevice, ESrvCtrlPTZType ePTZType);
#ifdef __cplusplus
}
#endif




#endif // !_SERVER_CONTROLLER_H

