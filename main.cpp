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

    auto digitizer = std::unique_ptr< ATS9462Engine >( new ATS9462Engine( 10e6, 10, 500e6 ) );
    digitizer->SetSampleRate( 10e6 );
    digitizer->StartCapture();

    digitizer->Start();

    sleep( 30 );

    if ( digitizer->Finished() ) {
        auto signal = digitizer->FinishedSignal();
        jaspl::plot( signal );
    } else {
        std::cout << "Wasn't finished" << std::endl;
    }

    digitizer->Stop();
}
