#include "string.h"
#include "DataBroker.h"
#include "OSDefine.h"

#define MAX_IP_LENGTH         256
#define MAX_USERNAME_LENGTH   256
#define MAX_PASSWORD_LENGTH   256
#define MAX_SERVERTYPE_LENGTH 16
#define CONNECTION_SIZE	      4


typedef enum
{	
  eConnecting,
  eConnected,
  eDisconnectd,

} EConnectionStatus;

typedef	struct TConnectionInfo
{
	HANDLE							hConn;
	EConnectionStatus				eStatus;
	TDataBrokerConnectionOptions	tOpt;

	TConnectionInfo():
	hConn(NULL),
	eStatus(eDisconnectd)
	{
		memset(&tOpt, 0, sizeof(tOpt));
	}

} TConnectionInfo;

#ifdef WIN32

typedef	struct TDataBrokerInfo
{
	HANDLE hDataBroker;
	HANDLE hExit;
	TConnectionInfo tConInfo[CONNECTION_SIZE];

	TDataBrokerInfo():
	hDataBroker(NULL),
		hExit(NULL)
	{

	}

} TDataBrokerInfo;

#else

typedef	struct TDataBrokerInfo
{
	HANDLE hDataBroker;
	bool bExit;
	TConnectionInfo tConInfo[CONNECTION_SIZE];

	TDataBrokerInfo():
	hDataBroker(NULL),
	bExit(false)
	{

	}

} TDataBrokerInfo;

#endif



// This callback function is used to receive media packet, the calling convention must be stdcall
SCODE __stdcall DataBrokerAVCallBack(DWORD dwContext,TMediaDataPacketInfo *ptMediaDataPacket)
{
  
   return S_OK;
}

// This callback function is used to receive status info, the calling convention must be stdcall
SCODE __stdcall DataBrokerStatusCallBack(DWORD dwContext, TDataBrokerStatusType tStatusType, PVOID pvParam1, PVOID pvParam2)
{
	TConnectionInfo* ptConInfo = (TConnectionInfo *) dwContext;

	switch (tStatusType)
	{
		case eOnConnectionInfo:
		{
			printf("Get eOnConnectionInfo, start to receive packet, %x\n", ptConInfo);
		}
		break;

		case eOnStopped:
		{
			printf("Get eOnStopped, stop to receive packet, %x\n", ptConInfo);
            
			//When receive eOnStopped, set connection status to disconnect.
			ptConInfo->eStatus = eDisconnectd;
		}
		break;

		default:
		{
		}	
		break;
	}

   return S_OK;
}

