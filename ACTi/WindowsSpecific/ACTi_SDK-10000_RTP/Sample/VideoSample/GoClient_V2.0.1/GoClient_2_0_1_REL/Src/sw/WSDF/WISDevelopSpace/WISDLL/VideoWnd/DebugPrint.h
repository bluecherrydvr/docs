// DebugPrint.h

#ifndef __DEBUGPRINT_H__
#define __DEBUGPRINT_H__

void DebugPrintInit(char *Prefix, int NumPerLine);
void PrintString(char *s);
void PrintInt(char *title, int i);
void PrintDualInt(char *title1, int i1,char *title2,int i2);
void PrintHex(char *title,int i);
void PrintDualHex(char *title1, int i1,char *title2,int i2);
void PrintBinary(char *title, unsigned char *buf, int len);

#endif
