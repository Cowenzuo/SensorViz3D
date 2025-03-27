#pragma once
#include <QXlsxQt5/xlsxdocument.h>
#include <QXlsxQt5/xlsxformat.h>

#include "app/ProjectData.h"
#include "ScalableCustomPlot.h"


class BaseChart
{
public:
	BaseChart() {}
	virtual ~BaseChart() {}

	QMap<QString, ScalableCustomPlot* >_imgTimeSeries{};		//时域过程图
	QMap<QString, ScalableCustomPlot* >_imgFrequencySpectrum{};	//频谱分析图

	QVector<QMap<QString, ScalableCustomPlot*>>_imgSegDataTimeSeries{};			//分段数据时域过程图
	QVector<QMap<QString, ScalableCustomPlot*>>_imgSegDataFrequencySpectrum{};	//分段数据频谱分析图
};

class FPChart : public BaseChart
{
public:
	FPChart() {}
	virtual ~FPChart() {}

	void setData(const FPData& data);

	void save(const QString& dirpath, int width, int height);
};