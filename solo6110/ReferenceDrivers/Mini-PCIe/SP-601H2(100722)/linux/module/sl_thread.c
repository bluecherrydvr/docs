#include <linux/kernel.h>
#include <linux/module.h>

#include "sl_thread.h"

void sl_thread_init(sl_thread_t *sl_thread, const char *name, ...)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	char thread_name[16];
	va_list args;

	va_start(args, name);
	vsnprintf(thread_name, 16, name, args);
	va_end(args);

	daemonize(thread_name);
	allow_signal(SIGTERM);
#else
	va_list args;

	daemonize();

	va_start(args, name);
	vsnprintf(current->comm, sizeof(current->comm), name, args);
	va_end(args);

#endif
}

