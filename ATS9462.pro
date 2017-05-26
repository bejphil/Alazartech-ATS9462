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

CONFIG(debug, debug|release) {
    DEFINES += "DEBUG"
} else {
    DEFINES += "NDEBUG"
    CONFIG += optimize_full
    QMAKE_CXXFLAGS_RELEASE *= -mtune=native
    QMAKE_CXXFLAGS_RELEASE *= -march=native
}

INCLUDEPATH += ../../../include
LIBS += -L$$OUT_PWD/../../../lib

LIBS += -L/usr/local/lib \
        -L/usr/lib \
        -lboost_iostreams \
        -lboost_python-py34 \
        -lboost_system \
        -lboost_filesystem \
        -lboost_thread \
        -pthread

INCLUDEPATH +=/opt/AMDAPPSDK-3.0/include/
LIBS +=-L/opt/AMDAPPSDK-3.0/lib/x86_64/sdk/ -lOpenCL

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib64 -lclFFT

LIBS += -lfftw3_threads -lfftw3 -lm

LIBS += -lATSApi -L/usr/local/AlazarTech/lib

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS +=  -fopenmp

QMAKE_CXXFLAGS = -Wno-deprecated-declarations

SOURCES += main.cpp\
    Digitizer/ATS9462/ats9462.cpp \
    Digitizer/ATS9462Engine/ats9462engine.cpp \

HEADERS  += Digitizer/ATS9462/ats9462.h \
    Debug/debug.h \
    Digitizer/ATS9462Engine/ats9462engine.h \
    Containers/ringbuffer.h

