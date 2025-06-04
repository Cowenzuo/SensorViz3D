#include "SceneViewerSettings.h"

SceneViewerSettings::SceneViewerSettings(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::SceneViewerSettingsClass())
{
	ui->setupUi(this);

	ui->hSliderMaxThreshold->setRange(0, 1000);
	connect(ui->hSliderMaxThreshold, &QSlider::valueChanged, this, [&](int value) {
		if (preMaxValue <= 0)
			return;
		auto newvalue = value / 1000.0 * preMaxValue;
		ui->labelMaxThreshold->setText(QString::number(newvalue, 'e', 1));
		emit maxThresholdChanged(newvalue);
		auto stepvalue = newvalue / 8.0;
		ui->label1->setText(QString::number(stepvalue * 8, 'e', 1));
		ui->label2->setText(QString::number(stepvalue * 7, 'e', 1));
		ui->label3->setText(QString::number(stepvalue * 6, 'e', 1));
		ui->label4->setText(QString::number(stepvalue * 5, 'e', 1));
		ui->label5->setText(QString::number(stepvalue * 4, 'e', 1));
		ui->label6->setText(QString::number(stepvalue * 3, 'e', 1));
		ui->label7->setText(QString::number(stepvalue * 2, 'e', 1));
		ui->label8->setText(QString::number(stepvalue * 1, 'e', 1));
		ui->label9->setText(QString::number(0, 'e', 1));
		});

	ui->hSliderRadiationThreshold->setRange(0, 1000);
	connect(ui->hSliderRadiationThreshold, &QSlider::valueChanged, this, [&](int value) {
		if (preRadiationValue <= 0)
			return;
		auto newvalue = value / 1000.0 * preRadiationValue;
		ui->labelRadiationThreshold->setText(QString::number(newvalue, 'e', 1));
		emit radiationThresholdChanged(newvalue);
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

void SceneViewerSettings::resetMaxThresholdValue(float value)
{
	preMaxValue = value;
	ui->labelMaxThreshold->setText(QString::number(value, 'e', 1));
	ui->hSliderMaxThreshold->setValue(1000);
}

float SceneViewerSettings::getMaxThresholdValue()
{
	return ui->labelMaxThreshold->text().toFloat();
}

void SceneViewerSettings::resetRadiationThresholdValue(float value)
{
	if (preRadiationValue != -1.0)
	{
		return;
	}
	preRadiationValue = value;
	ui->labelRadiationThreshold->setText(QString::number(value, 'e', 1));
	ui->hSliderRadiationThreshold->setValue(1000);
}

float SceneViewerSettings::getRadiationThresholdValue()
{
	return ui->labelRadiationThreshold->text().toFloat();
}
