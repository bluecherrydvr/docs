#ifndef __PTZCATEGORY_H__
#define __PTZCATEGORY_H__

#include "PTZAttribute.h"

class CPTZCategory
{
public:
	CPTZCategory();
	~CPTZCategory();
	bool SetName(std::string sName);
	void GetName(std::string &sName);
	bool InsertValue(std::string sValue);
	bool FindValue(std::string sValue);
	void Print();
	bool FindCommand(std::string sCommand, int nIndex, std::string& sResult);
	bool FindCommand(std::string sCommand, std::string& sResult);
	bool FindCommand(std::string sCommand, _Strings& sResults, int& nCount);
	bool CheckName(std::string sName);

private:
	typedef std::list<CPTZAttribute*> _Attributes;
	std::string m_sName;
	_Attributes m_Atts;
};
#endif // __PTZCATEGORY_H__