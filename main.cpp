//C System-Headers
#include <math.h>
#include <stdio.h>
//C++ System headers
#include <iostream>
#include <chrono>
#include <unistd.h>
//OpenCL Headers
//
//Boost Headers
//
//Qt Headers
#include <QApplication>
//Project specific headers
#include "jSpectrumAnalyzer/jspectrumanalyzer.h"

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

  QApplication a(argc, argv);

  jaspl::jSpectrumAnalyzer lyzer;
  lyzer.Activate();

  return a.exec();

}
