#pragma once

#include <QWidget>
#include "ui_Main3DWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Main3DWidgetClass; };
QT_END_NAMESPACE

class Main3DWidget : public QWidget
{
	Q_OBJECT

public:
	Main3DWidget(QWidget *parent = nullptr);
	~Main3DWidget();

private:
	Ui::Main3DWidgetClass *ui;
};
