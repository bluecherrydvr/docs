#include "ServerManager.h"
#include "ServerManagerError.h"
#include "GetDI.h"

void ReleaseServerManager(HANDLE hSrvMgr, HANDLE hDevice)
{
	// STEP 8: Release Server Manager handle
	if (hDevice)
	{
		ServerManager_DeleteDevice(&hDevice);
	}

	// STEP 9: Release Server Manager handle
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
	
	char szHostName[MAX_HOSTNAME_LEN] = {0};
	char szUserName[MAX_USERNAME_LEN] = {0};
	char szPassword[MAX_PASSWORD_LEN] = {0};
	char szHttpPort[MAX_HTTPPORT_LEN] = {0};
	char szTimeout[MAX_TIMEOUT_LEN] = {0};
	char szModel[MAX_PATH + 1] = {0};
	
	DWORD dwHttpPort = 0;
	DWORD dwTimeout = 0;

	// STEP 1: Set ServerManager.dll version
	tVerInfo.wMajor = SRVMNGR_VERSION_MAJOR;
	tVerInfo.wMinor = SRVMNGR_VERSION_MINOR;
	tVerInfo.wBuild = SRVMNGR_VERSION_BUILD;
	tVerInfo.wRevision = SRVMNGR_VERSION_REVISION;

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

	// STEP 7: Get DI status 
	// Param peDIStatus  The pointer to a ESrvCtrlDiDoLevelarray buffer to receive the retrieved DI status 
	//					 from the server. 
	// Param pdwMaxDINum The pointer to a DWORD to represent the size value of DI status array buffer.
	{
		ESrvCtrlDiDoLevel aeDIStatus[CONST_MAX_DIDO_NUM];
		DWORD dwDiNum = CONST_MAX_DIDO_NUM;
		
		scRet = ServerManager_GetDIStatusBlock(hDevice, aeDIStatus, &dwDiNum);
		if (scRet != S_OK)
		{
			printf("ServerManager_GetDIStatusBlock failed: %x\n", scRet);
			goto Lab_Exit;
		}
		else
		{
			DWORD dwI = 0;
			printf("ServerManager_GetDIStatusBlock success\n");
			for (dwI = 0; dwI < dwDiNum; dwI++)
			{
				printf("DI%d=%d\n", dwI, aeDIStatus[dwI]);
			}
		}
	}

Lab_Exit:
        
        // Release Server Manager handle and device handle
	ReleaseServerManager(hSrvMgr, hDevice);

	return 0;
}
