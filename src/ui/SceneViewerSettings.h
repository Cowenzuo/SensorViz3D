#pragma once

#include <QWidget>
#include "ui_SceneViewerSettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SceneViewerSettingsClass; };
QT_END_NAMESPACE

class SceneViewerSettings : public QWidget
{
	Q_OBJECT

public:
	SceneViewerSettings(QWidget* parent = nullptr);
	~SceneViewerSettings();

	QString getCurrentWeight();

	void resetMaxThresholdValue(float max);

	float getMaxThresholdValue();

	void resetRadiationThresholdValue(float max);

	float getRadiationThresholdValue();

signals:
	void currentWeightChanged(const QString& value);
	void maxThresholdChanged(float value);
	void radiationThresholdChanged(float value);

private:
	Ui::SceneViewerSettingsClass* ui;

	float preMaxValue{ -1.0 };
	float preRadiationValue{ -1.0 };
};
