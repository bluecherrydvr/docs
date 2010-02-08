#pragma once
#include <afxinet.h>
//#include <iphlpapi.h>
//#pragma comment(lib, "iphlpapi.lib")

typedef struct structural_HTTP_CONNECTION_CONFIG
{
	char			IP[16];
	unsigned long	HTTPPort;
	int				ChannelNumber;
	char			UserID[64];
	char			Password[64];
}HTTP_CONNECTION_CONFIG;

class CHTTPConnection
{
public:
	CHTTPConnection(void);
	~CHTTPConnection(void);

	void				SetConfig( HTTP_CONNECTION_CONFIG* IP );
	bool				ExecuteURLCommand( char* cURLCommand, char* result, int len );
};
