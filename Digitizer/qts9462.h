#ifndef QTS9462_H
#define QTS9462_H

// C System-Headers
//
// C++ System headers
//
//AlazarTech Headers
//
// Boost Headers
//
// Qt Headers
#include <QObject>
#include <QTimer>
// Project Specific Headers
#include "ATS9462/ats9462.h"

class QTS9462 : public QObject, public ATS9462 {

  Q_OBJECT

 public:

  explicit QTS9462(QObject *parent = 0, uint system_id = 1, uint board_id = 1);
  ~QTS9462();

 private:
  QTimer* auto_timer;
  uint number_of_samples = 1024;

 signals:
  void RawTimeSeries(std::vector< short unsigned int > time_series, uint s_rate );
  void VoltageTimeSeries(std::vector< float > time_series, uint s_rate );

 public slots:
//  void UpdateIntergrationTime(double int_time_sec);
//  void UpdateSampleRate(uint sample_rate);
  void UpdateSamplesSent( uint num_samples );
  void SendTimeSeries();

  void Start();
  void Stop();
};

#endif // QTS9462_H
