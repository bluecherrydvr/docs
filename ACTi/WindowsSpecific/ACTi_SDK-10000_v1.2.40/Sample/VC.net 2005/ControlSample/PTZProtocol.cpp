#include "stdafx.h"
#include "PTZProtocol.h"
#include <windows.h>

CPTZProtocol::CPTZProtocol()
{
	m_nCommandType = -1;
	m_sName.clear();
}

CPTZProtocol::~CPTZProtocol()
{
	for(_Categorys::iterator itor = m_Cats.begin(); itor != m_Cats.end(); ++itor)
	{
		delete (CPTZCategory*)(*itor);
	}
	m_Cats.clear();
	m_sName.clear();
}

bool CPTZProtocol::LoadProtocolFile(std::string sName)
{
	bool bRet =  true;

	if(sName.empty())
		return false;

	m_sName = sName;

	FILE *fp = fopen(m_sName.c_str(), "r" );

	// open file fail
	if(NULL == fp) 
	{
		return false;
	}

	while(!feof(fp))
	{
		char szLine[1024] = {0};
		if(NULL == fgets(szLine, 1024, fp))
		{
			if(feof(fp))
			{
				break;
			}
			bRet = false;
			break;
		}
		else
		{
			char szNewLine[1024] = {0};
			RemoveSpace(szLine, szNewLine);
			if('#' == szNewLine[0] || '\0' == szNewLine[0])
				continue;
			if(!InsertValue(szNewLine))
			{
				bRet = false;
				break;
			}
		}
	}

	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}

	if( bRet )
	{
		std::string strCommandType;
		GetCommandTypeStr( strCommandType );
		if( strCommandType == "1" )
			m_nCommandType = 1;
		else if( strCommandType == "2" )
			m_nCommandType = 2;
		else
			m_nCommandType = 0;
	}
	return bRet;
}

bool CPTZProtocol::LoadProtocolRS(std::string sName)
{
	bool bRet =  true;

	if(sName.empty())
		return false;

	m_sName = sName;

	HMODULE hModule = LoadLibrary("PTZParser.dll");
	if(NULL == hModule)
	{
		FreeLibrary(hModule);
	}

	HRSRC hRsrc = FindResource( hModule, sName.c_str(), RT_HTML);
	if(NULL == hRsrc)
	{
		int nError = GetLastError();
		FreeLibrary(hModule);
		return false;
	}

	DWORD dwSize = SizeofResource(hModule, hRsrc); 
	HGLOBAL hGlobal = LoadResource(hModule, hRsrc);
	if(NULL == hGlobal)
	{
		int nError = GetLastError();
		FreeLibrary(hModule);
		return false;
	}

	LPVOID pBuffer = LockResource(hGlobal); 
	if (NULL == pBuffer)
	{
		int nError = GetLastError();
		FreeLibrary(hModule);
		return false;
	}

	char szLine[1024] = {0};
	int nCount = 0;
	int nArrayPos = 0;
	while(nCount < dwSize)
	{
		memcpy(szLine + nArrayPos, &((BYTE*)pBuffer)[nCount], 1);
		if('\t' == szLine[nArrayPos] || '\r' == szLine[nArrayPos])
		{
			nCount++;
			szLine[nArrayPos] = '\0';
		}
		else if(szLine[nArrayPos] == '\n')
		{
			char szNewLine[1024] = {0};
			nCount++;
			szLine[nArrayPos] = '\0';
			RemoveSpace(szLine, szNewLine);
			if('#' == szNewLine[0] || '\0' == szNewLine[0])
			{
				memset(szNewLine, 0x00, 1024);
				memset(szLine, 0x00, 1024);
				nArrayPos = 0;
				continue;
			}
			if(!InsertValue(szNewLine))
			{
				bRet = false;
				break;
			}
			nArrayPos = 0;
		}
		else
		{
			nCount++;
			nArrayPos++;
		}
	}
	
	if(NULL != hModule)
	{
		FreeLibrary(hModule);
	}

	if( bRet )
	{
		std::string strCommandType;
		GetCommandTypeStr( strCommandType );
		if( strCommandType == "1" )
			m_nCommandType = 1;
		else if( strCommandType == "2" )
			m_nCommandType = 2;
		else
			m_nCommandType = 0;
	}

	return bRet;
}

bool CPTZProtocol::SetName(std::string sName)
{
	m_sName = sName;		// Protocol file name
	return true;
}

bool CPTZProtocol::InsertValue(std::string sValue)
{
	if(sValue.empty())
		return false;
	int nOpenPos = sValue.find('[');
	if(std::string::npos != nOpenPos)
	{
		CPTZCategory* pPTZCategory;
		pPTZCategory = new CPTZCategory();
		int nClosePos = sValue.find(']');
		sValue.erase(nClosePos, 1);
		sValue.erase(nOpenPos, 1);
		pPTZCategory->SetName(sValue);
		m_Cats.push_back(pPTZCategory);
		return true;
	}
	else
	{
		if(!m_Cats.back()->InsertValue(sValue))
			return false;
	}
	return true;
}

bool CPTZProtocol::RemoveSpace(char* szData, char* szNewData)
{
	if(NULL == szData)
		return false;

	int i = 0;
	int j = 0;

	while('\0' != szData[i] && '\n' != szData[i])
	{
		if(' ' != szData[i] && '\t' != szData[i] && '\r' != szData[i])
		{		
			szNewData[j] = szData[i];
			j++;
		}
		i++;
	}

	szNewData[j] = '\0';
	return true;
}

