#pragma once

#include "csp/autoconfig.h"

#if (CSP_ZEPHYR)
#include <zephyr/kernel.h>
#else
#if defined(__APPLE__)
#define __noinit __attribute__((section("__DATA,__noinit")))
#else
#define __noinit __attribute__((section(".noinit")))
#endif
#define __packed __attribute__((__packed__))
#define __maybe_unused __attribute__((__unused__))
#define __unused __attribute__((__unused__))
#ifdef __CYGWIN__
#define __weak
#else
#ifdef __APPLE__
/* AppleClang predefines __weak for ObjC ARC/GC even in C mode; undef
 * before redefining it to avoid a macro-redefined warning (fatal
 * under -Werror). */
#undef __weak
#endif
#define __weak   __attribute__((__weak__))
#endif

#define CONTAINER_OF(ptr, type, member) \
	((type *)(void *)((char *)(ptr) - offsetof(type, member)))

#endif
