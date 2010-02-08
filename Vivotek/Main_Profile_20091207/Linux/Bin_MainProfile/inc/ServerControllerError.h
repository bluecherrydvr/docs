#ifndef _SVRCTRL_ERROR_DEF_H_
#define	_SVRCTRL_ERROR_DEF_H_

#include "errordef.h"

// ************************** Error code for ServerController *********************************
#define	SERVER_BASE					0
#define	SERVER_END					0xF
#define SERVER_E_FIRST				MAKE_SCODE(1, 8, SERVER_BASE)
#define SERVER_E_LAST				MAKE_SCODE(1, 8, SERVER_END)
#define SERVER_S_FIRST				MAKE_SCODE(0, 8, SERVER_BASE)
#define SERVER_S_LAST				MAKE_SCODE(0, 8, SERVER_END)

/** @ingroup ServerController
  * The error code for number of created device had reach maximum
  */
#define	SERVER_E_MAXIMUM_CREATE_DEVICE		(SERVER_E_FIRST + 0)

/** @ingroup ServerController
  * The error code for number of created token for one device had reach maximum
  */
#define	SERVER_E_MAXIMUM_REQUEST		(SERVER_E_FIRST + 1)

/** @ingroup ServerController
  * The error code for length of model name buffer is not enough
  */
#define	SERVER_E_MODEL_NAME_LENGTH		(SERVER_E_FIRST + 2)

/** @ingroup ServerController
  * The error code for StopRequest is processing
  */
#define	SERVER_E_STOP_REQUEST_PROCESSING	(SERVER_E_FIRST + 3)

/** @ingroup ServerController
  * The error code for request is timeout
  */
#define	SERVER_E_REQUEST_TIMEOUT	(SERVER_E_FIRST + 4)

/** @ingroup ServerController
  * The error code for connecting to old model
  */
#define	SERVER_E_ERROR_MODEL	(SERVER_E_FIRST + 5)

#define SERVER_E_RESOLVE_IP_FAILED		(SERVER_E_FIRST + 6)
/** @ingroup ServerController
  * The error code for stoping requests issue successfully, but not complete yet
  */
#define SERVER_S_STOP_REQUEST_PENDING		(SERVER_S_FIRST + 0)

// ************************** Error code for ServerController *********************************

// ************************** Error code for ServerController_Config *********************************
#define	SERVER_CONFIG_BASE					0x10
#define	SERVER_CONFIG_END					0x1F
#define SERVER_CONFIG_E_FIRST				MAKE_SCODE(1, 8, SERVER_CONFIG_BASE)
#define SERVER_CONFIG_E_LAST				MAKE_SCODE(1, 8, SERVER_CONFIG_END)
#define SERVER_CONFIG_S_FIRST				MAKE_SCODE(0, 8, SERVER_CONFIG_BASE)
#define SERVER_CONFIG_S_LAST				MAKE_SCODE(0, 8, SERVER_CONFIG_END)

/** @ingroup ServerController_Config
  * The error code for name value is not valid
  */
#define	SERVER_E_INVALID_NAME		(SERVER_CONFIG_E_FIRST + 0)

/** @ingroup ServerController_Config
  * The error code for device is not opened yet
  */
#define	SERVER_E_DEVICE_NOT_OPEN	(SERVER_CONFIG_E_FIRST + 1)

/** @ingroup ServerController_Config
  * The error code for the key is not found in local config buffer
  */
#define	SERVER_E_KEY_NOT_FOUND		(SERVER_CONFIG_E_FIRST + 2)

/** @ingroup ServerController_Config
  * The error code for the buffer size is not enough to hold the value
  */
#define	SERVER_E_VALUE_LENGTH		(SERVER_CONFIG_E_FIRST + 3)

/** @ingroup ServerController_Config
  * The error code for the key name is too long
  */
#define	SERVER_E_KEYNAME_LENGTH		(SERVER_CONFIG_E_FIRST + 4)

// ************************** Error code for ServerController_Config *********************************

// ************************** Error code for ServerController_HttpCommand *********************************
#define	SERVER_HTTPCMD_BASE					0x20
#define	SERVER_HTTPCMD_END					0x2F
#define SERVER_HTTPCMD_E_FIRST				MAKE_SCODE(1, 8, SERVER_HTTPCMD_BASE)
#define SERVER_HTTPCMD_E_LAST				MAKE_SCODE(1, 8, SERVER_HTTPCMD_END)
#define SERVER_HTTPCMD_S_FIRST				MAKE_SCODE(0, 8, SERVER_HTTPCMD_BASE)
#define SERVER_HTTPCMD_S_LAST				MAKE_SCODE(0, 8, SERVER_HTTPCMD_END)

