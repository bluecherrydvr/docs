#include "ServerManager.h"
#include "ServerManagerError.h"
#include "GetSetVideoInfo.h"

SCODE GetVideoInfo(HANDLE hDevice)
{
	if (NULL == hDevice)
	{
		return E_DEVICE_HANDLE;
	}

	// STEP 7: Get Video Information : ex. Get Server Video Size
	// param pszName       The pointer to the name of the key you want to get the value.
	// param pszValue      The pointer to the buffer to receive the value.
	// param pdwValueSize  The pointer to the DWORD to indicate the size of buffer. After the function 
	//					   returned, it will be filled with the length of value.
	{
		SCODE scRet = S_OK;
		char szValue[MAX_PATH + 1] = {0};
		DWORD dwBufSize = MAX_PATH;

		scRet = ServerManager_GetValueByName(hDevice, "videoin_c0_s0_resolution", szValue, &dwBufSize);
		if (scRet != S_OK)
		{
			printf("ServerManager_GetValueByName failed: %x\n", scRet);
			return E_GETVALUE_FAIL;
		}
		else
		{
			printf("Video Size : %s\n", szValue);
		}
	}

	// STEP 7: Get Video Information : ex. Get Server Codec
	{
		SCODE scRet = S_OK;
		char szValue[MAX_PATH + 1] = {0};
		DWORD dwBufSize = MAX_PATH;

		scRet = ServerManager_GetValueByName(hDevice, "videoin_c0_s0_codectype", szValue, &dwBufSize);
		if (scRet != S_OK)
		{
			printf("ServerManager_GetValueByName failed: %x\n", scRet);
			return E_GETVALUE_FAIL;
		}
		else
		{
			printf("Codec type : %s\n", szValue);
		}
	}

	return S_OK;
}

SCODE SetVideoInfo(HANDLE hDevice)
{
	if (NULL == hDevice)
	{
		return E_DEVICE_HANDLE;
	}

	// STEP 8: Set Video Information : ex. Set Server Video Size
	// param pszName    The pointer to the name of the key you want to update the value.
	// param pszValue   The pointer to the buffer which contain the value.
	// param bMustExist The key must exist or not.
	{
		SCODE scRet = S_OK;

		scRet = ServerManager_SetValueByName(hDevice, "videoin_c0_s0_resolution", "1144x776", TRUE);
		if (scRet != S_OK)
		{
			printf("ServerManager_GetValueByName failed: %x\n", scRet);
			return E_SETVALUE_FAIL;
		}
		else
		{
			printf("ServerManager_SetValueByName success\n");
		}
	}

	// STEP 8: Set Video Information : ex. Set Codec
	{
		SCODE scRet = S_OK;

		scRet = ServerManager_SetValueByName(hDevice, "videoin_c0_s0_codectype", "mpeg4", TRUE);
		if (scRet != S_OK)
		{
			printf("ServerManager_GetValueByName failed: %x\n", scRet);
			return E_SETVALUE_FAIL;
		}
		else
		{
			printf("ServerManager_SetValueByName success\n");
		}
	}

	SCODE scRet = S_OK;

	// STEP 9: Update server value from ServerManager_SetValueByName
	scRet = ServerManager_UpdateRemoteConfigBlock(hDevice);
	if (scRet != S_OK)
	{	
		printf("ServerManager_UpdateRemoteConfigBlock failed: %x\n", scRet);
		return E_SETVALUE_FAIL;
	}

	return S_OK;
}

void ReleaseServerManager(HANDLE hSrvMgr, HANDLE hDevice)
{
	// STEP 10: Release Server Manager handle
	if (hDevice)
	{
		ServerManager_DeleteDevice(&hDevice);
	}

	// STEP 11: Release Server Manager handle
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

	// STEP 7: Get Video Information
	scRet = GetVideoInfo(hDevice);
	if (scRet != S_OK)
	{
		printf("GetNetworkInfo failed: %x\n", scRet);
		goto Lab_Exit;
	}

	// STEP 8: Set Video Information
	scRet = SetVideoInfo(hDevice);
	if (scRet != S_OK)
	{
		printf("SetNetworkInfo failed: %x\n", scRet);
		goto Lab_Exit;
	}

Lab_Exit:

	// Release Server Manager handle and device handle
	ReleaseServerManager(hSrvMgr, hDevice);

	return 0;
}