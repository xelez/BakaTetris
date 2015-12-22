#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T19:13:53
#
#-------------------------------------------------

QT       += core gui
QT       += core websockets
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BakaTetris
TEMPLATE = app

SOURCES += main.cpp \
    form.cpp \
    wsclient.cpp

HEADERS  += \
    form.h \
    wsclient.h

FORMS += \
    form.ui
