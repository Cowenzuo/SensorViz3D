#include "CustomFlowWidget.h"
#include "ui_CustomFlowWidget.h"

#include <QHBoxLayout>
#include <QScrollArea>

CustomFlowWidget::CustomFlowWidget(QWidget *parent) : QWidget(parent), ui(new Ui::CustomFlowWidget), _itemSuitableWidth(1), _itemSuitableHeight(1) {
  _scrollArea = new QScrollArea(this);
  _contentWiget = new QWidget();
  _scrollArea->setWidget(_contentWiget);

  ui->setupUi(this);

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(_scrollArea);
  layout->setSpacing(0);
  layout->setMargin(0);
  setLayout(layout);
}

CustomFlowWidget::~CustomFlowWidget() {
  delete ui;
}

void CustomFlowWidget::addItem(QWidget *itemWidget) {
  if (!_items.contains(itemWidget)) {
    if (_items.count() >= 100) {
      _items.removeLast();
    }
    _items.push_front(itemWidget);
    itemWidget->setParent(_contentWiget);
    itemWidget->show();
    resizeEvent(nullptr);
  }
}

bool CustomFlowWidget::removeItem(QWidget *itemWidget) {
  if (_items.contains(itemWidget)) {
    itemWidget->hide();
    itemWidget->setParent(NULL);
    _items.removeOne(itemWidget);
    resizeEvent(nullptr);
    return true;
  }
  return false;
}

void CustomFlowWidget::removeAll() {
  for (auto i = 0; i < _items.count(); i++) {
    QWidget *item = _items[i];
    item->hide();
    item->setParent(NULL);
  }

  if (!_items.isEmpty()) {
    _items.clear();
  }
  resizeEvent(nullptr);
}

void CustomFlowWidget::setSuitableItemSize(int width, int height) {
  _itemSuitableWidth = width;
  _itemSuitableHeight = height;
  setMinimumSize(358, 150);
}

// void CustomFlowWidget::setViewPortRowCount(int rowCount)
//{
//     _viewPortRowCount =rowCount;
// }
//
// void CustomFlowWidget::setViewPortColCount(int colCount)
//{
//     _viewPortColCount =colCount;
// }

void CustomFlowWidget::resizeEvent(QResizeEvent *event) {
  Q_UNUSED(event);
  _contentWiget->setFixedWidth(_scrollArea->width() - 20);
  int colCount = 0;
  {
    int integer = _contentWiget->width() / _itemSuitableWidth;
    int remainder = _contentWiget->width() % _itemSuitableWidth;
    if (0 != remainder) {
      integer += (remainder * 2 >= _itemSuitableWidth) ? 1 : 0;
    }
    colCount = integer;
  }
  int rowCount = (_items.count() / colCount) + ((0 == (_items.count() % colCount)) ? 0 : 1);
  int itemScaledWidth = float(_contentWiget->width()) / float(colCount);
  int itemScaledHeight = (float(_itemSuitableHeight) / float(_itemSuitableWidth)) * itemScaledWidth;
  _contentWiget->setFixedHeight(itemScaledHeight * rowCount);
  for (int row = 0; row < rowCount; ++row) {
    for (int col = 0; col < colCount; ++col) {
      int seq = row * colCount + col;
      if (seq < _items.count()) {
        QWidget *item = _items[seq];
        item->setFixedSize(itemScaledWidth, itemScaledHeight);
        item->move(itemScaledWidth * col, itemScaledHeight * row);
      }
    }
  }
}

void CustomFlowWidget::showEvent(QShowEvent *event) {
  Q_UNUSED(event);
  // resizeEvent(nullptr);
}
