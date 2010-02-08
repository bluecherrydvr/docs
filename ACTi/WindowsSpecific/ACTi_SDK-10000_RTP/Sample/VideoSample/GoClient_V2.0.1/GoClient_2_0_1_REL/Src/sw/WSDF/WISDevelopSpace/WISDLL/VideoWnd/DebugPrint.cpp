// DebugPrint.cpp

#include "stdafx.h"

#define DEBUGPRINT
#ifdef DEBUGPRINT

char m_Prefix[256];
int m_HasPrefix = 0;
int m_NumPerLine = 16;

void DebugPrintInit(char *Prefix, int NumPerLine)
{
	if(strcmp(Prefix,"") == 0)
	{
		strcpy(m_Prefix, "");
		m_HasPrefix = 0;
	}
	else
	{
		sprintf(m_Prefix,"<%s> ", Prefix);
		m_HasPrefix = 1;
	}

	m_NumPerLine = NumPerLine;
}

void PrintString(char *s)
{
	char s1[512];

	strcpy(s1,m_Prefix);
	strcat(s1,s);

	OutputDebugString(s1);
}

void PrintInt(char *title, int i)
{
	char s1[512];
	sprintf(s1,"%s : %d",title,i);
	PrintString(s1);
}

void PrintDualInt(char *title1, int i1,char *title2,int i2)
{
	char s1[512];
	sprintf(s1, "%s : %d, %s : %d", title1, i1, title2, i2);
	PrintString(s1);
}

void PrintHex(char *title,int i)
{
	char s1[512];
	sprintf(s1,"%s : 0x%08x", title, i);
	PrintString(s1);
}

void PrintDualHex(char *title1, int i1,char *title2,int i2)
{
	char s1[512];
	sprintf(s1, "%s : 0x%08x, %s : 0x%08x", title1, i1, title2, i2);
	PrintString(s1);
}

void PrintBinary(char *title, unsigned char *buf, int len)
{
    char    str[128], str2[64];
	int	i,j;
	unsigned char *ptr;

	ptr = buf;

	for (i = 0; i < ((len + m_NumPerLine - 1) / m_NumPerLine); i++)
	{
		sprintf(str,"%s : [%04X] ",title,(i*m_NumPerLine));
		for (j = 0; j < m_NumPerLine; j++)
		{
			if (((i * m_NumPerLine) + j) < len)
			{
				sprintf(str2,"%02X ",*ptr++);
				strcat(str,str2);
			}
		}
        PrintString(str);
	}
}

#else

void DebugPrintInit(char *Prefix, int NumPerLine){};
void PrintString(char *s){};
void PrintInt(char *title, int i){};
void PrintDualInt(char *title1, int i1,char *title2,int i2){};
void PrintHex(char *title,int i){};
void PrintDuaHex(char *title1, int i1,char *title2,int i2){};
void PrintBinary(char *title, unsigned char *buf, int len){};

#endif



