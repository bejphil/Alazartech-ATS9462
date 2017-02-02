//C System-Headers
#include <math.h>
#include <stdio.h>
//C++ System headers
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
//OpenCL Headers
//
//Boost Headers
//
//Qt Headers
#include <QApplication>
#include <QThread>
//Project specific headers
#include "jSpectrumAnalyzer/jspectrumanalyzer.h"
#include "../JASPL/jPlot/jplot.h"
#include "Digitizer/ats9462.h"
#include "Digitizer/ats9462engine.h"

/*! \mainpage Alazartech ATS9462 Digitier
 *
 * \section intro_sec Introduction
 *
 * Wrapper around Alazartech C API to make life a little less miserable
 *
 * \section Base Dependencies
 *      \li Alazartech SDK
 *      \li Qt-Charts
 *
 *
 */

int main(int argc, char *argv[]) {

//    QApplication a(argc, argv);

//    jaspl::jSpectrumAnalyzer lyzer;

//    return a.exec();

    auto digitizer = std::unique_ptr< ATS9462Engine >( new ATS9462Engine( 50e6, 50, 500e6 ) );

    digitizer->ThreadPoolSize( 50 );
    digitizer->SetSampleRate( 50e6 );

    digitizer->StartCapture();

    digitizer->Start();

    while( !digitizer->Finished() ) {
        sleep(5);
    }

    auto signal = digitizer->FinishedSignal();

    jaspl::plot( signal, "2 Signals Averaged", 1e5 );

    digitizer->Stop();

    digitizer->AbortCapture();
}
