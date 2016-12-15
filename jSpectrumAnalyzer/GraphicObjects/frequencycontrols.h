#ifndef FREQUENCYCONTROLS_H
#define FREQUENCYCONTROLS_H

#include <QDockWidget>
#include <QString>

namespace Ui {
class FrequencyControls;
}

class FrequencyControls : public QDockWidget {

  Q_OBJECT

 public:
  explicit FrequencyControls(QWidget *parent = 0);
  ~FrequencyControls();

 private:
  Ui::FrequencyControls *ui;


 signals:
  void MinSet(double min_val);
  void MaxSet(double max_val);
  void UnitSelected(QString units);
  void SpanSet(int span_val);
  void CenterSet(int cent_val);

 public slots:
  void SetFreqSpan(int span);
  void SetMinMax(std::pair< int, int > vals);

 private slots:
  void CenterSpanUpdate();
  void MinMaxUpdate();

};

#endif // FREQUENCYCONTROLS_H
