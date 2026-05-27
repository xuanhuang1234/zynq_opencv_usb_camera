#-------------------------------------------------
# Project created by QtCreator 2017-06-13T13:40:03
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opencv_test
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           v4l2_helper.c

HEADERS  += mainwindow.h \
            common.h \
            v4l2_helper.h

FORMS    += mainwindow.ui

ZYNQ_LIB=/opt/alinx/opencv_zynq_lib
HOST_LIB=/opt/alinx/opencv_host

if(contains(DEFINES,zynq)) {
    message("compile for zynq")

    INCLUDEPATH += $$ZYNQ_LIB/include $$ZYNQ_LIB/include/opencv

    QMAKE_LIBDIR_FLAGS += \
        -L$$ZYNQ_LIB/lib \
        -L$$ZYNQ_LIB/share/OpenCV/3rdparty/lib

    LIBS += -lopencv_shape -lopencv_stitching -lopencv_objdetect \
            -lopencv_superres -lopencv_videostab -lopencv_calib3d \
            -lopencv_features2d -lopencv_highgui -lopencv_videoio \
            -lopencv_imgcodecs -lopencv_video -lopencv_photo \
            -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core

    LIBS += -ljpeg -lpng -lz -ltiff \
            -lavcodec -lavformat -lavutil -lswscale -lswresample \
            -lx264 -lxvidcore \
            -lv4l2 -lv4l1 -lv4lconvert \
            -ldl -lm -lpthread -lrt

} else {
    message("compile for host")

    INCLUDEPATH += $$HOST_LIB/include

    QMAKE_LIBDIR_FLAGS += \
        -L$$HOST_LIB/lib \
        -L/usr/lib/x86_64-linux-gnu

    LIBS += -lv4l2 \
            -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs \
            -lopencv_imgproc -lopencv_flann -lopencv_core \
            -lz -lpng -ljpeg
}
