#include <isil5864/isil_common.h>

#define	FPGA_DATA_BASE	(0x80000)
#define	FPGA_DATA_END	(0x100000)
#define WDELAYTIME      (40)
//#define	DDRPAGE		(0x030)
//#define	MODE		(0x004)
void Write_One_Frame_To_DDR_D1_Frame(isil_h264_logic_encode_chan_t *encoder, u16 FrameIdx, u16 channel,u8 *buf, enum ISIL_VIDEO_SIZE video_size, enum ISIL_VIDEO_STANDARD standard)
{  
#if 1
    isil_chip_t	*chip;
    u32 i,j,k;
    u32 tmpDDR;
    u8	*line, *line1, *line2;
    u8  *srcBufY,*srcBufU,*srcBufV;
    u16	pageIdxY, pageIdxUV;
    u32 dataTemp;
    u32 width,height;
    chip = encoder->chip;
    width = VIDEO_SIZE_TO_WIDTH(video_size, standard);
    height = VIDEO_SIZE_TO_HEIGHT(video_size, standard);

    switch(FrameIdx)
    {
        case 1:
            pageIdxY = 3;
            break;
        case 2:
            pageIdxY = 6;
            break;
        case 3:
            pageIdxY = 9;
            break;
        default:
            pageIdxY = 0;
            break;
    }
    pageIdxUV = pageIdxY + 2;
    pageIdxY += chip->io_op->chip_read32(chip, 0xd800 + channel * 4) & 0x1fff;
    pageIdxUV += chip->io_op->chip_read32(chip, 0xd800 + channel * 4) & 0x1fff;

    printk("downLoad ddr[Y=%d, UV=%d] data\r\n", pageIdxY, pageIdxUV);
    chip->io_op->chip_write32(chip, DDRPAGE, pageIdxY);
    //write Y     
    for(k=0;k<=(width>>8);k++)
    {
        srcBufY = buf;    
        tmpDDR = FPGA_DATA_BASE+0x40000*k;
        if(tmpDDR == (FPGA_DATA_END))
        {
            chip->io_op->chip_write32(chip, DDRPAGE, (pageIdxY+1));
            tmpDDR = FPGA_DATA_BASE;
        }
        for(j=0;j<height;j++)
        { 
            line = srcBufY+256*k; 
            if((width-256*k)>256)
            {
                for(i=0;i<256;i+=4)
                { 
                    dataTemp = line[i+3];
                    dataTemp <<= 8;
                    dataTemp |= line[i+2];
                    dataTemp <<= 8;
                    dataTemp |= line[i+1];
                    dataTemp <<= 8;
                    dataTemp |= line[i]; 

                    chip->io_op->chip_write32(chip, tmpDDR, dataTemp);
                    //*tmpDDR = dataTemp;
                    tmpDDR += 4;
                }
                srcBufY += width;
            }
            else
            {
                //chip->io_op->chip_write32(chip, DDRPAGE, pageIdxY+1);
                tmpDDR = FPGA_DATA_BASE+256*j;
                for(i=0;i<(width-256*k);i+=4)
                {
                    dataTemp = line[i+3];
                    dataTemp <<= 8;
                    dataTemp |= line[i+2];
                    dataTemp <<= 8;
                    dataTemp |= line[i+1];
                    dataTemp <<= 8;
                    dataTemp |= line[i]; 
                    chip->io_op->chip_write32(chip, tmpDDR, dataTemp);
                    //*tmpDDR = dataTemp;
                    tmpDDR += 4;
                }
                srcBufY += width;
            }
        }
    }

    //write UV
    //platform_host_writel(pageIdxUV | 0x2000,chip->regs+DDRPAGE);
    chip->io_op->chip_write32(chip, DDRPAGE, pageIdxUV);
    for(k=0;k<=(width>>8);k++)
    {
        srcBufU = buf + height*width;
        srcBufV = srcBufU + ((height*width)>>2);   
        tmpDDR = FPGA_DATA_BASE+0x20000*k;
        for(j=0;j<(height>>1);j++)
        { 
            line1 = srcBufU+128*k;
            line2 = srcBufV+128*k;
            if((width-256*k)>256)
            {
                for(i=0;i<128;i+=2)
                {
                    dataTemp = line2[i+1];
                    dataTemp <<= 8;
                    dataTemp |= line1[i+1];
                    dataTemp <<= 8;
                    dataTemp |= line2[i];
                    dataTemp <<= 8;
                    dataTemp |= line1[i]; 
                    //*tmpDDR = dataTemp;
                    chip->io_op->chip_write32(chip, tmpDDR, dataTemp);
                    tmpDDR += 4;
                }
                srcBufU += (width>>1);
                srcBufV += (width>>1);
            }
            else
            {
                tmpDDR = FPGA_DATA_BASE+0x20000*k+256*j;
                for(i=0;i<((width>>1)-128*k);i+=2)
                {
                    dataTemp = line2[i+1];
                    dataTemp <<= 8;
                    dataTemp |= line1[i+1];
                    dataTemp <<= 8;
                    dataTemp |= line2[i];
                    dataTemp <<= 8;
                    dataTemp |= line1[i]; 
                    chip->io_op->chip_write32(chip, tmpDDR, dataTemp);
                    //*tmpDDR = dataTemp;
                    tmpDDR += 4;
                }
                srcBufU += (width>>1);
                srcBufV += (width>>1);
            }
        }
    } 
#else

    u32 i,j,s,k;
    isil_chip_t *chip;
    u16	pageIdxY, pageIdxUV;
    u32 tmpDDR;
    u8	*line,*line0,*line1;//, *line2, *line3;
    //u8	*line4, *line5, *line6, *line7;
    u8  *srcBufY,*srcBufU,*srcBufV;
    u32 dataTemp;
    u32 offset;
    u32 width,  height;

    width = VIDEO_SIZE_TO_WIDTH(video_size, standard);
    height = VIDEO_SIZE_TO_HEIGHT(video_size, standard);

    switch(FrameIdx)
    {
        case 1:
            pageIdxY = 3;
            break;
        case 2:
            pageIdxY = 6;
            break;
        case 3:
            pageIdxY = 9;
            break;
        default:
            pageIdxY = 0;
            break;
    }
    chip = encoder->chip;
    pageIdxUV = pageIdxY + 2;
    pageIdxY += chip->io_op->chip_read32(chip, 0xd800 + channel * 4) & 0x1fff;
    pageIdxUV += chip->io_op->chip_read32(chip, 0xd800 + channel * 4) & 0x1fff;
    printk("Write ddr[Y=%d, UV=%d] data for encoder channel %d's %dth frame,\r\n", pageIdxY, pageIdxUV,channel,FrameIdx);
    chip->io_op->chip_write32(chip, DDRPAGE, pageIdxY);
    mdelay(WDELAYTIME);
    tmpDDR = 0;
    mdelay(WDELAYTIME);
    srcBufY = buf;
    srcBufU =  srcBufY + (width * height);
    srcBufV =  srcBufU +(width * (height>>2));
    //write luma Y
    offset = width<<2;
    i = 0; j = 0; s =0;
    for(i = 0 ; i<= (width >> 8); i++, s = (s+1)%2){
        srcBufY = buf;
        if(2 == i){
            chip->io_op->chip_write32(chip, DDRPAGE, (u32)(pageIdxY+1));
            mdelay(WDELAYTIME);
            tmpDDR = 0;
            mdelay(WDELAYTIME);
        }
        tmpDDR = 0x40000 * s;
        for(j = 0; j < height; j++){
            line = srcBufY + 256*i;
            if((width - 256 *i) > 256){
                for(k = 0; k < 256; k += 4){
                    dataTemp  = 0;
                    dataTemp |= line[k + 0] << 0;
                    dataTemp |= line[k + 1] << 8;
                    dataTemp |= line[k + 2] << 16;
                    dataTemp |= line[k + 3] << 24;
                    chip->io_op->chip_write32(chip, FPGA_DATA_BASE + tmpDDR, dataTemp);
                    tmpDDR += 4;
                }
                srcBufY += width;
            }else{
                for(k = 0; k < (width - 256 * i); k += 4){
                    dataTemp  = 0;
                    dataTemp |= line[k + 0] << 0;
                    dataTemp |= line[k + 1] << 8;
                    dataTemp |= line[k + 2] << 16;
                    dataTemp |= line[k + 3] << 24;
                    chip->io_op->chip_write32(chip, FPGA_DATA_BASE + tmpDDR, dataTemp);
                    tmpDDR += 4;
                }
                srcBufY += width;
            }
        }

    }


    tmpDDR = 0;
    chip->io_op->chip_write32(chip, DDRPAGE, (u32)pageIdxUV);
    for(i = 0 ; i<= (width >> 8); i++){
        tmpDDR = 0x20000 * i;
        for(j = 0; j < (height >> 1); j++){
            line0 = srcBufU + 128*i;
            line1 = srcBufV + 128*i;
            if((width - 256 *i) > 256){
                for(k = 0; k < 128; k += 2){
                    dataTemp = 0;
                    dataTemp |= line0[k + 0] << 0;
                    dataTemp |= line1[k + 0] << 8;
                    dataTemp |= line0[k + 1] << 16;
                    dataTemp |= line1[k + 1] << 24;
                    chip->io_op->chip_write32(chip, FPGA_DATA_BASE + tmpDDR, dataTemp);
                    tmpDDR += 4;
                }
                //printk("UV tmpDDR=[%d],j=[%d],k=[%d]\n",tmpDDR, j,k);
                srcBufU += (width >> 1);
                srcBufV += (width >> 1);
            }else{
                for(k = 0; k < ((width >> 1) - 128 * i); k += 2){
                    dataTemp = 0;
                    dataTemp |= line0[k + 0] << 0;
                    dataTemp |= line1[k + 0] << 8;
                    dataTemp |= line0[k + 1] << 16;
                    dataTemp |= line1[k + 1] << 24;
                    chip->io_op->chip_write32(chip, FPGA_DATA_BASE + tmpDDR, dataTemp);
                    tmpDDR += 4;
                }
                srcBufU += (width >> 1);
                srcBufV += (width >> 1);
            }
        }

    }
#endif
}	


