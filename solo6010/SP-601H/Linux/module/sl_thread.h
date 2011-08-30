#ifndef SL_THREAD_H
#define SL_THREAD_H

#include <linux/version.h>
#include <linux/completion.h>
#include <linux/sched.h>

typedef struct {
	pid_t pid;
	int kill;
	struct completion exit;
	void *data;
} sl_thread_t;

static inline int sl_thread_create(sl_thread_t *sl_thread, int (*func)(void *),void *data)
{
	sl_thread->kill = 0;
	init_completion(&sl_thread->exit);
	sl_thread->data = data;
	sl_thread->pid = kernel_thread(func, (void *)sl_thread, CLONE_FS | CLONE_FILES | CLONE_SIGHAND);

	return sl_thread->pid;
}

static inline void sl_thread_kill(sl_thread_t *sl_thread)
{
	if(sl_thread->pid > 0)
	{
		sl_thread->kill = 1;
		kill_proc(sl_thread->pid, SIGTERM, 1);
		wait_for_completion(&sl_thread->exit);
	}
}

void sl_thread_init(sl_thread_t *sl_thread, const char *name, ...);

static inline void sl_thread_exit(sl_thread_t *sl_thread, long code)
{
	complete_and_exit(&sl_thread->exit, code);
}

#define sl_thread_data(sl_thread)	((sl_thread)->data)
#define sl_thread_alived(sl_thread)	(!(sl_thread)->kill)

#endif
