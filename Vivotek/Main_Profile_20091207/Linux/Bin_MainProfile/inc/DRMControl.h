/*
 *******************************************************************************
 * $Header: /RD_1/Project/VNDP/MainProfile/DRMControl/src/DRMControl.h 4     06/10/30 6:17p Shengfu $
 *
 *  Copyright (c) 2000-2002 Vivotek Inc. All rights reserved.
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
 *
 * $History: DRMControl.h $
 * 
 * *****************  Version 4  *****************
 * User: Shengfu      Date: 06/10/30   Time: 6:17p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DRMControl/src
 * update to version 2.0.0.2
 * 
 * *****************  Version 3  *****************
 * User: Shengfu      Date: 06/10/04   Time: 9:59a
 * Updated in $/RD_1/Project/VNDP/MainProfile/DRMControl/src
 * update to 2.0.0.1
 * 
 * *****************  Version 2  *****************
 * User: Shengfu      Date: 05/10/28   Time: 5:30p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DRMControl/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 4:16p
 * Created in $/RD_1/Project/VNDP/MainProfile/DRMControl/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 1:50p
 * Created in $/RD_1/Project/VNDP/MainProfile/DRMControl/DRMControl/src
 * 
 * *****************  Version 2  *****************
 * User: Perkins      Date: 05/06/28   Time: 4:09p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DRMControl/src
 * must add extern C for cpp user
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/11/24   Time: 2:08p
 * Created in $/RD_1/Project/VNDP/MainProfile/DRMControl/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/11/24   Time: 2:05p
 * Created in $/RD_1/Project/VNDP/MainProfile/DRMControl/DRMControl/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/11/24   Time: 2:04p
 * Created in $/RD_1/Project/VNDP/MainProfile/DRMControl/DRMControl/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/11/24   Time: 2:02p
 * Created in $/RD_1/Project/VNDP/MainProfile/DRMControl/DRMControl/src
 * 
 * *****************  Version 2  *****************
 * User: Shengfu      Date: 04/03/22   Time: 1:29p
 * Updated in $/rd_1/Protocol/Modules/DRMClientLib
 *
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2002 Vivotek, Inc. All rights reserved.
 *
 * \file
 * DRMControl.h
 *
 * \brief
 * Vivotek Machine Discovery, Registration Mechanism
 *
 * \date
 * 2002/12/05
 *
 * \author
 * Simon Chen
 *
 *
 *******************************************************************************
 */

#ifndef _DRMControl_H_
#define _DRMControl_H_

#include "typedef.h"
#include "common.h"

//typedef long SCODE;
/*!
 * FOUR_CC Version code of your \b DRMControl, you should update it if you have
 * changed your \b DRMControl's interface.
 */
#define DRMControl_VERSION   MAKEFOURCC(2, 0, 0, 7)
//(1) option to create virtual IP or  not

#define DRMCONTROL_MAX_CAMERA_NAME	96


/*! A data structure of \b TDRMServiceInfo object. Used in callback function
 * to report the discovery result or Service registration information. */
typedef struct tagTDRMServiceInfo{
	/*! Type of machine (firmware version) */
	char  acMachineType[44]; //(40+4)

	/*! Mac of machine */
	BYTE abyMac[6];

	/*! IP */
	char acIP[20];

	/*! Model name of machine */
	char  acModelName[44];  // (40+4)
	
	short sHTTPPort;
	short sFTPPort;
	char  acLanguage[6];
	/* EZ installation version */
	DWORD dwEZversion;

	/*! Camera name*/
	char acCameraName[DRMCONTROL_MAX_CAMERA_NAME + 1]; //(96+1)
} tagTDRMServiceInfo;

/*!  enumeration of /b FDRMControlCallback callback type. */
typedef enum
{

	/*! After the discovery has finished. */
	DRMControl_Callback_DiscoveryResult = 1,

	/*! When Service registers to this controller. */
	//DRMControl_Callback_ServiceRegistration = 2,

	/*! When Service unregisters to this controller or timeout to response. */
	//DRMControl_Callback_ServiceUnRegistration = 3,

	/*! When the DRMControl object thread has been stopped. */
	DRMControl_Callback_Stopped = 4,

	/*! When error occurs in thread. */
	DRMControl_Callback_Error = 5,

} EDRMControl_CallbackType;


