// nEdgeDetect, nJaggieThreshold : 필터 계수...

// 사용 가능한 포맷[S]...
#define IDX_DEFLT_CS_I420	0	// Y, U, V Planar
#define IDX_DEFLT_CS_YV12	1	// Y, V, U Planar
#define IDX_DEFLT_CS_YUY2	2	// Y0 U0 Y1 V0
// 사용 가능한 포맷[E]...

#define IDX_DEFLT_RET_OK	0
#define IDX_DEFLT_RET_ERR_NOT_SUPPORTED_CS	1

int __stdcall InitDeIntFilter (int idxTypeCS, int nEdgeDetect = 1, int nJaggieThreshold = 73);
void __stdcall ExecDeIntFilter (int szH, int szV, unsigned char *bufYUV);
