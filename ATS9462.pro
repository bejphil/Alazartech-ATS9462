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

LIBS += -L/usr/local/lib -L/usr/lib -lboost_iostreams -lboost_system -lboost_filesystem

INCLUDEPATH +=-I "/usr/local/cuda/include"
LIBS +=-L "/usr/local/cuda/lib64" -lOpenCL

INCLUDEPATH +=-I/usr/local/include
LIBS +=-L/usr/local/lib64 -lclFFT

LIBS +=-lfftw3_threads -lfftw3 -lm

LIBS += -lATSApi -L/usr/local/AlazarTech/lib

QMAKE_CXXFLAGS+= -fopenmp
QMAKE_LFLAGS +=  -fopenmp


SOURCES += main.cpp\
        mainwindow.cpp \
    Digitizer/ats9462.cpp \
    Digitizer/debug.cpp \
    Digitizer/qts9462.cpp \
    jSpectrumAnalyzer/jspectrumanalyzer.cpp \
    jSpectrumAnalyzer/jspectrumanalyzer.tpp \
    jSpectrumAnalyzer/GraphicObjects/chartscalecontrols.cpp \
    jSpectrumAnalyzer/GraphicObjects/frequencycontrols.cpp \
    jSpectrumAnalyzer/GraphicObjects/rightclickmenu.cpp

HEADERS  += mainwindow.h \
    Digitizer/ats9462.h \
    Digitizer/debug.h \
    Digitizer/qts9462.h \
    jSpectrumAnalyzer/jspectrumanalyzer.h \
    jSpectrumAnalyzer/GraphicObjects/chartscalecontrols.h \
    jSpectrumAnalyzer/GraphicObjects/frequencycontrols.h \
    jSpectrumAnalyzer/GraphicObjects/rightclickmenu.h

FORMS    += mainwindow.ui \
    jSpectrumAnalyzer/GraphicObjects/chartscalecontrols.ui \
    jSpectrumAnalyzer/GraphicObjects/frequencycontrols.ui
