#ifndef __PROC_ENTRY_H__
#define __PROC_ENTRY_H__

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef MESSAGE_LOG
    struct message_privte{
	wait_queue_head_t wait;
	spinlock_t lock;

	char *buf;
	u32 buf_len;
	u32 start;
	u32 end;
    };
#endif
    typedef struct {
	char name[64];
	atomic_t ref_count;
	struct proc_dir_entry *entry;
	ssize_t (*read) (struct seq_file *seq, void *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);

	void *private;
    }isil_proc_register_s;

#ifdef MESSAGE_LOG
    extern struct message_privte *message_log;
    extern int message_level;
#endif

    void *isil_seq_get_private(struct seq_file *seq);
    void *isil_file_get_seq_private(struct file *file);
    int isil_module_register(isil_chip_t *chip, isil_proc_register_s *reg);
    int isil_module_unregister(isil_chip_t *chip, isil_proc_register_s *reg);
    int proc_module_init(void);
    void proc_module_release(void);

    unsigned int atoi(char *str);
    struct file *isil_kernel_file_open(char *);
    void isil_kernel_file_close(struct file *);


#ifdef __cplusplus
}
#endif

#endif //__PROC_ENTRY_H__

