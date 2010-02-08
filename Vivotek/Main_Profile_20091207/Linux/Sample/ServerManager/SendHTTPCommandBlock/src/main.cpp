#include "ServerManager.h"
#include "ServerManagerError.h"
#include "SendHTTPCommandBlock.h"

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

	// STEP 7: Semd HTTP command to server
	// Param pszCommand  The string pointer to an http command.
	// Param bPost       True is for POST, false for GET
	{
		scRet = ServerManager_SendHttpCommandBlock(hDevice, "/cgi-bin/admin/setparam.cgi?system_hostname=test", TRUE);
		if (scRet != S_OK)
		{
			printf("ServerManager_SendHttpCommandBlock failed: %x\n", scRet);
			goto Lab_Exit;
		}
		else
		{
			printf("ServerManager_SendHttpCommandBlock success\n");	
		}

		scRet = ServerManager_SendHttpCommandBlock(hDevice, "/cgi-bin/admin/setparam.cgi?system_date=2009/08/13", FALSE);
		if (scRet != S_OK)
		{
			printf("ServerManager_SendHttpCommandBlock failed: %x\n", scRet);
			goto Lab_Exit;
		}
		else
		{
			printf("ServerManager_SendHttpCommandBlock success\n");	
		}
	}

Lab_Exit:

	// Release Server Manager handle and device handle
	ReleaseServerManager(hSrvMgr, hDevice);

	return 0;
}
