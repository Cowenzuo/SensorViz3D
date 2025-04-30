#include "Main3DWidget.h"

Main3DWidget::Main3DWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::Main3DWidgetClass())
{
	ui->setupUi(this);
}

Main3DWidget::~Main3DWidget()
{
	delete ui;
}
