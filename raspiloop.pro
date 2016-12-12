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
       playback_port_c.cpp \
    capture_port_c.cpp \
    playback_loop_c.cpp \
    capture_loop_c.cpp \
    interface.cpp \
    click_c.cpp \
    events.cpp \
    config_file.cpp \
    dialog_newcontrol.cpp \
    dialog_device.cpp \
    dialog_parameters.cpp \
    dialog_controllist.cpp \
    alsa_capture_device.cpp \
    alsa_playback_device.cpp \
    alsa_util.cpp \
    control.cpp \
    dialog_numeric.cpp \
    dialog_key.cpp \
    dialog_keylist.cpp \
    alsa_midi.cpp

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
    config_file.h \
    dialog_newcontrol.h \
    dialog_device.h \
    dialog_parameters.h \
    dialog_controllist.h \
    control.h \
    dialog_numeric.h \
    dialog_key.h \
    dialog_keylist.h \
    alsa_midi.h

FORMS    += mainwindow.ui \
    dialog_newcontrol.ui \
    dialog_device.ui \
    dialog_parameters.ui \
    dialog_controllist.ui \
    dialog_numeric.ui \
    dialog_key.ui \
    dialog_keylist.ui


LIBS += -lasound -lsndfile

target.path = /home/pi/test

INSTALLS += target


