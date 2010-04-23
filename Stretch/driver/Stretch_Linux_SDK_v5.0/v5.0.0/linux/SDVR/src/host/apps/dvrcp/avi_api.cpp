/****************************************************************************\
*  Copyright C 2007 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
\****************************************************************************/
//todo removeit#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "avi_api.h"

/* a v i _ o p e n */
bool avi_open(aviFile_t *avi, const char * filename, int scale, int rate, int width, int height, int maxFramesAllowed)
{
/* The scale and rate parameters will define the correct ratio for different video standards' framerate. 
   Known ratio values are (rate/scale): 
   NTSC 30000/1001 
   PAL 25/1 */

/* maxFramesAllowed is the maximum number of times you can call avi_add. (It is used allocate memory to 
   store the index table required at the end of the avi file.) */

  int word;
  short int wo16;
  
  if((avi->frameLen=(int *) malloc(maxFramesAllowed*sizeof(int)))==NULL) 
      return false; // error("avi_open: Out of memory");
  avi->totalFramesWrtn=0;
  avi->maxFramesAllowed=maxFramesAllowed;
  
  if((avi->fp=fopen(filename, "wb"))==NULL) 
      return false; // error("Cannot open avi file for writing");
  fprintf(avi->fp, "RIFF");                                 /* RIFF header */
  word=1111;       fwrite(&word, 4, 1, avi->fp);            /* length of chunk - 8 ### */

  fprintf(avi->fp, "AVI ");                                 /* */
  fprintf(avi->fp, "LIST");                                 /* */
  word=208;        fwrite(&word, 4, 1, avi->fp);            /* length of chunk - 8 ### */
  fprintf(avi->fp, "hdrl");                                 /* */
  fprintf(avi->fp, "avih");                                 /* fcc */
  word=14*4;       fwrite(&word, 4, 1, avi->fp);            /* DWORD cb */
  word=(int) (1000000*(((double) scale)/rate));
                   fwrite(&word, 4, 1, avi->fp);            /* DWORD dwMicroSecPerFrame */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwMaxBytesPerSec ### */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwPaddingGranularity */
  word=16;         fwrite(&word, 4, 1, avi->fp);            /* DWORD dwFlags */
  word=1;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwTotalFrames */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwInitialFrames */
  word=1;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwStreams */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwSuggestedBufferSize */
  word=width;      fwrite(&word, 4, 1, avi->fp);            /* DWORD dwWidth */
  word=height;     fwrite(&word, 4, 1, avi->fp);            /* DWORD dwHeight */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwReserved[4] */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwReserved[4] */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwReserved[4] */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD dwReserved[4] */

  fprintf(avi->fp, "LIST");                                 /* */
  word=132;        fwrite(&word, 4, 1, avi->fp);            /* length of chunk - 8 ### */
  fprintf(avi->fp, "strl");                                 /* */

  /*avistreamheader */
  fprintf(avi->fp, "strh");                                 /* FOURCC fcc */
  word=12*4;       fwrite(&word, 4, 1, avi->fp);            /* DWORD  cb */
  fprintf(avi->fp, "vids");                                 /* FOURCC fccType */
  fprintf(avi->fp, "MJPG");                                 /* FOURCC fccHandler */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwFlags */
  wo16=0;          fwrite(&word, 2, 1, avi->fp);            /* WORD   wPriority */
  wo16=0;          fwrite(&word, 2, 1, avi->fp);            /* WORD   wLanguage */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwInitialFrames */
  word=scale;      fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwScale */
  word=rate;       fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwRate ### */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwStart */
  word=1;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwLength ### */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwSuggestedBufferSize */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwQuality */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  dwSampleSize */
//  wo16=0;          fwrite(&wo16, 2, 1, avi->fp);          /* short int left */
//  wo16=0;          fwrite(&wo16, 2, 1, avi->fp);          /* short int top */
//  wo16=width-1;    fwrite(&wo16, 2, 1, avi->fp);          /* short int right */
//  wo16=height-1;   fwrite(&wo16, 2, 1, avi->fp);          /* short int bottom */

  fprintf(avi->fp, "strf");                                 /* */
  word=40;         fwrite(&word, 4, 1, avi->fp);            /* length of chunk - 8 ### */
  
  /* BITMAPINFOHEADER */
  word=40;         fwrite(&word, 4, 1, avi->fp);            /* DWORD  biSize */
  word=width;      fwrite(&word, 4, 1, avi->fp);            /* LONG   biWidth */
  word=height;     fwrite(&word, 4, 1, avi->fp);            /* LONG   biHeight */
  word=1;          fwrite(&word, 2, 1, avi->fp);            /* WORD   biPlanes */
  word=24;         fwrite(&word, 2, 1, avi->fp);            /* WORD   biBitCount */ 
  fprintf(avi->fp, "MJPG");                                 /* DWORD  biCompression */
  word=768;        fwrite(&word, 4, 1, avi->fp);            /* DWORD  biSizeImage */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* LONG   biXPelsPerMeter */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* LONG   biYPelsPerMeter */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  biClrUsed */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* DWORD  biClrImportant */
 
  fprintf(avi->fp, "LIST");                                 /* */
  word=16;         fwrite(&word, 4, 1, avi->fp);            /* length of chunk - 8 ### */
  fprintf(avi->fp, "odml");                                 /* */
  fprintf(avi->fp, "dmlh");                                 /* */
  word=4;          fwrite(&word, 4, 1, avi->fp);            /* length of chunk - 8 ### */
  word=1;          fwrite(&word, 4, 1, avi->fp);            /* dwTotalFrames */
  fprintf(avi->fp, "LIST");                                 /* */
  word=0;          fwrite(&word, 4, 1, avi->fp);            /* length of chunk - 8 ### */
  fprintf(avi->fp, "movi");                                 /* */

  return true;
}


