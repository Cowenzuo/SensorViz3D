#include "ScalableCustomPlot.h"

#include <cmath>  // for std::isnan
#include <limits>

#include <QMouseEvent>
#include <QToolTip>
#include <QWheelEvent>

ScalableCustomPlot::ScalableCustomPlot(QWidget *parent) : QCustomPlot(parent), tracer(nullptr) {
  // 启用基本交互（拖动、缩放）
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  setOriginalRanges();
}

void ScalableCustomPlot::setOriginalRanges() {
  originalXRange = xAxis->range();
  originalYRange = yAxis->range();
}

void ScalableCustomPlot::resetRanges() {
  xAxis->setRange(originalXRange);
  yAxis->setRange(originalYRange);
  replot();
}

void ScalableCustomPlot::setTitle(const QString &title) {
  if (plotLayout()) {
    plotLayout()->insertRow(0);
    QCPTextElement *titleElement = new QCPTextElement(this);
    titleElement->setText(title);
    titleElement->setFont(QFont("sans", 9, QFont::Normal));
    plotLayout()->addElement(0, 0, titleElement);
    plotLayout()->setRowSpacing(0);
  }
}

void ScalableCustomPlot::setDataSelectable(bool enable) {
  setMouseTracking(enable);
  if (enable && !tracer) {
    tracer = new QCPItemTracer(this);
    tracer->setGraph(graph(0));
    tracer->setInterpolating(false);
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setPen(QPen(Qt::red));
    tracer->setBrush(Qt::red);
    tracer->setSize(5);
  }
  if (tracer) {
    tracer->setVisible(enable);
  }
  if (!enable) {
    QToolTip::hideText();
  }
}

void ScalableCustomPlot::wheelEvent(QWheelEvent *event) {
  QCustomPlot::wheelEvent(event);
  enforceRangeLimits();
  replot();
}

void ScalableCustomPlot::mouseDoubleClickEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  resetRanges();
}

void ScalableCustomPlot::mouseMoveEvent(QMouseEvent *event) {
  QCustomPlot::mouseMoveEvent(event);
  if (!graph(0)) {
    return;
  }
  if (!rect().contains(event->pos())) {
    return;
  }
  enforceRangeLimits();
  if (tracer) {
    updateTracerPosition(event->pos());
  }
  replot();
}

void ScalableCustomPlot::enterEvent(QEvent *event) {
  Q_UNUSED(event);
  setDataSelectable(true);
  replot();
}

void ScalableCustomPlot::leaveEvent(QEvent *event) {
  Q_UNUSED(event);
  setDataSelectable(false);
  replot();
}

void ScalableCustomPlot::updateTracerPosition(const QPoint &pos) {
  double x = xAxis->pixelToCoord(pos.x());
  double y = yAxis->pixelToCoord(pos.y());

  if (std::isnan(x) || std::isnan(y)) {
    return;
  }

  double closestX = 0, closestY = 0;
  double minDistance = std::numeric_limits<double>::max();
  for (auto it = graph(0)->data()->constBegin(); it != graph(0)->data()->constEnd(); ++it) {
    double distance = qAbs(it->key - x);
    if (distance < minDistance) {
      minDistance = distance;
      closestX = it->key;
      closestY = it->value;
    }
  }

  tracer->setGraphKey(closestX);
  tracer->updatePosition();

  QString tooltip = QString("X: %1\nY: %2").arg(closestX).arg(closestY);
  QToolTip::showText(mapToGlobal(pos), tooltip, this);
}

void ScalableCustomPlot::enforceRangeLimits() {
  if (xAxis->range().lower < originalXRange.lower) {
    xAxis->setRangeLower(originalXRange.lower);
  }
  if (xAxis->range().upper > originalXRange.upper) {
    xAxis->setRangeUpper(originalXRange.upper);
  }
  if (yAxis->range().lower < originalYRange.lower) {
    yAxis->setRangeLower(originalYRange.lower);
  }
  if (yAxis->range().upper > originalYRange.upper) {
    yAxis->setRangeUpper(originalYRange.upper);
  }
}