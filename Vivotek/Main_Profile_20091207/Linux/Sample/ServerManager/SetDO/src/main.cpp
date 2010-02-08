#include "ServerManager.h"
#include "ServerManagerError.h"
#include "SetDO.h"

void ReleaseServerManager(HANDLE hSrvMgr, HANDLE hDevice)
{
	// STEP 9: Release Server Manager handle
	if (hDevice)
	{
		ServerManager_DeleteDevice(&hDevice);
	}

	// STEP 10: Release Server Manager handle
	if (hSrvMgr)
	{
		ServerManager_Release(&hSrvMgr);
	}
}

int main()
{
	SCODE scRet = S_OK;
	HANDLE hSrvMgr = NULL;
	HANDLE hDevice = NULL;
	TVersionInfo tVerInfo;
	TSRVMNGR_DEV_PROPERTY tDevProperty;

        memset(&tVerInfo, 0 , sizeof(tVerInfo));
	memset(&tDevProperty, 0 , sizeof(tDevProperty));
	
	// STEP 1: Set ServerManager.dll version
	tVerInfo.wMajor = SRVMNGR_VERSION_MAJOR;
	tVerInfo.wMinor = SRVMNGR_VERSION_MINOR;
	tVerInfo.wBuild = SRVMNGR_VERSION_BUILD;
	tVerInfo.wRevision = SRVMNGR_VERSION_REVISION;
	
	char szHostName[MAX_HOSTNAME_LEN] = {0};
	char szUserName[MAX_USERNAME_LEN] = {0};
	char szPassword[MAX_PASSWORD_LEN] = {0};
	char szHttpPort[MAX_HTTPPORT_LEN] = {0};
	char szTimeout[MAX_TIMEOUT_LEN] = {0};
	char szModel[MAX_PATH + 1] = {0};
	
	DWORD dwHttpPort = 0;
	DWORD dwTimeout = 0;

	// STEP 2: Initial server manager handle
	// param dwMaxDevice The maximum number of devices are allowed to open.
	// param dwFlag		 The flag to use in the ServerManager module.
	scRet = ServerManager_Initial(&hSrvMgr, CONST_MAX_DEVIDE, 0, &tVerInfo);
	if (scRet != S_OK)
	{
		printf("ServerManager_InitialEX failed %x\n", scRet);
		goto Lab_Exit;
	}

	// STEP 3: Create device handle
	scRet = ServerManager_CreateDevice(hSrvMgr, &hDevice);
	if (scRet != S_OK)
	{
		printf("ServerManager_CreateDevice failed %x\n", scRet);
		goto Lab_Exit;
	}

	printf("Please input server ip : ");
	gets(szHostName);

	printf("Please input server user name : ");
	gets(szUserName);

	printf("Please input server password : ");
	gets(szPassword);

	printf("Please input http port : ");
	gets(szHttpPort);

	printf("Please input timeout value (sec): ");
	gets(szTimeout);

	dwHttpPort = (DWORD) atoi(szHttpPort);
	dwTimeout = (DWORD) atoi(szTimeout) * 1000;

	// STEP 4: Set device properties
	strcpy(tDevProperty.tServerProperty.szHost, szHostName);
	strcpy(tDevProperty.tServerProperty.szUserName, szUserName);
	strcpy(tDevProperty.tServerProperty.szPassword, szPassword);
	tDevProperty.tServerProperty.dwHttpPort = dwHttpPort;
	tDevProperty.tServerProperty.dwTimeout = dwTimeout;

	// STEP 5: Update device property
	scRet = ServerManager_SetDeviceProperty(hDevice, &tDevProperty);
	if (scRet != S_OK)
	{
		printf("ServerManager_SetDeviceProperty failed %x\n", scRet);
		goto Lab_Exit;
	}

	// STEP 6: Open device (block mode)
	// param lpszModelName The char pointer to receive the retrieved model name
	// param dwMaxSize     The maximum size of model name
	scRet = ServerManager_OpenDeviceBlock(hDevice, szModel, MAX_PATH);
	if (scRet != S_OK)
	{
		printf("ServerManager_OpenDeviceBlock failed: %x\n", scRet);
		goto Lab_Exit;
	}
	else
	{
		printf("Model Name = %s\n", szModel);
	}

	{
		ESrvCtrlDiDoLevel aeDOStatus[CONST_MAX_DIDO_NUM];
		DWORD dwDoNum = CONST_MAX_DIDO_NUM;
		int iIndex = 0;

		// STEP 7: Get DO status
		// Param peDOStatus  The pointer to a ESrvCtrlDiDoLevelarray buffer to receive the retrieved DO status 
		//					 from the server.
		// Param pdwMaxDONum The pointer to a DWORD to represent the size value of DO status array buffer. 
        //					 And the module will replace it with the actual size value of DO status array. If the actual 
		//					 size is more than *pdwMaxDINum, the module will return error. And the give buffer will 
		//					 not be filled
		scRet = ServerManager_GetDOStatusBlock(hDevice, aeDOStatus, &dwDoNum);
		if (scRet != S_OK)
		{
			printf("ServerManager_GetDOStatusBlock failed: %x\n", scRet);

			goto Lab_Exit;
		}
		else
		{
			printf("ServerManager_GetDOStatusBlock success\n");
		}
			
		for(iIndex = 0; iIndex < (int) dwDoNum; iIndex++)
		{
			if (aeDOStatus[iIndex] == eSrvCtrlDiDoLow)
			{
				// set DO to eSrvCtrlDiDoHigh;
				aeDOStatus[iIndex] = eSrvCtrlDiDoHigh;
			}
			else
			{
				// set DO to eSrvCtrlDiDoLow
				aeDOStatus[iIndex] = eSrvCtrlDiDoLow;
			}	
		}
		
		// STEP 8: Set DO status 
		// Param peDOStatus The pointer to a ESrvCtrlDiDoLevelto update the DO status of the server. 
		//					 
		// Param dwMaxDONum The size value of DO status array
		//
		// bWaitRes			The Boolean value to wait for the server's response or not.
		scRet = ServerManager_SetDOStatusBlock(hDevice, aeDOStatus, dwDoNum, FALSE);
		if (scRet != S_OK)
		{
			printf("ServerManager_SetDOStatusBlock failed: %x\n", scRet);

			goto Lab_Exit;
		}
		else
		{
			printf("ServerManager_SetDOStatusBlock success\n");
		}
	}

Lab_Exit:

	// Release Server Manager handle and device handle
	ReleaseServerManager(hSrvMgr, hDevice);

	return 0;
}