/* this function even can be called when all the encoder channel was closed. */
void isil_read_encode_orignal_frame(isil_chip_t *chip, u16 channel, u16 FrameIdx, u8 *buf)
{
    u32 i,j,s,k;
    u16	pageIdxY, pageIdxUV;
    u32 tmpDDR;
    u8	*line,*line0,*line1;//, *line2, *line3;
    //u8	*line4, *line5, *line6, *line7;
    u8  *srcBufY,*srcBufU,*srcBufV;
    u32 dataTemp;
    u32 offset;
    u32 width,  height;

    width = 704;
    height = 576;
    printk("0x%p,w%d,h%d\n",buf,width,height);
    switch(FrameIdx)
    {
        case 1:
            pageIdxY = 3;
            break;
        case 2:
            pageIdxY = 6;
            break;
        case 3:
            pageIdxY = 9;
            break;
        default:
            pageIdxY = 0;
            break;
    }
    pageIdxUV = pageIdxY + 2;
    pageIdxY += chip->io_op->chip_read32(chip, 0xd800 + channel * 4) & 0x1fff;
    pageIdxUV += chip->io_op->chip_read32(chip, 0xd800 + channel * 4) & 0x1fff;
    printk("Readback ddr[Y=%d, UV=%d] data for encoder channel %d's %dth frame,\r\n", pageIdxY, pageIdxUV,channel,FrameIdx);
    //temp = chip->io_op->chip_read32(chip, MODE);
    //temp |= DDRON;
    //chip->io_op->chip_write32(chip, MODE, temp);
    chip->io_op->chip_write32(chip, DDRPAGE, pageIdxY);
    mdelay(WDELAYTIME);
    tmpDDR = 0;
    mdelay(WDELAYTIME);
    srcBufY = buf;
    srcBufU =  srcBufY + (width * height);
    srcBufV =  srcBufU +(width * (height>>2));
    //write luma Y
    offset = width<<2;
    i = 0; j = 0; s =0;
    for(i = 0 ; i<= (width >> 8); i++, s = (s+1)%2){
        srcBufY = buf;
        if(2 == i){
            chip->io_op->chip_write32(chip, DDRPAGE, (u32)(pageIdxY+1));
            mdelay(WDELAYTIME);
            tmpDDR = 0;
            mdelay(WDELAYTIME);
        }
        tmpDDR = 0x40000 * s;
        printk("tmpDDR=[%d],i =[%d]\n",tmpDDR, i);
        for(j = 0; j < height; j++){
            line = srcBufY + 256*i;
            if((width - 256 *i) > 256){
                for(k = 0; k < 256; k += 4){
                    //printk("tmpDDR=[%d],j=[%d],k=[%d]\n",tmpDDR, j,k);
                    dataTemp  = chip->io_op->chip_read32(chip, FPGA_DATA_BASE + tmpDDR);
                    line[k] = (dataTemp & 0xff);
                    line[k+1] = (dataTemp >> 8) & 0xff;
                    line[k+2] = (dataTemp >> 16) & 0xff;
                    line[k+3] = (dataTemp >> 24) & 0xff;
                    tmpDDR += 4;
                }
                srcBufY += width;
            }else{
                for(k = 0; k < (width - 256 * i); k += 4){
                    //printk("tmpDDR=[%d],j=[%d],k=[%d] in last\n",tmpDDR, j,k);
                    dataTemp  = chip->io_op->chip_read32(chip, FPGA_DATA_BASE + tmpDDR);
                    line[k] = (dataTemp & 0xff);
                    line[k+1] = (dataTemp >> 8) & 0xff;
                    line[k+2] = (dataTemp >> 16) & 0xff;
                    line[k+3] = (dataTemp >> 24) & 0xff;
                    tmpDDR += 4;                                
                }
                srcBufY += width;
            }
        }

    }


    tmpDDR = 0;
    chip->io_op->chip_write32(chip, DDRPAGE, (u32)pageIdxUV);
    for(i = 0 ; i<= (width >> 8); i++){
        tmpDDR = 0x20000 * i;
        for(j = 0; j < (height >> 1); j++){
            line0 = srcBufU + 128*i;
            line1 = srcBufV + 128*i;
            if((width - 256 *i) > 256){
                for(k = 0; k < 128; k += 2){
                    dataTemp  = chip->io_op->chip_read32(chip, FPGA_DATA_BASE + tmpDDR);
                    line0[k] = (dataTemp & 0xff);
                    line1[k] = (dataTemp >> 8) & 0xff;
                    line0[k + 1] = (dataTemp >> 16) & 0xff;
                    line1[k + 1] = (dataTemp >> 24) & 0xff;
                    tmpDDR += 4;
                }
                //printk("UV tmpDDR=[%d],j=[%d],k=[%d]\n",tmpDDR, j,k);
                srcBufU += (width >> 1);
                srcBufV += (width >> 1);
            }else{
                for(k = 0; k < ((width >> 1) - 128 * i); k += 2){
                    //printk("UV tmpDDR=[%d],j=[%d],k=[%d] in last\n",tmpDDR, j,k);
                    dataTemp  = chip->io_op->chip_read32(chip, FPGA_DATA_BASE + tmpDDR);
                    line0[k] = (dataTemp & 0xff);
                    line1[k] = (dataTemp >> 8) & 0xff;
                    line0[k + 1] = (dataTemp >> 16) & 0xff;
                    line1[k + 1] = (dataTemp >> 24) & 0xff;
                    tmpDDR += 4;                                
                }
                srcBufU += (width >> 1);
                srcBufV += (width >> 1);
            }
        }

    }



}



