#ifndef __PTZPROTOCOL_H__
#define __PTZPROTOCOL_H__

#include "PTZCategory.h"

class CPTZProtocol
{
public:
	CPTZProtocol();
	~CPTZProtocol();
	//load protocol from file  
	// call InsertValue, and build up whole data structure
	bool LoadProtocolFile(std::string sName);

	//load protocol from resource
	bool LoadProtocolRS(std::string sName);

	//set m_sName (object name)
	bool SetName(std::string sName);
	// build up m_Cats data structure, and m_Cats build up there's own data structure too
	bool InsertValue(std::string sValue);
	bool FindCommand(std::string sCommand, int nIndex, std::string& sResult);
	// search through all Categories to find the command
	bool FindCommand(std::string sCommand, std::string& sResult);
	bool FindCommand(std::string sCommand, _Strings& sResults, int& nCount);

	//clear m_Cats and m_sName
	bool Reset();
	// printf all content (應該是 debug 用)
	void Print();

	// return m_sName as the name of protocol
	bool GetProtocol(std::string& sProtocol);
	//push back all m_Cats name in sResults, nCount is the time of pushing back
	bool GetCategory(_Strings& sResults, int& nCount);

	// get content CHECKSUM from [ATTRIBUTES] Category  // This is used to get value for command only with one
	bool GetCheckSum(std::string& sResult);
	// get content ADDRIDSTART from [ATTRIBUTES] Category
	bool GetAddIDStart(std::string& sResult);
	// get content ADDRIDPOS from [ATTRIBUTES] Category
	bool GetAddIDPos(std::string& sResult);
	// get content PANEL from [ATTRIBUTES] Category
	bool GetPanel(std::string& sResult);

	// use FindCommand() to get [ATTRIBUTES] value // This is used to get value for command only with one
	bool GetAttValue(std::string sCommand, std::string& sResult);
	// input1: szPTZCommand  input2: iParam1  =>  output: sResult
	bool GetFunctionValue(std::string sCommand, int nIndex, std::string& sResult);
	// use GetAttValue() to get COMMANDTYPE content
	bool GetCommandTypeStr(std::string& sResult);
	// return m_nCommandType
	bool GetCommandType(int& nResult);

private:
	bool RemoveSpace(char* szData, char* szNewData);

private:
	// the list of categories ...  [ATTRIBUTES] ,[PANTILT], [MOVE] ...
	typedef std::list<CPTZCategory*> _Categorys;

	//file name or resource name
	std::string m_sName;

	// std::list of Category
	_Categorys m_Cats;

	// 這個的用法不明
	int m_nCommandType;
};
#endif // __PTZPROTOCOL_H__