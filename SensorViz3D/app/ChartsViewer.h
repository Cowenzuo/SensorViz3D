#pragma once

#include <QWidget>
#include "ui_ChartsViewer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChartsViewerClass; };
QT_END_NAMESPACE

class ChartsViewer : public QWidget
{
	Q_OBJECT

public:
	ChartsViewer(QWidget *parent = nullptr);
	~ChartsViewer();

private:
	Ui::ChartsViewerClass *ui;
};
