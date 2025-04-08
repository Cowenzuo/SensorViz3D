#include "CustomFlowWidget.h"
#include "ui_CustomFlowWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>

CustomFlowWidgetItem::CustomFlowWidgetItem(QWidget* parent) :QWidget(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	QVBoxLayout* layout = new QVBoxLayout;
	_container = new QWidget;
	_container->setObjectName("container");
	_title = new QLabel;
	_title->setObjectName("title");
	layout->addWidget(_title);
	layout->addWidget(_container);
	layout->setSpacing(0);
	layout->setMargin(5);
	layout->setStretch(0, 0);
	layout->setStretch(1, 1);
	setLayout(layout);
	setStyleSheet(R"(
			#title{
				font-size: 16px;
        		color:#ffffff;
				background:transparent;
				alignment:center;
			}
		)");
}

CustomFlowWidgetItem::CustomFlowWidgetItem(QWidget* child, const QString& tiltle, QWidget* parent) :CustomFlowWidgetItem(parent)
{
	setContainWidget(child, tiltle);
}

CustomFlowWidgetItem::~CustomFlowWidgetItem()
{
	auto layout = _container->layout();
	if (layout)
	{
		auto children = layout->children();
		for (auto& child : children)
		{
			child->setParent(NULL);
		}
	}
}

void CustomFlowWidgetItem::setContainWidget(QWidget* child, const QString& tiltle)
{
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(child);
	layout->setSpacing(0);
	layout->setMargin(0);
	_container->setLayout(layout);
	_title->setText(tiltle);
	_title->setVisible(!tiltle.isEmpty());
}


CustomFlowWidget::CustomFlowWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CustomFlowWidget), _itemSuitableWidth(1), _itemSuitableHeight(1) {
	_scrollArea = new QScrollArea(this);
	_contentWidget = new QWidget();
	_contentWidget->setStyleSheet("background:none;border:none");
	_contentWidget->setObjectName("contentWidget");
	_scrollArea->setWidget(_contentWidget);

	ui->setupUi(this);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(_scrollArea);
	layout->setSpacing(0);
	layout->setMargin(0);
	setLayout(layout);
}

CustomFlowWidget::~CustomFlowWidget() {
	delete ui;
}

void CustomFlowWidget::addItem(QWidget* itemWidget, bool asc/* = true*/) {
	if (_items.contains(itemWidget)) {
		return;
	}
	if (_items.count() >= 100) {
		_items.removeLast();
	}
	if (asc)
	{
		_items.push_back(itemWidget);
	}
	else
	{
		_items.push_front(itemWidget);
	}
	itemWidget->setParent(_contentWidget);
	itemWidget->show();
	resizeEvent(nullptr);
}

bool CustomFlowWidget::removeItem(QWidget* itemWidget) {
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
		QWidget* item = _items[i];
		//item->hide();
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
	setMinimumSize(_itemSuitableWidth, _itemSuitableHeight);
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

void CustomFlowWidget::resizeEvent(QResizeEvent* event) {
	Q_UNUSED(event);
	_contentWidget->setFixedWidth(_scrollArea->width() - 20);
	int colCount = 0;
	{
		int integer = _contentWidget->width() / _itemSuitableWidth;
		int remainder = _contentWidget->width() % _itemSuitableWidth;
		if (0 != remainder) {
			integer += (remainder * 2 >= _itemSuitableWidth) ? 1 : 0;
		}
		colCount = integer;
	}
	int rowCount = (_items.count() / colCount) + ((0 == (_items.count() % colCount)) ? 0 : 1);
	int itemScaledWidth = float(_contentWidget->width()) / float(colCount);
	int itemScaledHeight = (float(_itemSuitableHeight) / float(_itemSuitableWidth)) * itemScaledWidth;
	_contentWidget->setFixedHeight(itemScaledHeight * rowCount);
	for (int row = 0; row < rowCount; ++row) {
		for (int col = 0; col < colCount; ++col) {
			int seq = row * colCount + col;
			if (seq < _items.count()) {
				QWidget* item = _items[seq];
				item->setFixedSize(itemScaledWidth, itemScaledHeight);
				item->move(itemScaledWidth * col, itemScaledHeight * row);
			}
		}
	}
}

void CustomFlowWidget::showEvent(QShowEvent* event) {
	Q_UNUSED(event);
	// resizeEvent(nullptr);
}
