#include "qts9462.h"

QTS9462::QTS9462(QObject *parent, uint system_id,
                 uint board_id) : QObject(parent), ATS9462(system_id, board_id) {

  SetDefaultConfig();

  auto_timer = new QTimer(this);
  connect(auto_timer, &QTimer::timeout, this, &QTS9462::SendTimeSeries);

}

QTS9462::~QTS9462() {
  Stop();
}

void QTS9462::SendTimeSeries() {

  auto time_series_volts = PullVoltageDataTail(number_of_samples);

  if ( time_series_volts.size() > 1 ) {
    emit VoltageTimeSeries(time_series_volts, sample_rate);
  }

}

void QTS9462::UpdateSamplesSent(uint num_samples) {
  number_of_samples = num_samples;
}

void QTS9462::Start() {

  auto_timer->start(5);
  StartCapture();

}

void QTS9462::Stop() {

  auto_timer->stop();
  AbortCapture();
}
