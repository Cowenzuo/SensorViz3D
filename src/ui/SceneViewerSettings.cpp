#include "SceneViewerSettings.h"

SceneViewerSettings::SceneViewerSettings(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::SceneViewerSettingsClass())
{
	ui->setupUi(this);
}

SceneViewerSettings::~SceneViewerSettings()
{
	delete ui;
}

QString SceneViewerSettings::getCurrentWeight()
{
	return ui->comboBox->currentText();
}
