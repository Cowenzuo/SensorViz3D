#include "ChartsViewer.h"

ChartsViewer::ChartsViewer(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::ChartsViewerClass())
{
	ui->setupUi(this);
}

ChartsViewer::~ChartsViewer()
{
	delete ui;
}
