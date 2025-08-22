#include "SceneViewerSettings.h"

SceneViewerSettings::SceneViewerSettings(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::SceneViewerSettingsClass())
{
	ui->setupUi(this);

	ui->hSliderMaxThreshold->setRange(0, 1000);
	ui->hSliderMinThreshold->setRange(0, 1000);
	connect(ui->hSliderMaxThreshold, &QSlider::valueChanged, this, &SceneViewerSettings::updateColorMap);
	connect(ui->hSliderMinThreshold, &QSlider::valueChanged, this, &SceneViewerSettings::updateColorMap);

	ui->hSliderRadiationThreshold->setRange(0, 100);
	ui->labelRadiationThreshold->setText(QString::number(1.0));
	ui->hSliderRadiationThreshold->setValue(50);
	connect(ui->hSliderRadiationThreshold, &QSlider::valueChanged, this, [&](int value) {
		float newvalue;
		if (value <= 50) {
			// 0-50线性映射到0.0-1.0
			newvalue = static_cast<float>(value) / 50.0f;
		}
		else {
			// 50-100线性映射到1.0-50.0（9.0的增量范围）
			newvalue = 1.0f + (value - 50) * 49.0f / 50.0f;
		}

		// 保留两位小数显示
		ui->labelRadiationThreshold->setText(QString::number(newvalue, 'f', 2));

		// 发射信号通知其他组件
		emit dispmentScaledChanged(newvalue);
		});

	connect(ui->comboBox, &QComboBox::currentTextChanged, this, &SceneViewerSettings::currentWeightChanged);
}

SceneViewerSettings::~SceneViewerSettings()
{
	delete ui;
}

QString SceneViewerSettings::getCurrentWeight()
{
	return ui->comboBox->currentText();
}

int SceneViewerSettings::getCurrentWeightIndex()
{
	return ui->comboBox->currentIndex();
}

float SceneViewerSettings::getCurrentRange()
{
	return currentRange;
}

void SceneViewerSettings::resetMinmaxThresholdValue(float max, float min)
{
	preMaxValue = max;
	preMinValue = min;
	ui->hSliderMaxThreshold->blockSignals(true);
	ui->hSliderMinThreshold->blockSignals(true);
	ui->hSliderMaxThreshold->setValue(1000);
	ui->hSliderMinThreshold->setValue(0);
	ui->hSliderMaxThreshold->blockSignals(false);
	ui->hSliderMinThreshold->blockSignals(false);
	updateColorMap();
}

float SceneViewerSettings::getMaxThresholdValue()
{
	return ui->labelMaxThreshold->text().toFloat();
}

float SceneViewerSettings::getMinThresholdValue()
{
	return ui->labelMinThreshold->text().toFloat();
}

//void SceneViewerSettings::resetDispmentScaled(float value)
//{
//	if (preDispmentScaled != -1.0)
//	{
//		return;
//	}
//	preDispmentScaled = value;
//	ui->labelRadiationThreshold->setText(QString::number(value, 'f', 2));
//	ui->hSliderRadiationThreshold->setValue(50);
//}

float SceneViewerSettings::getDispmentScaled()
{
	return ui->labelRadiationThreshold->text().toFloat();
}

void SceneViewerSettings::updateColorMap()
{
	// 获取当前滑动条数值
	int maxRaw = ui->hSliderMaxThreshold->value();
	int minRaw = ui->hSliderMinThreshold->value();

	// 确定触发源
	QObject* trigger = sender();

	// 双向约束逻辑
	if (trigger == ui->hSliderMaxThreshold) {
		// 强制最小滑动条上限不超过当前最大值
		ui->hSliderMinThreshold->setMaximum(maxRaw);

		// 若当前最小值超过新上限，强制同步
		if (minRaw > maxRaw) {
			ui->hSliderMinThreshold->blockSignals(true);
			ui->hSliderMinThreshold->setValue(maxRaw);
			ui->hSliderMinThreshold->blockSignals(false);
			minRaw = maxRaw; // 同步内存值
		}
	}
	else if (trigger == ui->hSliderMinThreshold) {
		// 强制最大滑动条下限不低于当前最小值
		ui->hSliderMaxThreshold->setMinimum(minRaw);

		// 若当前最大值低于新下限，强制同步
		if (maxRaw < minRaw) {
			ui->hSliderMaxThreshold->blockSignals(true);
			ui->hSliderMaxThreshold->setValue(minRaw);
			ui->hSliderMaxThreshold->blockSignals(false);
			maxRaw = minRaw; // 同步内存值
		}
	}

	// 转换为实际物理值（假设滑动条0-1000映射到preMinValue-preMaxValue）
	const float actualMax = preMinValue + (maxRaw / 1000.0f) * (preMaxValue - preMinValue);
	const float actualMin = preMinValue + (minRaw / 1000.0f) * (preMaxValue - preMinValue);
	currentRange = actualMax - actualMin;
	// 更新界面显示（科学计数法保留1位小数）
	ui->labelMaxThreshold->setText(QString::number(actualMax, 'e', 1));
	ui->labelMinThreshold->setText(QString::number(actualMin, 'e', 1));

	emit minmaxThresholdChanged();
	auto stepvalue = (actualMax - actualMin) / 8.0;
	ui->label1->setText(QString::number(stepvalue * 8 + actualMin, 'e', 1));
	ui->label2->setText(QString::number(stepvalue * 7 + actualMin, 'e', 1));
	ui->label3->setText(QString::number(stepvalue * 6 + actualMin, 'e', 1));
	ui->label4->setText(QString::number(stepvalue * 5 + actualMin, 'e', 1));
	ui->label5->setText(QString::number(stepvalue * 4 + actualMin, 'e', 1));
	ui->label6->setText(QString::number(stepvalue * 3 + actualMin, 'e', 1));
	ui->label7->setText(QString::number(stepvalue * 2 + actualMin, 'e', 1));
	ui->label8->setText(QString::number(stepvalue * 1 + actualMin, 'e', 1));
	ui->label9->setText(QString::number(actualMin, 'e', 1));
}
