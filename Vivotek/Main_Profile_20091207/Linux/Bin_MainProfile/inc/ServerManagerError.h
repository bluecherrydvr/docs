#ifndef _SVRMNGR_ERROR_DEF_H_
#define	_SVRMNGR_ERROR_DEF_H_
#include "errordef.h"
#define	SVRMNGR_BASE				0
#define	SVRMNGR_END					0x3F
#define SVRMNGR_E_FIRST				MAKE_SCODE(1, 9, SVRMNGR_BASE)
#define SVRMNGR_E_LAST				MAKE_SCODE(1, 9, SVRMNGR_END)
#define SVRMNGR_S_FIRST				MAKE_SCODE(0, 9, SVRMNGR_BASE)
#define SVRMNGR_S_LAST				MAKE_SCODE(0, 9, SVRMNGR_END)

/// It has created too many devices in ServerManager
#define SVRMNGR_E_TOOMANY_DEVICE			(SVRMNGR_E_FIRST + 0)

/// ServerController library is not loaded successfully
#define SVRMNGR_E_NOTLOAD_SRVCTRLLIB		(SVRMNGR_E_FIRST + 1)

/// ServerUtility library is not loaded successfully
#define SVRMNGR_E_NOTLOAD_SRVUTILLIB		(SVRMNGR_E_FIRST + 2)

/// ServerManager is not open device yet (not call ServerManager_OpenDeviceBlock or ServerManager_OpenDevice)
#define SVRMNGR_E_DEVICE_NOT_OPEN			(SVRMNGR_E_FIRST + 3)

/// It may not find the appropriate function to work in some conditions
#define SVRMNGR_E_PARTIAL_NOT_IMPLEMENT		(SVRMNGR_E_FIRST + 4)

/// It can not find corresponding ServerUtility config item index of parameter item in RX (new model)
#define SVRMNGR_E_NOT_MAPPING				(SVRMNGR_E_FIRST + 5)

/// It can not find the item in functions ServerManager_SetValueByName and ServerManager_GetValueByName
#define SVRMNGR_E_KEY_NOT_FOUND				(SVRMNGR_E_FIRST + 6)

/// This is warning that you may do ServerManager_UpdateRemoteConfigBlock or ServerManager_UpdateRemoteConfig first
#define SVRMNGR_W_MUST_UPDATEREMOTE_FIRST	(SVRMNGR_E_FIRST + 7)

/// ServerManager occur timeout
#define SVRMNGR_E_TIMEOUT					(SVRMNGR_E_FIRST + 8)

/// It occurs authentication error when trying to connect to server
#define SVRMNGR_E_AUTH						(SVRMNGR_E_FIRST + 9)

/// Server is not exit, but has this ip address
#define SVRMNGR_E_CONNECT_FAILED			(SVRMNGR_E_FIRST + 10)

/// It can not find the page in Http request
#define SVRMNGR_E_PAGE_NOT_FOUND			(SVRMNGR_E_FIRST + 11)

/// error model between ServerCtrl and ServerUtil
#define SVRMNGR_E_ERROR_MODEL				(SVRMNGR_E_FIRST + 12)

/// Convert unicode to multibyte error
#define SVRMNGR_E_CONVERT_UNITOMB			(SVRMNGR_E_FIRST + 13)

/// The data that read from server is something wrong
#define SVRMNGR_E_READ_DATA_ERROR			(SVRMNGR_E_FIRST + 14)

/// Get data from ftp failed
#define SVRMNGR_E_FTP_GETFILE				(SVRMNGR_E_FIRST + 15)

/// Put data to ftp failed	
#define SVRMNGR_E_FTP_PETFILE				(SVRMNGR_E_FIRST + 16)

/// Open file failed	
#define SVRMNGR_E_OPENFILE					(SVRMNGR_E_FIRST + 17)

/// The request is aborted
#define SVRMNGR_E_ABORTING					(SVRMNGR_E_FIRST + 18)

/// Both libraries are not loaded
#define SVRMNGR_E_LOAD_LIBRARY				(SVRMNGR_E_FIRST + 19)

/// Too many non-block operations are proceeding
#define SVRMNGR_E_TOOMANY_NONBLOCK_OPER		(SVRMNGR_E_FIRST + 20)

/// The error code for name value is not valid
#define	SVRMNGR_E_INVALID_NAME				(SVRMNGR_E_FIRST + 21)

/// The error code for the buffer size is not enough to hold the value
#define	SVRMNGR_E_VALUE_LENGTH				(SVRMNGR_E_FIRST + 22)

/// The error code for the key name is too long
#define	SVRMNGR_E_KEYNAME_LENGTH			(SVRMNGR_E_FIRST + 23)

/// The 
#define SVRMNGR_S_BUFFER_SIZE				(SVRMNGR_S_FIRST + 0)
#endif

