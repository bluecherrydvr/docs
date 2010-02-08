#ifndef _L_DECODE_H_
#define _L_DECODE_H_

#ifndef _LIB
	#ifdef _DEBUG
		#pragma comment(lib, "h26ldecoderD.lib")
	#else
		#pragma comment(lib, "h26ldecoder.lib")
	#endif
#endif

struct DEC_PARAM
{
    int symbol_mode;			                  /* Specifies the mode the symbols are mapped on bits */
    int UseConstrainedIntraPred;               /* 0: Inter MB pixels are allowed for intra prediction 1: Not allowed */
    int NAL_mode;					              /* Specifies the mode of the output file             */
	int buf_cycle;									/* Frame buffer size */
};

#ifdef __cplusplus
extern "C"
{
#endif
int h26l_init_decoder(void *config);
int h26l_dec_frame(unsigned char *bitstream, int *length, unsigned char *pOutput);

int h26l_close_decoder();
#ifdef __cplusplus
}
#endif
#endif