/* this function even can be called when all the encoder channel was closed. */
void isil_read_encode_rebuild_frame(isil_chip_t *chip, u16 channel, u16 FrameIdx, u8 *buf)
{
    u32 i,j;
    u16	pageIdxY, pageIdxUV;
    u32 tmpDDR;
    u8	*line0, *line1, *line2, *line3;
    u8	*line4, *line5, *line6, *line7;
    u8  *srcBufY,*srcBufU,*srcBufV;
    u32 dataTemp;
    u32 offset;
    u32 width,  height;

    width = 704;
    height = 576;
    printk("0x%p,w%d,h%d\n",buf,width,height);
    switch(FrameIdx)
    {
        case 1:
            pageIdxY = 15;
            break;
        case 2:
            pageIdxY = 18;
            break;
        case 3:
            pageIdxY = 21;
            break;
        default:
            pageIdxY = 12;
            break;
    }
    pageIdxUV = pageIdxY + 2;
    pageIdxY += chip->io_op->chip_read32(chip, 0xd800 + channel * 4) & 0x1fff;
    pageIdxUV += chip->io_op->chip_read32(chip, 0xd800 + channel * 4) & 0x1fff;
    printk("Readback ddr[Y=%d, UV=%d] data for encoder channel %d's %dth frame,\r\n", pageIdxY, pageIdxUV,channel,FrameIdx);
    //temp = chip->io_op->chip_read32(chip, MODE);
    //temp |= DDRON;
    //chip->io_op->chip_write32(chip, MODE, temp);
    chip->io_op->chip_write32(chip, DDRPAGE, pageIdxY);
    mdelay(WDELAYTIME);
    tmpDDR = FPGA_DATA_BASE;
    mdelay(WDELAYTIME);
    srcBufY = buf;
    srcBufU =  srcBufY + (width * height);
    srcBufV =  srcBufU +(width * (height>>2));
    //write luma Y
    offset = width<<2;
    i = 0; j = 0;
    for (i=0; i<(height>>2); i++)
    {
        line0 = srcBufY;
        line1 = line0 + width;
        line2 = line1 + width;
        line3 = line2 + width;
        for (j=0; j<width; j++)
        {
            dataTemp  = 	chip->io_op->chip_read32(chip, tmpDDR);
            line3[j]  = 	(u8)(dataTemp&0xff);
            line2[j]  =  	(u8)((dataTemp>>8)&0xff);
            line1[j]  =  	(u8)((dataTemp>>16)&0xff);
            line0[j]  =  	(u8)((dataTemp>>24)&0xff);
            tmpDDR += 4;
        }

        srcBufY += offset;
        tmpDDR += (0x400 - width)<<2;
        if(tmpDDR == FPGA_DATA_END)
        {
            chip->io_op->chip_write32(chip, DDRPAGE, (u32)(pageIdxY+1));
            mdelay(WDELAYTIME);
            tmpDDR = FPGA_DATA_BASE;
            mdelay(WDELAYTIME);
        }
    }
    ////write chroma U
    chip->io_op->chip_write32(chip, DDRPAGE, pageIdxUV);
    mdelay(WDELAYTIME);
    tmpDDR = FPGA_DATA_BASE;
    mdelay(WDELAYTIME);
    height >>= 3;
    offset = width<<1;
    width >>= 1;
    for(i=0; i<height; i++)
    {
        line0 = srcBufU;
        line1 = line0 +  width;
        line2 = line1 +  width;
        line3 = line2 +  width;

        line4 = srcBufV;
        line5 = line4 +  width;
        line6 = line5 +  width;
        line7 = line6 +  width;

        for(j=0; j<width; j++)
        {
            dataTemp = chip->io_op->chip_read32(chip, tmpDDR);
            line3[j]  =  (u8)(dataTemp&0xff);
            line2[j]  =  (u8)((dataTemp>>8)&0xff);
            line1[j]  =  (u8)((dataTemp>>16)&0xff);
            line0[j]  =  (u8)((dataTemp>>24)&0xff);
            tmpDDR += 4;
            dataTemp = chip->io_op->chip_read32(chip, tmpDDR);
            line7[j]  =  (u8)(dataTemp&0xff);
            line6[j]  =  (u8)((dataTemp>>8)&0xff);
            line5[j]  =  (u8)((dataTemp>>16)&0xff);
            line4[j]  =  (u8)((dataTemp>>24)&0xff);
            tmpDDR += 4;
        }
        srcBufU += offset;
        srcBufV += offset;
        tmpDDR +=((0x400-(width<<1))<<2);
    }
    //chip->io_op->chip_write32(chip, MODE , temp);
}

