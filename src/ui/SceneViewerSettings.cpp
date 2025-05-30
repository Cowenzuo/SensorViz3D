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
		ui->labelMaxThreshold->setText(QString::number(newvalue));
		emit maxThresholdChanged(newvalue);
		});

	ui->hSliderRadiationThreshold->setRange(0, 1000);
	connect(ui->hSliderRadiationThreshold, &QSlider::valueChanged, this, [&](int value) {
		if (preRadiationValue <= 0)
			return;
		auto newvalue = value / 1000.0 * preRadiationValue;
		ui->labelRadiationThreshold->setText(QString::number(newvalue));
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
	ui->labelMaxThreshold->setText(QString::number(value));
	ui->hSliderMaxThreshold->setValue(1000);
}

float SceneViewerSettings::getMaxThresholdValue()
{
	return ui->labelMaxThreshold->text().toFloat();
}

void SceneViewerSettings::resetRadiationThresholdValue(float value)
{
	preRadiationValue = value;
	ui->labelRadiationThreshold->setText(QString::number(value));
	ui->hSliderRadiationThreshold->setValue(1000);
}

float SceneViewerSettings::getRadiationThresholdValue()
{
	return ui->labelRadiationThreshold->text().toFloat();
}
