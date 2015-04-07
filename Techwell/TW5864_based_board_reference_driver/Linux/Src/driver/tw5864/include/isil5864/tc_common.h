#ifndef	__ISIL_CORE_COMMON_H__
#define	__ISIL_CORE_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/types.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/i2c.h>
#include <linux/stringify.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/time.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/videodev2.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/highmem.h>
#include <linux/sysfs.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <net/sock.h>
#include <linux/netlink.h>

#if !defined(X86_PLATFORM)
#include <net/net_namespace.h>
#endif

#include <asm/io.h>
#include <asm/page.h>
#include <asm/irq.h>
#include <asm/atomic.h>
#include <asm/dma.h>
#include <asm/uaccess.h>


#include 	<isil5864/isil_common.h>
#include 	<isil5864/isil_chip_driver.h>
#include 	<isil5864/isil_codec_common.h>
#include 	<isil5864/tc_buffer.h>
#include	<isil5864/tc.h>
#include 	<isil5864/tc_audio_encode.h>
#include 	<isil5864/tc_buffer.h>
#include 	<isil5864/tc_helper.h>

#ifdef __cplusplus
}
#endif

#endif

