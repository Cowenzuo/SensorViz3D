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
	int getCurrentWeightIndex();
	float getCurrentRange();

	void resetMinmaxThresholdValue(float max, float min);

	float getMaxThresholdValue();
	float getMinThresholdValue();

	float getDispmentScaled();
private slots:
	void updateColorMap();
signals:
	void currentWeightChanged(const QString& value);
	void minmaxThresholdChanged();
	void dispmentScaledChanged(float value);

private:
	Ui::SceneViewerSettingsClass* ui;

	float preMaxValue{ -1.0 };
	float preMinValue{ -1.0 };
	float currentRange{ -1.0 };
	float preDispmentScaled{ 1.0 };
};