void CreateConnectionOption(TDataBrokerConnectionOptions *ptOpt, TConnectionInfo* ptConInfo)
{	
	//Set device IP
	printf("Please input device IP: ");
	fgets(ptOpt->pzIPAddr, MAX_IP_LENGTH, stdin);
	
	if (ptOpt->pzIPAddr[strlen(ptOpt->pzIPAddr) - 2] == '\r' && ptOpt->pzIPAddr[strlen(ptOpt->pzIPAddr) - 1] == '\n')
	{ 
           ptOpt->pzIPAddr[strlen(ptOpt->pzIPAddr) - 2]= '\0';
        }
        else if (ptOpt->pzIPAddr[strlen(ptOpt->pzIPAddr) - 1] == '\n')
	{ 
           ptOpt->pzIPAddr[strlen(ptOpt->pzIPAddr)-1]= '\0';
        }
        
	//Set device username
	printf("Please input device user name (press [ENTER] for empty): ");
	fgets(ptOpt->pzUID, MAX_USERNAME_LENGTH, stdin);
	
	if (ptOpt->pzUID[strlen(ptOpt->pzUID) - 2] == '\r' && ptOpt->pzUID[strlen(ptOpt->pzUID) - 1] == '\n')
	{ 
           ptOpt->pzUID[strlen(ptOpt->pzUID) - 2]= '\0';
        }
        else if (ptOpt->pzUID[strlen(ptOpt->pzUID) - 1] == '\n')
	{ 
           ptOpt->pzUID[strlen(ptOpt->pzUID)-1]= '\0';
        }

	//Set device password
	printf("Please input device password (press [ENTER] for empty): ");
	fgets(ptOpt->pzPWD, MAX_PASSWORD_LENGTH, stdin);
	
	if (ptOpt->pzPWD[strlen(ptOpt->pzPWD) - 2] == '\r' && ptOpt->pzPWD[strlen(ptOpt->pzPWD) - 1] == '\n')
	{ 
           ptOpt->pzPWD[strlen(ptOpt->pzPWD) - 2]= '\0';
        }
        else if (ptOpt->pzPWD[strlen(ptOpt->pzPWD) - 1] == '\n')
	{ 
           ptOpt->pzPWD[strlen(ptOpt->pzPWD)-1]= '\0';
        }

	//Set device HTTP port
	char szHTTPPort[128] = {0};
	printf("Please input device HTTP port: ");
	fgets(szHTTPPort, 128, stdin);
	
	if (szHTTPPort[strlen(szHTTPPort) - 2] == '\r' && szHTTPPort[strlen(szHTTPPort) - 1] == '\n')
	{ 
           szHTTPPort[strlen(szHTTPPort) - 2]= '\0';
        }
        else if (szHTTPPort[strlen(szHTTPPort) - 1] == '\n')
	{ 
           szHTTPPort[strlen(szHTTPPort)-1]= '\0';
        }
	ptOpt->wHttpPort = atoi(szHTTPPort);

	//Set connection protocol type to TCP, you may refer to TsdrProtocolType for other protocols.
	ptOpt->dwProtocolType = eptTCP;

	//Set connection media type to Video and Audio.
	ptOpt->dwMediaType =  (emtVideo | emtAudio);

	//Set device server type to auto, the DataBroker will auto detect the server type.
	strcpy(ptOpt->pzServerType, _T("auto"));

	//Pass context to DataBrokerStatusCallBack, the context is useful when we have to handle multi connections simultaneously.
	ptOpt->dwStatusContext = (DWORD) ptConInfo;

	//Pass context to DataBrokerAVCallBack, the context is useful when we have to handle multi connections simultaneously.
	ptOpt->dwAVContext = NULL;

	//Set status callback to connection option
	ptOpt->pfStatus = DataBrokerStatusCallBack;

	//Set Audio/Video callback to connection option
	ptOpt->pfAV = DataBrokerAVCallBack;

	//Set dwFlags to tell DataBroker what options we set.
	ptOpt->dwFlags = eConOptProtocolAndMediaType | eConOptHttpPort | 
		(eConOptStatusCallback | eConOptStatusContext) |
		(eConOptAVCallback | eConOptAVContext);
}

#ifdef WIN32

// This function is used to create a thread and monitor reconnect event. 
DWORD WINAPI MonitorThread(LPVOID lpParam)
{
	TDataBrokerInfo* ptDBInfo = (TDataBrokerInfo*) lpParam;
    SCODE scRet = S_FAIL;

	for (;;)
	{
		DWORD dwObj = WaitForSingleObject(ptDBInfo->hExit, 500);

		if (dwObj == WAIT_OBJECT_0)	//	exit the monitor
		{
			break;
		} 

		for (int i = 0; i < CONNECTION_SIZE; i++)
		{
			// STEP 1: Check connection status is disconnect
			if(ptDBInfo->tConInfo[i].eStatus == eDisconnectd)
			{
				// STEP 2: Delete connection
				scRet = DataBroker_DeleteConnection(ptDBInfo->hDataBroker, &ptDBInfo->tConInfo[i].hConn);

				if (IS_FAIL(scRet))
				{
					printf("DataBroker_DeleteConnection failed with error %X in MonitorThread\n", scRet);
					return -1;
				}

				// STEP 3: Create connection
				scRet = DataBroker_CreateConnection(ptDBInfo->hDataBroker, &ptDBInfo->tConInfo[i].hConn);

				if (IS_FAIL(scRet))
				{
					printf("DataBroker_CreateConnection failed with error %X in MonitorThread\n", scRet);
					return -1;
				}

				// STEP 4: Set option to connection
				scRet = DataBroker_SetConnectionOptions(ptDBInfo->tConInfo[i].hConn, &ptDBInfo->tConInfo[i].tOpt);

				if (IS_FAIL(scRet))
				{
					printf("DataBroker_SetConnectionOptions failed with error %X in MonitorThread\n", scRet);
					return -1;
				}

				// STEP 5: Set connection status is connecting
				ptDBInfo->tConInfo[i].eStatus = eConnecting;

				// STEP 6: Connect to device, this function is an asynchronous function, 
				scRet = DataBroker_Connect(ptDBInfo->tConInfo[i].hConn);

				if (IS_FAIL(scRet))
				{
					printf("DataBroker_Connect failed with error %X in MonitorThread\n", scRet);
					return -1;
				}
			}
		}		
	}

	return 0;
}

