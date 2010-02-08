#include "string.h"
#include "DataBroker.h"
#include "OSDefine.h"

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define MAX_IP_LENGTH 256
#define MAX_USERNAME_LENGTH 256
#define MAX_PASSWORD_LENGTH 256
#define MAX_SERVERTYPE_LENGTH 16
#define MAX_CONNECTIOM 32

// This callback function is used to receive media packet, the calling convention must be stdcall
SCODE __stdcall DataBrokerAVCallBack(DWORD dwContext,TMediaDataPacketInfo *ptMediaDataPacket)
{

   return S_OK;
}

// This callback function is used to receive status info, the calling convention must be stdcall
SCODE __stdcall DataBrokerStatusCallBack(DWORD dwContext, TDataBrokerStatusType tStatusType, PVOID pvParam1, PVOID pvParam2)
{
	switch (tStatusType)
	{
		case eOnConnectionInfo:
		{
			printf("Connection %d: Get eOnConnectionInfo, start to receive packet\n", dwContext);
		}
		break;

		case eOnStopped:
		{
			printf("Connection %d:Get eOnStopped, stop to receive packet\n", dwContext);
		}
		break;

		default:
		{
		}	
		break;
	}

   return S_OK;
}

void CreateConnectionOption(TDataBrokerConnectionOptions *ptOpt, int nConnectionIndex, bool bFileMode)
{	
	if (!bFileMode)
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
	}

	//Set connection protocol type to TCP, you may refer to TsdrProtocolType for other protocols.
	ptOpt->dwProtocolType = eptTCP;

	//Set connection media type to Video and Audio.
	ptOpt->dwMediaType =  (emtVideo | emtAudio);

	//Set device server type to auto, the DataBroker will auto detect the server type.
	strcpy(ptOpt->pzServerType, _T("auto"));

	//Pass context to DataBrokerStatusCallBack, the context is useful when we have to handle multi connections simultaneously.
	ptOpt->dwStatusContext = nConnectionIndex;

	//Pass context to DataBrokerAVCallBack, the context is useful when we have to handle multi connections simultaneously.
	ptOpt->dwAVContext = nConnectionIndex;

	//Set status callback to connection option
	ptOpt->pfStatus = DataBrokerStatusCallBack;

	//Set Audio/Video callback to connection option
	ptOpt->pfAV = DataBrokerAVCallBack;

	//Set dwFlags to tell DataBroker what options we set.
	ptOpt->dwFlags = eConOptProtocolAndMediaType | eConOptHttpPort | 
		(eConOptStatusCallback | eConOptStatusContext) |
		(eConOptAVCallback | eConOptAVContext);
}

SCODE CreateConnection(int nConnectionIndex, HANDLE hDataBroker, HANDLE* phConn, TCHAR* pzIP, TCHAR* pzUserName, TCHAR* pzPassword, TCHAR* pzServerType, DWORD dwHTTPPort, bool bFileMode)
{
	// STEP 1: Create connection
	SCODE scRet = DataBroker_CreateConnection(hDataBroker, &phConn[nConnectionIndex]);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_CreateConnection failed with error %X \n", scRet);
		return S_FAIL;
	}

	// STEP 2: Set option to connection
	TDataBrokerConnectionOptions tOpt;
	memset(&tOpt, 0, sizeof(tOpt));
	tOpt.pzIPAddr = pzIP;
	tOpt.pzUID = pzUserName;
	tOpt.pzPWD = pzPassword;
	tOpt.wHttpPort = dwHTTPPort;
	tOpt.pzServerType = pzServerType;

	CreateConnectionOption(&tOpt, nConnectionIndex, bFileMode);               
	scRet = DataBroker_SetConnectionOptions(phConn[nConnectionIndex], &tOpt);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_SetConnectionOptions failed with error %X \n", scRet);
		return S_FAIL;
	}

	// STEP 3: Connect to device, this function is an asynchronous function, 
	//		   the connection information should be get in StatusCallback function we set.
	scRet = DataBroker_Connect(phConn[nConnectionIndex]);

	if (IS_FAIL(scRet))
	{
		printf("DataBroker_Connect failed with error %X \n", scRet);
		return S_FAIL;
	}

	return S_OK;
}



