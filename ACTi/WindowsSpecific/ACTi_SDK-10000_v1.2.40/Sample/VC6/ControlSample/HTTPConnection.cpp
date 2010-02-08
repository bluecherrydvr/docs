#include "StdAfx.h"
#include ".\httpconnection.h"

CHTTPConnection::CHTTPConnection(void)
{
}

CHTTPConnection::~CHTTPConnection(void)
{
}

bool CHTTPConnection::ExecuteURLCommand( char* cURLCommand, char* result, int len )
{
	DWORD		dwRet = 0;
	int			nlen = len;
	CHttpFile*	pIF = NULL;
	bool		bRet = false;
	char		cHTTPSessionName[64];
	CInternetSession*	pIS = NULL;

	sprintf( cHTTPSessionName, "CHTTPConnection_2_0%d\0", rand() );
	pIS = new CInternetSession( cHTTPSessionName );

	if( pIS && cURLCommand )
	{
		try
		{
			pIF = (CHttpFile *)pIS->OpenURL( cURLCommand, 1, INTERNET_FLAG_TRANSFER_ASCII|INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE );
		}
		catch (...) {pIF = NULL;}
		
		if( pIF )
		{
			pIF->QueryInfoStatusCode( dwRet );
			if( dwRet == HTTP_STATUS_OK )
			{
				if( result )
				{
					nlen = pIF->Read( result , nlen );
					result[ nlen ] = 0;
				}
				bRet = true;
			}
		}
	}

	if( pIF )
	{
		pIF->Close();
		delete pIF;
	}

	if( pIS )
	{
		pIS->Close();
		delete pIS;
	}
	return bRet;
}
