#include "v4l2_helper.h"
int v4l2_parse_node(const char *cardname, char *devnode)
{
	int fd,i=0,ret=0;
	struct v4l2_capability cap;
	char newdev[DEV_NAME_LEN];

	while(1) {
		snprintf (newdev, sizeof (newdev), "/dev/video%u", i);
		if ((fd=open(newdev,O_RDWR))<0)	{
			ret=-ENOENT;
			break;
		}
		if (ioctl(fd,VIDIOC_QUERYCAP,&cap)<0){
			ret=-errno;
			break;
		}

        printf("found dev:cap.card:%s,cap.driver:%s\n",cap.card,cap.driver);
		/* Compare card name */
		if (!strcmp((const char *) cap.card, cardname))	{
			strcpy (devnode, newdev);
			break;
		}
		/* Compare driver name */
		if (!strcmp((const char *) cap.driver, cardname))	{
			strcpy (devnode, newdev);
			break;
		}
		i++;
		close(fd);
	}
	if (ret != -ENOENT)
		close(fd);
	return ret;
}

int v4l2_init(struct v4l2_dev *dev, unsigned int num_buffers)
{
	struct v4l2_capability caps;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers rqbufs;
	char name[5];
	int ret;
	unsigned int i;

	dev->fd = open(dev->devname, O_RDWR);
	ASSERT(dev->fd < 0, "failed to open %s: %s\n", dev->devname, ERRSTR);

	// TODO caps unused ?
	memset(&caps, 0, sizeof caps);
	ret = ioctl(dev->fd, VIDIOC_QUERYCAP, &caps);
	ASSERT(ret, "VIDIOC_QUERYCAP failed: %s\n", ERRSTR);
#ifdef DEBUG_MODE
    printf("driver:%s\n",caps.driver);
    printf("card:%s\n",caps.card);
    printf("bus_info:%s\n",caps.bus_info);
    printf("version:%d\n",caps.version);
    printf("capabilities:%x\n",caps.capabilities);
    if ((caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
    {
        printf("Device %s: supports capture.\n",dev->devname);
    }
    if ((caps.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
    {
        printf("Device %s: supports streaming.\n",dev->devname);
    }
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Support format:\n");
    while(ioctl(dev->fd,VIDIOC_ENUM_FMT,&fmtdesc) != -1)
    {
        printf("%d.%s\n",fmtdesc.index + 1,fmtdesc.description);
        fmtdesc.index++;
    }
#endif

	memset(&fmt, 0, sizeof fmt);
	fmt.type = dev->buf_type;
	ret = ioctl(dev->fd, VIDIOC_G_FMT, &fmt);
	ASSERT(ret < 0, "VIDIOC_G_FMT failed: %s\n", ERRSTR);

#ifdef DEBUG_MODE
	printf("G_FMT(start): width = %u, height = %u, bytes per line = %u, "
		   "4cc = %.4s, color space = %u\n", fmt.fmt.pix.width,
		   fmt.fmt.pix.height, fmt.fmt.pix.bytesperline,
		   (char*) &fmt.fmt.pix.pixelformat, fmt.fmt.pix.colorspace);
#endif

	fmt.fmt.pix = dev->format;
	ret = ioctl(dev->fd, VIDIOC_S_FMT, &fmt);
	ASSERT(ret < 0, "VIDIOC_S_FMT failed: %s\n", ERRSTR);

	ret = ioctl(dev->fd, VIDIOC_G_FMT, &fmt);
	ASSERT(ret < 0, "VIDIOC_G_FMT failed: %s\n", ERRSTR);

#ifdef DEBUG_MODE
	printf("G_FMT(final): width = %u, height = %u, bytes per line = %u, "
		   "4cc = %.4s, color space = %u\n", fmt.fmt.pix.width,
		   fmt.fmt.pix.height, fmt.fmt.pix.bytesperline,
		   (char*)&fmt.fmt.pix.pixelformat, fmt.fmt.pix.colorspace);
#endif

	// check if pixel format is supported
	if (fmt.fmt.pix.pixelformat != dev->format.pixelformat) {
		// look up pixel format fourcc code
		for (i = 0; i < 4; ++i) {
			name[i] = fmt.fmt.pix.pixelformat & 0xff;
			fmt.fmt.pix.pixelformat >>= 8;
		}
		name[4] = '\0';

		printf("Requested pixel format '%s' is not supported by device '%s'\n",
				name, dev->devname);
		return VLIB_ERROR;
	}

	// check if resolution is supported
	if (fmt.fmt.pix.width != dev->format.width ||
		fmt.fmt.pix.height != dev->format.height) {
		printf("Requested resolution '%dx%d' is not supported by device '%s'\n",
				dev->format.width, dev->format.height, dev->devname);
		return VLIB_ERROR;
	}

	// check if stride is supported
	if (fmt.fmt.pix.bytesperline != dev->format.bytesperline) {
		printf("Requested stride '%d' is not supported by device '%s'\n",
				dev->format.bytesperline, dev->devname);
		return VLIB_ERROR;
	}

	memset(&rqbufs, 0, sizeof rqbufs);
	rqbufs.count = num_buffers;
	rqbufs.type = dev->buf_type;
	rqbufs.memory = dev->mem_type;
	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &rqbufs);
	ASSERT(ret < 0, "VIDIOC_REQBUFS failed: %s\n", ERRSTR);
	ASSERT(rqbufs.count < num_buffers, "video node allocated only "
		"%u of %u buffers\n", rqbufs.count, num_buffers);

	dev->format = fmt.fmt.pix;

	return VLIB_SUCCESS;
}

 void v4l2_queue_buffer(struct v4l2_dev *dev, const struct buffer *buffer)
{
	struct v4l2_buffer buf;
	int ret;

	memset(&buf, 0, sizeof buf);
	buf.type = dev->buf_type;
	buf.index = buffer->index;
	buf.memory = dev->mem_type;
	if(dev->mem_type == V4L2_MEMORY_DMABUF) {
	buf.m.fd = buffer->dbuf_fd;
	}
	ret = ioctl(dev->fd, VIDIOC_QBUF, &buf);
	ASSERT(ret, "VIDIOC_QBUF(index = %d) failed: %s\n", buffer->index, ERRSTR);
}

 struct buffer *v4l2_dequeue_buffer(struct v4l2_dev *dev, struct buffer *buffers)
{
	struct v4l2_buffer buf;
	int ret;
	memset(&buf, 0, sizeof (buf));
	buf.type = dev->buf_type;
	buf.memory = dev->mem_type;
	ret = ioctl(dev->fd, VIDIOC_DQBUF, &buf);
	ASSERT(ret, "VIDIOC_DQBUF failed: %s\n", ERRSTR);
#ifdef DEBUG_MODE
    printf("dequeue buffer index is %d\n",buf.index);
#endif
	return &buffers[buf.index];
}

/* turn off video device */
 int  v4l2_device_off(struct v4l2_dev *d)
{
	int ret;
	ret = ioctl(d->fd, VIDIOC_STREAMOFF, &d->buf_type);
	return ret;
}

/* turn on video device */
 int  v4l2_device_on(struct v4l2_dev *d)
{
	int ret;
	ret = ioctl(d->fd, VIDIOC_STREAMON, &d->buf_type);
	return ret;
}

 int v4l2_capture(struct v4l2_dev *dev)
 {
     int i;
     int ret;
     for (i = 0; i < BUFFER_CNT; ++i)  {
         struct v4l2_buffer buffer;
         memset(&buffer, 0, sizeof(buffer));
         buffer.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;
         buffer.memory = V4L2_MEMORY_MMAP;
         buffer.index = i;
         if (-1 == ioctl (dev->fd, VIDIOC_QUERYBUF, &buffer)) {
             perror("VIDIOC_QUERYBUF");
             exit(EXIT_FAILURE);
         }

         dev->vid_buf[i].v4l2_buff_length=buffer.length;

         /* remember for munmap() */
         dev->vid_buf[i].v4l2_buff = mmap(NULL,buffer.length, PROT_READ|PROT_WRITE, MAP_SHARED,dev->fd, buffer.m.offset);

         /* If you do not exit here you should unmap() and free()
         the buffers mapped so far. */

         ASSERT(MAP_FAILED == dev->vid_buf[i].v4l2_buff , "mmap failed ");
         dev->vid_buf[i].index = i;

     }


     /* Assigning buffer index and set exported buff handle */
     for(i=0;i<BUFFER_CNT;i++) {
         v4l2_queue_buffer(dev,&(dev->vid_buf[i]));
     }

     /* Start streaming */
     ret = v4l2_device_on(dev);
     ASSERT (ret < 0, "v4l2_device_on [video_in] failed %d \n",ret);

 #ifdef DEBUG_MODE
     printf("Video Capture Pipeline started\n");
 #endif
     struct buffer *b;
     b = v4l2_dequeue_buffer(dev,dev->vid_buf);
     v4l2_device_off(dev);
 }
