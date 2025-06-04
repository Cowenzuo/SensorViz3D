#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>

#include "Application.h"
#include "ChartsViewer.h"
#include "ProjectData.h"
#include "SceneCtrl.h"
#include "ui/base/OpeMessageBox.h"
#include "ui/SceneViewerSettings.h"
#include "ui/RendPlayer.h"
#include "ui/SensorValues.h"

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

	auto sceneWidgetSize = ui->main3DWidget->size();
	_widgetSvs = new SceneViewerSettings(this);
	_widgetSvs->move(sceneWidgetSize.width() - 350, ui->headerWidget->height() + 10);
	_widgetSvs->raise();
	connect(_widgetSvs, &SceneViewerSettings::currentWeightChanged, this, &MainWindow::handleWeightChanged);
	connect(_widgetSvs, &SceneViewerSettings::maxThresholdChanged, this, &MainWindow::handleMaxThresholdChanged);
	connect(_widgetSvs, &SceneViewerSettings::radiationThresholdChanged, this, &MainWindow::handleRadiationThresholdChanged);


	_widgetRp = new RendPlayer(this);
	_widgetRp->move((sceneWidgetSize.width() - _widgetRp->width()) / 2.0, height() - 200);
	_widgetRp->raise();
	connect(_widgetRp, &RendPlayer::timestampChanged, this, &MainWindow::handleTimestampChanged);

	_sceneValue = new SensorValues(this);
	_sceneValue->setAttribute(Qt::WA_TransparentForMouseEvents);
	_sceneValue->move(10, ui->headerWidget->height() + 10);
	_sceneValue->raise();

	_sceneCtrl = new SceneCtrl(ui->main3DWidget);
	_sceneCtrl->installSimRender(QVector<SensorPositon>());
}

MainWindow::~MainWindow()
{
	delete ui;
}

SceneViewer* MainWindow::getSceneViewer()
{
	return ui->main3DWidget;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	auto sceneWidgetSize = ui->main3DWidget->size();
	if (_widgetSvs)
	{
		_widgetSvs->move(sceneWidgetSize.width() - 350, ui->headerWidget->height() + 10);
	}
	if (_widgetRp)
	{
		_widgetRp->move((sceneWidgetSize.width() - _widgetRp->width()) / 2.0, height() - 100);
	}
	if (_sceneValue)
	{
		_sceneValue->move(10, ui->headerWidget->height() + 10);
	}
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

void MainWindow::wcSelectChangedSlot(ResType type, QString wcname)
{
	_currentDimType = type;
	_currentWcname = wcname;
	_sceneCtrl->uninstallSimRender();

	auto pos = cApp->getProjData()->getSensorPositions(type);
	if (pos.isEmpty())
		return;
	auto& exdata = cApp->getProjData()->getExtraData(type, wcname);
	_widgetRp->setRange(0, exdata.dataCount - 1);
	auto weight = _widgetSvs->getCurrentWeight();
	double min, max;
	bool firstCompare = true;
	QVector<float> values;
	QStringList names;
	for (auto i = 0; i < pos.count(); i++)
	{
		auto fullName = pos[i].name + ((_currentDimType == ResType::GVA || _currentDimType == ResType::GVD) ? (QString("-") + weight) : "");
		if (exdata.statistics.contains(fullName))
		{
			if (firstCompare)
			{
				firstCompare = false;
				min = exdata.statistics[fullName].min;
				max = exdata.statistics[fullName].max;
			}
			else
			{
				min = qMin(exdata.statistics[fullName].min, min);
				max = qMax(exdata.statistics[fullName].max, max);
			}
			names.append(pos[i].name);
		}
	}
	max = qMax(qAbs(min), qAbs(max));//正负只是方向而已，因此极值需要做取模
	_widgetSvs->resetMaxThresholdValue(max);
	_widgetSvs->resetRadiationThresholdValue(ui->main3DWidget->getModelNodeRadius() * 2.0);
	_sceneValue->setSensorNames(names);
	_sceneCtrl->installSimRender(pos);
}

void MainWindow::headerMenuTriggered() {
	auto chartsViewerVisible = cApp->getChartsViewer()->isVisible();
	auto isSetData = !(cApp->getProjData()->getRootDirpath().isEmpty());
	auto isLoadData = cApp->getProjData()->hasLoadData();

	QMenu menu(this);
	QAction* importDataPackage = menu.addAction("导入数据包");
	QAction* generateReport = menu.addAction("生成数据图表");
	QAction* viewTableImg = menu.addAction("查看报表");
	viewTableImg->setCheckable(true);
	viewTableImg->setChecked(chartsViewerVisible);
	//QAction* saveToLoacl = menu.addAction("导出数据图表");

	importDataPackage->setEnabled(!isSetData);
	generateReport->setEnabled(isSetData && !isLoadData);
	viewTableImg->setEnabled(isLoadData);
	//saveToLoacl->setEnabled(isSetData);

	connect(importDataPackage, &QAction::triggered, this, &MainWindow::importDataPkgTriggered);
	connect(generateReport, &QAction::triggered, this, &MainWindow::generateReportTriggered);
	connect(viewTableImg, &QAction::toggled, this, &MainWindow::viewTableImgTriggered);
	//connect(saveToLoacl, &QAction::triggered, this, &MainWindow::saveToLoaclTriggered);
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
		cApp->getChartsViewer()->fill();
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

void MainWindow::handleTimestampChanged(int index)
{
	if (_currentWcname.isEmpty())
		return;
	auto pos = cApp->getProjData()->getSensorPositions(_currentDimType);
	if (pos.isEmpty())
		return;
	auto& exdata = cApp->getProjData()->getExtraData(_currentDimType, _currentWcname);
	auto weight = _widgetSvs->getCurrentWeight();
	QVector<float> values;
	for (auto i = 0; i < pos.count(); i++)
	{
		auto fullName = pos[i].name + ((_currentDimType == ResType::GVA || _currentDimType == ResType::GVD) ? (QString("-") + weight) : "");
		if (exdata.statistics.contains(fullName))
		{

			values.push_back(qAbs(exdata.data[fullName][index]));
		}
	}

	for (size_t i = 0; i < values.count(); i++)
	{
		values[i] = values[i] / _widgetSvs->getMaxThresholdValue();
		values[i] = qMin(values[i], 1.0f);
	}
	_sceneValue->setSensorValues(values);
	_sceneCtrl->updateSimValues(values);
}

void MainWindow::handleWeightChanged(const QString& value)
{
	handleTimestampChanged(_widgetRp->getCurrentTimpstamp());
}

void MainWindow::handleMaxThresholdChanged(float value)
{
	handleTimestampChanged(_widgetRp->getCurrentTimpstamp());
}

void MainWindow::handleRadiationThresholdChanged(float value)
{
	_sceneCtrl->updateRadiationThreshold(value);
	//handleTimestampChanged(_widgetRp->getCurrentTimpstamp());
}
