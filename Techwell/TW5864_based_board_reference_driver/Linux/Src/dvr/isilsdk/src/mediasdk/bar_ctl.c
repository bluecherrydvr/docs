/*
   crossbar_ctrol.c
   control Non-real-time frame rate
   create by rxchen <rxchen@techwellinc.com.cn>
 */

#include <string.h>
#include "bar_ctl.h"
#ifdef __KERENEL__
#define DBG(fmt...) \
    printf("%s,%d:", __FUNCTION__, __LINE__);\
    printf(fmt)
#else
#define DBG(fmt) \
    do{ \
    }while(0)
#endif

#define ROL(val, bits, wide, mask) do{ \
    *(val) = (*(val)<<(bits) | *(val)>>((wide) - (bits))) & (mask); \
    }while(0)
#define ROR(val, bits, wide, mask) do{ \
    *(val) = (*(val)>>(bits) | *(val)<<((wide) - (bits))) & (mask); \
    }while(0)
#define min(a,b) (((a) < (b)) ? (a) : (b))
static unsigned int PAL_MAP[13] = {
    0x00000000, 0x01000000, 0x01001000, 0x01010100,
    0x01041040, 0x01084210, 0x01111088, 0x01122448,
    0x01249248, 0x0124a492, 0x014a5294, 0x0154aa54,
    0x01555554,
};

static unsigned int NTSC_MAP[16] = {
    0x00000000, 0x20000000, 0x20004000, 0x20080200,
    0x20404080, 0x20820820, 0x21084210, 0x22211108,
    0x24444888, 0x24892248, 0x24924924, 0x24a4a4a4,
    0x25294a52, 0x2a54a952, 0x2aaa5554, 0x2aaaaaaa,
};

