#ifndef _CONTROL_COMMON_H_
#define _CONTROL_COMMON_H_

#define CTRL_COMMON_VERSION_MAJOR		1
#define CTRL_COMMON_VERSION_MINOR		0
#define CTRL_COMMON_VERSION_BUILD		0
#define CTRL_COMMON_VERSION_REVISION	4

/// the maximum length of name for private mask or motion detection window.
#define	SRVCTRL_MAX_WINDOWNAME		14

/// the maximum length of host name or IP of server to be connected to.
#define	SRVCTRL_MAX_HOST_LEN		128

/// the maximum length of user name.
#define	SRVCTRL_MAX_USERNAME_LEN	64

/// the maximum length of password.
#define	SRVCTRL_MAX_USERPASS_LEN	64

#define SRVCTRL_MAX_BUF_SIZE		1024

#define SRVCTRL_SRV_RESERVE_BUF_SIZE 30

/**
  * \brief The enumeration to define the DiDo status 
  */
typedef enum
{
	/// The DI/DO level is not set for this index
	eSrvCtrlDiDoNone = 0,

	/// The DI/DO level is low
	eSrvCtrlDiDoLow = 1,

	/// The DI/DO level is high
	eSrvCtrlDiDoHigh = 2,
} ESrvCtrlDiDoLevel;

/**
  * \brief The enumeration to define the PTZ type 
  */
typedef enum
{
	eSrvCtrlPTZFirst = 0,
	/// Use physical PTZ command
	eSrvCtrlPPTZ = eSrvCtrlPTZFirst,

	/// Use digital PTZ command
	eSrvCtrlEPTZ = 1,
	
	eSrvCtrlPTZLast = eSrvCtrlEPTZ,
} ESrvCtrlPTZType;

/**
  * \brief The enumeration for PTZ command
  */
typedef enum 
{
	/// move camera up
	eSrvCtrlPTZMoveUp = 1,

	/// move camera down
	eSrvCtrlPTZMoveDown,

	/// move camera left
	eSrvCtrlPTZMoveLeft,

	/// move camera right
	eSrvCtrlPTZMoveRight,

	/// move camera home
	eSrvCtrlPTZMoveHome,

	/// zoom in the camera
	eSrvCtrlPTZZoomIn,

	/// zoom out the camera
	eSrvCtrlPTZZoomOut,

	/// move focus nearer
	eSrvCtrlPTZFocusNear,

	/// move focus further
	eSrvCtrlPTZFocusFar,

	/// auto focus
	eSrvCtrlPTZFocusAuto,

	/// save current position to a given name as preset location
	eSrvCtrlPTZPresetAdd,

	/// delete a preset location by name
	eSrvCtrlPTZPresetDel,

	/// recall a preset location
	eSrvCtrlPTZPresetRecall,

	/// set pan spped
	eSrvCtrlPTZPanSpeed,

	/// set tilt spped
	eSrvCtrlPTZTiltSpeed,

	/// let PT(Z) camera to pan automatically
	eSrvCtrlPTZStartPan,

	/// stop panning of a PT(Z) camera
	eSrvCtrlPTZStopPan,

	/// let PT(Z) camera to patrol, the partol position is set in camera
	eSrvCtrlPTZStartPatrol,

	/// Let more light comes into the camera
	eSrvCtrlPTZIrisOpen,

	/// Let fewer light comes into the camera
	eSrvCtrlPTZIrisClose,

	/// Auto iris
	eSrvCtrlPTZIrisAuto,

	/// set zoom speed
	eSrvCtrlPTZZoomSpeed,

	/// Set define home
	eSrvCtrlPTZSetHome,

	/// Set back to default home
	eSrvCtrlPTZDefaultHome,

	/// Focus speed
	eSrvCtrlPTZFocusSpeed,

	/// call custom command
	eSrvCtrlPTZCustom,	

} ESRVCTRL_PTZ_COMMAND;


/**
  * \brief The enumeration for Joystick PTZ command
  */
typedef enum 
{
	/// move camera up
	eJoystickZoomStop,
	eJoystickZoomWide,
	eJoystickZoomTele,
} EJOYSTICK_ZOOMING_COMMAND;


/** 
  * \brief The structure that is used to hold the version information.
  */
typedef struct _Version_Info
{
	/// Major version number
	WORD wMajor;

	/// Minor version number
	WORD wMinor;

	/// The build number
	WORD wBuild;

	/// the revision number
	WORD wRevision;
} TVersionInfo;

/** 
  * \brief The structure that is used to hold the private mask window information
  */
typedef struct 
{
	/// The name for the window, it only lets user to be able to distinguish the window
	char	szName[SRVCTRL_MAX_WINDOWNAME + 1];

	/// The X coordinate of the left-upper corner
	int		nX;

	/// The Y coordinate of the left-upper corner
	int		nY;

	/// The width of the window
	int		nW;

	/// The height of the window
	int		nH;

	/// If the window is enable or not.
	BOOL	bWindowEnabled;	// false means the window is not shown when using plugin
} TSRVCTRL_PRIVATEMASK_INFO;

/** 
  * \brief The structure that is used to hold the motion detection window information
  */
