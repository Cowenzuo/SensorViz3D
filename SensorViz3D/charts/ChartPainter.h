#pragma once
#include <QXlsxQt5/xlsxdocument.h>
#include <QXlsxQt5/xlsxformat.h>

#include "app/ProjectData.h"
#include "ScalableCustomPlot.h"

namespace docx
{
	class Document;
}

class BaseChart
{
public:
	BaseChart(const QString& name, const QString& unit) : _titleRootName(name), _titleUnit(unit) {}
	virtual ~BaseChart() {}

	void processSensorData(const QString& sensorName, double* sensorData, int dataCount, double frequency, QMap<QString, ScalableCustomPlot*>& timeSeriesMap, QMap<QString, ScalableCustomPlot*>& frequencySpectrumMap);

public:
	QMap<QString, ScalableCustomPlot* >_imgTimeSeries{};		//时域过程图
	QMap<QString, ScalableCustomPlot* >_imgFrequencySpectrum{};	//频谱分析图

	QVector<QMap<QString, ScalableCustomPlot*>>_imgSegDataTimeSeries{};			//分段数据时域过程图
	QVector<QMap<QString, ScalableCustomPlot*>>_imgSegDataFrequencySpectrum{};	//分段数据频谱分析图

private:
	QString _titleRootName{ "" };
	QString _titleUnit{ "" };
};

class FPChart : public BaseChart
{
public:
	FPChart(const QString& name, const QString& unit) : BaseChart(name, unit) {}
	virtual ~FPChart() {}

	void setData(const FPData& data);

	void save(docx::Document* doc, const QString& dirpath, int width, int height);
};