/** @ingroup ServerController_HttpCommand
  * The error code for the length of command is invalid
  */
#define	SERVER_E_COMMAND_LENGTH		(SERVER_HTTPCMD_E_FIRST + 0)

/** @ingroup ServerController_HttpCommand
  * The success code for the reading responses, but buffer size isn't enough
  */
#define	SERVER_S_BUFFER_SIZE		(SERVER_HTTPCMD_S_FIRST + 0)
// ************************** Error code for ServerController_HttpCommand *********************************

// ************************** Error code for ServerController_DIDO *********************************
#define	SERVER_DIDO_BASE				0x30
#define	SERVER_DIDO_END					0x3F
#define SERVER_DIDO_E_FIRST				MAKE_SCODE(1, 8, SERVER_DIDO_BASE)
#define SERVER_DIDO_E_LAST				MAKE_SCODE(1, 8, SERVER_DIDO_END)
#define SERVER_DIDO_S_FIRST				MAKE_SCODE(0, 8, SERVER_DIDO_BASE)
#define SERVER_DIDO_S_LAST				MAKE_SCODE(0, 8, SERVER_DIDO_END)

/** @ingroup ServerController_DIDO
  * The error code for the buffer size is not enough to hold dido status
  */
#define	SERVER_E_DIDO_BUFSIZE		(SERVER_DIDO_E_FIRST + 0)
// ************************** Error code for ServerController_DIDO *********************************

// ************************** Error code for ServerController_MD*********************************
#define	SERVER_MD_BASE					0x40
#define	SERVER_MD_END					0x4F
#define SERVER_MD_E_FIRST				MAKE_SCODE(1, 8, SERVER_MD_BASE)
#define SERVER_MD_E_LAST				MAKE_SCODE(1, 8, SERVER_MD_END)
#define SERVER_MD_S_FIRST				MAKE_SCODE(0, 8, SERVER_MD_BASE)
#define SERVER_MD_S_LAST				MAKE_SCODE(0, 8, SERVER_MD_END)

/** @ingroup ServerController_MotionDetection
  * The error code for the parsing motion detection status
  */
#define	SERVER_E_PARSE_MD		(SERVER_MD_E_FIRST + 0)

/** @ingroup ServerController_MotionDetection
  * The error code for the parsing private mask status
  */
#define	SERVER_E_PARSE_PM		(SERVER_MD_E_FIRST + 1)
// ************************** Error code for ServerController_MD *********************************

// ************************** Error code for ServerController_UART*********************************
#define	SERVER_UART_BASE				0x50
#define	SERVER_UART_END					0x5F
#define SERVER_UART_E_FIRST				MAKE_SCODE(1, 8, SERVER_UART_BASE)
#define SERVER_UART_E_LAST				MAKE_SCODE(1, 8, SERVER_UART_END)
#define SERVER_UART_S_FIRST				MAKE_SCODE(0, 8, SERVER_UART_BASE)
#define SERVER_UART_S_LAST				MAKE_SCODE(0, 8, SERVER_UART_END)

/** @ingroup ServerController_Uart
  * The error code for reading timeout
  */
#define	SERVER_E_UART_READ_TIMEOUT		(SERVER_UART_E_FIRST + 0)


// ************************** Error code for ServerController_UART *********************************

// sam 2007/06/21 support SSL
// ************************** Error code for ServerController_UART*********************************
#define	SERVER_SSL_BASE				0x60
#define	SERVER_SSL_END				0x6F
#define SERVER_SSL_E_FIRST			MAKE_SCODE(1, 8, SERVER_SSL_BASE)
#define SERVER_SSL_E_LAST			MAKE_SCODE(1, 8, SERVER_SSL_END)
#define SERVER_SSL_S_FIRST			MAKE_SCODE(0, 8, SERVER_SSL_BASE)
#define SERVER_SSL_S_LAST			MAKE_SCODE(0, 8, SERVER_SSL_END)

/** @ingroup ServerController_SSL
  * The error code for buhher size is not enough
  */
#define SSL_E_BUFFER_SIZE	(SERVER_SSL_E_FIRST + 0)

/** @ingroup ServerController_SSL
  * The error code for invalid client mode
  */
#define SSL_E_INVALID_MODE	(SERVER_SSL_E_FIRST + 1)

/** @ingroup ServerController_SSL
  * This device is not created in SSL mode
  */
#define SSL_E_NOT_CREATE_IN_SSL_MOE	(SERVER_SSL_E_FIRST + 2)
// ************************** Error code for ServerController_UART *********************************
#endif

