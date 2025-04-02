#pragma once
#include "ui/base/NativeBaseWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChartsViewerClass; };
QT_END_NAMESPACE

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
	Ui::ChartsViewerClass* ui;
};
