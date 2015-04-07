#ifndef ISIL_IO_UTILS_H
#define ISIL_IO_UTILS_H




#ifdef __cplusplus
extern "C" {
#endif





extern int set_noblock(int fd);

extern int set_block(int fd);



extern ssize_t r_read(int fd, char *buf, size_t size);

extern ssize_t r_write(int fd, char *buf, size_t size);





#ifdef __cplusplus
}
#endif //__cplusplus

#endif