unsigned int GROUP_MAP[121] = {
    /*d1 = 5, cif = 1 ~ 11*/
    0xc8880400, 0xc8880600, 0xc8880700, 0xc8880740, 0xc8880760, 0xc8880770, 0xc8880774, 0xc8880776, 0xc8880777, 0xf80007f7, 0xf80007ff,
   /*d1 = 6, cif = 1 ~ 10*/
    0xe8880400, 0xe8880600, 0xe8880700, 0xe8880740, 0xe8880760, 0xe8880770, 0xe8880774, 0xe8880776, 0xc8880777, 0xf880077f, 0x0,
   /*d1 = 7, cif = 1 ~ 9*/
    0xf8880400, 0xf8880600, 0xf8880700, 0xf8880740, 0xf8880760, 0xf8880770, 0xf8880774, 0xf8880776, 0xf8880777, 0x0, 0x0,
   /*d1 = 8, cif = 1 ~ 8*/
    0xee880040, 0xee880060, 0xee880070, 0xee880074, 0xee880076, 0xee880077, 0xfe80007f, 0xff0000ff, 0x0, 0x0, 0x0,
   /*d1 = 9, cif = 1 ~ 7*/
    0xeec80004, 0xeec80006, 0xeec80007, 0xfe880074, 0xfe880076, 0xfe880077, 0xff80007f, 0x0, 0x0, 0x0, 0x0,
   /*d1 = 10, cif = 1 ~ 6*/
    0xff880040, 0xff880060, 0xff880070, 0xff880074, 0xff880076, 0xff880077, 0x0, 0x0, 0x0, 0x0, 0x0,
   /*d1 = 11, cif = 1 ~ 5*/
    0xffc80004, 0xffc80006, 0xffc80007, 0xffe0000f, 0x0 /*not exist */, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
   /*d1 = 12, cif = 1 ~ 4*/
    0xffe80004, 0xffe80006, 0xffe80007, 0xfff0000f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
   /*d1 = 13, cif = 1 ~ 3*/
    0xfff80004, 0xfff80006, 0xfff80007, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
   /*d1 = 14, cif = 1 ~ 2*/
    0x0,        0x0,   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
   /*d1 = 15, cif = 1*/
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

static int PREVIEW_TABLE[ISIL_MAX_GRP][ISIL_CHN_NO] = {
    {2, 3, 0, 1}, {0, 1, 2, 3}, {0, 1, 2, 3}, {0, 1, 2, 3},
};
int get_bits_map(int m, int n)
{
    int i, div, half, small;
    unsigned int map, mask;

    mask = 0;
    for(i = 0; i < m; i++){
        mask |= (1 << i);
    }

    if(n > m){
        return mask;
    }

    map =0x00000000;
    if((m == 0) || (n == 0)){
        return map;
    }else if(m <= n){
        return mask;
    }

    half = ((m) / 2);
    if(n <= half){
        small = n;
    }else{
        small = m - n;
    }
    div = m / small;

    if(m > 0){
        for(i = 0; i < small; i++){
            map |= 1 << (i * div);
        }
    }

    if(n > half){
        map = (~map) & mask;
    }
    while(!(map & 0x00000001)){
        ROR(&map, 1, m, mask);
    }

    return (map & mask);
}
static inline int locate_map(int d1, int cif)
{

    if((d1 > 15) || (d1 < 5) || (cif < 1) || (cif > 11))
    {
        return 54;
    }

    
    return (d1 - 5) * 11 + cif - 1;
}

/*
cb_resolve_grp: configure group input select,format and frame rate
in_fmt: input format from every vi
in_frm: input frame rate
vgrp:   point to video group
total:  total channel
grpcnt: group count
 */
static int cb_resolve_grp(int *in_chnl, int *in_fmt, int *in_frm,  
        VGROUP_S *vgrp, int total, int grpcnt)
{
    int i = 0, j = 0, k = 0, d1_cnt = 0, halfd1_cnt = 0, temp = 0, cif_cnt = 0;
    int inval = 0;
    int format[ISIL_MAX_VI], frm_rate[ISIL_MAX_VI];
    int temp_ch[ISIL_MAX_VI] = {0};
    int d1_ptr, cif_ptr, dir = 0;
    unsigned int grp_map = 0x0;

    if((total <= 0) || (grpcnt <= 0))
    {
        return 0;
    }

    memset(format, 0, sizeof(int) * ISIL_MAX_VI);
    memset(frm_rate, 0, sizeof(int) * ISIL_MAX_VI);
    for(i = 0; i < grpcnt; i++)
    {
        for(j = 0; j < ISIL_CHN_NO; j++)
        {
            vgrp[i].in_select[j]    = ISIL_INVAL_CHN;
            vgrp[i].in_fmt[j]       = ISIL_INVAL;
            vgrp[i].in_rate[j]      = -1;
        }
    }

    /*count d1, and sort by d1*/
    for(i = 0, j = 0, k = 0; i < total; i++)
    {
        if(in_chnl[i] == ISIL_INVAL_CHN)
        {
            inval++;
            continue;
        }
        if((in_fmt[i] == ISIL_D1) || (in_fmt[i] == ISIL_HalfD1))
        {
            temp_ch[j]   = in_chnl[i];
            format[j]    = in_fmt[i];
            frm_rate[j]  = in_frm[i];
            d1_cnt++;
            if(in_fmt[i] == ISIL_HalfD1)
            {
                halfd1_cnt++;
            }
            j++;
        }
        else
        {
            temp_ch[total - k - 1] = in_chnl[i];
            format[total - k - 1]    = in_fmt[i];
            frm_rate[total - k - 1] = in_frm[i];
            k++;
        }
    }
    total -= inval;
    /*L->H sort d1 by frame*/
    for(i = 0; i < (d1_cnt -1); i++)
    {
        for(j = i; j < d1_cnt; j++)
        {
            if(frm_rate[i] > frm_rate[j])
            {
                temp        = temp_ch[i];
                temp_ch[i]  = temp_ch[j];
                temp_ch[j]  = temp;

                temp        = format[i];
                format[i]   = format[j];
                format[j]   = temp;

                temp        = frm_rate[i];
                frm_rate[i] = frm_rate[j];
                frm_rate[j] = temp;
            }
        }
    }
    
    /*L->H sort cif by frame*/
    for(i = d1_cnt; i < total -1; i++)
    {
        for(j = i; j < total; j++)
        {
            if(frm_rate[i] > frm_rate[j])
            {
                temp        = temp_ch[i];
                temp_ch[i]  = temp_ch[j];
                temp_ch[j]  = temp;

                temp        = format[i];
                format[i]   = format[j];
                format[j]   = temp;

                temp        = frm_rate[i];
                frm_rate[i] = frm_rate[j];
                frm_rate[j] = temp;
            }
        }
    }
	cif_cnt = total - d1_cnt;
    /*channel limit*/
    //memset(vgrp, 0x0, sizeof(VGROUP_S) * grpcnt);
    if(total <= grpcnt)
    {
        for(i = 0; i < total; i++)
        {
            vgrp[i].in_select[0]    = temp_ch[i];
            vgrp[i].in_fmt[0]       = format[i];
            vgrp[i].in_rate[0]      = frm_rate[i];
            vgrp[i].total           = 1;
            
        }
        
        return 0;       
    }
    
    if((d1_cnt <= grpcnt) || (d1_cnt == total) || (halfd1_cnt == total))
    {
		/*
        for(i = 0, j = 0, k = 0; j < total ; i++, j++)
        {
            if(i == grpcnt)
            {
                i = 0;
                k++;
            }
            vgrp[i].total           = k + 1;
            vgrp[i].in_select[k]    = temp_ch[j];
            vgrp[i].in_fmt[k]       = format[j];
            vgrp[i].in_rate[k]      = frm_rate[j];
        }

        return 0;
		*/
		grp_map = 0xffffffff;
    }else{
		grp_map = GROUP_MAP[locate_map(d1_cnt, cif_cnt)];
	}
  
    if(!grp_map)
    {
        /*don't exsit map*/
        return -1;
    }
    d1_ptr = d1_cnt - 1;
    cif_ptr= d1_cnt;
    for(j = 0; j < grpcnt; j++){
    	dir = !dir;
    	if(dir){
    	    for(i = (grpcnt - 1); i >= 0; i--){
        		if(grp_map & (0x80000000 >> (i * ISIL_CHN_NO + j)))/*mask D1*/
        		{
					if(d1_ptr < 0){
						continue;
					}
        		    vgrp[i].in_select[j]    = temp_ch[d1_ptr];
        		    vgrp[i].in_fmt[j]       = format[d1_ptr];
        		    vgrp[i].in_rate[j]      = frm_rate[d1_ptr];
        		    vgrp[i].total++;
        		    d1_ptr--;
        		}
    	    }
    	}else{
    	    for(i = 0; i < grpcnt; i++){
        		if(grp_map & (0x80000000 >> (i * ISIL_CHN_NO + j)))/*mask D1*/
        		{
					if(d1_ptr < 0){
						continue;
					}
        		    vgrp[i].in_select[j]    = temp_ch[d1_ptr];
        		    vgrp[i].in_fmt[j]       = format[d1_ptr];
        		    vgrp[i].in_rate[j]      = frm_rate[d1_ptr];
        		    vgrp[i].total++;
        		    d1_ptr--;
        		}
    	    }
    
    	}
    }

    for(i = 0; i < grpcnt; i++)
    {
        for(j = 0; j < ISIL_CHN_NO; j++)
        {
            if(grp_map & (0x00008000 >> (i * ISIL_CHN_NO + j)))/*mask CIF*/
            {
				if(cif_ptr >= total){
					continue;
				}
                if(vgrp[i].in_select[j] != ISIL_INVAL_CHN) {
                    continue;
                }
                vgrp[i].in_select[j]    = temp_ch[cif_ptr];
                vgrp[i].in_fmt[j]       = format[cif_ptr];
                vgrp[i].in_rate[j]      = frm_rate[cif_ptr];
                cif_ptr++;
				vgrp[i].total++;
            }
        }
    }

    return 0;
}

/*adjust channel frame rate*/
static int cb_resolve_outframe(VGROUP_S *vgrp, int norm, int ispreview)
{
    
    int i = 0, j = 0, cif_cnt = 0, halfd1_cnt = 0, d1_cnt = 0;
    int sum = 0, lost = 0, leave = 0;
    int max_frmrate = (norm == ISIL_VI_PAL) ? ISIL_GRP_MAX_PFRAME:ISIL_GRP_MAX_NFRAME;
    
    /*only real time allow*/
    if(ispreview)
    {
        memcpy(vgrp->out_rate, vgrp->in_rate, sizeof(int) * ISIL_CHN_NO);
		for(i = 0; i < vgrp->total; i++)
		{
			switch(vgrp->in_fmt[i])
			{
			case ISIL_CIF:
				cif_cnt++;
				break;
			case ISIL_HCIF:
				cif_cnt++;
				break;
			case ISIL_QCIF:
				cif_cnt++;
				break;
			case ISIL_HalfD1:
				halfd1_cnt++;
				sum += vgrp->in_rate[i];
				break;
			case ISIL_D1:
				d1_cnt++;
				sum += vgrp->in_rate[i];
				break;
			default:
				return -1;
			}
		}


		if(d1_cnt && !halfd1_cnt && !cif_cnt)
		{
			vgrp->mode = MODE_D1;
		}
		else if(d1_cnt && halfd1_cnt && !cif_cnt)
		{
			vgrp->mode = MODE_D1_HALFD1;
		}
		else if(d1_cnt && halfd1_cnt && cif_cnt)
		{
			vgrp->mode = MODE_D1_HALFD1_CIF;
		}
		else if(d1_cnt && !halfd1_cnt && cif_cnt)
		{
			vgrp->mode = MODE_D1_CIF;
		}
		else if(!d1_cnt && halfd1_cnt && !cif_cnt)
		{
			vgrp->mode = MODE_HALFD1;
		}
		else if(!d1_cnt && halfd1_cnt && cif_cnt)
		{
			vgrp->mode = MODE_HALFD1_CIF;
		}
		else if(!d1_cnt && !halfd1_cnt && cif_cnt)
		{
			vgrp->mode = MODE_CIF;
		}
		else if(!d1_cnt && !halfd1_cnt && !cif_cnt)
		{
			vgrp->mode = MODE_NONE;
		}

        return 0;
    }

    for(i = 0; i < vgrp->total; i++)
    {
        switch(vgrp->in_fmt[i])
        {
            case ISIL_CIF:
                cif_cnt++;
                break;
            case ISIL_HCIF:
                cif_cnt++;
                break;
            case ISIL_QCIF:
                cif_cnt++;
                break;
            case ISIL_HalfD1:
                halfd1_cnt++;
                sum += vgrp->in_rate[i];
                break;
            case ISIL_D1:
                d1_cnt++;
                sum += vgrp->in_rate[i];
                break;
            default:
                return -1;
        }
    }

    if((((d1_cnt + halfd1_cnt) == 2)||((d1_cnt + halfd1_cnt) == 3)) && cif_cnt)
    {
        return -1;
    }

    if(d1_cnt && !halfd1_cnt && !cif_cnt)
    {
        vgrp->mode = MODE_D1;
    }
    else if(d1_cnt && halfd1_cnt && !cif_cnt)
    {
        vgrp->mode = MODE_D1_HALFD1;
    }
    else if(d1_cnt && halfd1_cnt && cif_cnt)
    {
        vgrp->mode = MODE_D1_HALFD1_CIF;
    }
    else if(d1_cnt && !halfd1_cnt && cif_cnt)
    {
        vgrp->mode = MODE_D1_CIF;
    }
    else if(!d1_cnt && halfd1_cnt && !cif_cnt)
    {
        vgrp->mode = MODE_HALFD1;
    }
    else if(!d1_cnt && halfd1_cnt && cif_cnt)
    {
        vgrp->mode = MODE_HALFD1_CIF;
    }
    else if(!d1_cnt && !halfd1_cnt && cif_cnt)
    {
        vgrp->mode = MODE_CIF;
    }
    else if(!d1_cnt && !halfd1_cnt && !cif_cnt)
    {
        vgrp->mode = MODE_NONE;
    }

    lost = sum;
    memcpy(vgrp->out_rate, vgrp->in_rate, sizeof(int) * ISIL_CHN_NO);
    
    if((d1_cnt + halfd1_cnt) > 1)
    {
        while(sum > max_frmrate)
        {
            for(i = 0; i < vgrp->total; i++)
            {
                if(vgrp->out_rate[i] > (max_frmrate / ISIL_CHN_NO))
                {
                    vgrp->out_rate[i]--;
                    sum--;
                    if(sum <= max_frmrate)
                    {
                        break;
                    }
                }
            }
        }
    }

    vgrp->totalfrm = sum;
    lost -= sum;

    return lost;
}

static int verfiy_map(int total, int *map, int bitcnt)
{
    int i = 0, j = 0, frate = 0;
    int *temp_map = map;

    for(i = 0; i < bitcnt; i++)
    {
        frate = 0;
        for(j = 0; j < total; j++)
        {
            frate += (temp_map[j]>>i) & 0x01;
        }
        if(frate > 2)
        {
            return -1;
        }
        if((frate >= 2) && ((
                    ((temp_map[0]>>(i+1)) & 0x01)
                   + ((temp_map[1]>>(i+1)) & 0x01)
                   + ((temp_map[2]>>(i+1)) & 0x01)
                   + ((temp_map[3]>>(i+1)) & 0x01)) >=2))
        {
            return -1;
        }
    }

    return 0;
}

/*calculate frame bit map*/
//param  :offsetmap.changebusflag 仅用于预览，其中offsetmap 用作MAP移位；changebusflag:用于处理8D1以上情况
static int cb_resolve_map(VGROUP_S *vgrp, int norm, int ispreview,int *offsetmap,int changebusflag)
{
    int i = 0, j = 0, k = 0, have_cif = 0;
    int frate = 0;
    int max_frmrate = (norm == ISIL_VI_PAL) ? 25:30;
    int *temp_rate = vgrp->out_rate;
    int *temp_ch   = vgrp->in_select;
    int *temp_map  = vgrp->in_map;
	int *temp_fmt  = vgrp->in_fmt;
    unsigned int mask = 0x1ffffff;

	int *offset = offsetmap;
//	changebusflag = 0;                    //不用进行开启一次后再关一次的处理

    /*bit map*/
    memcpy(temp_ch,   vgrp->in_select,  sizeof(int) * ISIL_CHN_NO);
    memset(temp_map, 0, sizeof(int) * ISIL_CHN_NO);
   
    if(ispreview)
    {
        for(i = 0; i < ISIL_CHN_NO; i++)
        {
            if(vgrp->in_select[i] != -1)
            {
                if(norm == ISIL_VI_PAL){
					mask = 0x1ffffff;
					if(vgrp->out_rate[i] < ((max_frmrate + 1) >> 1)){
						(*offset)++;
						if (changebusflag)
						{
							(*offset)++;
						}
		
						temp_map[i] = PAL_MAP[vgrp->out_rate[i]];
						ROL(&temp_map[i], *offset, max_frmrate, mask);
						vgrp->in_map[i]  = temp_map[i];
						
					}
					else{
						(*offset)++;
						if (changebusflag)
						{
							(*offset)++;
						}
						temp_map[i] = ~PAL_MAP[max_frmrate - vgrp->out_rate[i]];
						temp_map[i] &= mask;
             
						ROL(&temp_map[i], *offset, max_frmrate, mask);
						vgrp->in_map[i]  = temp_map[i];
						
					}
            }
            else{
                mask = 0x3fffffff;
                if(vgrp->out_rate[i] <= ((max_frmrate + 1) >> 1)){
					(*offset)++;
					if (changebusflag)
					{
						(*offset)++;
					}
					temp_map[i] = NTSC_MAP[vgrp->out_rate[i]];
                    ROL(&temp_map[i], *offset, max_frmrate, mask);
					vgrp->in_map[i]  = temp_map[i];
                }
                else{
					(*offset)++;
					if (changebusflag)
					{
						(*offset)++;
					}
					temp_map[i] = ~NTSC_MAP[max_frmrate - vgrp->out_rate[i]];
					temp_map[i] &= mask;
                    ROL(&temp_map[i], *offset, max_frmrate, mask);
                    vgrp->in_map[i]  = temp_map[i];
					
                }
             
                }
                vgrp->in_map[i] &= mask;
            }
        }

        return 0;
    } 

    for(i = 0; i < vgrp->total; i++)
    {
        if((vgrp->in_fmt[i] == ISIL_CIF) ||  (vgrp->in_fmt[i] == ISIL_HCIF) || (vgrp->in_fmt[i] == ISIL_QCIF))
        {
            have_cif = 1;
        }
        if(norm == ISIL_VI_PAL)
        {
            if(vgrp->out_rate[i] < ((max_frmrate + 1) >> 1))
            {
                vgrp->in_map[i]  = PAL_MAP[vgrp->out_rate[i]];
            }
            else
            {
                vgrp->in_map[i]  = ~PAL_MAP[max_frmrate - vgrp->out_rate[i]];
            }
            mask = 0x1ffffff;
        }
        else
        {
            if(vgrp->out_rate[i] <= ((max_frmrate + 1) >> 1))
            {
                vgrp->in_map[i]  = NTSC_MAP[vgrp->out_rate[i]];
            }
            else
            {
                vgrp->in_map[i]  = ~NTSC_MAP[max_frmrate - vgrp->out_rate[i]];
            }
            mask = 0x3fffffff;
        }
        vgrp->in_map[i] &= mask;
    }

    if(have_cif || ((vgrp->mode == MODE_HALFD1) && (vgrp->total == 2)))
    {
        return 0;
    }
    /*sort by frame rate msb*/
    for(i = 0; i < vgrp->total - 1; i++)
    {
        for(j = i; j < vgrp->total; j++)
        {
            if(temp_rate[i] < temp_rate[j])
            {
                /*switch in frame  rate*/
                frate = vgrp->in_rate[i];
                vgrp->in_rate[i] = vgrp->in_rate[j];
                vgrp->in_rate[j] = frate;
                /*switch out frame rate*/
                frate = temp_rate[i];
                temp_rate[i] = temp_rate[j];
                temp_rate[j] = frate;
                /*switch channel number*/
                frate = temp_ch[i];
                temp_ch[i] = temp_ch[j];
                temp_ch[j] = frate;
                /*switch map*/
                frate = temp_map[i];
                temp_map[i]= temp_map[j];
                temp_map[j]= frate;
				/*switch formet*/
				frate = temp_fmt[i];
				temp_fmt[i]= temp_fmt[j];
				temp_fmt[j]= frate;
            }
        }

    }

    if(norm == ISIL_VI_NTSC)
    {
	if(temp_rate[0] == 9)
	    j=1;
	else
	    j=2;
    }
    else
    {
	if((temp_rate[0] > 7) && (temp_rate[0] < 10))
	    j=1;
	else
	    j=3;
    }
    for(i = 0; i < vgrp->total; i++)
    {
        /*Rotate Left*/
        ROL(&temp_map[i], j*i, max_frmrate, mask);
    }
    /*verfiy*/
    if(!verfiy_map(vgrp->total, temp_map, max_frmrate))
    {
        return 0;
    }

    for(i = 0; i <= max_frmrate; i++)
    {
        ROR(&temp_map[1], 1, max_frmrate, mask);
        for(j = 0; j <= max_frmrate; j++)
        {
            ROR(&temp_map[2], 1, max_frmrate, mask);
            for(k = 0; k <= max_frmrate; k++)
            {
                ROR(&temp_map[3], 1, max_frmrate, mask);
                if(!verfiy_map(vgrp->total, temp_map, max_frmrate))
                {
                    return 0;
                }
            }
        }
    }

    return -1; 
}

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
    adjust: adjust out put frame rate, 0->disable, other->enable; 
output:
    successful 0, if adjust == 0 then return lost frame count, other -1;
 */
int cb_ctrl_calculate(int *chn, int *format, int *rate, int norm, VGROUP_S *vgroup, int count, int adjust)
{
    int i = 0, j = 0, lost = 0;
    int s32Ret = 0;
    int invalid = 0;
    VGROUP_S local_grp[ISIL_MAX_GRP];
    int local_chn[ISIL_MAX_VI];
    int local_format[ISIL_MAX_VI];
    int local_rate[ISIL_MAX_VI]; 
    int chan_mask = 0;

    memset(local_grp, 0, sizeof(VGROUP_S) * ISIL_MAX_GRP);
    memset(local_chn, 0, sizeof(local_chn));
    memset(local_format, 0, sizeof(local_format));
    memset(local_rate, 0, sizeof(local_rate));

    if((count < 0) || (count > ISIL_MAX_VI)) {
        return -1;
    }
    for(i = 0; i < count; i++)
    {
        if((chn[i] >= ISIL_MAX_VI) || (chn[i] < 0))
        {
            return -1;
        }
        local_chn[i]    = chn[i];
        local_format[i] = format[i];
        local_rate[i]   = rate[i];
        chan_mask |= 1<<local_chn[i];
    }
    /*locate unused channel*/
    j = 0;
    for(; i < ISIL_MAX_VI; i++)
    {
        while((chan_mask&(1 << j)) && (j < ISIL_MAX_VI))
            j++;
        local_chn[i]    = j++;
        local_format[i] = ISIL_D1;
        local_rate[i]   = 0;
    }

    s32Ret = cb_resolve_grp(local_chn, local_format, local_rate, local_grp, count, ISIL_MAX_GRP);
    if(s32Ret){
        return s32Ret;
    }

    invalid = count;
    for(i = 0; i < ISIL_MAX_GRP; i++)
    {
        lost += cb_resolve_outframe(&local_grp[i], norm, 0);
        s32Ret = cb_resolve_map(&local_grp[i], norm, 0,&count,count);
        if(s32Ret){
            return -1;
        }
        for(j = 0; j < ISIL_CHN_NO; j++)
        {
            if(local_grp[i].total < (j + 1))
            {
                local_grp[i].in_select[j] = local_chn[invalid++];
                local_grp[i].in_map[j]    = 0x0;
                local_grp[i].in_rate[j]   = 0x0;
                local_grp[i].out_rate[j]  = 0x0;
                local_grp[i].in_fmt[j]    = ISIL_D1;
            }
        }
        local_grp[i].grp_id = i;
    }

    if(lost < 0)
    {
        return -1;
    }
    memcpy(vgroup, local_grp, sizeof(VGROUP_S) * ISIL_MAX_GRP);

    if(lost && !adjust)
    {
        return lost;
    }


    return 0;
}

static int inline sort(int *src, int *data1, int *data2, int len)
{
    int i, j, temp;

    for(i = 0; i < (len -1); i++)
    {
        for(j = i; j < len; j++)
        {
            if(src[i] > src[j])
            {
                temp    = src[i];
                src[i]  = src[j];
                src[j]  = temp;

                temp     = data1[i];
                data1[i] = data1[j];
                data1[j] = temp;

                temp     = data2[i];
                data2[i] = data2[j];
                data2[j] = temp;
            }
        }
    }

    return 0;
}

static int setvgrp(int *in_chnl, int *in_fmt, int *in_frm,  
        VGROUP_S *vgrp, int bufNc, int channelNc,int offset,int fps)
{
	int i,j;
    i = bufNc;
	j = channelNc;
	if(vgrp[i].in_select[j] != -1)
	{
		return -1;
	}
	else
	{
		vgrp[i].in_select[j] = in_chnl[offset];
		vgrp[i].in_rate[j] = fps;
		vgrp[i].in_fmt[j] = in_fmt[offset];
		vgrp[i].total++;
	}
    return 0;
}
/*
preview_resolve_grp: configure group input select,format and frame rate
in_fmt: input format from every vi
in_frm: input frame rate
vgrp:   point to video group
total:  total channel
grpcnt: group count
 */
static int preview_resolve_grp(int *in_chnl, int *in_fmt, int *in_frm,  
        VGROUP_S *vgrp, int total, int grpcnt,int norm)
{
    int i, j, k, temp,offset, d1_cnt = 0, cif_cnt = 0, qcif_cnt = 0;
	int flagchannel=0,validflag = 0;  
    int totalcnt = 0;
	int fps = 0;
	//validflag    0:RT  1:PRT;  3: error
	/*resort*/
    if(!in_chnl || !in_fmt || !in_frm || !vgrp || (total > ISIL_MAX_VI) 
            || (total < 0) || (grpcnt > ISIL_MAX_GRP) || (grpcnt < 0))
    {
        DBG("inval\n");
        return -1;
    }

    sort(in_fmt, in_chnl, in_frm, total);
    offset = 0;

	for(i=0;i<total;i++)
	{
		if((in_fmt[i]==ISIL_D1)||(in_fmt[i]==ISIL_HalfD1))
			d1_cnt++;
		else if((in_fmt[i]==ISIL_HCIF)||(in_fmt[i]==ISIL_CIF))
			cif_cnt++;
		else
			qcif_cnt++;
	}
//	totalcnt =2*d1_cnt + cif_  cnt;//+(qcif_cnt>>1); 
    totalcnt = 2*d1_cnt + cif_cnt + (qcif_cnt>>1);
	
	if(totalcnt<9)           //实时
		validflag = 0;
	else if (totalcnt<17)    //非实时
	{
		validflag = 1;
	}
	else validflag = 3;

	if(((cif_cnt==9)||(cif_cnt==10))&&(qcif_cnt==0)&&(d1_cnt==0))
		validflag = 0;

	////////////////////////////////////////////////
	//for 9～16D1
	if ((d1_cnt>8)&&(qcif_cnt==0)&&(cif_cnt==0))
	{
		validflag = 1;
	}

	if (validflag == 3)
	{
		DBG("inval\n");
        return -1;
	}
	if (!validflag)
	{
		fps = (norm == ISIL_VI_PAL) ? 25:30; 
	}
	else
	{
		temp = (norm == ISIL_VI_PAL) ? 200:240;
		  //maybe we get a table;
		if(d1_cnt<5)
		{
			fps = temp/totalcnt;  
		}
		else if((d1_cnt>4)&&(d1_cnt<9))      //5,6,7,8
		{
			int t = (temp*0.35)/(d1_cnt);
	        fps = temp/totalcnt;
			fps = min(fps,t);       //8 channels
		}
		else if((d1_cnt>8)&&(d1_cnt<13))
		{
			fps = temp/(24);       //12 channels
		}
		else if(d1_cnt>12)
		{
			fps = temp/32;         //16 channels

		}

		if (d1_cnt>8)
		{
			fps=(fps>>1);
		}
		
	}

    
	if(d1_cnt>8)
	{
	   for(j=0;j<4;j++)
	   {
			for(i=0;i<4;i++)
			{
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,i,j,offset,fps);
				d1_cnt--;
				if (d1_cnt==0)
				{
					break;
				}
				offset++;
			}
			if (d1_cnt==0)  break;
				
	   }
      return 0;
	}

	i = 0; j = 0;offset = 0;
//给各通道，分配Buffer
	while(offset < total)
	{
		i = 1;j = 0;
		if(d1_cnt%2)
		{
			i=0;
			j=2;
			setvgrp(in_chnl, in_fmt, in_frm, vgrp,i,j,offset,fps);
			d1_cnt--;
			offset++;
			i++;
		}
		while((!(d1_cnt%2))&&(d1_cnt!=0)&&(vgrp[i].in_select[j]==-1))
		{
			for(j=0;j<2;j++)
			{
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,i,j*2,offset,fps);
				d1_cnt--;
				offset++;
			}
			i++;
			if((i==4)&&(d1_cnt))
			{
				i=0;j=0;
			}
		}
		temp = (cif_cnt%4);
        switch(temp)
		{
		  case 1:
			  if(vgrp[0].in_select[2]==-1)
			  {
				  i = 0;j = 2;
				  setvgrp(in_chnl, in_fmt, in_frm, vgrp,i,j,offset,fps);
				  cif_cnt--;
				  offset++;
			  }
			  else
			  {
				  i = 0;j=0; 
				  setvgrp(in_chnl, in_fmt, in_frm, vgrp,i,j,offset,fps);
				  cif_cnt--;
				  offset++;
			  }
			  break;
		  case 2:
			  setvgrp(in_chnl, in_fmt, in_frm, vgrp,0,0,offset,fps);
			  cif_cnt--;
			  offset++;
			  setvgrp(in_chnl, in_fmt, in_frm, vgrp,0,1,offset,fps);
			  cif_cnt--;
			  offset++;
			  break;
		  default:
			  break;
		}
		i= 1;j =0;
		while((cif_cnt!=0))
		{
			j = 0;
			while((vgrp[i].in_select[j]!=-1)&&(i<4)) i++;
			if((i==4)&&(cif_cnt))
			{
				i=0;j=0;
			}
			for(j=0;j<4;j++)
			{
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,i,j,offset,fps);
				cif_cnt--;
				offset++;
				if(cif_cnt==0)break;
			}
		}
		//qcif
		if (!qcif_cnt)
		{
			return 0;
		}
		temp = (qcif_cnt%4);
        switch(temp)
		{
		case 1:
			if(vgrp[0].in_select[2]==-1)
			{
				i = 0;j = 2;
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,i,j,offset,fps);
				qcif_cnt--;
				offset++;
			}
			break;
		case 2:
			if(vgrp[0].in_select[0]==-1)
			{
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,0,0,offset,fps);
				qcif_cnt--;
				offset++;
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,0,1,offset,fps);
				qcif_cnt--;
			    offset++;
			}
			break;
		case 3:
			if(vgrp[0].in_select[0]==-1)
			{
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,0,0,offset,fps);
				cif_cnt--;
				offset++;
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,0,1,offset,fps);
				cif_cnt--;
				offset++;
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,0,3,offset,fps);
				cif_cnt--;
				offset++;
			}
			break;
		default:
			break;
		}
		while((qcif_cnt!=0))
		{
			i =1;j=0;
			while((vgrp[i].in_select[j]!=-1)&&(i<4)) i++;
            if((i==4)&&(qcif_cnt))
			{
				i =0;j=0;
			}
			for(j=0;j<4;j++)
			{
				setvgrp(in_chnl, in_fmt, in_frm, vgrp,i,j,offset,fps);
				qcif_cnt--;
				offset++;
				if(qcif_cnt==0)break;
			}
		}

	}
    return 0;
}

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
    adjust: this version not supprot; 
