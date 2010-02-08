#include "string.h"
#include "DataBroker.h"
#include "OSDefine.h"

#define MAX_IP_LENGTH	    256
#define MAX_USERNAME_LENGTH 256
#define MAX_PASSWORD_LENGTH 256
#define MAX_SERVERTYPE_LENGTH 16



// This callback function is used to receive media packet, the calling convention must be stdcall
SCODE __stdcall DataBrokerAVCallBack(DWORD dwContext,TMediaDataPacketInfo *ptMediaDataPacket)
{
   // Show time stamp for receive packet
   printf("Time stamp : %u %u \n", ptMediaDataPacket->dwFirstUnitSecond, ptMediaDataPacket->dwFirstUnitMilliSecond);
  
   return S_OK;
}

// This callback function is used to receive status info, the calling convention must be stdcall
SCODE __stdcall DataBrokerStatusCallBack(DWORD dwContext, TDataBrokerStatusType tStatusType, PVOID pvParam1, PVOID pvParam2)
{
	switch (tStatusType)
	{
		case eOnConnectionInfo:
		{
			printf("Get eOnConnectionInfo, start to receive packet\n");
		}
		break;

		case eOnStopped:
		{
			printf("Get eOnStopped, stop to receive packet\n");
		}
		break;

		default:
		{
		}	
		break;
	}

   return S_OK;
}

void CreateConnectionOption(TDataBrokerConnectionOptions *ptOpt)
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
	printf("Please input device HTTP port: ");
	_tscanf("%hu", &(ptOpt->wHttpPort));

	//Set connection protocol type to TCP, you may refer to TsdrProtocolType for other protocols.
	ptOpt->dwProtocolType = eptTCP;

	//Set connection media type to Video and Audio.
	ptOpt->dwMediaType =  (emtVideo | emtAudio);

	//Set device server type to auto, the DataBroker will auto detect the server type.
	strcpy(ptOpt->pzServerType, _T("auto"));

	//Pass context to DataBrokerStatusCallBack, the context is useful when we have to handle multi connections simultaneously.
	ptOpt->dwStatusContext = NULL;

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


void SelectStreamIndex(DWORD *pdwStreamIndex)
{
	DWORD dwStreamID = 0;
	printf("Please input the stream ID (start from 1):");
	_tscanf("%u", &dwStreamID);

	*pdwStreamIndex = dwStreamID - 1;	//Stream index is zero-based.
}

int main()
{
	TCHAR szIP[MAX_IP_LENGTH] = {0};
	TCHAR szUserName[MAX_PASSWORD_LENGTH] = {0};
	TCHAR szPassword[MAX_USERNAME_LENGTH] = {0};
	TCHAR szServerType[MAX_SERVERTYPE_LENGTH] = {0};
	
	SCODE scRet = S_FAIL;
	HANDLE	hDataBroker = NULL;
	HANDLE	hConn = NULL;
	int nMaxChannel = 32;
	DWORD dwStreamIndex = 0;
	
	// STEP 1: Initial DataBroker
	scRet = DataBroker_Initial(&hDataBroker, nMaxChannel, DataBrokerStatusCallBack, DataBrokerAVCallBack, mctALLCODEC, 0, DATABROKER_VERSION);
	
	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Initial failed with error %X \n", scRet);
		return -1;
	}

	
	// STEP 2: Create connection
	scRet = DataBroker_CreateConnection(hDataBroker, &hConn);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_CreateConnection failed with error %X \n", scRet);
		return -1;
	}

	// STEP 3: Set option to connection
	TDataBrokerConnectionOptions tOpt;
	memset(&tOpt, 0, sizeof(tOpt));
	tOpt.pzIPAddr = szIP;
	tOpt.pzUID = szUserName;
	tOpt.pzPWD = szPassword;
	tOpt.pzServerType = szServerType;

        CreateConnectionOption(&tOpt);	

	scRet = DataBroker_SetConnectionOptions(hConn, &tOpt);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_SetConnectionOptions failed with error %X \n", scRet);
		return -1;
	}

	// STEP 4: Set extra option for multiple stream index, the index is zero-based.
	SelectStreamIndex(&dwStreamIndex);
	scRet = DataBroker_SetConnectionExtraOption(hConn, eOptStreamIndex, dwStreamIndex, 0);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_SetConnectionExtraOption  failed with error %X \n", scRet);
		return -1;
	}

        // STEP 5: Connect to device, this function is an asynchronous function, 
	//		   the connection information should be get in StatusCallback function we set.
	scRet = DataBroker_Connect(hConn);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Connect failed with error %X \n", scRet);
		return -1;
	}

        // Press any key to stop the receiving.
	while( !_kbhit() )
	{
		// sleep 16 ms
		Sleep(16);
	}
		

	// STEP 6: Disconnect to device
	scRet = DataBroker_Disconnect(hConn);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Disconnect failed with error %X \n", scRet);
		return -1;
	}

        // STEP 7: Delete connection
        scRet = DataBroker_DeleteConnection(hDataBroker, &hConn);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_DeleteConnection failed with error %X \n", scRet);
		return -1;
	}

	// STEP 8: Release DataBroker
	scRet = DataBroker_Release(&hDataBroker);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Release failed with error %X \n", scRet);
		return -1;
	}

	return 0;
}
