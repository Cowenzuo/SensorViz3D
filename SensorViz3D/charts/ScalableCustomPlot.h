#pragma once

#include <QWidget>
#include "qcustomplot.h"

class ScalableCustomPlot : public QCustomPlot {
 public:
  explicit ScalableCustomPlot(QWidget *parent = nullptr);
  void setOriginalRanges();
  void resetRanges();
  void setTitle(const QString &title);
  void setDataSelectable(bool enable);

 protected:
  void wheelEvent(QWheelEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void enterEvent(QEvent *event) override;
  void leaveEvent(QEvent *event) override;

 private:
  void updateTracerPosition(const QPoint &pos);
  void enforceRangeLimits();

  QCPRange originalXRange;
  QCPRange originalYRange;

  QCPItemTracer *tracer;  // 用于显示数据点的标记
};