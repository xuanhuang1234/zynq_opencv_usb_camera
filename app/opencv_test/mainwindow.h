#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <QFileDialog>
#include <QPixmap>
#include <QImage>
#include <QLabel>
#include <QTimer>

#include <sys/mman.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include "v4l2_helper.h"

#define IMAGEWIDTH 640
#define IMAGEHEIGHT 480
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void slot_timer();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    v4l2_dev dev_0;
    IplImage* frame_yuv;
    IplImage* frame_rgb;
};

#endif // MAINWINDOW_H
