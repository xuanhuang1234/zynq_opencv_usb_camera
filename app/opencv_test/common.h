#ifndef COMMON_H
#define COMMON_H

/* Common defines / utils for video_lib */

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <linux/videodev2.h>

#define DEV_NAME_LEN 32
int warn(char *args, ... );
#define ARRAY_SIZE(a)  (sizeof(a)/sizeof((a)[0]))
#define VDMA_SKIP_FRM_INDEX 0
#define MAX_EVENT_CNT 4


#define BUFFER_CNT 1
#define MAX_BUFFER_CNT 5

#define VLIB_SUCCESS 0
#define VLIB_ERROR  -1
//#define DEBUG_MODE
/* helper macros */
#define ERRSTR strerror(errno)
#define ASSERT(cond, ...) 					\
        do {							\
            if (cond) { 				\
                int errsv = errno;			\
                fprintf(stderr, "ERROR(%s:%d) : ",	\
                        __FILE__, __LINE__);	\
                errno = errsv;				\
                fprintf(stderr,  __VA_ARGS__);		\
                abort();				\
            }						\
        } while(0)
#define WARN_ON(cond, ...) \
                ((cond) ? warn(__FILE__, __LINE__, __VA_ARGS__) : 0)

#endif