output:
    successful 0, other -1;
 */
int preview_ctrl_calculate(int *chn, int *format, int *rate, int norm, VGROUP_S *vgroup, int count, int adjust)
{
    int i = 0, j = 0, lost = 0;
    int s32Ret = 0;
    int invalid = 0;
    VGROUP_S local_grp[ISIL_MAX_GRP];
    int local_chn[ISIL_MAX_VI];
    int local_format[ISIL_MAX_VI];
    int local_rate[ISIL_MAX_VI]; 
    int chan_mask = 0;

	int offsetmap = -1;
	int d1_cnt=0,cif_cnt=0,qcif_cnt=0,changeBusflag=0;

    if(!chn || !format || !rate || !vgroup || (count > ISIL_MAX_VI)){
        return -1;
    }
    memset(local_grp, 0xff, sizeof(VGROUP_S) * ISIL_MAX_GRP);
    memset(local_chn, 0, sizeof(local_chn));
    memset(local_format, 0, sizeof(local_format));
    memset(local_rate, 0, sizeof(local_rate));

    /*always discard frame*/    
    adjust = 1;

    for(i = 0; i < count; i++)
    {
        if((chn[i] >= ISIL_MAX_VI) || (chn[i] < 0))
        {
            return -1;
        }
        local_chn[i]    = chn[i];
        local_format[i] = format[i];
        local_rate[i]   = rate[i];
        chan_mask |= 1<<local_chn[i];
    }

    /*locate unused channel*/
    j = 0;
    for(; i < ISIL_MAX_VI; i++)
    {
        while((chan_mask&(1 << j)) && (j < ISIL_MAX_VI))
            j++;
        local_chn[i]    = j++;
        local_format[i] = ISIL_D1;
        local_rate[i]   = 0;
    }

    s32Ret = preview_resolve_grp(local_chn, local_format, local_rate, local_grp, count, ISIL_MAX_GRP,norm);
    if(s32Ret){
        return s32Ret;
    }

    invalid = count;

	//D大于8时，分别对待
	for(i=0;i<count;i++)
	{
		if((local_format[i]==ISIL_D1)||(local_format[i]==ISIL_HalfD1))
			d1_cnt++;
		else if((local_format[i]==ISIL_HCIF)||(local_format[i]==ISIL_CIF))
			cif_cnt++;
		else
			qcif_cnt++;
	}

	if ((d1_cnt>8)&&(qcif_cnt==0)&&(cif_cnt==0))
	{
		changeBusflag = 1;
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

    for(i = 0; i < ISIL_MAX_GRP; i++)
    {
		if (changeBusflag)
		{
			if((i%2)==0)
				offsetmap = -1;
			else offsetmap = 0;
		}

        local_grp[i].total = 0;
        lost += cb_resolve_outframe(&local_grp[i], norm, 1);
	
        s32Ret = cb_resolve_map(&local_grp[i], norm, 1,&offsetmap,changeBusflag);
        if(s32Ret){
            return -1;
        }
        local_grp[i].grp_id = i;
        for(j = 0; j < ISIL_CHN_NO; j++)
        {
            if(local_grp[i].in_select[j] == -1)
            {
                local_grp[i].in_select[j] = local_chn[invalid++];
                local_grp[i].in_map[j]    = 0x0;
                local_grp[i].in_rate[j]   = 0x0;
                local_grp[i].out_rate[j]  = 0x0;
                local_grp[i].in_fmt[j]    = ISIL_QCIF;
            }
        }
    }

    memcpy(vgroup, local_grp, sizeof(VGROUP_S) * ISIL_MAX_GRP);
    if(lost < 0)
    {
        return -1;
    }

    if(lost && !adjust)
    {
        return lost;
    }


    return 0;
}


