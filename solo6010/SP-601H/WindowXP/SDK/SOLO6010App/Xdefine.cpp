#include "stdafx.h"
#include "Xdefine.h"

static	char g_report[400];

void	ResetReport(char* fname)
{	
#ifdef _DEBUG
	strcpy(g_report,fname);
	FILE* fp=fopen(g_report,"w");
	fclose(fp);
#endif
	return;
}

void	PrepareReport(char* fname)
{	
	strcpy(g_report,fname);
}

void	AddReport(char* sz,...)
{
#ifdef _DEBUG
//	return;
	static	char szOutput[400];
    va_list va;	
    va_start(va, sz);
    vsprintf (szOutput,sz,va);      /* Format the string */
    va_end(va);

	if(strlen(g_report)<1)	return;

	FILE* fp=fopen(g_report,"a");
	if(fp==NULL)	return;

	SYSTEMTIME	tt;	
	GetLocalTime(&tt);	
	fprintf(fp,"%02d:%02d:%03d %s\n",tt.wMinute,tt.wSecond,tt.wMilliseconds, szOutput);
//	fprintf(fp,"%s", szOutput);
	fclose(fp);
#endif
	return;
}