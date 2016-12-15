#ifndef CHARTSCALECONTROLS_H
#define CHARTSCALECONTROLS_H

#include <QDockWidget>
#include <QString>

namespace Ui {
class ChartScaleControls;
}

class PowerControls : public QDockWidget {
  Q_OBJECT

 public:
  explicit PowerControls(QWidget *parent = 0);
  ~PowerControls();

 private:
  Ui::ChartScaleControls *ui;

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

#endif // CHARTSCALECONTROLS_H
