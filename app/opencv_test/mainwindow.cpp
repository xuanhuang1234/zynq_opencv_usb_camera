#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(nullptr)
{
    ui->setupUi(this);

    ui->label->setMinimumSize(640, 480);
    ui->label->setText("");

    // 上电自动打开摄像头（原按钮槽逻辑）
    on_pushButton_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (timer)
        delete timer;
}

void MainWindow::on_pushButton_clicked()
{
    dev_0.buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    dev_0.mem_type = V4L2_MEMORY_MMAP;
    dev_0.format.pixelformat = V4L2_PIX_FMT_YUYV;
    dev_0.format.width = IMAGEWIDTH;
    dev_0.format.height = IMAGEHEIGHT;
    dev_0.format.bytesperline = 2 * IMAGEWIDTH;
    dev_0.format.colorspace = V4L2_COLORSPACE_SRGB;

    if (v4l2_parse_node("uvcvideo", dev_0.devname))
    {
        printf("no dev find\n");
        return;
    }
    v4l2_init(&dev_0, BUFFER_CNT);
    frame_yuv = cvCreateImageHeader(cvSize(IMAGEWIDTH, IMAGEHEIGHT), IPL_DEPTH_8U, 2);
    frame_rgb = cvCreateImage(cvSize(IMAGEWIDTH, IMAGEHEIGHT), IPL_DEPTH_8U, 3);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_timer()));
    timer->start(66);
    // 已删除: ui->pushButton->setDisabled(true);
}

void MainWindow::slot_timer()
{
    v4l2_capture(&dev_0);
    frame_yuv->imageData = (char *)dev_0.vid_buf[0].v4l2_buff;
    cvCvtColor(frame_yuv, frame_rgb, CV_YUV2RGB_YUYV);
    QImage *image;
    uchar *imgData = (uchar *)frame_rgb->imageData;
    image = new QImage(imgData, frame_rgb->width, frame_rgb->height, QImage::Format_RGB888);
    if (frame_rgb->width > 640)
        ui->label->resize(640, 480);
    else
        ui->label->resize(frame_rgb->width, frame_rgb->height);
    ui->label->setPixmap(QPixmap::fromImage(*image).scaled(ui->label->size()));
    munmap(dev_0.vid_buf[0].v4l2_buff, dev_0.vid_buf[0].v4l2_buff_length);
}
