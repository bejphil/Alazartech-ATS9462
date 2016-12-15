#include "chartscalecontrols.h"
#include "ui_chartscalecontrols.h"
#include <QDebug>

PowerControls::PowerControls(QWidget *parent) :
  QDockWidget(parent),
  ui(new Ui::ChartScaleControls) {

  ui->setupUi(this);

  connect(ui->centerSlider, &QSlider::valueChanged, [ = ](int val) {
    emit SpanSet(val);
  });
  connect(ui->spanSlider, &QSlider::valueChanged, [ = ](int val) {
    emit CenterSet(val);
  });

  connect(ui->centerSlider, &QSlider::valueChanged, this,
          &PowerControls::CenterSpanUpdate);
  connect(ui->spanSlider, &QSlider::valueChanged, this,
          &PowerControls::CenterSpanUpdate);

  connect(ui->minBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ](
  double val) {
    emit MinSet(val);
  });
  connect(ui->maxBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ](
  double val) {
    emit MaxSet(val);
  });

  connect(ui->minBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &PowerControls::MinMaxUpdate);
  connect(ui->maxBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &PowerControls::MinMaxUpdate);

}

PowerControls::~PowerControls() {
  delete ui;
}

void PowerControls::SetFreqSpan(int span) {

  ui->spanSlider->setMaximum(span);

  double span_f = static_cast<double>(span);
  ui->maxBox->setMaximum(span_f);
}

void PowerControls::SetMinMax(std::pair< int, int > vals) {

  int min = vals.first;
  int max = vals.second;

  int delta = max - min;

  ui->spanSlider->setMaximum(delta);

  ui->centerSlider->setMinimum(min);
  ui->centerSlider->setMaximum(max);


  ui->maxBox->blockSignals(true);
  ui->minBox->blockSignals(true);

  ui->maxBox->setValue(max);
  ui->minBox->setValue(min);

  ui->maxBox->blockSignals(false);
  ui->minBox->blockSignals(false);

}

void PowerControls::CenterSpanUpdate() {

  int span = ui->spanSlider->value();
  int center = ui->centerSlider->value();

  double span_half = static_cast<double>(span / 2);

  double min = static_cast<double>(center - span_half);
  double max = static_cast<double>(center + span_half);

  ui->maxBox->setValue(max);
  ui->minBox->setValue(min);

}

void PowerControls::MinMaxUpdate() {

  double min = ui->minBox->value();
  double max = ui->maxBox->value();

  int delta = static_cast<int>( max - min );

  int mid_point =static_cast<int>( (max + min) / 2 );

  ui->spanSlider->blockSignals(true);
  ui->centerSlider->blockSignals(true);

  ui->spanSlider->setValue(delta);
  ui->spanLCD->display(delta);

  ui->centerSlider->setValue(mid_point);
  ui->centerLCD->display(mid_point);


  ui->spanSlider->blockSignals(false);
  ui->centerSlider->blockSignals(false);
}
