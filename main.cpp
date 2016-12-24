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

//  alazar::ATS9462 digitizer;

    auto digitizer = std::unique_ptr< ATS9462Engine >( new ATS9462Engine( 180e6, 5 ) );
    digitizer->SetSampleRate( 180e6 );
    digitizer->SetupRingBuffer( 500e6 );
    digitizer->StartCapture();

    digitizer->Start();

    sleep( 10 );

    if ( digitizer->Finished() ) {
        auto signal = digitizer->FinishedSignal();
        jaspl::plot( signal, 1604 );
    } else {
        std::cout << "Wasn't finished" << std::endl;
    }

    digitizer->Stop();

}
