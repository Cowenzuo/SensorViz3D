#include "ChartPainter.h"

#include "PSDAnalyzer.h"

void FPChart::setData(const FPData& data)
{
	// 全局数据
	for (auto iter = data.data.begin(); iter != data.data.end(); ++iter)
	{
		auto sensorName = iter.key();
		auto sensorData = iter.value();
		// 准备Y轴数据
		QVector<double> fluctuation;
		double min = 0, max = 0;
		if (PSDA::preprocessData(sensorData, data.dataCount, fluctuation, min, max, data.frequency, 1.96)) {
			// 准备时间轴数据
			QVector<double> xData(fluctuation.count());
			for (int i = 0; i < fluctuation.count(); ++i) {
				xData[i] = i / double(data.frequency);
			}
			double totalTime = fluctuation.count() / data.frequency;
			// 创建时域图
			auto tschart = new ScalableCustomPlot();
			tschart->setTitle(QString("脉动压力时域过程 #%1").arg(sensorName));
			tschart->xAxis->setLabel("时间(s)");
			tschart->yAxis->setLabel("脉动压力(kPa)");
			tschart->xAxis->setRange(0, totalTime);
			tschart->yAxis->setRange(min, max);
			tschart->yAxis->rescale(true);
			tschart->setDataSelectable(true);
			tschart->setOpenGl(true);
			tschart->addGraph();
			tschart->graph(0)->setData(xData, fluctuation);
			tschart->graph(0)->setPen(QPen(Qt::black));
			tschart->graph(0)->setSelectable(QCP::stSingleData);

			tschart->setOriginalRanges();
			tschart->replot();
			_imgTimeSeries[sensorName] = tschart;

			// 创建频谱图
			QVector<double> freqs;
			QVector<double> pxx;
			PSDA::calculatePowerSpectralDensity(fluctuation.data(), fluctuation.count(), data.frequency, freqs, pxx);
			//PSDA::calculatePSD(fluctuation, data.frequency, freqs, psd, psddb, true);
			double maxY = *std::max_element(pxx.constBegin(), pxx.constEnd());
			auto fschart = new ScalableCustomPlot();
			fschart->setTitle(QString("频谱分析 #%1").arg(sensorName));
			fschart->xAxis->setLabel("频率(Hz)");
			fschart->yAxis->setLabel("功率谱密度(kPa²/Hz)");
			fschart->xAxis->setRange(0, data.frequency * 0.2);
			fschart->yAxis->setRange(0, maxY);
			fschart->yAxis->rescale(true);
			fschart->setDataSelectable(true);
			fschart->setOpenGl(true);
			fschart->addGraph();
			fschart->graph(0)->setData(freqs, pxx);
			fschart->graph(0)->setPen(QPen(Qt::black));
			fschart->setOriginalRanges();
			fschart->replot();
			_imgFrequencySpectrum[sensorName] = fschart;
		}
		else {
			qWarning() << "No valid data for sensor:" << sensorName;
		}

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
			// 准备Y轴数据
			QVector<double> fluctuation;
			double min = 0, max = 0;
			if (PSDA::preprocessData(sensorData, data.dataCountEach, fluctuation, min, max, data.frequency, 1.96)) {
				// 准备时间轴数据
				QVector<double> xData(fluctuation.count());
				for (int i = 0; i < fluctuation.count(); ++i) {
					xData[i] = i / data.frequency;
				}
				double totalTime = fluctuation.count() / data.frequency;
				// 创建时域图
				auto tschart = new ScalableCustomPlot();
				tschart->setTitle(QString("脉动压力时域过程 #%1").arg(sensorName));
				tschart->xAxis->setLabel("时间(s)");
				tschart->yAxis->setLabel("脉动压力(kPa)");
				tschart->xAxis->setRange(0, totalTime);
				tschart->yAxis->setRange(min, max);
				tschart->yAxis->rescale(true);
				tschart->setDataSelectable(true);
				tschart->setOpenGl(true);
				tschart->addGraph();
				tschart->graph(0)->setData(xData, fluctuation);
				tschart->graph(0)->setPen(QPen(Qt::black));
				tschart->graph(0)->setSelectable(QCP::stSingleData);

				tschart->setOriginalRanges();
				tschart->replot();
				_imgSegDataTimeSeries[i][sensorName] = tschart;

				// 创建频谱图
				QVector<double> freqs;
				QVector<double> pxx;
				PSDA::calculatePowerSpectralDensity(fluctuation.data(), fluctuation.count(), data.frequency, freqs, pxx);
				double maxY = *std::max_element(pxx.constBegin(), pxx.constEnd());
				auto fschart = new ScalableCustomPlot();
				fschart->setTitle(QString("频谱分析 #%1").arg(sensorName));
				fschart->xAxis->setLabel("频率(Hz)");
				fschart->yAxis->setLabel("功率谱密度(kPa²/Hz)");
				fschart->xAxis->setRange(0, data.frequency * 0.2);
				fschart->yAxis->setRange(0, maxY * 1.5);
				fschart->yAxis->rescale(true);
				fschart->setDataSelectable(true);
				fschart->setOpenGl(true);
				fschart->addGraph();
				fschart->graph(0)->setData(freqs, pxx);
				fschart->graph(0)->setPen(QPen(Qt::black));
				fschart->setOriginalRanges();
				fschart->replot();
				_imgSegDataFrequencySpectrum[i][sensorName] = fschart;
			}
			else {
				qWarning() << "No valid data for sensor:" << sensorName;
			}
		}
	}
}

void FPChart::save(const QString& dirpath, int width, int height)
{
	// 保存时域过程图
	for (auto iter = _imgTimeSeries.begin(); iter != _imgTimeSeries.end(); ++iter)
	{
		QPixmap pixmap = iter.value()->toPixmap(width, height);
		QString savepath = QString("%1/测点%2_时域图.png").arg(dirpath, iter.key());
		pixmap.save(savepath);
	}
	// 保存频谱分析图,附带加一个融合图
	for (auto iter = _imgFrequencySpectrum.begin(); iter != _imgFrequencySpectrum.end(); ++iter)
	{
		QPixmap pixmap = iter.value()->toPixmap(width, height);
		QString savepath = QString("%1/测点%2_频谱图.png").arg(dirpath, iter.key());
		pixmap.save(savepath);

		//auto tspixmap = _imgTimeSeries[iter.key()]->toPixmap(width, height);
		//QPixmap combinedPixmap(width, height * 2);
		//QPainter painter(&combinedPixmap);
		//painter.drawPixmap(0, 0, tspixmap);
		//painter.drawPixmap(0, height, pixmap);
	}

	// 保存分段数据时域过程图
	for (int i = 0; i < _imgSegDataTimeSeries.size(); i++)
	{
		auto& segData = _imgSegDataTimeSeries[i];
		// 保存时域过程图
		for (auto iter = segData.begin(); iter != segData.end(); ++iter)
		{
			QPixmap pixmap = iter.value()->toPixmap(width, height);
			QString savepath = QString("%1/段%2_测点%3_时域图.png").arg(dirpath, QString::number(i), iter.key());
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
			QString savepath = QString("%1/段%2_测点%3_频谱图.png").arg(dirpath, QString::number(i), iter.key());
			pixmap.save(savepath);
		}
	}
}
