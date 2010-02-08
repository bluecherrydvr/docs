#include "stdafx.h"
#include "PTZCategory.h"

CPTZCategory::CPTZCategory() 
{
	m_sName.clear();
}

CPTZCategory::~CPTZCategory()
{
	for(_Attributes::iterator itor = m_Atts.begin(); itor != m_Atts.end(); itor++)
	{
		delete (CPTZAttribute*) (*itor);
	}
	m_Atts.clear();
	m_sName.clear();
}

bool CPTZCategory::SetName(std::string sName)
{
	if(sName.empty())
		return false;

	m_sName = sName;
	return true;
}

void CPTZCategory::GetName(std::string &sName)
{
	sName = m_sName;
}

bool CPTZCategory::InsertValue(std::string sValue)
{
	if(sValue.empty())
		return false;

	int nIndex = sValue.find(';');

	if(-1 == nIndex)
		return false;

	std::string sAttName = sValue.substr(0, nIndex);
	sAttName[nIndex] = '\0';

	// Find Attribute
	bool bFind = false;
	_Attributes::iterator itor = m_Atts.begin();
	for(itor = m_Atts.begin(); itor != m_Atts.end(); itor++)
	{
		if((*itor)->CheckName(sAttName))
		{
			bFind = true;
			break;
		}
	}

	if(bFind)
	{
		(*itor)->InsertValue(sValue);
	}
	else
	{
		CPTZAttribute* pPTZAtrribute;
		pPTZAtrribute = new CPTZAttribute();
		pPTZAtrribute->SetName(sAttName);
		pPTZAtrribute->InsertValue(sValue);
		m_Atts.push_back(pPTZAtrribute);
	}
	return true;
}

bool CPTZCategory::FindValue(std::string sValue)
{
	for(_Attributes::iterator itor = m_Atts.begin(); itor != m_Atts.end(); itor++)
	{
		if((*itor)->CheckName(sValue))
			return true;
	}
	return false;
}

void CPTZCategory::Print()
{
	printf("Category: %s\n", m_sName.c_str());
	for(_Attributes::iterator itor = m_Atts.begin(); itor != m_Atts.end(); itor++)
	{
		(*itor)->Print();
	}
}

bool CPTZCategory::FindCommand(std::string sCommand, int nIndex, std::string& sResult)
{
	for(_Attributes::iterator itor = m_Atts.begin(); itor != m_Atts.end(); itor++)
	{
		if((*itor)->FindCommand(sCommand, nIndex, sResult))
			return true;
	}
	return false;
}

bool CPTZCategory::CheckName(std::string sName)
{
	if(sName == m_sName)
		return true;
	return false;
}

bool CPTZCategory::FindCommand(std::string sCommand, std::string& sResult)
{
	for(_Attributes::iterator itor = m_Atts.begin(); itor != m_Atts.end(); itor++)
	{
		if((*itor)->FindCommand(sCommand, sResult))
			return true;
	}
	return false;
}

bool CPTZCategory::FindCommand(std::string sCommand, _Strings& sResults, int& nCount)
{
	for(_Attributes::iterator itor = m_Atts.begin(); itor != m_Atts.end(); itor++)
	{
		(*itor)->FindCommand(sCommand, sResults, nCount);
	}
	return true;
}
