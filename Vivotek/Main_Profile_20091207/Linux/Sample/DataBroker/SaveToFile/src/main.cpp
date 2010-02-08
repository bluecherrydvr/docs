#include <string.h>
#include "DataBroker.h"
#include "OSDefine.h"


#define MAX_IP_LENGTH 256
#define MAX_USERNAME_LENGTH 256
#define MAX_PASSWORD_LENGTH 256
#define MAX_SERVERTYPE_LENGTH 16

#ifndef HTONL
#define HTONL(x) \
	(((DWORD)x) << 24) + \
	((((DWORD)x) & 0xff00) << 8) + \
	((((DWORD)x) >> 8) & 0xff00) + \
	(((DWORD)x) >> 24)
#endif

// This callback function is used to receive media packet, the calling convention must be stdcall
SCODE __stdcall DataBrokerAVCallBack(DWORD dwContext,TMediaDataPacketInfo *ptMediaDataPacket)
{
   // Cast dwContext to FILE handle
   FILE* fp = (FILE *) dwContext;
   
   TMediaDataPacketInfoV3 *ptPacketV3 = (TMediaDataPacketInfoV3 *) ptMediaDataPacket;

   if (fp)
   {
    // STEP 1: write out pbyTLVExt
	if (ptPacketV3->tIfEx.tRv1.tExt.pbyTLVExt)
	{
		fwrite(ptPacketV3->tIfEx.tRv1.tExt.pbyTLVExt, 1, ptPacketV3->tIfEx.tRv1.tExt.dwTLVExtLen, fp);
	}
	else
	{
		DWORD dwDummyData = 0;
		fwrite(&dwDummyData, 1, sizeof(dwDummyData), fp);
	}

	// STEP 2: write out pbyBuff
	if (ptPacketV3->tIfEx.tInfo.pbyBuff)
	{
		fwrite(ptPacketV3->tIfEx.tInfo.pbyBuff, 1, ptPacketV3->tIfEx.tInfo.dwOffset + ptPacketV3->tIfEx.tInfo.dwBitstreamSize, fp);
	}
	else
	{
		DWORD dwDummyData = 0;
		fwrite(&dwDummyData, 1, sizeof(dwDummyData), fp);
	}

	// STEP 3: write out pbyVExtBuf
	if (ptPacketV3->pbyVExtBuf)
	{
		DWORD dwBEVExtLen = HTONL(ptPacketV3->dwVExtLen);
		fwrite(&dwBEVExtLen, 1, sizeof(DWORD), fp);
		fwrite(ptPacketV3->pbyVExtBuf, 1, ptPacketV3->dwVExtLen, fp);
	}
	else
	{
		DWORD dwDummyData = 0;
		fwrite(&dwDummyData, 1, sizeof(dwDummyData), fp);
	}
   }
  
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

void CreateConnectionOption(TDataBrokerConnectionOptions *ptOpt, FILE* fp)
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
	ptOpt->dwAVContext = (DWORD) fp;

	//Set status callback to connection option
	ptOpt->pfStatus = DataBrokerStatusCallBack;

	//Set Audio/Video callback to connection option
	ptOpt->pfAV = DataBrokerAVCallBack;

	//Set dwFlags to tell DataBroker what options we set.
	ptOpt->dwFlags = eConOptProtocolAndMediaType | eConOptHttpPort | 
		(eConOptStatusCallback | eConOptStatusContext) |
		(eConOptAVCallback | eConOptAVContext);
}


int main()
{
	FILE* fp = NULL;
	TCHAR szIP[MAX_IP_LENGTH] = {0};
	TCHAR szUserName[MAX_PASSWORD_LENGTH] = {0};
	TCHAR szPassword[MAX_USERNAME_LENGTH] = {0};
	TCHAR szServerType[MAX_SERVERTYPE_LENGTH] = {0};

	SCODE scRet = S_FAIL;
	HANDLE	hDataBroker = NULL;
	HANDLE	hConn = NULL;
	int nMaxChannel = 32;
		
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
    // Open file handle pass to DataBrokerAVCallBack context
	fp = fopen("SaveFile.raw","wb");

	if (fp == NULL)
	{
		printf("Open file failed \n");
		return -1;
	}

	TDataBrokerConnectionOptions tOpt;
	memset(&tOpt, 0, sizeof(tOpt));
	tOpt.pzIPAddr = szIP;
	tOpt.pzUID = szUserName;
	tOpt.pzPWD = szPassword;
        tOpt.pzServerType = szServerType;
  
        CreateConnectionOption(&tOpt, fp);               
	scRet = DataBroker_SetConnectionOptions(hConn, &tOpt);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_SetConnectionOptions failed with error %X \n", scRet);
		return -1;
	}

	// STEP 4: Connect to device, this function is an asynchronous function, 
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
		
	// STEP 5: Disconnect to device
	scRet = DataBroker_Disconnect(hConn);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Disconnect failed with error %X \n", scRet);
		return -1;
	}

    // STEP 6: Delete connection
    scRet = DataBroker_DeleteConnection(hDataBroker, &hConn);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_DeleteConnection failed with error %X \n", scRet);
		return -1;
	}

	// STEP 7: Release DataBroker
	scRet = DataBroker_Release(&hDataBroker);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Release failed with error %X \n", scRet);
		return -1;
	}
    
	// STEP 8: Close file handle
	fclose(fp);

	return 0;
}