#else

void *MonitorThread(void* lpParam)
{
	TDataBrokerInfo* ptDBInfo = (TDataBrokerInfo*) lpParam;
	SCODE scRet = S_FAIL;

	for (;;)
	{
		if (ptDBInfo->bExit)	//	exit the monitor
		{
			break;
		}
		 
		for (int i = 0; i < CONNECTION_SIZE; i++)
		{
			// STEP 1: Check connection status is disconnect
			if(ptDBInfo->tConInfo[i].eStatus == eDisconnectd)
			{
				// STEP 2: Delete connection
				scRet = DataBroker_DeleteConnection(ptDBInfo->hDataBroker, &ptDBInfo->tConInfo[i].hConn);

				if (IS_FAIL(scRet))
				{
					printf("DataBroker_DeleteConnection failed with error %X in MonitorThread\n", scRet);
					pthread_exit(NULL);
				}

				// STEP 3: Create connection
				scRet = DataBroker_CreateConnection(ptDBInfo->hDataBroker, &ptDBInfo->tConInfo[i].hConn);

				if (IS_FAIL(scRet))
				{
					printf("DataBroker_CreateConnection failed with error %X in MonitorThread\n", scRet);
					pthread_exit(NULL);
				}

				// STEP 4: Set option to connection
				scRet = DataBroker_SetConnectionOptions(ptDBInfo->tConInfo[i].hConn, &ptDBInfo->tConInfo[i].tOpt);

				if (IS_FAIL(scRet))
				{
					printf("DataBroker_SetConnectionOptions failed with error %X in MonitorThread\n", scRet);
					pthread_exit(NULL);
				}

				// STEP 5: Set connection status is connecting
				ptDBInfo->tConInfo[i].eStatus = eConnecting;

				// STEP 6: Connect to device, this function is an asynchronous function, 
				scRet = DataBroker_Connect(ptDBInfo->tConInfo[i].hConn);

				if (IS_FAIL(scRet))
				{
					printf("DataBroker_Connect failed with error %X in MonitorThread\n", scRet);
					pthread_exit(NULL);
				}
			}
		}
		
		Sleep(500);		
	}

	pthread_exit(NULL);
}

#endif



