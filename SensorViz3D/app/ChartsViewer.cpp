#include "ChartsViewer.h"
#include "ui_ChartsViewer.h"

#include "Application.h"
#include "ProjectData.h"

ChartsViewer::ChartsViewer(QWidget* parent) : NativeBaseWindow(parent), ui(new Ui::ChartsViewerClass())
{
	ui->setupUi(this);
	setAttribute(Qt::WA_StyledBackground);

	ui->headerWidget->setMenuButtonVisible(false);
	ui->headerWidget->setTitleVisible(false);

	connect(ui->headerWidget, &HeaderWidget::minBtnClicked, this, &QWidget::showMinimized);
	connect(ui->headerWidget, &HeaderWidget::maxBtnClicked, this, &QWidget::showMaximized);
	connect(ui->headerWidget, &HeaderWidget::restoreBtnClicked, this, &QWidget::showNormal);
	connect(ui->headerWidget, &HeaderWidget::closeBtnClicked, this, [&]() {
		setVisible(false);
		});
	connect(ui->comboBoxAnalyseDim, qOverload<int>(&QComboBox::currentIndexChanged), this, &ChartsViewer::dimSelectChanged);
	connect(ui->comboBoxWorkConditions, qOverload<int>(&QComboBox::currentIndexChanged), this, &ChartsViewer::wcSelectChanged);
	connect(ui->comboBoxSense, qOverload<int>(&QComboBox::currentIndexChanged), this, &ChartsViewer::update);
}

ChartsViewer::~ChartsViewer()
{
	delete ui;
}

void ChartsViewer::fill()
{
	auto dims = cApp->getProjData()->getDimNames();
	ui->comboBoxAnalyseDim->blockSignals(true);
	for (auto& dim : dims)
	{
		ui->comboBoxAnalyseDim->addItem(dim.first, QVariant::fromValue<ResType>(dim.second));
	}
	ui->comboBoxAnalyseDim->blockSignals(false);
	ui->comboBoxAnalyseDim->setCurrentIndex(0);
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

void ChartsViewer::update()
{
	if (!_currentCharts)
	{
		ui->customFlowWidget->removeAll();
		return;
	}

	auto sensorname = ui->comboBoxSense->currentText();


}

ChartsViewer::ShowMode ChartsViewer::getShowMode()
{
	if (ui->radioButtonAll->isChecked())
	{
		return ShowMode::ALL;
	}
	else if (ui->radioButtonOnlyts->isChecked())
	{
		return ShowMode::ONLYTS;
	}
	else if (ui->radioButtonOnlyfs->isChecked())
	{
		return ShowMode::ONLYFS;
	}
	return ShowMode::ALL;
}

void ChartsViewer::dimSelectChanged(int index)
{
	auto type = ui->comboBoxAnalyseDim->itemData(index, Qt::UserRole).value<ResType>();
	auto wcnames = cApp->getProjData()->geWorkingConditionsNames(type);

	ui->comboBoxWorkConditions->blockSignals(true);
	ui->comboBoxWorkConditions->clear();
	for (auto& wc : wcnames)
	{
		ui->comboBoxWorkConditions->addItem(wc.first, wc.second);
	}
	ui->comboBoxWorkConditions->blockSignals(false);
	ui->comboBoxWorkConditions->setCurrentIndex(0);
}

void ChartsViewer::wcSelectChanged(int index)
{
	auto type = ui->comboBoxAnalyseDim->itemData(index).value<ResType>();
	auto wcname = ui->comboBoxWorkConditions->itemData(index, Qt::DisplayRole).toString();

	auto sensenames = cApp->getProjData()->geSensorNames(type, wcname);
	_currentCharts = cApp->getProjData()->getCharts(type, wcname);

	ui->comboBoxSense->blockSignals(true);
	ui->comboBoxSense->clear();
	ui->comboBoxSense->addItem("全部");
	for (auto& sn : sensenames)
	{
		ui->comboBoxSense->addItem(sn);
	}
	ui->comboBoxSense->blockSignals(false);
	ui->comboBoxWorkConditions->setCurrentIndex(0);
}

//void ChartsViewer::senseSelectChanged(int index)
//{
//	//auto type = ui->comboBoxAnalyseDim->itemData(index).value<ResType>();
//	//auto wcname = ui->comboBoxWorkConditions->itemData(index, Qt::DisplayRole).toString();
//	//
//	//auto sensorname = ui->comboBoxSense->itemData(index, Qt::DisplayRole).toString();
//	update();
//}
