#pragma once
#include "ui/base/NativeBaseWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChartsViewerClass; };
QT_END_NAMESPACE

class ChartPainter;
class ChartsViewer : public NativeBaseWindow
{
	Q_OBJECT

public:
	ChartsViewer(QWidget* parent = nullptr);
	virtual~ChartsViewer();

	void fill();

protected:
	void changeEvent(QEvent* event) override;
	bool hitTestCaption(const QPoint& pos) override;

private:

	enum class ShowMode
	{
		ALL,
		ONLYTS,
		ONLYFS
	};
	ShowMode getShowMode();

private:
	Q_SLOT void dimSelectChanged(int index);
	Q_SLOT void wcSelectChanged(int index);
	//Q_SLOT void senseSelectChanged(int index);
	Q_SLOT void update();
private:
	Ui::ChartsViewerClass* ui;
	const ChartPainter* _currentCharts{};
};
