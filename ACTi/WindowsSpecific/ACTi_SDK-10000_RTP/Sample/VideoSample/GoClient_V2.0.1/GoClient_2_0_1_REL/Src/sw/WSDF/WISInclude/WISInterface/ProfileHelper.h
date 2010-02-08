// ProfileHelper.h: interface for some auxiliary class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _PROFILE_HELP_H_
#define _PROFILE_HELP_H_

#include <stdio.h>
#include <WSDF.H>
#include <Struct.h>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class IProfileMgr  
{
public:

	STATIC SINT32 CreateInstance(void **pp);
	void Release();

	virtual SINT32 SetProfileRoot(SINT8 *RootPath) PURE;
	virtual SINT32 EnumStreamSetting(SINT32 SourceWidth, SINT32 SourceHeight, SINT8 Name[][256], SINT32 *Num) PURE;
	virtual SINT32 EnumBitrateSetting(SINT8 *StreamName, SINT8 Name[][256], SINT32 *Num) PURE;
	virtual SINT32 GetConfig(SINT8 *StreamName, SINT8 *BitrateName, TVIDEOCFG *pconfig) PURE;
	virtual SINT32 GetConfigFromFile(SINT8 *StreamConfigFileName, SINT8 *BitrateConfigFileName, TVIDEOCFG *pconfig) PURE;
	virtual SINT32 GetConfigFromOneFile(SINT8 *ConfigFileName, TVIDEOCFG *pconfig) PURE;
};

#endif 