/* a v i _ a d d */
int avi_add(aviFile_t *avi, unsigned char *buffer, int len)
{
  int buflen;
  int word;

  buflen=len;
  fprintf(avi->fp, "00db");                                 /* */
  word=((buflen+3)/4)*4;
  fwrite(&word, 4, 1, avi->fp);            /* length of chunk - 8 ### */
  fwrite(buffer, buflen, 1, avi->fp);
  while((buflen & 0x3))
  {
    word=0;
    fwrite(&word, 1, 1, avi->fp);
    buflen++;
  }
  avi->frameLen[avi->totalFramesWrtn++]=buflen;

  return buflen;
}


/* a v i _ c l o s e */
int avi_close(aviFile_t *avi)
{
  unsigned int word;
  unsigned int i, j;

  fprintf(avi->fp, "idx1");                                 /* */
  word=16*avi->totalFramesWrtn; fwrite(&word, 4, 1, avi->fp);        /* length of chunk */
  j=4;
  for(i=0; i<avi->totalFramesWrtn; i++)
  {
    fprintf(avi->fp, "00db");                               /*  */
    word=0x12;     fwrite(&word, 4, 1, avi->fp);            /*  */
    word=j;        fwrite(&word, 4, 1, avi->fp);            /*  */
    word=avi->frameLen[i];
                   fwrite(&word, 4, 1, avi->fp);            /*  */
    j+=avi->frameLen[i]+8;
  }

  /* fill in some information not known during first pass */
  fseek(avi->fp, 4, SEEK_SET);
  word=j+268;      fwrite(&word, 4, 1, avi->fp);            /* length of chunk */
  fseek(avi->fp, 48, SEEK_SET);
  word=avi->totalFramesWrtn; fwrite(&word, 4, 1, avi->fp);           /* dwTotalFrames */
  fseek(avi->fp, 140, SEEK_SET);
  word=avi->totalFramesWrtn; fwrite(&word, 4, 1, avi->fp);           /* dwTotalFrames */
  fseek(avi->fp, 224, SEEK_SET);
  word=avi->totalFramesWrtn; fwrite(&word, 4, 1, avi->fp);           /* dwTotalFrames */
  fseek(avi->fp, 232, SEEK_SET);
  word=j;          fwrite(&word, 4, 1, avi->fp);            /* length of chunk */

  fclose(avi->fp);
  free(avi->frameLen);

  return 0;
}


