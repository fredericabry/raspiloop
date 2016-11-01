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
    alsa_playback.cpp \
    loop_c.cpp \
    alsa_capture.cpp \
    playback_port_c.cpp \
    capture_port_c.cpp

HEADERS  += mainwindow.h \
    alsa_util.h \
    alsa_playback.h \
    loop_c.h \
    alsa_capture.h \
    playback_port_c.h \
    capture_port_c.h

FORMS    += mainwindow.ui


LIBS += -lasound -lsndfile

target.path = /home/pi/test

INSTALLS += target
