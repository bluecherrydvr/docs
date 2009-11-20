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

#ifndef AVI_H
#define AVI_H

typedef struct
{
  FILE *fp;               /* file pointer for AVI file to be written */
  unsigned int maxFramesAllowed;   /* maximum number of frames in AVI movie */
  unsigned int totalFramesWrtn;    /* actual number of frames in AVI movie */
  int *frameLen;          /* an array holding each length of each JPEG image */
                          // in the AVI file
} aviFile_t;

/* p r o t o t y p i n g */
bool avi_open(aviFile_t *avi,const char * filename, int scale, int rate, int width, int height, int maxframe);
int avi_add(aviFile_t *avi, unsigned char *buffer, int len);
int avi_close(aviFile_t *avi);

typedef struct
{
  FILE *fp;               /* file pointer for AVI file to be written */
  unsigned int maxFramesAllowed;   /* maximum number of frames in AVI movie */
  unsigned int totalFramesWrtn;    /* actual number of frames in AVI movie */
  int *frameLen;          /* an array holding each length of each JPEG image */
                          // in the AVI file
} aviFile_t_sec;

bool avi_open_sec(aviFile_t_sec *avi,const char * filename, int scale, int rate, int width, int height, int maxframe);
int avi_add_sec(aviFile_t_sec *avi, unsigned char *buffer, int len);
int avi_close_sec(aviFile_t_sec *avi);

#endif