/*!
 *******************************************************************************
 * \brief
 * Callback function to report the event of \b DRMControl object.
 *
 * \param dwInstance
 * \a (i) bypass external 32-bit data to callback function.
 *
 * \param dwCallbackType
 * \a (i) the report type at this callback. 
 *
 * \param pvParam
 * \a (i) point to the parameter that every type has its definition.
 *
 * \retval S_OK
 * callback response OK.
 * \retval S_FAIL
 * callback response failed.
 *
 * \remark
 * Callback type is described as following:
 * \b DRMControl_Callback_DiscoveryResult: When discovery result is finished,   
 * DRMControl object feedbacks the result via callback function. The pvParam is
 * the structure \b TDRMServiceInfo that contains all the Service informations
 * found by this controller.
 *
 * \b DRMControl_Callback_ServiceRegistration: When controller receives the   
 * registration information from other Service, DRMControl object feedbacks
 * the information via callback function. The pvParam is the structure 
 * \b TDRMServiceInfo that contains the Service informations.
 * 
 * \b DRMControl_Callback_ServiceUnRegistration: When controller receives the   
 * unregistration information from other Service, DRMControl object feedbacks
 * the information via callback function. The pvParam is the structure 
 * \b TDRMServiceInfo that contains the Service informations.  
 * 
 * \b DRMControl_Callback_Stopped: When the thread of DRMControl object has
 * stopped, DRMControl object responses the status via callback function. The
 * pvParam is NULL.  
 * 
 * \b DRMControl_Callback_Error: When the error status of DRMControl object 
 * happens, DRMControl object responses the status via callback function. The
 * pvParam is NULL.  
 *
 * \see DRMControl_SetCallback
 * \see EDRMControl_CallbackType
 * \see TDRMServiceInfo
 *
 **************************************************************************** */
#ifdef _WIN32
typedef DWORD (__stdcall *FDRMControlCallback)(HANDLE hInstance,	EDRMControl_CallbackType eDRMControlCallbackType,void  *pvParam);
#else
typedef DWORD (*FDRMControlCallback)(DWORD dwInstance,	EDRMControl_CallbackType eDRMControlCallbackType,void  *pvParam);
#endif


/*! A data structure of \b DRMControl object. Used in function
 * \b DRMControl_Create to set the initial options. */
typedef struct
{

	/*! Maximum discovery items that the controller can handle. */
	DWORD dwMaxDiscoveryItem;

	
	/*! DRMControl port number. */
	unsigned short usDRMControlPort;

	/*! Create 169.254.x.x interface or not (TRUE or FALSE*/
	DWORD		dwAliasIP;

} TDRMControlCreateOptions;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *******************************************************************************
 * \brief
 * Create DRMControl instance and initialize it.
 *
 * \param phObject
 * \a (o) pointer to receive the instance's handle of this object.
 *
 * \param pDRMControlInitOptions
 * \a (i/o) the pointer of data structure \b TDRMControlCreateOptions for setting
 * the initialization parameters of the \b DRMControl instance. If you did not
 * set some parameters which are needed, the default value will be set and write
 * back to the corresponding fields.
 *
 * \retval S_OK
 * Initialize \b DRMControl OK.
 * \retval S_FAIL
 * Initialize \b DRMControl failed.
 *
 * \remark
 * This function should be called before using this instance.
 *
 * \see DRMControl_Close
 *
 **************************************************************************** */
SCODE DLLAPI DRMControl_Create(HANDLE *phObject, TDRMControlCreateOptions *pDRMControlInitOptions);

