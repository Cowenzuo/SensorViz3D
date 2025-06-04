#include "SensorValues.h"

SensorValues::SensorValues(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

SensorValues::~SensorValues()
{
}

void SensorValues::setSensorNames(QStringList names)
{
	ui.tableWidget->setRowCount(names.size());
	for (auto i = 0;i < names.size(); ++i)
	{
		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(names[i]));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(0.0, 'e')));
	}
}

void SensorValues::setSensorValues(QVector<float> values)
{
	auto rc = ui.tableWidget->rowCount();
	if (rc < values.count())
	{
		return;
	}
	for (auto i = 0;i < values.size(); ++i)
	{
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(values[i], 'e')));
	}
}
