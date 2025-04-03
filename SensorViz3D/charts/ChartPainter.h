#pragma once
#include <QWidget>

#include "app/ProjectData.h"
#include "ScalableCustomPlot.h"

class ChartPainter
{
public:
	ChartPainter(const QString& name, const QString& unit) : _titleRootName(name), _titleUnit(unit) {}
	virtual ~ChartPainter();

	void setData(const ExtraData& exdata);
	void save(const QString& dirpath, int width, int height);
	void saveSeg(const QString& dirpath, int width, int height);

	QWidget* getChart(const QString& sensorname, int mode);
	QVector<QWidget*> getSegChart(const QString& sensorname, int mode);

	QString getTiltleRootName() { return _titleRootName; }
	QString getTiltleUnit() { return _titleUnit; }
private:
	void processSensorData(
		const QString& sensorName,
		double* sensorData,
		int dataCount,
		double frequency,
		QMap<QString, ScalableCustomPlot*>& timeSeriesMap,
		QMap<QString, ScalableCustomPlot*>& frequencySpectrumMap
	);

private:
	QMap<QString, ScalableCustomPlot* >_imgTimeSeries{};		//时域过程图
	QMap<QString, ScalableCustomPlot* >_imgFrequencySpectrum{};	//频谱分析图

	QVector<QMap<QString, ScalableCustomPlot*>>_imgSegDataTimeSeries{};			//分段数据时域过程图
	QVector<QMap<QString, ScalableCustomPlot*>>_imgSegDataFrequencySpectrum{};	//分段数据频谱分析图

	QVector<QWidget*>_mixWidgets;//在返回时域和频谱图二合一的时候临时包装器

	QString _titleRootName{ "" };
	QString _titleUnit{ "" };
};

class MagChartPainter {
public:
	static ScalableCustomPlot* paintMagChart(
		const QString& title,
		const QString& xlabel,
		const QString& ylabel,
		const QStringList& wcsnames,
		const QStringList& sensornames,
		const QMap<QString, QVector<double>>& values
	);
};
