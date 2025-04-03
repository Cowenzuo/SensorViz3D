#ifndef CUSTOMFLOWWIDGET_H
#define CUSTOMFLOWWIDGET_H

#include <QVector>
#include <QWidget>
#include <QLabel>

class QScrollArea;
QT_BEGIN_NAMESPACE
namespace Ui {
	class CustomFlowWidget;
}
QT_END_NAMESPACE
class CustomFlowWidgetItem : public QWidget {
	Q_OBJECT
public:
	CustomFlowWidgetItem(QWidget* parent = nullptr);
	CustomFlowWidgetItem(QWidget* child, const QString& tiltle,QWidget* parent = nullptr);
	virtual ~CustomFlowWidgetItem();

	void setContainWidget(QWidget* child, const QString& tiltle);
private:
	QWidget* _child{};
	QWidget* _container{};
	QLabel* _title{};
};

class CustomFlowWidget : public QWidget {
	Q_OBJECT
private:
	QScrollArea* _scrollArea;
	QWidget* _contentWidget;
	QVector<QWidget*> _items;
	int _itemSuitableWidth;
	int _itemSuitableHeight;
	int _viewPortRowCount{};
	int _viewPortColCount{};

public:
	CustomFlowWidget(QWidget* parent = nullptr);
	~CustomFlowWidget();
	void addItem(QWidget* itemWidget, bool asc = true);
	bool removeItem(QWidget* itemWidget);
	void removeAll();
	void setSuitableItemSize(int width, int height);
	// void setViewPortRowCount(int rowCount);
	// void setViewPortColCount(int colCount);
protected:
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void showEvent(QShowEvent* event) override;

private:
	Ui::CustomFlowWidget* ui;
};
#endif  // CUSTOMFLOWWIDGET_H
