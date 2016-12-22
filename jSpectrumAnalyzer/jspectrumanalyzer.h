#ifndef JSPECTRUMANALYZER_H
#define JSPECTRUMANALYZER_H

//C System-Headers
#include <cxxabi.h> //abi::__cxa_demangle
//C++ System headers
#include <typeinfo> //typeid(T).name()
//OpenCL Headers
//
//Boost Headers
//
//Qt Headers
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCore/QDateTime>
#include <QtCharts/QSplineSeries>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QtCharts/QValueAxis>
#include <QMainWindow>
#include <QTimer>
//Project specific headers
#include "../JASPL/jPlot/jplot.h"
#include "../JASPL/jTypeTraits/jtypetraits.h"
#include "../JASPL/jFFT/jfft.h"
#include "../JASPL/jAlgorithm/jalgorithm.h"
#include "../ATS9462/Digitizer/qts9462.h"

namespace jaspl {

QT_CHARTS_USE_NAMESPACE

class jSpectrumAnalyzer : public QMainWindow {

    Q_OBJECT

  public:
    explicit jSpectrumAnalyzer(QWidget *parent = 0);
    ~jSpectrumAnalyzer();

    void Activate();

    template <class T, typename F>
    void PlotAutoScale(T y_signal_elements , F x_frequency_range);

    template <class T>
    void Plot(T y_signal_elements , double x_frequency_range);

  private:

    void ProcessSignal( std::vector < float > & series );

    QLineSeries *spectrum_series;
    QValueAxis *x_axis;
    QValueAxis *y_axis;
    QChartView *chartView;
    QTimer *auto_timer;
    QChart *chart;

    QString title = "Spectrum Analyzer";

    JFFT fft_er;
    uint fft_points = 1024;

    QTS9462 *digitizer;
    jaspl::RecurseMean < std::vector <float > > avg;

  public slots:
    void UpdateSignal( std::vector<float> time_series , uint sample_rate );
    void UpdateAndAverage( std::vector<float> time_series , uint sample_rate );

  signals:
    void SignalChanged();
};

#include "../ATS9462/jSpectrumAnalyzer/jspectrumanalyzer.tpp"

}

#endif // JSPECTRUMANALYZER_H
