#include "ChartPainter.h"

#include "PSDAnalyzer.h"

void BaseChart::processSensorData(const QString& sensorName, double* sensorData, int dataCount, double frequency, QMap<QString, ScalableCustomPlot*>& timeSeriesMap, QMap<QString, ScalableCustomPlot*>& frequencySpectrumMap)
{
	// 准备Y轴数据
	QVector<double> fluctuation;
	double min = 0, max = 0;
	if (PSDA::preprocessData(sensorData, dataCount, fluctuation, min, max, frequency, 1.96)) {
		// 准备时间轴数据
		QVector<double> xData(fluctuation.count());
		for (int i = 0; i < fluctuation.count(); ++i) {
			xData[i] = i / double(frequency);
		}
		double totalTime = fluctuation.count() / frequency;
		// 创建时域图
		auto tschart = new ScalableCustomPlot();
		tschart->setTitle(QString("%1时域过程 测点#%2").arg(_titleRootName, sensorName));
		tschart->xAxis->setLabel("时间(s)");
		tschart->yAxis->setLabel(QString("脉动压力(%1)").arg(_titleUnit));
		tschart->xAxis->setRange(0, totalTime);
		tschart->yAxis->setRange(min, max);
		tschart->yAxis->rescale(true);
		//tschart->setOpenGl(true);
		auto tsgraph = tschart->addGraph();
		tsgraph->setData(xData, fluctuation);
		tsgraph->setPen(QPen(Qt::black));
		tschart->setSelectableVisible(true);
		tsgraph->setSelectable(QCP::stSingleData);
		tschart->setOriginalRanges();
		tschart->replot();
		timeSeriesMap[sensorName] = tschart;
	}
	else {
		qWarning() << "No valid data for sensor:" << sensorName;
	}
	// 创建频谱图
	QVector<double> freqs;
	QVector<double> pxx;
	PSDA::calculatePowerSpectralDensity(fluctuation.data(), fluctuation.count(), frequency, freqs, pxx);
	double maxY = *std::max_element(pxx.constBegin(), pxx.constEnd());
	auto fschart = new ScalableCustomPlot();
	fschart->setTitle(QString("频谱分析 测点#%1").arg(sensorName));
	fschart->xAxis->setLabel("频率(Hz)");
	fschart->yAxis->setLabel(QString("功率谱密度((%1)²/Hz)").arg(_titleUnit));
	fschart->xAxis->setRange(freqs.first(), freqs.last());
	fschart->yAxis->setRange(0, maxY);
	fschart->yAxis->rescale(true);
	//fschart->setOpenGl(true);
	auto fsgraph = fschart->addGraph();
	fsgraph->setData(freqs, pxx);
	fsgraph->setPen(QPen(Qt::black));
	fsgraph->setSelectable(QCP::stSingleData);
	fschart->setSelectableVisible(true);
	fschart->setOriginalRanges();
	fschart->replot();
	frequencySpectrumMap[sensorName] = fschart;
}

void FPChart::setData(const FPData& data)
{
	// 全局数据
	for (auto iter = data.data.begin(); iter != data.data.end(); ++iter)
	{
		auto sensorName = iter.key();
		auto sensorData = iter.value();
		processSensorData(sensorName, sensorData, data.dataCount, data.frequency, _imgTimeSeries, _imgFrequencySpectrum);
	}

	// 分段数据
	_imgSegDataTimeSeries.resize(data.segData.count());
	_imgSegDataFrequencySpectrum.resize(data.segData.count());
	for (int i = 0; i < data.segData.count(); i++)
	{
		auto segData = data.segData[i];
		for (auto segiter = segData.begin(); segiter != segData.end(); ++segiter)
		{
			auto sensorName = segiter.key();
			auto sensorData = segiter.value();
			processSensorData(sensorName, sensorData, data.dataCountEach, data.frequency, _imgSegDataTimeSeries[i], _imgSegDataFrequencySpectrum[i]);
		}
	}
}

void FPChart::save(const QString& dirpath, int width, int height)
{

	// 时域过程图和频谱分析图在逻辑上一定是一对一的
	// 所以使用其中一个Key即可
	QList<QString> keys = _imgTimeSeries.keys();
	//对keys进行排序，转为数字
	auto numericCompare = [](const QString& a, const QString& b) -> bool {
		bool ok1, ok2;
		int numA = a.toInt(&ok1), numB = b.toInt(&ok2);
		if (ok1 && ok2)
			return numA < numB;
		else if (ok1)
			return true;
		else if (ok2)
			return false;
		else
			return a < b;
		};
	std::sort(keys.begin(), keys.end(), numericCompare);
	for (int i = 0; i < keys.count(); i++)
	{
		QPixmap tspixmap = _imgTimeSeries[keys[i]]->toPixmap(width, height);
		QPixmap fspixmap = _imgFrequencySpectrum[keys[i]]->toPixmap(width, height);
		QString savepathts = QString("%1/测点%2_时域图.png").arg(dirpath, keys[i]);
		QString savepathfs = QString("%1/测点%2_频谱图.png").arg(dirpath, keys[i]);
		tspixmap.save(savepathts);
		fspixmap.save(savepathfs);

		QPixmap combinedPixmap(width, height * 2);
		QPainter painter(&combinedPixmap);
		painter.drawPixmap(0, 0, tspixmap);
		painter.drawPixmap(0, height, fspixmap);
		QString savepath = QString("%1/测点%2.png").arg(dirpath, keys[i]);
		fspixmap.save(savepath);
	}

	// 保存分段数据时域过程图
	for (int i = 0; i < _imgSegDataTimeSeries.size(); i++)
	{
		auto& segData = _imgSegDataTimeSeries[i];
		// 保存时域过程图
		for (auto iter = segData.begin(); iter != segData.end(); ++iter)
		{
			QPixmap pixmap = iter.value()->toPixmap(width, height);
			QString savepath = QString("%1/测点%2_时域图_段%3.png").arg(dirpath, iter.key(), QString::number(i));
			pixmap.save(savepath);
		}
	}
	// 保存分段数据频谱分析图
	for (int i = 0; i < _imgSegDataFrequencySpectrum.size(); i++)
	{
		auto& segData = _imgSegDataFrequencySpectrum[i];
		// 保存时域过程图
		for (auto iter = segData.begin(); iter != segData.end(); ++iter)
		{
			QPixmap pixmap = iter.value()->toPixmap(width, height);
			QString savepath = QString("%1/测点%2_频谱图_段%3.png").arg(dirpath, iter.key(), QString::number(i));
			pixmap.save(savepath);
		}
	}
}
