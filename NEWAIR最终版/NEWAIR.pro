#-------------------------------------------------
#
# Project created by QtCreator 2016-06-05T04:30:54
#
#-------------------------------------------------

QT       += core gui
QT       += core sql
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NEWAIR
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    statusinfo.cpp

HEADERS  += mainwindow.h \
    connection.h \
    statusinfo.h

FORMS    += mainwindow.ui

OTHER_FILES +=

RESOURCES +=