int isil_read_jpeg_yuv(isil_chip_t *chip, int page, int format, char *buf, int isB)
{
    u32 i,j,k;
    u16	pageIdxY, pageIdxUV;
    u8	*src, *dst, *database;
    u8  *srcBufY,*srcBufU,*srcBufV;
    u32 offset, yuv_offset, uv_stride;
    u32 width,  height;
    width = 720;
    height = 576;

    pageIdxY  = page;
    pageIdxUV = pageIdxY + 1;
    printk("Readback ddr[Y=%d, UV=%d] data 0x%p\n", pageIdxY, pageIdxUV, buf);
    chip->io_op->chip_write32(chip, DDRPAGE, pageIdxY | (isB<<14));
    mdelay(WDELAYTIME);
    srcBufY = buf;
    srcBufU =  srcBufY + width*height;
    srcBufV =  srcBufU +(width*(height>>2));
    //write luma Y
    printk("................Y\n");
    offset = 0;
    dst = srcBufY;
    database = chip->regs + 0x80000;
    for (i = 0; i < 36; i++)
    {
        for(j = 0; j < 16; j++)
        {
            for(k = 0; k < 6; k++)
            {
                yuv_offset = ((i * 16 * 6 * 128) + (j * 128) + (k * 128 * 16));
                src = database + yuv_offset;
                //printk("i = %d, j = %d, k = %d, src 0x%08x, dst 0x%08x, offset 0x%08x\n", i, j, k, src, dst, yuv_offset);
                if(k < 5){
                    memcpy(dst, src, 128);
                    dst += 128;
                    offset += 128;
                    if(offset == 0x80000){
                        //src = chip->regs + 0x80000;
                        chip->io_op->chip_write32(chip, DDRPAGE, pageIdxUV|(isB<<14));
                    }
                }else{
                    memcpy(dst, src, 80);
                    dst += 80;
                    offset += 128;
                    if(offset == 0x80000){
                        //src = chip->regs + 0x80000;
                        chip->io_op->chip_write32(chip, DDRPAGE, pageIdxUV|(isB<<14));
                    }
                }
            }
        }
    }
    printk("................UV ,offset 0x%08x\n", offset);
    ////write chroma U,V
    uv_stride = 0;
    src = database + offset;
    for(i = 0; i < ((width*height) >> 2); i++)
    {
        *srcBufU++ = *src++;
        //dst++;
        *srcBufV++ = *src++;
        //dst++;
        offset += 2;
        uv_stride++;
        if(uv_stride == (width>>1)){
            src += 80;
            offset += 80;
            uv_stride = 0;
        }
        if(offset == 0x80000){
            src = chip->regs + 0x80000;
            chip->io_op->chip_write32(chip, DDRPAGE, pageIdxUV|(isB<<14));
        }
    }

    return 0;
}

