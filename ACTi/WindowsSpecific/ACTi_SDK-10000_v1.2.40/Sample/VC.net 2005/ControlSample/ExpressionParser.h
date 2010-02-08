#ifndef __EXPRESSION_PARSER_H__
#define __EXPRESSION_PARSER_H__

#include <string>

class CExpressionParser
{
public:
	CExpressionParser();
	~CExpressionParser();
	bool SetExpression(std::string sExpression);
	int GetResult();
	void ResetExpression();

private:
	void SpaceFree(std::string sExpression, std::string& sNewExpression);
	void EscapeDevideByZero(std::string sExpression, std::string& sNewExpression, int& nClosePos);
	int GetParentheses(std::string sExpression, std::string& sPathExpression, int& nClosePos);
	void CheckPath(std::string sExpression, std::string& sNewExpression);
	void CheckFirstMinus(std::string sExpression, std::string& sNewExpression);
	int RightToLeft(std::string sExpression);
	void CheckNotOperator(std::string sExpression, std::string& sNewExpression);
	void CheckComplementOperaqtor(std::string sExpression, std::string& sNewExpression);

private:
	std::string m_sExpression;
};

#endif // __EXPRESSION_PARSER_