bool CPTZProtocol::Reset()
{
	for(_Categorys::iterator itor = m_Cats.begin(); itor != m_Cats.end(); ++itor)
	{
		delete (CPTZCategory*)(*itor);
	}
	m_Cats.clear();
	m_sName.clear();
	return true;
}

void CPTZProtocol::Print()
{
	printf("Protocol %s\n", m_sName.c_str());
	for(_Categorys::iterator itor = m_Cats.begin(); itor != m_Cats.end(); ++itor)
	{
		(*itor)->Print();
	}
}

// Because we don't know the category name, so, this find command will parse all the category from beginning.
bool CPTZProtocol::FindCommand(std::string sCommand, int nIndex, std::string& sResult)
{
	if(0 >= sCommand.length())
		return false;

	for(_Categorys::iterator itor = m_Cats.begin(); itor != m_Cats.end(); itor++)
	{
		if((*itor)->FindCommand(sCommand, nIndex, sResult))
			return true;
	}
	return false;
}

bool CPTZProtocol::FindCommand(std::string sCommand, std::string& sResult)
{
	if(0 >= sCommand.length())
		return false;

	for(_Categorys::iterator itor = m_Cats.begin(); itor != m_Cats.end(); itor++)
	{
		if((*itor)->FindCommand(sCommand, sResult))
			return true;
	}
	return false;
}

bool CPTZProtocol::FindCommand(std::string sCommand, _Strings& sResults, int& nCount)
{
	if(0 >= sCommand.length())
		return false;

	nCount = 0;

	for(_Categorys::iterator itor = m_Cats.begin(); itor != m_Cats.end(); itor++)
	{
		(*itor)->FindCommand(sCommand, sResults, nCount);
	}
	return true;
}

bool CPTZProtocol::GetProtocol(std::string& sProtocol)
{
	if(m_sName.empty())
	{
		sProtocol.clear();
		return false;
	}
	else
	{
		sProtocol = m_sName;
		return true;
	}
}

bool CPTZProtocol::GetCategory(_Strings& sResults, int& nCount)
{
	bool bret = false;
	nCount = 0;
	for(_Categorys::iterator itor = m_Cats.begin(); itor != m_Cats.end(); itor++)
	{
		std::string sCat;
		(*itor)->GetName(sCat);
		sResults.push_back(sCat);
		bret = true;
		nCount++;
	}
	return bret;
}

// This is used to get value for command only with one ;
bool CPTZProtocol::GetAttValue(std::string sCommand, std::string& sResult)
{
	std::string sTemp;
	if(FindCommand(sCommand, sTemp))
	{
		if(sTemp.length() <= sCommand.length() + 1)
			return false;

		int nIndex = sTemp.find(';');
		if(std::string::npos != nIndex)
		{
			sResult = sTemp.substr(nIndex + 1);
			return true;
		}
	}
	return false;
}

bool CPTZProtocol::GetCheckSum(std::string& sResult)
{
	std::string sCheckSum = "CHECKSUM\0";
	if(GetAttValue(sCheckSum, sResult))
		return true;
	return false;
}


bool CPTZProtocol::GetAddIDStart(std::string& sResult)
{
	std::string sAddIDStart = "ADDRIDSTART\0";
	if(GetAttValue(sAddIDStart, sResult))
		return true;
	return false;
}

bool CPTZProtocol::GetAddIDPos(std::string& sResult)
{
	std::string sAddIDPos = "ADDRIDPOS\0";
	if(GetAttValue(sAddIDPos, sResult))
		return true;
	return false;
}

bool CPTZProtocol::GetPanel(std::string& sResult)
{
	std::string sPanel = "PANEL\0";
	if(GetAttValue(sPanel, sResult))
		return true;
	return false;
}

bool CPTZProtocol::GetFunctionValue(std::string sCommand, int nIndex, std::string& sResult)
{
	std::string sTemp;
	if(FindCommand(sCommand, nIndex, sTemp))
	{
		if(sTemp.length() <= sCommand.length() + 1)
			return false;

		if( m_nCommandType == 0 )
		{
			int nIndex = sTemp.find("0x");
			if(std::string::npos != nIndex)
			{
				sResult = sTemp.substr(nIndex);
				return true;
			}
		}
		
		else if( m_nCommandType == 1 || m_nCommandType == 2 )
		{
			int nCnt = 0;
			int nIndex = 0;
			std::string sTempRes = sTemp;
			while( nCnt++ < 3 )
			{
				sTemp = sTempRes;
				if(std::string::npos != nIndex)
				{
					nIndex = sTemp.find(";");
					sTempRes = sTemp.substr(nIndex+1);
				}
				else
				{
					nIndex = 0;
					break;
				}
			}
			if(std::string::npos != nIndex)
			{
				sResult = sTempRes;//sTemp.substr(nIndex);
				return true;
			}
		}
	}
	return false;
}


bool CPTZProtocol::GetCommandTypeStr(std::string& sResult)
{
	std::string sAddIDStart = "COMMANDTYPE\0";
	if(GetAttValue(sAddIDStart, sResult))
		return true;
	return false;
}

bool CPTZProtocol::GetCommandType(int& nResult)
{
	if( m_nCommandType == -1 )
	{
		nResult = 0;
		return false;
	}
	else
	{
		nResult = m_nCommandType;
	}
	return true;
}
