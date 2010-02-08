#include "stdafx.h"
#include "ExpressionParser.h"


CExpressionParser::CExpressionParser()
{
	m_sExpression.clear();
}

CExpressionParser::~CExpressionParser()
{
	m_sExpression.clear();
}

bool CExpressionParser::SetExpression(std::string sExpression)
{
	m_sExpression = sExpression;
	return true;
}

void CExpressionParser::ResetExpression()
{
	m_sExpression.clear();
}

// Free spaces
void CExpressionParser::SpaceFree(std::string sExpression, std::string& sNewExpression)
{
	int j = 0;
	char szNewExpression[1024] = {0};
	int nExpressionLen = sExpression.length();
	for(int i = 0; i < nExpressionLen; i++)
	{
		if(sExpression[i] != ' ')
		{
			szNewExpression[j] = sExpression[i];
			j++;
		}
	}
	szNewExpression[j] = '\0';
	sNewExpression.append(szNewExpression);
}

// Aviod error devide 0
void CExpressionParser::EscapeDevideByZero(std::string sExpression, std::string& sNewExpression, int& nClosePos)
{
	int i = sExpression.length();
	int j = i - 1;

	std::string sTemp;
	int nPos;
	if(sExpression[j] == ')')
	{	
		int nPath = GetParentheses(sExpression.substr(j), sTemp, nPos);
		sNewExpression = sExpression.substr(0, nPos);
		sNewExpression = sNewExpression + '0';
		nClosePos = nPos + 1;
		return;
	}
	bool bFind = true;
	while(bFind)
	{
		if(0 == j)
			break;
		if(sExpression[j] == '+' || sExpression[j] == '-' || sExpression[j] == '*' || sExpression[j] == '/')
		{
			sNewExpression = sExpression.substr(0, j+1);
			sNewExpression = sNewExpression + '0';
			bFind = false;
			nClosePos = j+2;
			break;
		}
		j--;
	}	
}

// Check first letter of expression.
void CExpressionParser::CheckFirstMinus(std::string sExpression, std::string& sNewExpression)
{
	char szNewExpression[1024] = {0};
	if(sExpression[0] == '-')
	{
		szNewExpression[0] = '0';
		strcpy(szNewExpression+1, sExpression.c_str());
	}
	else
	{
		sNewExpression = sExpression;
	}
	sNewExpression.append(szNewExpression);
}

int CExpressionParser::RightToLeft(std::string sExpression)
{
	int i = 0;
	std::string s1;
	std::string s2;
	int nExpressionLen = sExpression.length();
	char szTemp[1024] = {0};

	std::string sPathExpression;
	for(i = nExpressionLen;  i >= 0; i--)
	{
		if(sExpression[i] == '+')
		{
			s1 = sExpression.substr(0, i);
			s2 = sExpression.substr(i + 1, nExpressionLen - i - 1);
			int nPath = 0;
			int nClosePos = 0;
			if(s1[i-1] == ')')
			{
				nPath = GetParentheses(s1.substr(0, i-1), sPathExpression, nClosePos);
				s1 = sExpression.substr(0, nClosePos);
				sprintf(szTemp, "%s%d", s1.c_str(), nPath);
				s1 = szTemp;
			}
			return RightToLeft(s1) + atoi(s2.c_str());
		}
		else if(sExpression[i] == '-')
		{
			s1 = sExpression.substr(0, i);
			s2 = sExpression.substr(i + 1, nExpressionLen - i - 1);
			int nPath = 0;
			int nClosePos = 0;
			if(s1[i-1] == ')')
			{
				nPath = GetParentheses(s1.substr(0, i-1), sPathExpression, nClosePos);
				s1 = sExpression.substr(0, nClosePos);
				sprintf(szTemp, "%s%d", s1.c_str(), nPath);
				s1 = szTemp;
			}
			return RightToLeft(s1) - atoi(s2.c_str());
		}
		else if(sExpression[i] == '/')
		{
			s1 = sExpression.substr(0, i);
			s2 = sExpression.substr(i + 1, nExpressionLen - i - 1);
			int nPath = 0;
			int nClosePos = 0;
			int nDenominator =  atoi(s2.c_str());
			if(0 == nDenominator)
			{
				std::string sTemp;
				std::string sTemp1;
				int nPos;
				EscapeDevideByZero(s1, sTemp, nPos);
				sTemp1 = sTemp;
				return RightToLeft(sTemp1) + 0;
			}
			if(s1[i-1] == ')')
			{
				nPath = GetParentheses(s1.substr(0, i-1), sPathExpression, nClosePos);
				s1 = sExpression.substr(0, nClosePos);
				sprintf(szTemp, "%s%d", s1.c_str(), nPath);
				s1 = szTemp;
			}

			return RightToLeft(s1) / atoi(s2.c_str());
		}
		else if(sExpression[i] == '*')
		{
			s1 = sExpression.substr(0, i);
			s2 = sExpression.substr(i + 1, nExpressionLen - i - 1);
			int nPath = 0;
			int nClosePos = 0;
			if(s1[i-1] == ')')
			{
				nPath = GetParentheses(s1.substr(0, i-1), sPathExpression, nClosePos);
				s1 = sExpression.substr(0, nClosePos);
				sprintf(szTemp, "%s%d", s1.c_str(), nPath);
				s1 = szTemp;
			}
			return  RightToLeft(s1) * atoi(s2.c_str());
		}
		else if(sExpression[i] == '^')
		{
			s1 = sExpression.substr(0, i);
			s2 = sExpression.substr(i + 1, nExpressionLen - i - 1);
			int nPath = 0;
			int nClosePos = 0;
			if(s1[i-1] == ')')
			{
				nPath = GetParentheses(s1.substr(0, i-1), sPathExpression, nClosePos);
				s1 = sExpression.substr(0, nClosePos);
				sprintf(szTemp, "%s%d", s1.c_str(), nPath);
				s1 = szTemp;
			}
			return  RightToLeft(s1) ^ atoi(s2.c_str());
		}
		else if(sExpression[i] == '|')
		{
			s1 = sExpression.substr(0, i);
			s2 = sExpression.substr(i + 1, nExpressionLen - i - 1);
			int nPath = 0;
			int nClosePos = 0;
			if(s1[i-1] == ')')
			{
				nPath = GetParentheses(s1.substr(0, i-1), sPathExpression, nClosePos);
				s1 = sExpression.substr(0, nClosePos);
				sprintf(szTemp, "%s%d", s1.c_str(), nPath);
				s1 = szTemp;
			}
			return  RightToLeft(s1) | atoi(s2.c_str());
		}
		else if(sExpression[i] == '&')
		{
			s1 = sExpression.substr(0, i);
			s2 = sExpression.substr(i + 1, nExpressionLen - i - 1);
			int nPath = 0;
			int nClosePos = 0;
			if(s1[i-1] == ')')
			{
				nPath = GetParentheses(s1.substr(0, i-1), sPathExpression, nClosePos);
				s1 = sExpression.substr(0, nClosePos);
				sprintf(szTemp, "%s%d", s1.c_str(), nPath);
				s1 = szTemp;
			}
			return  RightToLeft(s1) & atoi(s2.c_str());
		}
	}

	if(sExpression.length() > 0)
	{
		return atoi(sExpression.c_str());
	}
	return 0;
}

