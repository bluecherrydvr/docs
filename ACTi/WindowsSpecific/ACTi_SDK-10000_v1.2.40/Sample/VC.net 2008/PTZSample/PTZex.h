#pragma once
// #include "cPTZ.h"

enum PTZ_POST_MODE{SerialPort = 1, URL};

/*
enum PTZ_COMMAND
{
	PTZ_COMMAND_INIT = 1,
	PTZ_MOVE_LEFT, 
	PTZ_MOVE_RIGHT, 
	PTZ_MOVE_UP, 
	PTZ_MOVE_DOWN, 
	PTZ_MOVE_STOP, 
	PTZ_MOVE_HOME, 
	PTZ_OSD_ON, 
	PTZ_OSD_OFF, 
	PTZ_OSD_UP, 
	PTZ_OSD_DOWN, 
	PTZ_OSD_LEFT, 
	PTZ_OSD_RIGHT,
	PTZ_OSD_LEAVE, 
	PTZ_OSD_STOP, 
	PTZ_OSD_ENTER,
	PTZ_FOCUS_IN, 
	PTZ_FOCUS_OUT, 
	PTZ_FOCUS_STOP,
	PTZ_IRIS_OPEN, 
	PTZ_IRIS_CLOSE, 
	PTZ_IRIS_STOP, 
	PTZ_IRIS_AUTO,
	PTZ_PRESET_SET, 
	PTZ_PRESET_GOTO, 
	PTZ_PRESET_CLEAR, 
	PTZ_PRSSET_TOUR,
	PTZ_ZOOM_IN,
	PTZ_ZOOM_OUT,
	PTZ_ZOOM_STOP,
		PTZ_BLC_ON,
	PTZ_BLC_OFF,
	PTZ_DAYNIGHT_ON,
	PTZ_DAYNIGHT_OFF,
	PTZ_DAYNIGHT_AUTO_ON,
	PTZ_DAYNIGHT_AUTO_OFF,
	PTZ_MOVE_UPLEFT,
	PTZ_MOVE_UPRIGHT,
	PTZ_MOVE_DOWNLEFT,
	PTZ_MOVE_DOWNRIGHT
};
*/



#define PTZAPI // __declspec(dllimport) 


enum PTZ_ENUMERATE { PTZ_ENUM_CATEGORY = 1, 
					 PTZ_ENUM_COMMAND,
					 PTZ_ENUM_VENDER,
					 PTZ_ENUM_PROTOCOL,
					 PTZ_ENUM_PANEL};




// struct
struct PTZInfo
{
	DWORD dwAddrID;			 // Address ID
	PTZ_POST_MODE dwMode;    // SerialPort = 1, URL =2

	char caVender[32];
	char caProtocol[32];
	char caFilename[512];
	char caServerIP[16];
	char caAccount[16];
	char caPassword[16];
};




#ifdef __cplusplus
extern "C" {
#endif

// Create a PTZ device
//PTZAPI HANDLE PTZOpenInterface(PTZInfo *pPTZInfo);
// HANDLE  PTZOpenInterface(char *pcaProtocol, char *pcaServerIP = NULL, char *pcaAccount= NULL, char *pcaPassword= NULL, PTZ_POST_MODE modePost = SerialPort, DWORD addrid = 1);


// Create a device with file
//PTZAPI HANDLE PTZOpenInterfaceWithFile(PTZInfo *pPTZInfo);
// HANDLE  PTZOpenInterfaceWithFile(char *pcaFileName, char *pcaServerIP = NULL, char *pcaAccount= NULL, char *pcaPassword= NULL, PTZ_POST_MODE modePost = SerialPort, DWORD addrid = 1);


// Close a PTZ device Control Handle
//PTZAPI   BOOL PTZCloseInterface(HANDLE hPTZ);					



// Get a array that is the hex and can be sent by serial port.
//PTZAPI BYTE* PTZGetString(HANDLE hPTZ, void *pStrCommand, int tiParam1, int tiParam2, DWORD *pdwLen);

// Get a URL string by PTZ protocol file command
//PTZAPI char* PTZGetStringURL(HANDLE hPTZ, void *pStrCommand, int tiParam1, int tiParam2, DWORD *pdwLen);

// Get Information of the PTZ command
//PTZAPI char* GetURLCommand(char *pcaServerIP, char *pcaAccount, char *pcaPassword, char *pcaCommand);

// URL command
// PTZAPI bool SendURL(char *caCommand);

// Utility
//PTZAPI void* PTZEnumerate(HANDLE hPTZ, PTZ_ENUMERATE Enumerate, DWORD *dwLen);
//PTZAPI void* PTZEnumerateProtocol(char *pVender, DWORD *dwLen);



#ifdef __cplusplus
}
#endif