#-------------------------------------------------
#
# Project created by QtCreator 2016-12-13T01:47:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += charts

TARGET = ATS9462
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += ../../../include
LIBS += -L$$OUT_PWD/../../../lib

LIBS += -lATSApi -L/usr/local/AlazarTech/lib

QMAKE_CXXFLAGS+= -fopenmp
QMAKE_LFLAGS +=  -fopenmp


SOURCES += main.cpp\
        mainwindow.cpp \
    Digitizer/ats9462.cpp \
    Digitizer/debug.cpp \
    Digitizer/qts9462.cpp \
    jSpectrumAnalyzer/jspectrumanalyzer.cpp \
    jSpectrumAnalyzer/jspectrumanalyzer.tpp

HEADERS  += mainwindow.h \
    Digitizer/ats9462.h \
    Digitizer/debug.h \
    Digitizer/qts9462.h \
    jSpectrumAnalyzer/jspectrumanalyzer.h

FORMS    += mainwindow.ui