typedef struct 
{
	/// The name for the window, it only lets user to be able to distinguish the window
	char	szName[SRVCTRL_MAX_WINDOWNAME + 1];

	/// The X coordinate of the left-upper corner
	int		nX;

	/// The Y coordinate of the left-upper corner
	int		nY;

	/// The width of the window
	int		nW;

	/// The height of the window
	int		nH;

	/// The motion detection percent setting
	int		nPercent;

	/// The motion detection sensitivity setting
	int		nSensitivity;

	/// If the window is enable or not.
	BOOL	bWindowEnabled;	// false means the window is not shown when using plugin
} TSRVCTRL_MODETECT_INFO;


/** 
  * \brief The structure that is used to specify the device property
  */
typedef struct _SRVCTRL_DEV_PROPERTY
{
	/// The host name or IP of server to be connected to.
	char	szHost[SRVCTRL_MAX_HOST_LEN + 1];

	/// user name for the server
	char	szUserName[SRVCTRL_MAX_USERNAME_LEN + 1];

	/// password for the server
	char	szPassword[SRVCTRL_MAX_USERPASS_LEN + 1];

	/// http port for the server
	DWORD	dwHttpPort;	// 0 means to use default 80

	/// Sam 2007/05/22 ftp port for the server
	DWORD	dwFtpPort;	// 0 means to use default 21

	/// The timeout for the operation in milliseconds
	DWORD	dwTimeout;	// timeout value in milliseconds

	// sam 2007/06/20 support ssl
	/// Be sue HTTPS connection protocol
	BOOL	bUseSSL;

} TSRVCTRL_DEV_PROPERTY, *PTSRVCTRL_DEV_PROPERTY;

#if defined _WIN32 || defined _WIN32_WCE
typedef struct _SRVCTRL_DEV_PROPERTYW
{
	/// The host name or IP of server to be connected to.
	wchar_t	szHost[SRVCTRL_MAX_HOST_LEN + 1];

	/// user name for the server
	wchar_t	szUserName[SRVCTRL_MAX_USERNAME_LEN + 1];

	/// password for the server
	wchar_t	szPassword[SRVCTRL_MAX_USERPASS_LEN + 1];

	/// http port for the server
	DWORD	dwHttpPort;	// 0 means to use default 80

	/// Sam 2007/05/22 ftp port for the server
	DWORD	dwFtpPort;	// 0 means to use default 21

	/// The timeout for the operation in milliseconds
	DWORD	dwTimeout;	// timeout value in milliseconds

	// sam 2007/06/20 support ssl
	/// Connect to server by using SSL protocol
	BOOL	bUseSSL;
	
} TSRVCTRL_DEV_PROPERTYW, *PTSRVCTRL_DEV_PROPERTYW;

#endif //_WIN32 || _WIN32_WCE

/** 
  * \brief The structure that is used to hold the notified data when callback is called.
  */
typedef struct _SVRCTRL_NOTIFY_CONT
{
	/// The device that is associated with this callback
	HANDLE		hDevice;

	/// The context for the notification. 
	void		*pvContext;

	/// The status code for the callback
	SCODE		scStatusCode;

	/// extra parameter 1 for each status code if any
	void		*pvParam1;

	/// extra parameter 2 for each status code if any
	void		*pvParam2;
} TSVRCTRL_NOTIFY_CONT; 

typedef SCODE (__stdcall *SVRCTRL_STATUS_CALLBACK)(TSVRCTRL_NOTIFY_CONT *ptContent);

/** 
  * \brief The structure that is used to specify the notification interface and parameter for asynchronous
  * operation.
  */
typedef struct _SVRCTRL_NOTIFY
{
	/// The callback function for certain operation.
	SVRCTRL_STATUS_CALLBACK		pfCB;

	/// The context for that operation.
	void						*pvContext;
} TSVRCTRL_NOTIFY; 

// sam 2008/11/20
/** 
  * \brief The structure that is used to specify the proxy property
  */
typedef struct _SRVCTRL_DEV_PROXY
{
	/// The host name or IP of proxy server.
	char	szProxyHost[SRVCTRL_MAX_HOST_LEN + 1];

	/// user name for the proxy server
	char	szProxyUserName[SRVCTRL_MAX_USERNAME_LEN + 1];

	/// password for the proxy server
	char	szProxyPassword[SRVCTRL_MAX_USERPASS_LEN + 1];

	/// proxy port for the server
	DWORD	dwProxyPort;	// 0 means to use default 80

	/// Be use proxy
	BOOL	bUseProxy;	

} TSRVCTRL_DEV_PROXY, *PTSRVCTRL_DEV_PROXY;

#if defined _WIN32 || defined _WIN32_WCE
typedef struct _SRVCTRL_DEV_PROXYW
{
	/// The host name or IP of proxy server.
	wchar_t	szProxyHost[SRVCTRL_MAX_HOST_LEN + 1];

	/// user name for the proxy server
	wchar_t	szProxyUserName[SRVCTRL_MAX_USERNAME_LEN + 1];

	/// password for the proxy server
	wchar_t	szProxyPassword[SRVCTRL_MAX_USERPASS_LEN + 1];

	/// proxy port for the server
	DWORD	dwProxyPort;	// 0 means to use default 80

	/// Be use proxy
	BOOL	bUseProxy;	

} TSRVCTRL_DEV_PROXYW, *PTSRVCTRL_DEV_PROXYW;
#endif ////_WIN32 || _WIN32_WCE

#endif // _CONTROL_COMMON_H_
