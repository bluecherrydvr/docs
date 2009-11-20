/********************************************************************
	created:	2008/06/23	
	filename: 	main.c
	author:		xcl
	
	purpose:	testbed for h264 decoder
*********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "vdec264.h"

#define INBUF_SIZE 9624
//#define INBUF_SIZE 4096

double diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks = clock1-clock2;
    double diffms = (diffticks)/(CLOCKS_PER_SEC/1000);
    return diffms;
}


void pgm_save(unsigned char *buf, int wrap, int xsize,int ysize, FILE *fout)
{
    int i;
	
	for(i = 0; i < ysize; i++)
	{
		fwrite(buf + i * wrap, 1, xsize, fout );  
	}
}

int main(int argc, char **argv)
{
    char *outrecfilename = "c:\\temp\\out_rec.yuv";
    char *filename = "c:\\temp\\foreman.264";
    vdec264_t handle;
    int frame, size, len;
    FILE *fin, *fout;
    vdec264_picture_t *picture;
    sx_uint8 inbuf[INBUF_SIZE], *inbuf_ptr;
    double total_time = 0.0;
    clock_t start_time;
    vdec264_err_e err;

    printf("Video decoding\n");

    err = vdec264_open(&handle);
   
    
    /* the codec gives us the frame size, in samples */

    fin = fopen(filename, "rb");
    if (!fin) {
        fprintf(stderr, "could not open %s\n", filename);
        exit(1);
    }

	fout = fopen(outrecfilename, "wb+");
    if (!fin) {
        fprintf(stderr, "could not open %s\n", outrecfilename);
        exit(1);
    }

    frame = 0;
    for(;;) {
        size = fread(inbuf, 1, INBUF_SIZE, fin);
        if (size == 0)
            break;

        inbuf_ptr = inbuf;
        while (size > 0) {
            start_time = clock();
            err = vdec264_decode(handle, inbuf_ptr, size, &len);
            total_time += diffclock(clock(), start_time);
            
            if( err != VDEC264_OK) {
                fprintf(stderr, "Error while decoding frame %d\n", frame);
                exit(1);
            }

            err = vdec264_get_picture_full(handle, &picture);
            if(err == VDEC264_OK && picture) {
                printf("saving frame %3d\n", frame);
                fflush(stdout);

                /* the picture is allocated by the decoder. no need to
                   free it */
                pgm_save(picture->plane[0], picture->padded_width[0],
                         picture->width, picture->height, fout);
				pgm_save(picture->plane[1], picture->padded_width[1],
                         picture->width/2, picture->height/2, fout);
				pgm_save(picture->plane[2], picture->padded_width[2],
                         picture->width/2, picture->height/2, fout);
                frame++;
                err = vdec264_put_picture_empty(handle, picture);

            }

            size -= len;
            inbuf_ptr += len;
        }
    }

     /* some codecs, such as MPEG, transmit the I and P frame with a
       latency of one frame. You must do the following to have a
       chance to get the last frame of the video */
    start_time = clock();
    err = vdec264_decode(handle, inbuf_ptr, 0, &len);
    total_time += diffclock(clock(), start_time);
    
    if( err != VDEC264_OK) {
        fprintf(stderr, "Error while decoding frame %d\n", frame);
        exit(1);
    }
    err = vdec264_get_picture_full(handle, &picture);
    if(err == VDEC264_OK && picture) {
        printf("saving frame %3d\n", frame);
        fflush(stdout);

        /* the picture is allocated by the decoder. no need to
           free it */
        pgm_save(picture->plane[0], picture->padded_width[0],
                         picture->width, picture->height, fout);
		pgm_save(picture->plane[1], picture->padded_width[1],
                 picture->width/2, picture->height/2, fout);
		pgm_save(picture->plane[2], picture->padded_width[2],
                 picture->width/2, picture->height/2, fout);
        frame++;
        err = vdec264_put_picture_empty(handle, picture);

    }

    printf("Decode %d frames using %lf ms\n", frame, total_time);

    fclose(fin);
    fclose(fout);
    vdec264_close(handle);
    printf("\n");
}