int main()
{
	TCHAR szIP[CONNECTION_SIZE][MAX_IP_LENGTH] = {0};
	TCHAR szUserName[CONNECTION_SIZE][MAX_PASSWORD_LENGTH] = {0};
	TCHAR szPassword[CONNECTION_SIZE][MAX_USERNAME_LENGTH] = {0};
	TCHAR szServerType[CONNECTION_SIZE][MAX_SERVERTYPE_LENGTH] = {0};

	SCODE scRet = S_FAIL;
	TDataBrokerInfo tDBInfo;

	#ifdef WIN32
	HANDLE hThread = NULL;
        #else
	pthread_t hThread;
	void* pThreadResule;
        #endif

	int nMaxChannel = 32;
	
	
	// STEP 1: Initial DataBroker
	scRet = DataBroker_Initial(&tDBInfo.hDataBroker, nMaxChannel, DataBrokerStatusCallBack, DataBrokerAVCallBack, mctALLCODEC, 0, DATABROKER_VERSION);
	
	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Initial failed with error %X \n", scRet);
		return -1;
	}
	
	// STEP 2: Create connection

	for (int i = 0; i < CONNECTION_SIZE; i++)
	{
	   scRet = DataBroker_CreateConnection(tDBInfo.hDataBroker, &tDBInfo.tConInfo[i].hConn);

	   if (IS_FAIL(scRet))
	   {
	     	printf("DataBroker_CreateConnection failed with error %X \n", scRet);
	    	return -1;
	   }
	}

	// STEP 3: Set option to connection

	for(int i = 0; i < CONNECTION_SIZE; i++)
	{
		printf("Connection %d: \n", i);
		tDBInfo.tConInfo[i].tOpt.pzIPAddr = szIP[i];
		tDBInfo.tConInfo[i].tOpt.pzUID = szUserName[i];
		tDBInfo.tConInfo[i].tOpt.pzPWD = szPassword[i];
		tDBInfo.tConInfo[i].tOpt.pzServerType = szServerType[i];

		CreateConnectionOption(&tDBInfo.tConInfo[i].tOpt, &tDBInfo.tConInfo[i]);               
		scRet = DataBroker_SetConnectionOptions(tDBInfo.tConInfo[i].hConn, &tDBInfo.tConInfo[i].tOpt);

		if (IS_FAIL(scRet))
		{
			printf("DataBroker_SetConnectionOptions failed with error %X \n", scRet);
			return -1;
		}
	}

	// STEP 4: Connect to device, this function is an asynchronous function, 
	//		   the connection information should be get in StatusCallback function we set.

	for(int i = 0; i < CONNECTION_SIZE; i++)
	{
		tDBInfo.tConInfo[i].eStatus = eConnecting;
		scRet = DataBroker_Connect(tDBInfo.tConInfo[i].hConn);
		
		if (IS_FAIL(scRet))
		{
			printf("DataBroker_Connect failed with error %X \n", scRet);
			return -1;
		}
	}

	#ifdef WIN32
        // STEP 5: Create event for notifying the monitor thread to exit
	tDBInfo.hExit = CreateEvent(NULL, FALSE, FALSE, TEXT("ExitEvent"));
	// STEP 6: Create a thread for monitor reconnect and pass TDataBrokerInfo to MoniterThread.
	hThread = CreateThread(NULL, 0, MonitorThread, &tDBInfo, 0, NULL);  
	#else
	pthread_create(&hThread , NULL, MonitorThread, &tDBInfo);   
	#endif

	// Press any key to stop the receiving.
	while ( !_kbhit() )
	{
		// sleep 16 ms 
		Sleep(16);
	}


         #ifdef WIN32
	// STEP 7: Set exit event and wait monitor thread terminate.
	SetEvent(tDBInfo.hExit);
	WaitForSingleObject(hThread, INFINITE);

	// STEP 8: Close exit event 
	CloseHandle(tDBInfo.hExit);
    
        #else
	
	tDBInfo.bExit = true;
	pthread_join(hThread, &pThreadResule);
    
        #endif

	for(int i = 0; i < CONNECTION_SIZE; i++)
	{
		// STEP 9: Disconnect to device
		scRet = DataBroker_Disconnect(tDBInfo.tConInfo[i].hConn);

		if (IS_FAIL(scRet))
		{
			printf("DataBroker_Disconnect failed with error %X \n", scRet);
			return -1;
		}

		// STEP 10: Delete connection
		scRet = DataBroker_DeleteConnection(tDBInfo.hDataBroker, &tDBInfo.tConInfo[i].hConn);

		if (IS_FAIL(scRet))
		{
			printf("DataBroker_DeleteConnection failed with error %X \n", scRet);
			return -1;
		}
	}

	// STEP 11: Release DataBroker
	scRet = DataBroker_Release(&tDBInfo.hDataBroker);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Release failed with error %X \n", scRet);
		return -1;
	}

	return 0;
}

