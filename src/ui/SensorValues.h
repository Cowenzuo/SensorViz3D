#pragma once

#include <QWidget>
#include <QStringList>
#include <QVector>
#include "ui_SensorValues.h"

class SensorValues : public QWidget
{
	Q_OBJECT

public:
	SensorValues(QWidget *parent = nullptr);
	~SensorValues();

	void setSensorNames(QStringList names);
	void setSensorValues(QVector<float> values);
private:
	Ui::SensorValuesClass ui;
};
