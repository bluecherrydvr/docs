#ifndef __CROSSBAR_CTROL_H__
#define __CROSSBAR_CTROL_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /*max input number*/
#define ISIL_MAX_VI 16
    /*max video input group number*/
#define ISIL_MAX_GRP 4
    /*max capability*/
#define ISIL_MAX_CAPABILITY (7)
    /*at the same time, only two frame pass BUS*/
#define ISIL_BUS_MAX_D1 2

    /*input number for every group*/
#define ISIL_CHN_NO   (ISIL_MAX_VI / ISIL_MAX_GRP)

#define IO_BLOCK   1
#define IO_NOBLOCK 0

#define ISIL_INVAL_CHN (-1)

    /*max total frame rate in one group*/
#define ISIL_GRP_MAX_PFRAME    25//PAL
#define ISIL_GRP_MAX_NFRAME    30//NTSC

    /*support input format*/
    /*typedef enum{
      ISIL_INVAL = 0,
      ISIL_D1,
      ISIL_HalfD1,
      ISIL_CIF,
      ISIL_HCIF,
      ISIL_QCIF,
      }ISIL_FORMAT_U; 
      */ 

    enum GROUP_MODE{
	MODE_D1,
	MODE_D1_HALFD1,
	MODE_D1_HALFD1_CIF,
	MODE_D1_CIF,
	MODE_HALFD1,
	MODE_HALFD1_CIF,
	MODE_CIF,
	MODE_NONE,
    };

#define ISIL_VI_PAL  ISIL_VIDEO_STANDARD_PAL
#define ISIL_VI_NTSC ISIL_VIDEO_STANDARD_NTSC

    typedef struct{
	int grp_id;                 /*group id*/
	int in_select[ISIL_CHN_NO];	/*input select for every group*/
	int in_map[ISIL_CHN_NO];	/*map table*/
	int in_fmt[ISIL_CHN_NO];      /*input format*/
	int in_rate[ISIL_CHN_NO];     /*input rate*/
	int out_rate[ISIL_CHN_NO];    /*output rate*/
	int totalfrm;               /*total frame rate*/
	enum GROUP_MODE mode;
	int total;
    }VGROUP_S;

    /*
       cb_ctrl_calculate
desc: calculate cross bus map;
input:
chn: channel number;
format: input format, ISIL_D1, ISIL_CIF ,...;
rate: frame rate, PAL [0, 25], NTSC [0, 30];
norm: video norm, only support ISIL_VI_PAL and ISIL_VI_NTSC
vgroup: out put every bus configure;
count: channel count;
adjust: adjust out put frame rate; 
output:
successful 0, if adjust == 1 then return lost frame count, other -1;
*/
    int cb_ctrl_calculate(int *chn, int *format, int *rate, int norm, VGROUP_S *vgroup, int count, int adjust);

    /*
       preview_ctrl_calculate
desc: calculate cross bus map;
input:
chn: channel number;
format: input format, ISIL_D1, ISIL_CIF ,...;
rate: frame rate, PAL [0, 25], NTSC [0, 30];
norm: video norm, only support ISIL_VI_PAL and ISIL_VI_NTSC
vgroup: out put every bus configure;
count: channel count;
adjust: this version not supprot; 
output:
successful 0, other -1;
*/
    int preview_ctrl_calculate(int *chn, int *format, int *rate, int norm, VGROUP_S *vgroup, int count, int adjust);

    /*calculate bit map*/
    u32 get_bits_map(u32 src, u32 target);

#ifdef __cplusplus
}
#endif

#endif //__CROSSBAR_CTROL_H__

