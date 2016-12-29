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

DEFINES += "DEBUG"

CONFIG(debug, debug|release) {
#    DEFINES += "DEBUG"
} else {
#    DEFINES += "NDEBUG"
    QMAKE_CXXFLAGS -= -O2
    QMAKE_CXXFLAGS += -O3
}

INCLUDEPATH += ../../../include
LIBS += -L$$OUT_PWD/../../../lib

LIBS += -L/usr/local/lib \
        -L/usr/lib \
        -lboost_iostreams \
        -lboost_system \
        -lboost_filesystem \
        -lboost_system \
        -lboost_thread \
        -pthread

INCLUDEPATH +=/opt/AMDAPPSDK-3.0/include/
LIBS +=-L/opt/AMDAPPSDK-3.0/lib/x86_64/sdk/ -lOpenCL

INCLUDEPATH +=/usr/local/include
LIBS +=-L/usr/local/lib64 -lclFFT

LIBS +=-lfftw3_threads -lfftw3 -lm

LIBS += -lATSApi -L/usr/local/AlazarTech/lib

QMAKE_CXXFLAGS+= -fopenmp
QMAKE_LFLAGS +=  -fopenmp

QMAKE_CXXFLAGS = -Wno-deprecated-declarations

SOURCES += main.cpp\
        mainwindow.cpp \
    Digitizer/ats9462.cpp \
    Digitizer/debug.cpp \
    Digitizer/qts9462.cpp \
    jSpectrumAnalyzer/jspectrumanalyzer.cpp \
    jSpectrumAnalyzer/jspectrumanalyzer.tpp \
    jSpectrumAnalyzer/GraphicObjects/chartscalecontrols.cpp \
    jSpectrumAnalyzer/GraphicObjects/frequencycontrols.cpp \
    jSpectrumAnalyzer/GraphicObjects/rightclickmenu.cpp \
    Digitizer/ats9462engine.cpp

HEADERS  += mainwindow.h \
    Digitizer/ats9462.h \
    Digitizer/debug.h \
    Digitizer/qts9462.h \
    jSpectrumAnalyzer/jspectrumanalyzer.h \
    jSpectrumAnalyzer/GraphicObjects/chartscalecontrols.h \
    jSpectrumAnalyzer/GraphicObjects/frequencycontrols.h \
    jSpectrumAnalyzer/GraphicObjects/rightclickmenu.h \
    Digitizer/ats9462engine.h \
    Containers/ringbuffer.h

FORMS    += mainwindow.ui \
    jSpectrumAnalyzer/GraphicObjects/chartscalecontrols.ui \
    jSpectrumAnalyzer/GraphicObjects/frequencycontrols.ui
