#ifndef _ISIL_FILE_RECORD_SESSION_H_

#define _ISIL_FILE_RECORD_SESSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FILE_REC_DEVICE_NODE "/dev/sda1"

#define FILE_REC_MOUNT_NODE "/mnt/sda1"


extern int start_record_file(void *p_fm_start_rec);

extern int stop_file_record( int chan);

extern int file_record_init( void );


#ifdef __cplusplus
}
#endif

#endif
