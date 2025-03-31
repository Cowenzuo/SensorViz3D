#include "ChartPainter.h"

#include "PSDAnalyzer.h"

BaseChart::~BaseChart()
{
	for (auto it = _imgTimeSeries.begin(); it != _imgTimeSeries.end(); ++it) {
		delete[] it.value();
	}
	_imgTimeSeries.clear();
	for (auto it = _imgFrequencySpectrum.begin(); it != _imgFrequencySpectrum.end(); ++it) {
		delete[] it.value();
	}
	_imgFrequencySpectrum.clear();

	for (auto& segMap : _imgSegDataTimeSeries) {
		for (auto it = segMap.begin(); it != segMap.end(); ++it) {
			delete[] it.value();
		}
		segMap.clear();
	}
	_imgSegDataTimeSeries.clear();
	
	for (auto& segMap : _imgSegDataFrequencySpectrum) {
		for (auto it = segMap.begin(); it != segMap.end(); ++it) {
			delete[] it.value();
		}
		segMap.clear();
	}
	_imgSegDataFrequencySpectrum.clear();
}

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
		tschart->setTitle(QString("%1时域过程 测点P%2").arg(_titleRootName, sensorName));
		tschart->xAxis->setLabel("时间(s)");
		tschart->yAxis->setLabel(QString("%1(%2)").arg(_titleRootName, _titleUnit));
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
	fschart->setTitle(QString("频谱分析 测点P%1").arg(sensorName));
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

void BaseChart::setData(const ExtraData& exdata)
{
	// 全局数据
	for (auto iter = exdata.data.begin(); iter != exdata.data.end(); ++iter)
	{
		auto sensorName = iter.key();
		auto sensorData = iter.value();
		processSensorData(sensorName, sensorData, exdata.dataCount, exdata.frequency, _imgTimeSeries, _imgFrequencySpectrum);
	}

	if (!exdata.hasSegData)
	{
		return;
	}

	// 分段数据
	_imgSegDataTimeSeries.resize(exdata.segData.count());
	_imgSegDataFrequencySpectrum.resize(exdata.segData.count());
	for (int i = 0; i < exdata.segData.count(); i++)
	{
		auto segData = exdata.segData[i];
		for (auto segiter = segData.begin(); segiter != segData.end(); ++segiter)
		{
			auto sensorName = segiter.key();
			auto sensorData = segiter.value();
			processSensorData(sensorName, sensorData, exdata.dataCountEach, exdata.frequency, _imgSegDataTimeSeries[i], _imgSegDataFrequencySpectrum[i]);
		}
	}
}

void BaseChart::save(const QString& dirpath, int width, int height)
{
	// 时域过程图和频谱分析图在逻辑上一定是一对一的
	// 所以使用其中一个Key即可
	QList<QString> keys = _imgTimeSeries.keys();
	std::sort(keys.begin(), keys.end(), &numericCompare);

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
		combinedPixmap.save(savepath);
	}
}

void BaseChart::saveSeg(const QString& dirpath, int width, int height)
{
	auto count = _imgSegDataTimeSeries.size();
	// 保存分段数据图
	for (int i = 0; i < count; i++)
	{
		auto& tssegData = _imgSegDataTimeSeries[i];
		auto& fssegData = _imgSegDataFrequencySpectrum[i];
		QList<QString> keys = tssegData.keys();
		std::sort(keys.begin(), keys.end(), &numericCompare);
		for (int j = 0; j < keys.count(); j++)
		{
			QPixmap tspixmap = _imgSegDataTimeSeries[i][keys[j]]->toPixmap(width, height);
			QPixmap fspixmap = _imgSegDataFrequencySpectrum[i][keys[j]]->toPixmap(width, height);
			QString tssavepath = QString("%1/测点%2_时域图_段%3.png").arg(dirpath, keys[j], QString::number(i));
			QString fssavepath = QString("%1/测点%2_频谱图_段%3.png").arg(dirpath, keys[j], QString::number(i));
			tspixmap.save(tssavepath);
			fspixmap.save(fssavepath);

			QPixmap combinedPixmap(width, height * 2);
			QPainter painter(&combinedPixmap);
			painter.drawPixmap(0, 0, tspixmap);
			painter.drawPixmap(0, height, fspixmap);
			QString savepath = QString("%1/测点%2_段%3.png").arg(dirpath, keys[j], QString::number(i));
			combinedPixmap.save(savepath);
		}

	}

}

ScalableCustomPlot* MagChart::paintMagChart(
	const QString& title,
	const QString& xlabel,
	const QString& ylabel,
	const QStringList& wcsnames,
	const QStringList& sensornames,
	const QMap<QString, QVector<double>>& values)
{
	// 1. 创建图表对象
	ScalableCustomPlot* plot = new ScalableCustomPlot();

	// 2. 设置图表标题
	plot->plotLayout()->insertRow(0);
	QCPTextElement* titleElement = new QCPTextElement(plot);
	titleElement->setText(title);
	plot->plotLayout()->addElement(0, 0, titleElement);

	// 3. 准备横轴数据（使用QSharedPointer管理数据）
	QVector<double> xTicks;
	for (int i = 0; i < wcsnames.size(); ++i) {
		xTicks << i + 1;
	}

	// 4. 设置坐标轴
	plot->xAxis->setLabel(xlabel);
	plot->yAxis->setLabel(ylabel);

	// 正确设置刻度标签的方式
	QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
	for (int i = 0; i < wcsnames.size(); ++i) {
		textTicker->addTick(i + 1, wcsnames[i]);
	}
	plot->xAxis->setTicker(textTicker);
	plot->xAxis->setRange(0, wcsnames.size() + 1);
	plot->xAxis->setTickLabelRotation(60);

	// 5. 添加传感器数据
	QVector<QColor> colors = {
		QColor(31, 119, 180), QColor(214, 39, 40),
		QColor(44, 160, 44), QColor(148, 103, 189),
		QColor(140, 86, 75), QColor(227, 119, 194)
	};

	for (int i = 0; i < sensornames.size(); ++i) {
		const QString& name = sensornames[i];
		if (!values.contains(name)) continue;

		const QVector<double>& yData = values[name];
		if (yData.size() != wcsnames.size()) continue;

		// 创建数据容器
		QVector<double> xData;
		for (int j = 0; j < wcsnames.size(); ++j) {
			xData << j + 1;
		}

		// 添加折线
		QCPGraph* graph = plot->addGraph();
		graph->setName(name);
		graph->setData(xData, yData);

		// 设置样式
		QPen pen(colors[i % colors.size()]);
		pen.setWidth(2);
		graph->setPen(pen);
		graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 6));
	}

	// 6. 调整Y轴范围
	plot->yAxis->rescale();
	double yMin = plot->yAxis->range().lower;
	double yMax = plot->yAxis->range().upper;
	plot->yAxis->setRange(yMin - 0.1 * (yMax - yMin), yMax + 0.1 * (yMax - yMin));

	// 7. 设置图例
	plot->legend->setVisible(true);
	plot->legend->setBrush(QBrush(QColor(255, 255, 255, 230)));
	plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);

	// 8. 启用交互
	plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

	// 9. 刷新绘图
	plot->replot();

	return plot;
}
