#include "SDK7007.h"

int SDlgThreadBegin(int *hdlThread);

int SDlgInit(int hdlThread, int sensorType, ISDK7007 *sdk);

int SDlgShow(int hdlThread);

int SDlgHide(int hdlThread);

int SDlgEnd(int hdlThread);

int SDlgGetRegValues(int hdlThread, TI2CREG7007 **regVal, int *numberReg);

int SDlgThreadEnd(int hdlThread);

int SDlgEndCheck(int hdlThread, HANDLE event);