int isil_write_jpeg_yuv(isil_chip_t *chip, int page, int format, char *buf, int isB)
{
    u32 i,j,k;
    u16	pageIdxY, pageIdxUV;
    u8	*src, *dst;
    u8  *srcBufY,*srcBufU,*srcBufV;
    u32 offset;
    u32 width,  height;
    width = 720;
    height = 576;

    pageIdxY  = page;
    pageIdxUV = pageIdxY + 1;
    printk("write ddr[Y=%d, UV=%d] data 0x%p\n", pageIdxY, pageIdxUV, buf);
    chip->io_op->chip_write32(chip, DDRPAGE, pageIdxY | (isB<<14));
    mdelay(WDELAYTIME);
    srcBufY = buf;
    srcBufU =  srcBufY + width*height;
    srcBufV =  srcBufU +(width*(height>>2));
    //write luma Y
    printk("................Y\n");
    offset = 0;
    src = srcBufY;
    dst = chip->regs + 0x80000;
    for (i = 0; (i < height/16); i++)
    {
        for(j = 0; j < 16; j++)
        {
            for(k = 0; k < 6; k++)
            {
                src = srcBufY + ((i * 16 * 6 * 128) + (j * 128) + (k * 128 * 16));
                if(k < 5){
                    memcpy(dst, src, 128);
                    dst += 128;
                    offset += 128;
                    if(offset == 0x80000){
                        dst = chip->regs + 0x80000;
                        chip->io_op->chip_write32(chip, DDRPAGE, pageIdxUV|(isB<<14));
                    }
                }else{
                    memcpy(dst, src, 80);
                    dst += 128;
                    offset += 128;
                    if(offset == 0x80000){
                        dst = chip->regs + 0x80000;
                        chip->io_op->chip_write32(chip, DDRPAGE, pageIdxUV|(isB<<14));
                    }
                }
            }
        }
    }
    printk("................UV\n");
    ////write chroma U,V
    for(i = 0; i < ((width*height) >> 2); i++)
    {
        *dst++ = *srcBufU++;
        //dst++;
        *dst++ = *srcBufV++;
        //dst++;
        offset += 2;
        if(offset == 0x80000){
            dst = chip->regs + 0x80000;
            chip->io_op->chip_write32(chip, DDRPAGE, pageIdxUV|(isB<<14));
        }
    }

    return 0;
}
#if 0
void DVM_New_Read_Enc_One_Frame_from_DDR(isil_h264_logic_decode_chan_t* decode, u32 width, u32 height, u16 FrameIdx, u8 *buf)
{

    isil_chip_t	*chip;
    u16 channel;
    u32 i, j;
    u16	pageIdxY, pageIdxUV;
    u32 *tmpDDR;
    u8	*line0, *line1, *line2, *line3;
    u8	*line4, *line5, *line6, *line7;
    u8  *srcBufY, *srcBufU, *srcBufV;
    u32 dataTemp;
    u32 offset;
    u32 temp;
    u32 width,  height;
    chip = decode->chip;

    printf( "0x%x,w%d,h%d\n", chip->regs, width, height );

    switch(FrameIdx)
    {

        case 1:
            pageIdxY = 3;
            pageIdxUV = 0x5;
            break;
        case 2:
            pageIdxY = 6;
            pageIdxUV = 0x8;
            break;
        case 3:
            pageIdxY = 9;
            pageIdxUV = 0xb;
            break;
        default:
            pageIdxY = 0;
            pageIdxUV =0x2;
            break;
    }
    temp = reg_readl(chip_device->reg_base+ (ENCODER_PARA_REG+(4*channel)));
    temp =(temp&0x1fff)+0xc;
    pageIdxY +=temp;
    pageIdxUV +=temp;


    printf( "Readback ddr[Y=%d, UV=%d] data\r\n", pageIdxY, pageIdxUV );
    temp = reg_readl( chip_device->reg_base + MODE );
    reg_writel( 1, chip_device->reg_base + MODE );
    reg_writel( pageIdxY, chip_device->reg_base + DDRPAGE );
    delay( WDELAYTIME );
    tmpDDR = ( u32 * )( chip_device->reg_base + ( FPGA_DATA_BASE ) );
    delay( WDELAYTIME );
    *len = 0;
    srcBufY = buf;
    srcBufU =  srcBufY + width * height;
    srcBufV =  srcBufU + ( width * ( height >> 2 ) );

    offset = width << 2;

    for ( i = 0; i<( height >> 2 ); i++ ) {
        line0 = srcBufY;
        line1 = line0 + width;
        line2 = line1 + width;
        line3 = line2 + width;

        for ( j = 0; j < width; j++ ) {
            dataTemp  = 	*tmpDDR;
            line3[j]  = 	( u8 )( dataTemp & 0xff );
            line2[j]  =  	( u8 )( ( dataTemp >> 8 ) & 0xff );
            line1[j]  =  	( u8 )( ( dataTemp >> 16 ) & 0xff );
            line0[j]  =  	( u8 )( ( dataTemp >> 24 ) & 0xff );
            tmpDDR++;
            *len += 4;
        }

        srcBufY += offset;
        tmpDDR += ( 0x400 - width );

        if( tmpDDR == ( u32 * )( chip_device->reg_base + ( FPGA_DATA_END ) ) ) {

            reg_writel( ( u32 )( pageIdxY + 1 ), chip_device->reg_base + DDRPAGE );
            delay( WDELAYTIME );
            tmpDDR = ( u32 * )( chip_device->reg_base + ( FPGA_DATA_BASE ) );
            delay( WDELAYTIME );
        }
    }


    reg_writel( pageIdxUV, chip_device->reg_base + DDRPAGE );
    delay( WDELAYTIME );
    tmpDDR = ( u32 * )( chip_device->reg_base + ( FPGA_DATA_BASE ) );
    delay( WDELAYTIME );
    height >>= 3;
    offset = width << 1;
    width >>= 1;

    for( i = 0; i < height; i++ ) {
        line0 = srcBufU;
        line1 = line0 +  width;
        line2 = line1 +  width;
        line3 = line2 +  width;

        line4 = srcBufV;
        line5 = line4 +  width;
        line6 = line5 +  width;
        line7 = line6 +  width;

        for( j = 0; j < width; j++ ) {
            dataTemp = *tmpDDR;
            line3[j]  =  ( u8 )( dataTemp & 0xff );
            line2[j]  =  ( u8 )( ( dataTemp >> 8 ) & 0xff );
            line1[j]  =  ( u8 )( ( dataTemp >> 16 ) & 0xff );
            line0[j]  =  ( u8 )( ( dataTemp >> 24 ) & 0xff );
            tmpDDR++;
            *len += 4;
            dataTemp = *tmpDDR;
            line7[j]  =  ( u8 )( dataTemp & 0xff );
            line6[j]  =  ( u8 )( ( dataTemp >> 8 ) & 0xff );
            line5[j]  =  ( u8 )( ( dataTemp >> 16 ) & 0xff );
            line4[j]  =  ( u8 )( ( dataTemp >> 24 ) & 0xff );
            tmpDDR++;
            *len += 4;
        }

        srcBufU += offset;
        srcBufV += offset;
        tmpDDR += ( 0x400 - ( width << 1 ) );
    }

    reg_writel( temp, chip_device->reg_base + MODE );
}

#endif
