#ifndef CUSTOMFLOWWIDGET_H
#define CUSTOMFLOWWIDGET_H

#include <QVector>
#include <QWidget>

class QScrollArea;
QT_BEGIN_NAMESPACE
namespace Ui {
class CustomFlowWidget;
}
QT_END_NAMESPACE

class CustomFlowWidget : public QWidget {
  Q_OBJECT
 private:
  QScrollArea *_scrollArea;
  QWidget *_contentWidget;
  QVector<QWidget *> _items;
  int _itemSuitableWidth;
  int _itemSuitableHeight;
  int _viewPortRowCount{};
  int _viewPortColCount{};

 public:
  CustomFlowWidget(QWidget *parent = nullptr);
  ~CustomFlowWidget();
  void addItem(QWidget *itemWidget);
  bool removeItem(QWidget *itemWidget);
  void removeAll();
  void setSuitableItemSize(int width, int height);
  // void setViewPortRowCount(int rowCount);
  // void setViewPortColCount(int colCount);
 protected:
  virtual void resizeEvent(QResizeEvent *event) override;
  virtual void showEvent(QShowEvent *event) override;

 private:
  Ui::CustomFlowWidget *ui;
};
#endif  // CUSTOMFLOWWIDGET_H
