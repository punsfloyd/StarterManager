/*
 * utils.h
 *
 *  Created on: Sep 25, 2015
 *      Author: RHolal
 */



#ifndef UTILS_H_
#define UTILS_H_
#include <stdint.h>
#include <sys/syspage.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/iofunc.h>
#include <sys/syspage.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>


#define PRIVATE_SLOG _SLOG_SETCODE(_SLOG_SYSLOG, 1)

// Note : This string needs to be changed if you want module specific logging
// Those who want to set the module name they can use this variable and change accordingly
#define Module "[VD] "

#define DBG_MSG(x,...) (slogf(_SLOGC_CONSOLE,_SLOG_INFO,"[VD] "x,## __VA_ARGS__))
#define DBG_WARNING(x,...) (slogf(PRIVATE_SLOG,_SLOG_WARNING,"[VD] "x,## __VA_ARGS__))
#define DBG_ERROR(x,...) (slogf(PRIVATE_SLOG,_SLOG_ERROR,"[VD] "x,## __VA_ARGS__))


#endif /* UTILS_H_ */
