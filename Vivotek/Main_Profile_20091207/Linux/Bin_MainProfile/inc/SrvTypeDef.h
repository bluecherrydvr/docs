#ifndef _SRVTYPEDEF_H
#define _SRVTYPEDEF_H

typedef enum {
	eptHTTP,
	eptTCP,
	eptUDP,
	eptMULTICAST,
	eptScalableMULTICAST = eptMULTICAST,
	eptBackchannelMULTICAST
} TsdrProtocolType;

typedef enum {
	emtAudio = 1,
	emtVideo = 2,
	emtTransmitAudio = 4
} TsdrMediaType;

typedef enum {
	eVCodecNone = 0x0000,
	eVCodecMJPEG = 0x0001,
	eVCodecH263 = 0x0002,
	eVCodecMPEG4 = 0x0004,
	eVCodecH264 = 0x0008
} TsdrVideoCodec;

typedef enum {
	eACodecNone = 0x0000,
	eACodecG7221 = 0x0100,
	eACodecG729A = 0x0200,
	eACodecAAC = 0x0400,
	eACodecGAMR = 0x0800,
	eACodecG711 = 0x1000,
} TsdrAudioCodec;

#endif
