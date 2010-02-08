#include "stdafx.h"
#include <windows.h>
#include <conio.h>
#include <iostream>
#include "ATCP20.h"

/************************************************************************/
/* This function will handle all the control events                                                                     */
/************************************************************************/
void WINAPI ControlDataCB(DWORD UserParam, void* pData, DWORD nDataLength)
{
	DWORD dwTag = 0;
	DWORD dwtmp = 0;
	BYTE* bData = NULL;
	BYTE btTmp = 0;
	if( nDataLength < 4 || nDataLength > 25600 )
		return;
	char sz[1024] = {0};
	int n = 0;
	int i;

	bData = new BYTE[nDataLength];
	memcpy( bData, pData, nDataLength );
	memcpy( &dwTag, bData, 4 );

	switch( dwTag )
	{
	case NET_RSP_CONTROL_DI:
	case NET_RSP_DI:
		btTmp = 0x03 ^ bData[8];		
		if(0x00 == btTmp)				// No DI
		{
			printf("DI off\n");
		}
		else if(0x01 == btTmp)			// Check for DI 1
		{
			printf("DI 1\n");
		}
		else if(0x02 == btTmp)			// Check for DI 2
		{
			printf("DI 2\n");
		}
		else if(0x03 == btTmp)			// Check for both DI 1 and 2
		{
			printf("DI 1 & 2\n");
		}
		break;
	case NET_RSP_CONTROL_MD:
	case NET_RSP_MD:
		btTmp = bData[8] & 0x0E;		
		if(btTmp & 0x02)				// Check for motion 1
		{
			printf("Motion 1\n");
		}
		if(btTmp & 0x04)				// Check for motion 2
		{
			printf("Motion 2\n");
		}
		if(btTmp & 0x08)				// Check for motion 3
		{
			printf("Motion 3\n");
		}
		break;
	case NET_RSP_CONTROL_VLOSS:
	case NET_RSP_VIDEOLOST:
		if(0 == bData[8])							// Video loss
		{
			printf("Receive video loss\n");
		}
		else if(1 == bData[8])						// Video recovery
		{
			printf("Receive video recovery\n");
		}
		break;
	case NET_RSP_CONTROL_SERIAL :
	case NET_RSP_RS232_RECV :
		memcpy( &dwtmp, &bData[4], 4 );
		for(i = 0; i < dwtmp; i++)
		{
			n += sprintf( &sz[n], "0x%02x ", bData[8+i]);
		}
		printf("Receive RS232 response:[%s]\n", sz);
		break;
	case NET_CONTROL_ONLY_NETWORK_LOSS:
		printf("Receive network loss\n");			// Network loss
		break;
	default:
		break;
	}
	if( bData )
		delete [] bData;
}

/************************************************************************/
/* Main function 
/* Shows how to connect to device using ATCP20.
/* press 'q' to quit the program.
/* press 's' to send PTZ command (might need to change depend on protocol).                                                              */
/************************************************************************/
int main(int argc, char* argv[])
{
	WSADATA wsaData;
	HANDLE hThread = NULL;
	HANDLE hTCP20 = NULL;
	hTCP20 = XInit();
	bool bStart = false;

	if(NULL != hTCP20)
	{
		WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
		// Set connect information
		MEDIA_CONNECTION_CONFIG mcc;
		memset(&mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));// Init. the structure.
		mcc.ContactType = CONTACT_TYPE_CONTROL_ONLY;		// We are only going to connect to control port
		strcpy(mcc.UniCastIP, "172.16.1.82");               // Connect IP address
		mcc.RegisterPort = 6000;							// Register port number
		mcc.ControlPort = 6001;								// Control port number
		mcc.StreamingPort = 6002;							// Stream port number
		strcpy(mcc.UserID, "Admin");						// Login ID
		strcpy(mcc.Password, "123456");						// Login password
		mcc.HTTPPort = 80;									// HTTP port number.
		XSetMediaConfig(hTCP20, &mcc);						// Set connection config to object
		if(XConnect(hTCP20))								// Connect to device.
		{
			if(XStartStreaming(hTCP20))						// Start streaming
			{
				std::cout<<"StartStreaming OK"<<std::endl;		
				bStart = true;
				XSetControlDataCallBack(hTCP20, 1, (CONTROL_DATA_CALLBACK)ControlDataCB);		// Set call back function
			}
			else
			{
				std::cout<<"StartStreaming fail"<<std::endl;
			}
		}
		else
		{
			std::cout<<"Connect fail"<<std::endl;
		}
	}
	else 
	{
		std::cout<<"Unable to create handle"<<std::endl;
	}

	if(bStart)
	{
		std::cout<<"running, press 'q' to quit"<<std::endl;
		int nch;
		while(nch = _getch())
		{
			if('q' == nch || 'Q' == nch)													// Quit application
				break;
			if('s' == nch || 'S' == nch)													// Send serial command
			{
				// Send two PTZ command
				// 1. Turn right
				// 2. Stop PTZ.
				MEDIA_COMMAND mcPTZRight;
				MEDIA_COMMAND mcPTZStop;
				// PTZ protocol DynaColor_DynaColor
				BYTE PANRIGHT[6] = {0x01,0x00,0x18,0x00,0x08,0x11};			// PTZ right
				BYTE PANSTOP[6] = {0x01,0x00,0x13,0x00,0x00,0x12};			// PTZ stop
				memset(&mcPTZRight, 0x00, sizeof(MEDIA_COMMAND));
				memset(&mcPTZStop, 0x00, sizeof(MEDIA_COMMAND));
				mcPTZRight.dwCommandType = NET_REQ_CONTROL_SERIAL;			// Set command type for serial.
				mcPTZRight.pCommand = new char[7];	
				memset(mcPTZRight.pCommand, 0x00, 7);
				mcPTZRight.nCommandLength = 6;
				memcpy((void*)mcPTZRight.pCommand, PANRIGHT, 6);			// Actual command
				XSendCommand(hTCP20, &mcPTZRight);							// Send command to turn camera
				if( mcPTZRight.pCommand )
					delete [] mcPTZRight.pCommand;							// Clean up

				Sleep(1500);												// Let it turn for 1.5 Seconds

				mcPTZStop.dwCommandType = NET_REQ_CONTROL_SERIAL;			// Set command type for serial.
				mcPTZStop.pCommand = new char[7];	
				memset(mcPTZStop.pCommand, 0x00, 7);
				mcPTZStop.nCommandLength = 6;
				memcpy((void*)mcPTZStop.pCommand, PANSTOP, 6);				// Actual command
				XSendCommand(hTCP20, &mcPTZStop);							// Send command to stop camera
				if( mcPTZStop.pCommand )
					delete [] mcPTZStop.pCommand;							// Clean up
			}
			Sleep(100);
		}
	}

	std::cout<<"terminating, please wait ..."<<std::endl;
	
	// Clean up object
	if(NULL != hTCP20)
	{
		XSetControlDataCallBack(hTCP20, 0, NULL);
		XStopStreaming(hTCP20);		
		XDisconnect(hTCP20);
		XExit(hTCP20);
		hTCP20 = NULL;
	}
	WSACleanup();
	Sleep(1000);
	return 0;
}