/*!
 *******************************************************************************
 * \brief
 * Delete an instance of the \b DRMControl object.
 *
 * \param phObject
 * \a (i/o) the pointer to the handle of the \b DRMControl instance.
 *
 * \retval S_OK
 * Release DRMControl OK.
 * \retval S_FAIL
 * Release DRMControl failed.
 *
 * \remark
 * After the instance is released, the handle of this instance will be set to
 * zero.
 *
 * \see DRMControl_Create
 *
 **************************************************************************** */
SCODE DLLAPI DRMControl_Close(HANDLE *phObject);


/*!
 *******************************************************************************
 * \brief
 * Set the callback function of DRMControl object
 *
 * \param hObject
 * \a (i) a handle of \b DRMControl instance.
 *
 * \param dwInstance
 * \a (i) bypass external 32-bit data to callback function.
 *
 * \param pfDRMControlCallback
 * \a (i) the function pointer of function type \b FDRMControlCallback. 
 *
 * \retval S_OK
 * set DRMControl callback function OK.
 * \retval S_FAIL
 * set DRMControl callback function failed.
 *
 * \remark
 * NONE
 *
 * \see DRMControl_Create
 *
 **************************************************************************** */
SCODE DLLAPI DRMControl_SetCallback(HANDLE hObject, HANDLE hInstance, FDRMControlCallback fDRMControlCallback);


/*!
 *******************************************************************************
 * \brief
 * Start the DRMControl mechanism
 *
 * \param hObject
 * \a (i) a handle of \b DRMControl instance.
 *
 * \param dwThreadPriority
 * \a (i) the thread priority of \b DRMControl thread. 
 *
 * \retval S_OK
 * start DRMControl mechanism OK.
 * \retval S_FAIL
 * start DRMControl mechanism failed.
 *
 * \remark
 * NONE
 *
 * \see DRMControl_Create 
 * \see DRMControl_SetParameters
 * \see DRMControl_SetCallback
 *
 **************************************************************************** */
SCODE DLLAPI DRMControl_Start(HANDLE hDRMControlHandle);



/*!
 *******************************************************************************
 * \brief
 * Stop the DRMControl mechanism
 *
 * \param hObject
 * \a (i) a handle of \b DRMControl instance.
 *
 *
 * \retval S_OK
 * stop DRMControl mechanism OK.
 * \retval S_FAIL
 * stop DRMControl mechanism failed.
 *
 * \remark
 * NONE
 *
 * \see DRMControl_Create 
 * \see DRMControl_Start
 *
 **************************************************************************** */
SCODE DLLAPI DRMControl_Stop(HANDLE hDRMControlHandle);

/*!
 *******************************************************************************
 * \brief
 * Manual to discover the DRM's Service of specific MAS address. The result is 
 * reported via callback
 *
 * function.
 *
 * \param hObject
 * \a (i) a handle of \b DRMControl instance.
 *
 *
 * \retval S_OK
 * Start the procedure of Service discovery successfully.
 *
 * \retval S_FAIL
 * Start the procedure of Service discovery failed.
 *
 * \remark
 * Before you discover DRM's Service manually, be sure that the DRM control has
 * been started. The result is reported by callback function.
 *
 * \see DRMControl_Start
 * \see DRMControl_SetCallback
 *
 **************************************************************************** */
SCODE DLLAPI DRMControl_DiscoveryByMAC(HANDLE hDRMControlHandle,char* pcMACAddress);

/*!
 *******************************************************************************
 * \brief
 * Manual to discover all the DRM's Service. The result is reported via callback
 * function.
 *
 * \param hObject
 * \a (i) a handle of \b DRMControl instance.
 *
 *
 * \retval S_OK
 * Start the procedure of Service discovery successfully.
 *
 * \retval S_FAIL
 * Start the procedure of Service discovery failed.
 *
 * \remark
 * Before you discover DRM's Service manually, be sure that the DRM control has
 * been started. The result is reported by callback function.
 *
 * \see DRMControl_Start
 * \see DRMControl_SetCallback
 *
 **************************************************************************** */
SCODE DLLAPI DRMControl_Discovery(HANDLE hDRMControlHandle);

#ifdef __cplusplus
}
#endif

#endif	// DRMControl
