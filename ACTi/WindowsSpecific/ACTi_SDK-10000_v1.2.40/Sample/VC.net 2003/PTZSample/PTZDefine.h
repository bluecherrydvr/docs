#ifndef	__PTZ_DEFINE_H__
#define	__PTZ_DEFINE_H__

#define PTZ_MEDIA_SOURCE_RS		1
#define PTZ_MEDIA_SOURCE_FILE	2
//
// 10.24.2007 Marty updated
//
typedef HANDLE	(* XPTZOpenInterface		)();
typedef void	(* XPTZCloseInterface		)( HANDLE );
typedef bool	(* XPTZLoadProtocolFile		)( HANDLE, char* );
typedef bool	(* XPTZLoadProtocolRS		)( HANDLE, char* );
typedef bool	(* XPTZUnloadProtocol		)( HANDLE);
typedef bool	(* XPTZGetCommandExt		)( HANDLE, char*, int, int, int, BYTE*, DWORD& );
typedef bool	(* XPTZEnumerateProtocol	)( HANDLE, char*, char*,  DWORD& );
typedef bool	(* XPTZEnumerateVender		)( HANDLE, char*,  DWORD& );
typedef bool	(* XPTZEnumerateFunction	)( HANDLE, char*,  DWORD& );
typedef bool    (* XPTZGetAbsPTZCommand     )( HANDLE, 
											  char*,
											  int,
											  int,
											  bool,
											  float,
											  float,
											  float,
											  BYTE*,
											  DWORD&);

typedef bool    (* XPTZGetAbsPTZCommandByUnit)( HANDLE, 
											  char*,
											  int,
											  int,
											  bool,
											  int,
											  int,
											  int,
											  BYTE*,
											  DWORD&);
typedef void    (* XPTZUnitToDegree)(HANDLE, 
										int,
										int,
										int,
										float&,
										float&,
										float&);
typedef void    (* XPTZDegreeToUnit)(HANDLE, 
										float,
										float,
										float,
										int&,
										int&,
										int&);

typedef bool    (* XPTZGetUnitFromBuffer)(HANDLE, 
										  BYTE*,
										  DWORD,
										  int&,
										  int&,
										  int&);

typedef bool    (* XPTZGetRequestAbsPTZCommand)(HANDLE, 
												int,
												BYTE*,
												DWORD&);


typedef struct structural_MEDIA_PTZ_PROTOCOL
{
	int nSourceType;
	char szVender[32];
	char szProtocol[32];
	char szProtocolFileName[512];
	DWORD dwAddressID;
} MEDIA_PTZ_PROTOCOL;

enum PTZ_MOVE_OPERATION
{
	PTZ_MOVE_UP, 
	PTZ_MOVE_DOWN, 
	PTZ_MOVE_LEFT, 
	PTZ_MOVE_RIGHT, 
	PTZ_MOVE_UP_LEFT, 
	PTZ_MOVE_UP_RIGHT, 
	PTZ_MOVE_DOWN_LEFT,
	PTZ_MOVE_DOWN_RIGHT, 
	PTZ_MOVE_STOP
};

enum PTZ_ZOOM_OPERATION
{
	PTZ_ZOOM_IN,
	PTZ_ZOOM_OUT, 
	PTZ_ZOOM_STOP
};

enum PTZ_RESEST_OPERATION
{
	PTZ_PRESET_SET,
	PTZ_PRESET_GOTO 
};

enum PTZ_OSD_OPERATION
{
	PTZ_OSD_ON, 
	PTZ_OSD_OFF, 
	PTZ_OSD_UP, 
	PTZ_OSD_DOWN,
	PTZ_OSD_LEFT, 
	PTZ_OSD_RIGHT, 
	PTZ_OSD_ENTER, 
	PTZ_OSD_LEAVE,
	PTZ_OSD_STOP
};

enum PTZ_BLC_OPERATION
{
	PTZ_BLC_ON, 
	PTZ_BLC_OFF
};

enum PTZ_DN_OPERATION
{
	PTZ_DN_ON,
	PTZ_DN_OFF,
	PTZ_DN_AUTO_ON,
	PTZ_DN_AUTO_OFF
};

enum PTZ_FOCUS_OPERATION
{
	PTZ_FOCUS_IN, 
	PTZ_FOCUS_OUT,
	PTZ_FOCUS_STOP
};

enum PTZ_IRIS_OPERATION
{
	PTZ_IRIS_OPEN,
	PTZ_IRIS_CLOSE,
	PTZ_IRIS_STOP, 
	PTZ_IRIS_AUTO
};

#endif	// __PTZ_DEFINE_H__