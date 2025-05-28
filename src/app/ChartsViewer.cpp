#include "ChartsViewer.h"
#include "ui_ChartsViewer.h"

#include <QButtonGroup>

#include "Application.h"
#include "ProjectData.h"
#include "charts/ChartPainter.h"

ChartsViewer::ChartsViewer(QWidget* parent) : NativeBaseWindow(parent), ui(new Ui::ChartsViewerClass())
{
	ui->setupUi(this);
	setAttribute(Qt::WA_StyledBackground);

	ui->headerWidget->setMenuButtonVisible(false);
	ui->headerWidget->setTitleVisible(false);

	ui->customFlowWidgetSeg->setVisible(false);

	connect(ui->headerWidget, &HeaderWidget::minBtnClicked, this, &QWidget::showMinimized);
	connect(ui->headerWidget, &HeaderWidget::maxBtnClicked, this, &QWidget::showMaximized);
	connect(ui->headerWidget, &HeaderWidget::restoreBtnClicked, this, &QWidget::showNormal);
	connect(ui->headerWidget, &HeaderWidget::closeBtnClicked, this, [&]() {
		setVisible(false);
		});
	connect(ui->comboBoxAnalyseDim, qOverload<int>(&QComboBox::currentIndexChanged), this, &ChartsViewer::dimSelectChanged);
	connect(ui->comboBoxWorkConditions, qOverload<int>(&QComboBox::currentIndexChanged), this, &ChartsViewer::wcSelectChanged);
	connect(ui->comboBoxSense, qOverload<int>(&QComboBox::currentIndexChanged), this, &ChartsViewer::updateCharts);

	// 创建按钮组
	QButtonGroup* radioGroup = new QButtonGroup(this);
	// 将三个QRadioButton添加到按钮组
	radioGroup->addButton(ui->radioButtonAll);
	radioGroup->addButton(ui->radioButtonOnlyts);
	radioGroup->addButton(ui->radioButtonOnlyfs);
	connect(radioGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked), this, &ChartsViewer::modeChanged);

	connect(ui->checkBoxShowSeg, &QCheckBox::stateChanged, this, &ChartsViewer::updateSegCharts);
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
	ui->comboBoxAnalyseDim->setCurrentIndex(0);
	dimSelectChanged(0);
	ui->comboBoxAnalyseDim->blockSignals(false);
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

void ChartsViewer::updateCharts()
{
	ui->customFlowWidget->removeAll();
	if (!_currentCharts)
		return;

	auto mode = getShowMode();
	switch (mode)
	{
	case ChartsViewer::ShowMode::ALL:
	{
		ui->customFlowWidget->setSuitableItemSize(1000, 360);
		break;
	}
	case ChartsViewer::ShowMode::ONLYTS:
	case ChartsViewer::ShowMode::ONLYFS:
	{
		ui->customFlowWidget->setSuitableItemSize(1000, 180);
		break;
	}
	default:
		break;
	}

	auto index = ui->comboBoxSense->currentIndex();
	if (0 == index)
	{
		for (int i = 1; i < ui->comboBoxSense->count(); i++)
		{
			auto sensorname = ui->comboBoxSense->itemData(i, Qt::DisplayRole).toString();
			auto widget = _currentCharts->getChart(sensorname, int(mode));
			ui->customFlowWidget->addItem(new CustomFlowWidgetItem(widget,""));
		}
	}
	else
	{
		auto sensorname = ui->comboBoxSense->currentText();
		auto widget = _currentCharts->getChart(sensorname, int(mode));
		ui->customFlowWidget->addItem(new CustomFlowWidgetItem(widget, ""));
	}

	auto type = ui->comboBoxAnalyseDim->currentData().value<ResType>();
	auto wcname = ui->comboBoxWorkConditions->currentData(Qt::DisplayRole).toString();
	auto hasSegData = cApp->getProjData()->hasSegData(type, wcname);
	ui->checkBoxShowSeg->setEnabled(hasSegData);
	if (0 == ui->comboBoxSense->currentIndex())
	{
		ui->checkBoxShowSeg->setEnabled(false);
		ui->checkBoxShowSeg->setChecked(false);
	}
	else
	{
		ui->checkBoxShowSeg->setEnabled(hasSegData);
	}
}

void ChartsViewer::updateSegCharts(int state)
{
	if (!_currentCharts)
		return;

	auto mode = getShowMode();
	switch (mode)
	{
	case ChartsViewer::ShowMode::ALL:
	{
		ui->customFlowWidgetSeg->setSuitableItemSize(1000, 380);
		break;
	}
	case ChartsViewer::ShowMode::ONLYTS:
	case ChartsViewer::ShowMode::ONLYFS:
	{
		ui->customFlowWidgetSeg->setSuitableItemSize(1000, 200);
		break;
	}
	default:
		break;
	}

	if (state == Qt::CheckState::Checked)
	{
		ui->customFlowWidgetSeg->setVisible(true);
		ui->customFlowWidgetSeg->removeAll();
		auto sensorname = ui->comboBoxSense->currentText();
		auto widgets = _currentCharts->getSegChart(sensorname, int(mode));
		auto wcname = ui->comboBoxWorkConditions->currentData(Qt::DisplayRole).toString();
		auto segnames = cApp->getProjData()->getSegWorkingConditionsNames(wcname);
		if (segnames.count() != widgets.count())
		{
			return;
		}
		for (auto i = 0;i < segnames.count();++i)
		{
			ui->customFlowWidgetSeg->addItem(new CustomFlowWidgetItem(widgets[i], segnames[i]));
		}
	}
	else if (state == Qt::CheckState::Unchecked)
	{
		ui->customFlowWidgetSeg->setVisible(false);
	}

}

void ChartsViewer::modeChanged(QAbstractButton* button)
{
	QRadioButton* selectedRadio = qobject_cast<QRadioButton*>(button);
	updateCharts();
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
	ui->comboBoxWorkConditions->setCurrentIndex(0);
	wcSelectChanged(0);
	ui->comboBoxWorkConditions->blockSignals(false);
}

void ChartsViewer::wcSelectChanged(int index)
{
	if (_currentCharts)
	{
		delete _currentCharts;
	}
	auto type = ui->comboBoxAnalyseDim->currentData().value<ResType>();
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
	ui->comboBoxSense->setCurrentIndex(0);
	updateCharts();
	ui->comboBoxSense->blockSignals(false);
}

//void ChartsViewer::senseSelectChanged(int index)
//{
//	//auto type = ui->comboBoxAnalyseDim->itemData(index).value<ResType>();
//	//auto wcname = ui->comboBoxWorkConditions->itemData(index, Qt::DisplayRole).toString();
//	//
//	//auto sensorname = ui->comboBoxSense->itemData(index, Qt::DisplayRole).toString();
//	updateCharts();
//}
