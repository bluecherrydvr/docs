// ProfileHelper.h: interface for some auxiliary class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CFG_FILE_PARSER_H_
#define _CFG_FILE_PARSER_H_

#include "WSDF.H"
#include "Struct.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C"
{
#endif

SINT32 GetNextConfigInt(FILE *fp, SINT8 *strItem, SINT32 *Val);

SINT32 GetSensorInitDataFromFile(SINT8 *filename, UINT16 *dev_addr, SINT32 *reg_num, UINT16 *reg_data);
SINT32 GetSystemSettingFromFile(SINT8 *filename, SINT32 *Num, TCFGSYSTEM *syscfgs);

SINT32 EnumStreamSetting(SINT8 Name[][256], SINT32 CompressMode[], SINT32 *Num);
SINT32 EnumResolutionSetting(SINT8 *StreamName, SINT32 SourceWidth, SINT32 SourceHeight, SINT8 Name[][256], SINT32 *Num);
SINT32 EnumBitrateSetting(SINT8 *ResolutionName, SINT8 Name[][256], SINT32 *Num);
SINT32 GetVideoSettingsByName(SINT8 *StreamName, SINT8 *ResolutionName, SINT8 *BitrateName, TVIDEOCFG *pconfig);

#ifdef __cplusplus
}
#endif



#endif 

