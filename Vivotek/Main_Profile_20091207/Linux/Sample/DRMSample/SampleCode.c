// Control.cpp : Defines the entry point for the console application.
//
#ifdef _WIN32
#include <winsock.h>
#endif // _WIN32

#include "stdlib.h"
#include "stdio.h"
#include "DRMControl.h"

DWORD __stdcall DRMControlCallback(DWORD dwParentObject, EDRMControl_CallbackType eCallbackType, void* pvCallbackData)
{
	tagTDRMServiceInfo* ptDRMServiceInfo;
	HANDLE hDRMControl;
	int i;

	switch(eCallbackType)
	{
		case DRMControl_Callback_DiscoveryResult :
			 ptDRMServiceInfo= (tagTDRMServiceInfo*)pvCallbackData;
				printf("IP address : %s\n",ptDRMServiceInfo->acIP);
				printf("Machine Type : %s\n",ptDRMServiceInfo->acMachineType);
			//	printf("HostName :%s\n",ptDRMServiceInfo->acServiceName);
				printf("MAC address:");
				for(i=0;i<6;i++)
					printf("%02x",ptDRMServiceInfo->abyMac[i]);

				printf("\n");

				printf("HTTP port %d\n",ptDRMServiceInfo->sHTTPPort);
				printf("FTP port %d\n",ptDRMServiceInfo->sFTPPort);
				printf("Language %s\n",ptDRMServiceInfo->acLanguage);
			break;

		case DRMControl_Callback_Stopped:
			hDRMControl = (HANDLE)pvCallbackData;
			printf("DRM thread stopped\n");
//			DRMControl_Close(&hDRMControl);
			break;

		case DRMControl_Callback_Error :
			printf("broadcast error!!\n");
			break;

		default:
			break;
	}
	return 0;
}


int main(int argc, char* argv[])
{
	HANDLE hDRMControl;
	TDRMControlCreateOptions DRMOption;
	int i;
	char c,ac[3];
#ifdef _WIN32
	WSADATA wsaData;

	if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR)
	{
		fprintf(stderr,"WSAStartup error %d\n",WSAGetLastError());
		return -1;
	}
	else
	{
		printf("WSAStartup successed.\n");
	}
#endif // _WIN32

	DRMOption.usDRMControlPort = 9000;
	DRMControl_Create(&hDRMControl, &DRMOption);

	DRMControl_SetCallback(hDRMControl, (DWORD) NULL, DRMControlCallback);
	DRMControl_Start(hDRMControl);

	for(i=0;i<10;i++)
	{
	    
		DRMControl_Discovery(hDRMControl);    // discover all device in the LAN      
//		DRMControl_DiscoveryByMAC(hDRMControl,"0002D100089C"); // discover this MAC only 

#ifdef _WIN32
		Sleep(30000);
#else
		sleep(30);
#endif //_WIN32
	}
	
	DRMControl_Stop(hDRMControl);

	
	return 0;
}