int CExpressionParser::GetParentheses(std::string sExpression, std::string& sPathExpression, int& nClosePos)
{
	int nCounter = 1;
	int i = sExpression.length();
	int j = i - 1;
	while(nCounter != 0)
	{
		if(sExpression[j] == ')')
		{
			nCounter++;
		}
		if(sExpression[j] == '(')
		{
			nCounter--;
			if(nCounter == 0)
			{
				nClosePos = j;
				std::string sTemp;
				CheckPath(sPathExpression, sTemp);
				return RightToLeft(sTemp);
			}
		}
		sPathExpression = sExpression[j] + sPathExpression;
		j--;
	}
	return 0;
}

int CExpressionParser::GetResult()
{
	std::string sExpressionSpaceFree;
	std::string sExpressionPathCheck;
	std::string sExpressionFirstMinus;
	//std::string sExpressionComplement;
	SpaceFree(m_sExpression, sExpressionSpaceFree);
	CheckPath(sExpressionSpaceFree, sExpressionPathCheck);
	CheckFirstMinus(sExpressionPathCheck, sExpressionFirstMinus);
	//CheckComplementOperaqtor(sExpressionFirstMinus, sExpressionComplement);
	int nResult  = RightToLeft(sExpressionFirstMinus);
	//char szPrint[2048] = {0};
	//sprintf(szPrint, "%s = 0x%02x", sExpressionFirstMinus.c_str(), nResult);
	//AfxMessageBox(szPrint);
	return nResult;
}

void CExpressionParser::CheckPath(std::string sExpression, std::string& sNewExpression)
{
	char szNewExpression[1024] = {0};
	int nExpressionLen = sExpression.length();
	int i = 0;
	if(sExpression[0] == '(')
	{
		szNewExpression[i] = '0';
		i++;
		szNewExpression[i] = '+';
		i++;
	}

	int j;
	for(j = 0; j < nExpressionLen; j++)
	{
		szNewExpression[i] = sExpression[j];
		i++;
	}

	if(sExpression[j-1] == ')')
	{
		szNewExpression[i] = '+';
		i++;
		szNewExpression[i] = '0';
		i++;
	}

	szNewExpression[i] = '\0';
	sNewExpression.append(szNewExpression);
}

void CExpressionParser::CheckNotOperator(std::string sExpression, std::string& sNewExpression)
{
	char szNewExpression[1024] = {0};
	int nExpressionLen = sExpression.length();
	int i = 0;
	for(i = 0; i < nExpressionLen; i++)
	{
	}
}

void CExpressionParser::CheckComplementOperaqtor(std::string sExpression, std::string& sNewExpression)
{
	int j = 0;
	char szNewExpression[1024] = {0};
	int nExpressionLen = sExpression.length();
	bool bFind = false;
	char szInt[16] = {0};
	int nIntPos = 0;
	for(int i = 0; i < nExpressionLen; i++)
	{
		if(sExpression[i] == '(')
		{
			if(sExpression[i+1] == '~')
			{
				for(int k = i+2; k < nExpressionLen; k++)
				{
					if(sExpression[k] == ')')
					{
						unsigned int nInt = 0;
						nInt = atoi(szInt);
						nInt = ~nInt;
						nInt &= 0x000000FF;
						memset(szInt, 0x00, 16);
						sprintf(szInt, "%u", nInt);
						int nszIntLen = strlen(szInt);
						memcpy(szNewExpression+j, szInt, nszIntLen);
						memset(szInt, 0x00, 16);
						i = k;
						j += nszIntLen;
						nIntPos = 0;
						break;
					}
					else
					{
						szInt[nIntPos] = sExpression[k];
						nIntPos++;
					}
				}
			}
			else
			{
				szNewExpression[j] = sExpression[i];
				j++;
			}
		}
		else
		{
			szNewExpression[j] = sExpression[i];
			j++;
		}
	}
	szNewExpression[j] = '\0';
	sNewExpression.append(szNewExpression);
}