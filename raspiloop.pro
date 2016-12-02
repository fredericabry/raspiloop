#-------------------------------------------------
#
# Project created by QtCreator 2016-06-07T12:17:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = raspiloop
TEMPLATE = app




SOURCES += main.cpp\
        mainwindow.cpp \
    util.cpp \
    alsa_util.cpp \
    playback_port_c.cpp \
    capture_port_c.cpp \
    playback_loop_c.cpp \
    capture_loop_c.cpp \
    interface.cpp \
    click_c.cpp \
    events.cpp \
    alsa_playback_device.cpp \
    alsa_capture_device.cpp \
    dialogdevice.cpp

HEADERS  += mainwindow.h \
    alsa_util.h \
    playback_port_c.h \
    capture_port_c.h \
    playback_loop_c.h \
    capture_loop_c.h \
    interface.h \
    parameters.h \
    click_c.h \
    events.h \
    alsa_playback_device.h \
    alsa_capture_device.h \
    dialogdevice.h

FORMS    += mainwindow.ui \
    dialogdevice.ui


LIBS += -lasound -lsndfile

target.path = /home/pi/test

INSTALLS += target


