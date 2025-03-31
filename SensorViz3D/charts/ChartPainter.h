#pragma once
#include "app/ProjectData.h"
#include "ScalableCustomPlot.h"

class BaseChart
{
public:
	BaseChart(const QString& name, const QString& unit) : _titleRootName(name), _titleUnit(unit) {}
	virtual ~BaseChart() {}

	void processSensorData(const QString& sensorName, double* sensorData, int dataCount, double frequency, QMap<QString, ScalableCustomPlot*>& timeSeriesMap, QMap<QString, ScalableCustomPlot*>& frequencySpectrumMap);

	void setData(const ExtraData& exdata);

	void save(const QString& dirpath, int width, int height);
	void saveSeg(const QString& dirpath, int width, int height);

	QString getTiltleRootName() { return _titleRootName; }
	QString getTiltleUnit() { return _titleUnit; }
public:
	QMap<QString, ScalableCustomPlot* >_imgTimeSeries{};		//时域过程图
	QMap<QString, ScalableCustomPlot* >_imgFrequencySpectrum{};	//频谱分析图

	QVector<QMap<QString, ScalableCustomPlot*>>_imgSegDataTimeSeries{};			//分段数据时域过程图
	QVector<QMap<QString, ScalableCustomPlot*>>_imgSegDataFrequencySpectrum{};	//分段数据频谱分析图

private:
	QString _titleRootName{ "" };
	QString _titleUnit{ "" };
};

class MagChart {
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
