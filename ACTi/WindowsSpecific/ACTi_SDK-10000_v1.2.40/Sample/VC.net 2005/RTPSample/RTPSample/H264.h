#pragma once
#include "version.h"

typedef struct _H26L_NAL_UNIT{
	
	
	unsigned char TYPE:5;	// 1-23	NAL unit	Single NAL unit packet per H.264
							// 28	FU-A		Fragmentation unit ; Fragmentation Units
							// 29	FU-B		Fragmentation unit ; Fragmentation Units
							// 24	STAP-A		Single-time aggregation packet
							// 25	STAP-B		Single-time aggregation packet
							// 26	MTAP16		Multi-time aggregation packet
							// 27	MTAP24		Multi-time aggregation packet
	unsigned char NRI:2;
	unsigned char F:1;
}H26L_NAL_UNIT;

typedef H26L_NAL_UNIT FU_INDICATOR;

typedef struct _FU_HAEDER{
	unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;
}FU_HAEDER;

//STAP-A, and an FU-A do not include DON. STAP-B and FU-B structures include DON,





