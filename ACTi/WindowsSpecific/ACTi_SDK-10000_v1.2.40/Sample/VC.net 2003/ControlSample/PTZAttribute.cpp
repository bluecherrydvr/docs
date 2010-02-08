#include "stdafx.h"
#include "PTZAttribute.h"

CPTZAttribute::CPTZAttribute()
{
	m_sName.clear();
}

CPTZAttribute::~CPTZAttribute()
{
	m_Values.clear();
	m_sName.clear();
}

bool CPTZAttribute::SetName(std::string sName)
{
	if(sName.empty())
		return false;

	m_sName = sName;
	return true;
}

bool CPTZAttribute::InsertValue(std::string sValue)
{
	if(sValue.empty())
		return false;

	m_Values.push_back(sValue);
	return true;
}

bool CPTZAttribute::CheckName(std::string sName)
{
	if(sName == m_sName)
		return true;
	return false;
}

void CPTZAttribute::Print()
{
	//printf("Attribute Name: %s\n", m_sName.c_str());
	for(_Strings::iterator itor = m_Values.begin(); itor != m_Values.end(); itor++)
	{
		printf("Attribute: %s\n", (*itor).c_str());
	}
}

// Find command with index.
bool CPTZAttribute::FindCommand(std::string sCommand, int nIndex, std::string& sResult)
{
	for(_Strings::iterator itor = m_Values.begin(); itor != m_Values.end(); itor++)
	{
		// this is bug //"abc" and "abcd" return true
		//if(strncmp(sCommand.c_str(), (*itor).c_str(), sCommand.length()) == 0)
		std::string debugtempstr = (*itor); 
		std::string::size_type compare_idx;
		compare_idx = (*itor).find(';');
		if((*itor).compare(0,compare_idx,sCommand) == 0)
		{
			int nIndexSemicolon = (*itor).find(';');

			if(-1 == nIndexSemicolon)
				continue;
			
			int nIndexComma = (*itor).find(';', nIndexSemicolon+1);

			if(-1 == nIndexComma)
				continue;

			std::string sIndex = (*itor).substr(nIndexSemicolon+1, nIndexComma-nIndexSemicolon-1);
			if(atoi(sIndex.c_str()) == nIndex)
			{
				sResult = (*itor);
				return true;
			}
		}
	}
	return false;
}

// Find command with no index.
bool CPTZAttribute::FindCommand(std::string sCommand, std::string& sResult)
{
	for(_Strings::iterator itor = m_Values.begin(); itor != m_Values.end(); itor++)
	{
		// this is bug //"abc" and "abcd" return true
		//if(strncmp(sCommand.c_str(), (*itor).c_str(), sCommand.length()) == 0)
		std::string::size_type compare_idx;
		compare_idx = (*itor).find(';');
		if((*itor).compare(0,compare_idx,sCommand) == 0)
		{
			sResult = (*itor);
			return true;
		}
	}
	return false;
}

// Find all the commands with same name.
bool CPTZAttribute::FindCommand(std::string sCommand, _Strings& sResults, int& nCount)
{
	int n = 0;
	for(_Strings::iterator itor = m_Values.begin(); itor != m_Values.end(); itor++)
	{
		//printf("%s %s\n", sCommand.c_str(), (*itor).c_str());
		
		// this is bug //"abc" and "abcd" return true
		//if(strncmp(sCommand.c_str(), (*itor).c_str(), sCommand.length()) == 0)
		std::string::size_type compare_idx;
		compare_idx = (*itor).find(';');
		if((*itor).compare(0,compare_idx,sCommand) == 0)
		{
			sResults.push_back(*itor);
			n++;
		}
	}
	nCount += n;
	return true;
}
