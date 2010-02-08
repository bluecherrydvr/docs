#ifndef __PTZATTRIBUTE_H__
#define __PTZATTRIBUTE_H__

#include "PTZDefine.h"

class CPTZAttribute
{
public:
	CPTZAttribute();
	~CPTZAttribute();
	bool SetName(std::string sName);
	bool InsertValue(std::string sValue);
	bool CheckName(std::string sName);
	void Print();
	bool FindCommand(std::string sCommand, int nIndex, std::string& sResult);
	bool FindCommand(std::string sCommand, std::string& sResult);
	bool FindCommand(std::string sCommand, _Strings& sResults, int& nCount);

private:
	std::string m_sName;
	_Strings m_Values;
};
#endif // __PTZATTRIBUTE_H__