int main()
{
	TCHAR aszIP[MAX_CONNECTIOM * 2][MAX_IP_LENGTH] = {0};
	TCHAR aszUserName[MAX_CONNECTIOM * 2][MAX_PASSWORD_LENGTH] = {0};
	TCHAR aszPassword[MAX_CONNECTIOM * 2][MAX_USERNAME_LENGTH] = {0};
	TCHAR aszServerType[MAX_CONNECTIOM * 2][MAX_SERVERTYPE_LENGTH] = {0};

	SCODE scRet = S_FAIL;
	HANDLE	hDataBroker1 = NULL;
	HANDLE	hDataBroker2 = NULL;
	HANDLE	ahConn1[MAX_CONNECTIOM] = {NULL};
	HANDLE	ahConn2[MAX_CONNECTIOM] = {NULL};
	int nMaxChannel = MAX_CONNECTIOM;
	int nConnectionIndex = 0;

	printf("Press c key to create connection\n");
	printf("Press f key to read Device.ini file \n");
	printf("Press e key to stop connect\n");

	char szbuffer[32] = {0};

	while (1)
	{
		fgets(szbuffer, 32, stdin);

		if (szbuffer[0] == 'c')
		{
			if (nConnectionIndex < MAX_CONNECTIOM)
			{
				if (hDataBroker1 == NULL)
				{
					// STEP 1: Initial DataBroker for 1 to 32 connection
					scRet = DataBroker_Initial(&hDataBroker1, nMaxChannel, DataBrokerStatusCallBack, DataBrokerAVCallBack, mctALLCODEC, 0, DATABROKER_VERSION);

					if (IS_FAIL(scRet))
					{
						printf("DataBroker_Initial failed with error %X \n", scRet);
						return -1;
					}
				}

				// STEP 2:  Create connection and connect to device.
				//          For 1 to 32 connection
				CreateConnection(nConnectionIndex, hDataBroker1, ahConn1, aszIP[nConnectionIndex], aszUserName[nConnectionIndex], aszPassword[nConnectionIndex], aszServerType[nConnectionIndex], 0, false);
				nConnectionIndex++;
			}
			else if (nConnectionIndex < 2 * MAX_CONNECTIOM)
			{
				if (hDataBroker2 == NULL)
				{
					// STEP 3: Initial DataBroker for 33 to 64 connection
					scRet = DataBroker_Initial(&hDataBroker2, nMaxChannel, DataBrokerStatusCallBack, DataBrokerAVCallBack, mctALLCODEC, 0, DATABROKER_VERSION);

					if (IS_FAIL(scRet))
					{
						printf("DataBroker_Initial failed with error %X \n", scRet);
						return -1;
					}
				}

				// STEP 4:  Create connection and connect to device.
				//          For 33 to 64 connection
				CreateConnection(nConnectionIndex - 32, hDataBroker2, ahConn2, aszIP[nConnectionIndex], aszUserName[nConnectionIndex], aszPassword[nConnectionIndex], aszServerType[nConnectionIndex], 0, false);
				nConnectionIndex++;
			}
			else
			{
				printf("Create too many connection !!\n");
			}
		
		}
		else if (szbuffer[0] == 'f')
		{
			BYTE *pbyInBuffer = NULL;
	                DWORD dwInputSize = 0;
			
			DWORD dwChannelID = 0;
			char szHttpPort[10] = {0};

                        //Open Device.ini file to get device information
			FILE* pfInput = fopen("Device.ini", "rb");

			if (pfInput == NULL)
			{
				printf("Open file failed \n");
				break;
			}

			while (EOF != fscanf(pfInput,"[Channel %d]\n", &dwChannelID))
			{
				if (nConnectionIndex < 2* MAX_CONNECTIOM)
				{
					fscanf(pfInput,"IP=%s\n", &aszIP[nConnectionIndex]);
					fscanf(pfInput,"UserName=%s\n", &aszUserName[nConnectionIndex]);
					fscanf(pfInput,"Password=%s\n", &aszPassword[nConnectionIndex]);
					fscanf(pfInput,"HTTPPort=%s\n", &szHttpPort);
				}
				
				if (nConnectionIndex < MAX_CONNECTIOM)
				{
					if (hDataBroker1 == NULL)
					{
						// STEP 1: Initial DataBroker for 1 to 32 connection
						scRet = DataBroker_Initial(&hDataBroker1, nMaxChannel, DataBrokerStatusCallBack, DataBrokerAVCallBack, mctALLCODEC, 0, DATABROKER_VERSION);

						if (IS_FAIL(scRet))
						{
							printf("DataBroker_Initial failed with error %X \n", scRet);
							return -1;
						}
					}

					// STEP 2:  Create connection and connect to device.
					//          For 1 to 32 connection
					CreateConnection(nConnectionIndex, hDataBroker1, ahConn1, aszIP[nConnectionIndex], aszUserName[nConnectionIndex], aszPassword[nConnectionIndex], aszServerType[nConnectionIndex], atoi(szHttpPort), true);
					nConnectionIndex++;
				}
				else if (nConnectionIndex < 2 * MAX_CONNECTIOM)
				{
					if (hDataBroker2 == NULL)
					{
						// STEP 3: Initial DataBroker for 33 to 64 connection
						scRet = DataBroker_Initial(&hDataBroker2, nMaxChannel, DataBrokerStatusCallBack, DataBrokerAVCallBack, mctALLCODEC, 0, DATABROKER_VERSION);

						if (IS_FAIL(scRet))
						{
							printf("DataBroker_Initial failed with error %X \n", scRet);
							return -1;
						}
					}

					// STEP 4:  Create connection and connect to device.
					//          For 33 to 64 connection
					CreateConnection(nConnectionIndex - 32, hDataBroker2, ahConn2, aszIP[nConnectionIndex], aszUserName[nConnectionIndex], aszPassword[nConnectionIndex], aszServerType[nConnectionIndex], atoi(szHttpPort), true);
					nConnectionIndex++;
				}
				else
				{
					printf("Create too many connection !!\n");
					break;
				}
			}

                     //Close file
		    fclose(pfInput); 

		}
		else if (szbuffer[0] == 'e')
		{
			break;
		}
	}
		
	// STEP 4: Disconnect and delete all connection. 
	for (int i = 0; i < MAX_CONNECTIOM; i++)
	{
		if (ahConn1[i] != NULL)
		{
			
			scRet = DataBroker_Disconnect(ahConn1[i]);

			if (IS_FAIL(scRet))
			{
				printf("DataBroker_Disconnect failed with error %X \n", scRet);
				return -1;
			}

			scRet = DataBroker_DeleteConnection(hDataBroker1, &ahConn1[i]);

			if (IS_FAIL(scRet))
			{
				printf("DataBroker_DeleteConnection failed with error %X \n", scRet);
				return -1;
			}
		}

		if (ahConn2[i] != NULL)
		{

			scRet = DataBroker_Disconnect(ahConn2[i]);

			if (IS_FAIL(scRet))
			{
				printf("DataBroker_Disconnect failed with error %X \n", scRet);
				return -1;
			}

			scRet = DataBroker_DeleteConnection(hDataBroker2, &ahConn2[i]);

			if (IS_FAIL(scRet))
			{
				printf("DataBroker_DeleteConnection failed with error %X \n", scRet);
				return -1;
			}
		}
	}
	
	// STEP 4: Release DataBroker

	if (hDataBroker1 != NULL)
	{
		scRet = DataBroker_Release(&hDataBroker1);

		if (IS_FAIL(scRet))
		{
			printf("DataBroker_Release failed with error %X \n", scRet);
			return -1;
		}
	}

	if (hDataBroker2 != NULL)
	{
		scRet = DataBroker_Release(&hDataBroker2);

		if (IS_FAIL(scRet))
		{
			printf("DataBroker_Release failed with error %X \n", scRet);
			return -1;
		}
	}
	
	return 0;
}
