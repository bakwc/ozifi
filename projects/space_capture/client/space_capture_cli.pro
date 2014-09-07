#-------------------------------------------------
#
# Project created by QtCreator 2014-09-07T12:11:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = space_capture_cli
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
           display.cpp \
           network.cpp \
           control.cpp \
           world.cpp \
           graphic_manager.cpp \
           main_menu.cpp \
           application.cpp \
           world_display.cpp

HEADERS  += mainwindow.h \
            application.h \
            control.h \
            display.h \
            graphic_manager.h \
            main_menu.h \
            network.h \
            sphere.h \
            world.h \
            world_display.h

RESOURCES = resources.qrc

CONFIG += mobility
MOBILITY = 

