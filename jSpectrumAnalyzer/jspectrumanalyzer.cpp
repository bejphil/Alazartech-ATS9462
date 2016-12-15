//Header for this file
#include "jSpectrumAnalyzer/jspectrumanalyzer.h"
#include "jSpectrumAnalyzer/GraphicObjects/chartscalecontrols.h"
#include "jSpectrumAnalyzer/GraphicObjects/frequencycontrols.h"
//C System-Headers
#include <math.h>
//C++ System headers
//
//OpenCL Headers
//
//Boost Headers
//
//Qt Headers
//
//Project specific headers
//

namespace jaspl {

jSpectrumAnalyzer::jSpectrumAnalyzer(QWidget *parent) : QMainWindow(parent),
  avg(fft_points / 2) {

  spectrum_series = new QLineSeries();

  x_axis = new QValueAxis;
  y_axis = new QValueAxis;
  chart = new QChart();

  chart->addSeries(spectrum_series);
  chart->legend()->hide();

  chart->createDefaultAxes();
  chart->setAxisX(x_axis, spectrum_series);
  chart->setAxisY(y_axis, spectrum_series);

  chart->setTheme(QChart::ChartThemeDark);

  QPen pen(Qt::yellow);
  pen.setWidth(1);
  spectrum_series->setPen(pen);

  QPalette pal = palette();
  pal.setColor(QPalette::Window, QRgb(0x121218));
  pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));
  setPalette(pal);

  chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);

  fft_points = 1024;

  fft_er.Setup(fft_points);

  setCentralWidget(chartView);


  digitizer = new QTS9462(this);
  digitizer->SetSampleRate(10e6);
  digitizer->UpdateSamplesSent(fft_points);

  digitizer->Start();
  connect(digitizer, &QTS9462::VoltageTimeSeries, this,
          &jSpectrumAnalyzer::UpdateSignal);

  auto power_ctrls = new PowerControls(this);
  power_ctrls->setAttribute(Qt::WA_DeleteOnClose);
  power_ctrls->setPalette(pal);

  connect(power_ctrls, &PowerControls::MinSet, [ = ](double val) {
    chart->axisY()->setMin(val);
  });
  connect(power_ctrls, &PowerControls::MaxSet, [ = ](double val) {
    chart->axisY()->setMax(val);
  });

  addDockWidget(Qt::RightDockWidgetArea, power_ctrls);

//    auto frequency_ctrls = new FrequencyControls( this );
//    frequency_ctrls->setAttribute(Qt::WA_DeleteOnClose);

//    connect(frequency_ctrls, &FrequencyControls::MinSet, [ = ](double val) { chart->axisX()->setMin( val ); });
//    connect(frequency_ctrls, &FrequencyControls::MaxSet, [ = ](double val) { chart->axisX()->setMax( val ); });

//    addDockWidget(Qt::RightDockWidgetArea, frequency_ctrls);

  setWindowTitle(title);

  resize(1280, 720);
  setAttribute(Qt::WA_DeleteOnClose);
  show();

}

void jSpectrumAnalyzer::Activate() {
  //
}

void VoltsTodBm(float &voltage) {
  voltage = 20.0f * log10f(voltage / 50.0f);
}

//void VoltsTodBm_FFTCorrection(float &voltage, float signal_size) {
//  voltage = 10.0f * log10f(voltage) - log10f(signal_size * 50);
//}

//Combination of operations designed to compensate for
//1/N term introduce in FFT, and then convert from volts
//to dBm
//1) compensate for FFT effects: voltage/signal_size
//2) convert to dBm: volts -> dBm
//3) compensate for discarding half of spectrum: dBm/2
struct VoltsTodBm_FFTCorrection {
  float val;
  VoltsTodBm_FFTCorrection(float signal_size) : val(signal_size) {}

  void operator()(float &voltage) const {
    voltage = 10.0f * log10f(voltage / (50.0f * val));
  }
};

void jSpectrumAnalyzer::ProcessSignal(std::vector < float > &time_series) {

  fft_er.PowerSpectrum(time_series);
  uint spectrum_size = time_series.size();
  float spectrum_size_f = static_cast<float>(spectrum_size);


  uint n_half = (spectrum_size % 2 == 0) ? (spectrum_size / 2) : ((
                  spectrum_size - 1) / 2);
  time_series.erase(time_series.end() - n_half , time_series.end());

  std::for_each(time_series.begin(), time_series.end(),
                VoltsTodBm_FFTCorrection(spectrum_size_f));

}

void jSpectrumAnalyzer::UpdateSignal(std::vector< float > time_series,
                                     uint sample_rate) {

  ProcessSignal(time_series);

  float fft_span = static_cast<float>( sample_rate / 2 );

  Plot( time_series , fft_span );
}

void jSpectrumAnalyzer::UpdateAndAverage(std::vector< float > time_series,
                                     uint sample_rate) {

  ProcessSignal(time_series);

  avg( time_series );

  float fft_span = static_cast<float>( sample_rate / 2 );

  Plot( avg.ReturnValue() , fft_span );
}

jSpectrumAnalyzer::~jSpectrumAnalyzer() {
  fft_er.TearDown();
}

}
