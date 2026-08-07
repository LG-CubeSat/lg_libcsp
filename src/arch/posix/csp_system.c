#include <csp/csp_hooks.h>

#include <unistd.h>
#if defined(__APPLE__)
#include <csp/csp_debug.h>
#include <mach/mach.h>
#elif defined(__CYGWIN__)
#include <csp/csp_debug.h>
#else
#include <sys/sysinfo.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#endif

uint32_t csp_memfree_hook(void) {
#if defined(__APPLE__)
	mach_port_t host = mach_host_self();
	vm_size_t page_size = 0;
	vm_statistics64_data_t vm_stat;
	mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

	if ((host_page_size(host, &page_size) != KERN_SUCCESS) ||
	    (host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&vm_stat, &count) != KERN_SUCCESS)) {
		return 0;
	}

	return (uint32_t)(vm_stat.free_count * page_size);
#else
	uint32_t total = 0;
	struct sysinfo info;
	sysinfo(&info);
	total = info.freeram * info.mem_unit;
	return total;
#endif
}

unsigned int csp_ps_hook(csp_packet_t * packet) {
	(void)packet; /* Avoid compiler warnings about unused parameter */
	return 0;
}

void csp_reboot_hook(void) {
#if defined(__APPLE__) || defined(__CYGWIN__)
    csp_print("HALTED - Please reboot\n");
    while (true)
        sleep(1);
#else
	sync();
	reboot(LINUX_REBOOT_CMD_RESTART);
#endif
}

void csp_shutdown_hook(void) {
#if defined(__APPLE__) || defined(__CYGWIN__)
    csp_print("HALTED - Please power off\n");
    while (true)
        sleep(1);
#else
	sync();
	reboot(LINUX_REBOOT_CMD_HALT);
#endif
}
