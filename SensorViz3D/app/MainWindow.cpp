#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>

#include "Application.h"
#include "ChartsViewer.h"
#include "ProjectData.h"
#include "ui/base/OpeMessageBox.h"

MainWindow::MainWindow(QWidget* parent) : NativeBaseWindow(parent), ui(new Ui::MainWindowClass())
{
	ui->setupUi(this);
	setAttribute(Qt::WA_StyledBackground);

	connect(ui->headerWidget, &HeaderWidget::minBtnClicked, this, &QWidget::showMinimized);
	connect(ui->headerWidget, &HeaderWidget::maxBtnClicked, this, &QWidget::showMaximized);
	connect(ui->headerWidget, &HeaderWidget::restoreBtnClicked, this, &QWidget::showNormal);
	connect(ui->headerWidget, &HeaderWidget::closeBtnClicked, this, [&]() {
		cApp->getChartsViewer()->close();
		close();
		});
	connect(ui->headerWidget, &HeaderWidget::menuButtonTriggered, this, &MainWindow::headerMenuTriggered);
	//connect(ui->headerWidget, &HeaderWidget::closeBtnClicked, _customPlots, &QWidget::close);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::changeEvent(QEvent* event)
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

bool MainWindow::hitTestCaption(const QPoint& pos)
{
	QWidget* clickedWidget = childAt(pos);
	if (clickedWidget == ui->headerWidget && pos.y() < ui->headerWidget->height()) {
		return ui->headerWidget->hitTestCaption(ui->headerWidget->mapFrom(this, pos));
	}
	return NativeBaseWindow::hitTestCaption(pos);
}

void MainWindow::headerMenuTriggered() {
	QMenu menu(this);
	QAction* importDataPackage = menu.addAction("导入数据包");
	QAction* generateReport = menu.addAction("生成数据图表");
	QAction* viewTableImg = menu.addAction("查看报表");
	viewTableImg->setCheckable(true);
	viewTableImg->setChecked(cApp->getChartsViewer()->isVisible());
	QAction* saveToLoacl = menu.addAction("导出数据图表");
	generateReport->setEnabled(!(cApp->getProjData()->getRootDirpath().isEmpty()));
	viewTableImg->setEnabled(!(cApp->getProjData()->getRootDirpath().isEmpty()));
	saveToLoacl->setEnabled(!(cApp->getProjData()->getRootDirpath().isEmpty()));
	connect(importDataPackage, &QAction::triggered, this, &MainWindow::importDataPkgTriggered);
	connect(generateReport, &QAction::triggered, this, &MainWindow::generateReportTriggered);
	connect(viewTableImg, &QAction::toggled, this, &MainWindow::viewTableImgTriggered);
	connect(saveToLoacl, &QAction::triggered, this, &MainWindow::saveToLoaclTriggered);
	menu.exec(QCursor::pos());
}

void MainWindow::importDataPkgTriggered() {
	auto resDir = cApp->getProjData()->getRootDirpath();
	if (resDir.isEmpty())
	{
		resDir = cApp->applicationDirPath();
	}
	QString dirPath = QFileDialog::getExistingDirectory(this,
		tr("选择数据包文件夹"),
		resDir,
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
	);
	if (dirPath.isEmpty()) {
		return;
	}

	if (cApp->getProjData()->setDataPackage(dirPath))
	{
		OpeMessageBox::info(this, "消息", "数据包打开成功");
		ui->headerWidget->setTitle(cApp->getProjData()->getRootName());
	}
	else
	{
		OpeMessageBox::warning(this, "错误", "数据包无效，请重新选择");
	}
}

void MainWindow::generateReportTriggered()
{
	if (cApp->getProjData()->loadForVisual())
	{
		OpeMessageBox::info(this, "消息", "生成数据图表成功");
		ui->headerWidget->setTitle(cApp->getProjData()->getRootName());
	}
	else
	{
		OpeMessageBox::warning(this, "错误", "生成数据图表失败");
	}
}

void MainWindow::viewTableImgTriggered(bool open)
{
	cApp->getChartsViewer()->setVisible(open);
}

void MainWindow::saveToLoaclTriggered()
{
	QString dirPath = QFileDialog::getExistingDirectory(this,
		tr("选择保存文件夹"),
		cApp->applicationDirPath(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
	);
	if (dirPath.isEmpty()) {
		return;
	}
	QDir dir(dirPath);
	auto contains = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	if (!contains.isEmpty())
	{
		auto result = OpeMessageBox::question(this, "确认", "当前选择文件夹为非空文件夹,为避免检索困难，将自动添加时间名文件夹作为根导出路径，是否接受？");
		if (result == QMessageBox::StandardButton::No)
		{
			return;
		}
	}
	dirPath += QString("/Export_%1").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_HH_mm_ss"));
	cApp->getProjData()->saveBackground(dirPath, "");
}
