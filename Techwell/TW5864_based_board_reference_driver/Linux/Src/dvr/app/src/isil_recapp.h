#ifndef _ISIL_RECAPP_H_

#define _ISIL_RECAPP_H_

#ifdef __cplusplus
extern "C" {
#endif

//start rec
typedef struct
{
	unsigned int	chan;
	unsigned int    maintype;
    unsigned int	subtype;
}RECAPP_START;

typedef struct
{
	unsigned int	chan;
}RECAPP_COMM;


extern int isil_recapp_startrec(void* startrec);
extern int isil_recapp_sync(void* recsync);
extern int isil_recapp_close(void* recclose);
extern int isil_recapp_write(void* recwrite);

extern int new_isil_recapp_write(void* recwrite ,void *src);

#ifdef __cplusplus
}
#endif

#endif