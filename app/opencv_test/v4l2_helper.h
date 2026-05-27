#ifndef V4L2_HELPER_H
#define V4L2_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

struct buffer {
	unsigned int index;
	int dbuf_fd;
	unsigned char *v4l2_buff;
	unsigned int v4l2_buff_length;
};

/* video device */
struct v4l2_dev {
	char devname[DEV_NAME_LEN];		/* device name */
	int fd;					/* device node fd */
	enum v4l2_buf_type buf_type;   /* type of buffer */
	enum v4l2_memory mem_type;  /* type of memory */
	struct v4l2_pix_format format;
	struct buffer  vid_buf[ MAX_BUFFER_CNT];
	struct video_pipeline *setup_ptr;
};

/* Return video node for card name */
extern int v4l2_parse_node(const char *cardname, char *devnode);
/* Initialize v4l2 video device */
extern int v4l2_init(struct v4l2_dev *, unsigned int );
/* Queue buffer to video device */
extern void v4l2_queue_buffer(struct v4l2_dev *, const struct buffer *);
/* Dequeue buffer from video device */
extern struct buffer *v4l2_dequeue_buffer(struct v4l2_dev *, struct buffer *);
/* Start the capture or output process during streaming */
extern int  v4l2_device_off(struct v4l2_dev *);
/*Stop the capture or output process during streaming */
extern int  v4l2_device_on(struct v4l2_dev *);
extern int v4l2_capture(struct v4l2_dev *dev);

#ifdef __cplusplus
}
#endif


#endif
