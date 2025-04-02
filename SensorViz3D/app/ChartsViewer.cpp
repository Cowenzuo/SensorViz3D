#include "ChartsViewer.h"
#include "ui_ChartsViewer.h"

#include <QDesktopWidget>

ChartsViewer::ChartsViewer(QWidget* parent) : NativeBaseWindow(parent), ui(new Ui::ChartsViewerClass())
{
	ui->setupUi(this);
	setAttribute(Qt::WA_StyledBackground);

	ui->headerWidget->setMenuButtonVisible(false);
	ui->headerWidget->setTitleVisible(false);

	connect(ui->headerWidget, &HeaderWidget::minBtnClicked, this, &QWidget::showMinimized);
	connect(ui->headerWidget, &HeaderWidget::maxBtnClicked, this, &QWidget::showMaximized);
	connect(ui->headerWidget, &HeaderWidget::restoreBtnClicked, this, &QWidget::showNormal);
	connect(ui->headerWidget, &HeaderWidget::closeBtnClicked, this, [&](){
		setVisible(false);
	});
}

ChartsViewer::~ChartsViewer()
{
	delete ui;
}

void ChartsViewer::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange) {
		// 解决最大化/还原时右上角按钮不能正确显示的问题
		if (this->windowState() == Qt::WindowNoState) {
			ui->headerWidget->handleWindowNormalState();
		}
		else if (this->windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen)) {
			ui->headerWidget->handleWindowMaximizedState();
		}
	}
	QWidget::changeEvent(event);
}

bool ChartsViewer::hitTestCaption(const QPoint& pos)
{
	QWidget* clickedWidget = childAt(pos);
	if (clickedWidget == ui->headerWidget && pos.y() < ui->headerWidget->height()) {
		return ui->headerWidget->hitTestCaption(ui->headerWidget->mapFrom(this, pos));
	}
	return NativeBaseWindow::hitTestCaption(pos);
}
