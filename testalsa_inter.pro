#-------------------------------------------------
#
# Project created by QtCreator 2016-06-07T12:17:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testalsa
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    util.cpp \
    alsa_util.cpp \
    alsa_playback.cpp

HEADERS  += mainwindow.h \
    alsa_util.h \
    alsa_playback.h

FORMS    += mainwindow.ui


LIBS += -lasound -lsndfile
