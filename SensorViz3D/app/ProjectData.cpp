#include "ProjectData.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QVariant>

#include "rwmat/ReadWriteMatFile.h"
#include "charts/ChartPainter.h"


ProjectData::ProjectData(QObject* parent)
	: QObject(parent)
{
}

ProjectData::~ProjectData()
{
}

bool ProjectData::loadDataPackage(const QString& dirPath)
{
	// Check
	QDir rootDir(dirPath);
	if (!rootDir.exists())
	{
		qDebug() << "Directory not exists: " << dirPath;
		return false;
	}

	// 获取所有子文件夹
	QStringList folders = rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	// 匹配“工况列表”子文件夹并加载工况
	QString workingConditionsFolder;
	for (const QString& folder : folders)
	{
		if (folder.contains("工况列表"))
		{
			workingConditionsFolder = folder;
			break;
		}
	}
	if (workingConditionsFolder.isEmpty())
	{
		qDebug() << "No working conditions folder found.";
		return false;
	}
	if (!loadWorkingConditions(rootDir.filePath(workingConditionsFolder)))
	{
		qDebug() << "Failed to load working conditions.";
		return false;
	}

	// 匹配“脉动压力”子文件夹并加载脉动压力
	QString fluctuationPressureFolder;
	for (const QString& folder : folders)
	{
		if (folder.contains("脉动压力"))
		{
			fluctuationPressureFolder = folder;
			break;
		}
	}
	if (fluctuationPressureFolder.isEmpty())
	{
		qDebug() << "No fluctuation pressure folder found.";
		return false;
	}
	if (!loadFluctuationPressure(rootDir.filePath(fluctuationPressureFolder)))
	{
		qDebug() << "Failed to load fluctuation pressure.";
		return false;
	}

	// 保存根目录信息
	_rootName = QFileInfo(dirPath).baseName();
	_rootDirPath = QDir(dirPath).absolutePath();
	return true;
}

// Python 异常检查宏
#define PY_CHECK() if (PyErr_Occurred()) { PyErr_Print(); return false; }

bool ProjectData::save(const QString& dirPath)
{
	QDir exportRootDir(dirPath + "/export");
	if (!exportRootDir.exists())
	{
		exportRootDir.mkpath(".");
	}

	auto filepath = QString("%1/export/%2.docx").arg(dirPath, _rootName);

	initWordDocment();
	saveWorkingConditionsToDocx();

	saveFluctuationPressureToDocx();
	//// 遍历_fpCharts调用save
	//for (auto iter = _fpCharts.begin(); iter != _fpCharts.end(); ++iter)
	//{
	//	//构建一个文档对象
	//	auto savedir = exportRootDir.absolutePath() + "/" + iter.key() + "/脉动压力";
	//	QDir savedirpath(savedir);
	//	if (!savedirpath.exists())
	//	{
	//		savedirpath.mkpath(".");
	//	}
	//	iter.value()->save(savedir, 450, 170);
	//	//iter.value()->save(doc, savedir, 450, 170);
	//}
	//如果filepath文件存在，则先删除
	QFile docFile(filepath);
	if (docFile.exists()) {
		if (!docFile.remove()) {
			qDebug() << "Failed to remove existing file: " << filepath;
			return false;
		}
	}
	_wordDocument->dynamicCall("SaveAs(const QString&)", QFileInfo(docFile).absoluteFilePath());
	_wordDocument->dynamicCall("Close()");
	_wordWriter->dynamicCall("Quit()");

	delete _wordWriter;

	return true;
}

bool ProjectData::loadWorkingConditions(const QString& dirPath)
{
	_workingConditions.clear();
	QDir wcDir(dirPath);
	QStringList txtFiles = wcDir.entryList(QStringList() << "*.txt", QDir::Files);
	foreach(const QString & txtFile, txtFiles) {
		QFile file(wcDir.absoluteFilePath(txtFile));
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qDebug() << "Failed to open file: " << file.fileName();
			return false;
		}
		WorkingConditions wc;
		wc.name = QFileInfo(file).baseName();
		QTextStream in(&file);
		in.setCodec("utf-8");
		QStringList lines;

		// 一次性读取所有行
		for (int i = 0; i < WORKING_CONDITIONS_LINE_COUNT; ++i) {
			if (in.atEnd()) {
				qWarning() << "Error: Working condition file has only" << i << "lines, expected" << WORKING_CONDITIONS_LINE_COUNT;
				return false;
			}
			lines.append(in.readLine());
		}

		// 使用临时变量存储转换结果，便于错误检查
		bool conversionOk = false;

		// 逐行解析
		for (int line = 0; line < lines.size(); ++line) {
			const QString& lineStr = lines[line];

			try {
				switch (line) {
				case 0:
					wc.description = lineStr.trimmed();
					break;

				case 1: {
					int type = lineStr.toInt(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid type format at line" << line + 1;
						return false;
					}
					wc.type = type;
					break;
				}

				case 2: {
					double value = lineStr.toDouble(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid upWaterLevelStart format at line" << line + 1;
						return false;
					}
					wc.upWaterLevelStart = value;
					break;
				}

				case 3: {
					double value = lineStr.toDouble(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid upWaterLevelEnd format at line" << line + 1;
						return false;
					}
					wc.upWaterLevelEnd = value;
					break;
				}

				case 4: {
					double value = lineStr.toDouble(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid downWaterLevelStart format at line" << line + 1;
						return false;
					}
					wc.downWaterLevelStart = value;
					break;
				}

				case 5: {
					double value = lineStr.toDouble(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid downWaterLevelEnd format at line" << line + 1;
						return false;
					}
					wc.downWaterLevelEnd = value;
					break;
				}

				case 6: {
					double value = lineStr.toDouble(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid gateOpenStart format at line" << line + 1;
						return false;
					}
					wc.gateOpenStart = value;
					break;
				}

				case 7: {
					double value = lineStr.toDouble(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid gateOpenEnd format at line" << line + 1;
						return false;
					}
					wc.gateOpenEnd = value;
					break;
				}

				case 8: {
					double value = lineStr.toDouble(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid pistonOpenStart format at line" << line + 1;
						return false;
					}
					wc.pistonOpenStart = value;
					break;
				}

				case 9: {
					double value = lineStr.toDouble(&conversionOk);
					if (!conversionOk) {
						qWarning() << "Error: Invalid pistonOpenEnd format at line" << line + 1;
						return false;
					}
					wc.pistonOpenEnd = value;
					break;
				}

				default:
					// 正常情况下不会执行到这里
					qWarning() << "Error: Unexpected line number" << line;
					return false;
				}
			}
			catch (const std::exception& e) {
				qWarning() << "Error parsing line" << line + 1 << ":" << e.what();
				return false;
			}
		}

		// 添加业务逻辑验证
		if (wc.upWaterLevelStart < 0 || wc.upWaterLevelEnd < 0 ||
			wc.downWaterLevelStart < 0 || wc.downWaterLevelEnd < 0) {
			qWarning() << "Error: Water level values cannot be negative";
			return false;
		}

		if (wc.gateOpenStart < 0 || wc.gateOpenEnd < 0 ||
			wc.pistonOpenStart < 0 || wc.pistonOpenEnd < 0) {
			qWarning() << "Error: Open values cannot be negative";
			return false;
		}

		_workingConditions[wc.name] = wc;
		file.close();
	}
	return true;
}

bool ProjectData::loadFluctuationPressure(const QString& dirPath)
{
	//固定读取当前文件夹下的一个名为settings文件，内容为传感器名称
	QFile file(dirPath + "/settings");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Failed to open file: " << file.fileName();
		return false;
	}
	QTextStream in(&file);
	in.setCodec("utf-8");
	QString line0 = in.readLine();
	QStringList sensorNames = line0.split(",");
	QString line1 = in.readLine();
	QStringList sensorValid = line1.split(",");
	QString line2 = in.readLine();
	QStringList segwcnames = line2.split(",");
	file.close();


	QDir fpDir(dirPath);
	QStringList matFiles = fpDir.entryList(QStringList() << "*.mat", QDir::Files);
	foreach(const QString & matFile, matFiles) {
		//通过文件名在工况列表中查找对应的工况
		QString wcName = QFileInfo(matFile).baseName();
		if (!_workingConditions.contains(wcName) /*|| _workingConditions[wcName].type == 0*/) {
			continue;
		}
		qDebug() << "Loading mat file: " << matFile;
		FPData fp;
		fp.wcname = wcName;
		if (!RWMAT::readMatFile(fpDir.filePath(matFile), sensorNames, fp))
		{
			qWarning() << "Failed to load mat file: " << matFile;
			continue;
		}

		const int segmentSize = fp.dataCount / SEGMENT_COUNT;
		if (segmentSize < 1) {
			qWarning() << "Data count too small for segmentation in file:" << matFile;
			continue;
		}
		const int offset = qMax(0, (segmentSize / 2) - 1);  // 确保offset非负
		fp.dataCountEach = offset;

		for (int i = 0; i < SEGMENT_COUNT - 1; i++)
		{
			QMap<QString, double*>segData{};
			QMap<QString, Statistics>segStatistics{};
			for (auto& sensorName : sensorNames)
			{
				double* data = new double[segmentSize];
				for (int j = 0; j < segmentSize; j++)
				{
					data[j] = fp.data[sensorName][i * segmentSize + j + offset];
					//统计最大值、最小值、均方根
					if (j == 0) {
						segStatistics[sensorName].max = data[j];
						segStatistics[sensorName].min = data[j];
						segStatistics[sensorName].rms = data[j] * data[j];
					}
					else {
						segStatistics[sensorName].max = qMax(segStatistics[sensorName].max, data[j]);
						segStatistics[sensorName].min = qMin(segStatistics[sensorName].min, data[j]);
						segStatistics[sensorName].rms += data[j] * data[j];
					}
				}
				segStatistics[sensorName].rms = sqrt(segStatistics[sensorName].rms / segmentSize);
				segData[sensorName] = data;
			}
			fp.segData.append(segData);
			fp.segStatistics.append(segStatistics);
		}
		// 生成脉动压力时序频谱图
		FPChart* chart = new FPChart("脉动压力", "kPa");
		chart->setData(fp);
		_fpCharts[wcName] = chart;
		_fpData[wcName] = fp;
	}

	return true;
}

bool ProjectData::initWordDocment()
{
	_wordWriter = new QAxObject("Word.Application");
	if (!_wordWriter) {
		qWarning() << "Failed to initialize Word application";
		return false;
	}
	// 设置Word不可见（静默操作）
	_wordWriter->setProperty("Visible", false);
	// 获取文档集合
	QAxObject* documents = _wordWriter->querySubObject("Documents");
	if (!documents) {
		_wordWriter->dynamicCall("Quit()");
		delete _wordWriter;
		return false;
	}
	// 添加新文档
	_wordDocument = documents->querySubObject("Add()");
	if (!_wordDocument) {
		_wordWriter->dynamicCall("Quit()");
		delete _wordWriter;
		return false;
	}
	// 获取选区对象（即光标）
	_wordSelection = _wordWriter->querySubObject("Selection");
	if (!_wordSelection) {
		_wordDocument->dynamicCall("Close()");
		_wordWriter->dynamicCall("Quit()");
		delete _wordWriter;
		return false;
	}
	return true;
}

void ProjectData::setNormalSelectionStyle(ParagraphFormat pf)
{
	QScopedPointer<QAxObject> paragraphFormat(_wordSelection->querySubObject("ParagraphFormat"));
	QScopedPointer<QAxObject> font(_wordSelection->querySubObject("Font"));

	switch (pf) {
	case ParagraphFormat::TextBody: {
		// 正文样式
		font->setProperty("Name", "宋体");
		font->setProperty("Size", 12);
		font->setProperty("Bold", false);
		paragraphFormat->setProperty("FirstLineIndent", 24);  // 首行缩进2字符（约24磅）
		paragraphFormat->setProperty("LineSpacingRule", 5);   // 1.5倍行距（wdLineSpace1pt5）
		paragraphFormat->setProperty("Alignment", 0);
		paragraphFormat->setProperty("SpaceAfter", 0);
		break;
	}
	case ParagraphFormat::ChartCaption: {
		// 图表上下标
		font->setProperty("Name", "宋体");
		font->setProperty("Size", 10);
		font->setProperty("Bold", false);
		paragraphFormat->setProperty("FirstLineIndent", 0);
		paragraphFormat->setProperty("LineSpacingRule", 0);
		paragraphFormat->setProperty("Alignment", 1);
		paragraphFormat->setProperty("SpaceAfter", 0);
		// 添加标号（需配合文档自动编号或手动添加）
		// _wordSelection->dynamicCall("TypeText(const QString&)", "图1：");
		break;
	}
	case ParagraphFormat::Level1Heading: {
		// 1级标题
		font->setProperty("Name", "黑体");
		font->setProperty("Size", 24);
		font->setProperty("Bold", true);
		paragraphFormat->setProperty("FirstLineIndent", 0);
		paragraphFormat->setProperty("LineSpacingRule", 5);
		paragraphFormat->setProperty("SpaceAfter", 10);  // 段后10磅
		paragraphFormat->setProperty("Alignment", 0);
		//applyListTemplate(1); // 级别1
		break;
	}
	case ParagraphFormat::Level2Heading: {
		// 2级标题
		font->setProperty("Name", "黑体");
		font->setProperty("Size", 18);
		font->setProperty("Bold", true);
		paragraphFormat->setProperty("FirstLineIndent", 0);
		paragraphFormat->setProperty("LineSpacingRule", 5);
		paragraphFormat->setProperty("SpaceAfter", 8);   // 段后8磅
		paragraphFormat->setProperty("Alignment", 0);
		//applyListTemplate(2); // 级别2
		break;
	}
	case ParagraphFormat::Level3Heading: {
		// 3级标题
		font->setProperty("Name", "黑体");
		font->setProperty("Size", 16);
		font->setProperty("Bold", false);
		paragraphFormat->setProperty("FirstLineIndent", 0);
		paragraphFormat->setProperty("LineSpacingRule", 0);
		paragraphFormat->setProperty("SpaceAfter", 0);
		paragraphFormat->setProperty("Alignment", 0);
		//applyListTemplate(3); // 级别3
		break;
	}
	}
}

void ProjectData::addCaption(const QString& text, bool isTable)
{
	_wordSelection->dynamicCall("TypeText(const QString&)", isTable ? "表" : "图");

	QAxObject* fields = _wordSelection->querySubObject("Fields");
	QAxObject* range1 = _wordSelection->querySubObject("Range");
	fields->dynamicCall("Add(QAxObject*,QVariant, QVariant, QVariant)",
		range1->asVariant(),
		"12",  // wdFieldSequence
		isTable ? "table \\# \"0.\"" : "figure \\# \"0.\"",
		true
	);
	fields->dynamicCall("Update()");

	_wordSelection->dynamicCall("TypeText(const QString&)", text);
	_wordSelection->dynamicCall("TypeParagraph()");
}

bool ProjectData::saveWorkingConditionsToDocx()
{
	//章节标题
	setNormalSelectionStyle(ParagraphFormat::Level1Heading);
	_wordSelection->dynamicCall("TypeText(const QString&)", "一、 工况");
	_wordSelection->dynamicCall("TypeParagraph()");

	// 创建表格
	setNormalSelectionStyle(ParagraphFormat::ChartCaption);
	addCaption("工况列表", true);
	int wcsize = _workingConditions.count();
	int columns = 7; // 对应header0的大小
	QAxObject* tables = _wordDocument->querySubObject("Tables");
	QAxObject* table = tables->querySubObject(
		"Add(Range*, int, int,QVariant,QVariant)"
		, _wordSelection->querySubObject("Range")->asVariant(), wcsize + 2, columns, "1", "2");
	//table->dynamicCall("AutoFitBehavior(QVariant)", "2");
	//table->querySubObject("Range")->querySubObject("ParagraphFormat")->setProperty("Alignment", 1);

	// 填充表头
	QStringList header0{ "序号", "名称","描述","闸门开度","","活塞杆开度","" };
	//setNormalSelectionStyle(ParagraphFormat::TableHeader);
	for (int i = 0; i < header0.size(); i++) {
		QScopedPointer<QAxObject> cell(table->querySubObject("Cell(int, int)", 1, i + 1));
		QScopedPointer<QAxObject> range(cell->querySubObject("Range"));
		range->dynamicCall("SetText(const QString&)", header0[i]);
		QScopedPointer<QAxObject> font(range->querySubObject("Font"));
		font->setProperty("Name", "宋体");
		font->setProperty("Size", 10);
		font->setProperty("Bold", true);
		range->querySubObject("ParagraphFormat")->setProperty("Alignment", 1);
		cell->setProperty("VerticalAlignment", 1); // 垂直居中
	}

	// 填充第二行表头
	QStringList header1{ "", "","","起始","终止","起始","终止" };
	for (int i = 0; i < header1.size(); i++) {
		QScopedPointer<QAxObject> cell(table->querySubObject("Cell(int, int)", 2, i + 1));
		QScopedPointer<QAxObject> range(cell->querySubObject("Range"));
		range->dynamicCall("SetText(const QString&)", header1[i]);
		QScopedPointer<QAxObject> font(range->querySubObject("Font"));
		font->setProperty("Name", "宋体");
		font->setProperty("Size", 10);
		font->setProperty("Bold", true);
		range->querySubObject("ParagraphFormat")->setProperty("Alignment", 1);
		cell->setProperty("VerticalAlignment", 1); // 垂直居中
	}

	// 执行合并
	// 行合并不会减少序列，但是列合并，会直接少列序号!!!
	mergeCells(table, 1, 1, 2, 1); // 合并序号列
	mergeCells(table, 1, 2, 2, 2); // 合并名称列
	mergeCells(table, 1, 3, 2, 3); // 合并描述列
	mergeCells(table, 1, 4, 1, 5); // 合并闸门开度标题
	mergeCells(table, 1, 5, 1, 6); // 合并活塞杆开度标题（不是6、7的原因是前面合并单元格了）

	// 对keys进行排序并填充数据行
	QList<QString> keys = _workingConditions.keys();
	std::sort(keys.begin(), keys.end(), &numericCompare);

	for (int i = 0; i < wcsize; i++) {
		auto wc = _workingConditions[keys[i]];
		int row = i + 3; // Word表格行从1开始，前两行是表头
		// 填充各列数据
		fillTableDataCell(table, row, 1, QString::number(i + 1), true);
		fillTableDataCell(table, row, 2, "工况-" + wc.name, true);
		fillTableDataCell(table, row, 3, wc.description, false);
		fillTableDataCell(table, row, 4, QString::number(wc.gateOpenStart, 'f', 2), true);
		fillTableDataCell(table, row, 5, QString::number(wc.gateOpenEnd, 'f', 2), true);
		fillTableDataCell(table, row, 6, QString::number((int)wc.pistonOpenStart), true);
		fillTableDataCell(table, row, 7, QString::number((int)wc.pistonOpenEnd), true);
	}
	skipTable();
	return true;
}

bool ProjectData::saveFluctuationPressureToDocx()
{
	//章节标题
	setNormalSelectionStyle(ParagraphFormat::Level1Heading);
	_wordSelection->dynamicCall("TypeText(const QString&)", "二、 脉动压力");
	_wordSelection->dynamicCall("TypeParagraph()");
	setNormalSelectionStyle(ParagraphFormat::Level2Heading);
	_wordSelection->dynamicCall("TypeText(const QString&)", "1. 全过程时域频谱分析");
	_wordSelection->dynamicCall("TypeParagraph()");
	//表格标题
	setNormalSelectionStyle(ParagraphFormat::ChartCaption);
	addCaption("脉动压力特征值", true);
	WorkingConditionsList wcs;
	QStringList sensorsName;
	QStringList fpwcs = _fpData.keys();
	std::sort(fpwcs.begin(), fpwcs.end(), &numericCompare);
	for (int i = 0; i < fpwcs.count(); i++) {
		wcs.push_back(_workingConditions[fpwcs[i]]);
		if (0 == i)
		{
			const auto& fps = _fpData[fpwcs[i]].statistics;
			sensorsName = fps.keys();
			std::sort(sensorsName.begin(), sensorsName.end(), &numericCompare);
		}
	}
	auto table = createEigenvalueTable(wcs, sensorsName);
	for (int i = 0; i < fpwcs.count(); i++)
	{
		const auto& fps = _fpData[fpwcs[i]].statistics;
		for (int j = 0; j < sensorsName.count(); j++)
		{
			auto sensorname = sensorsName[j];
			auto stats = fps[sensorname];
			fillTableDataCell(table, 3 + i * 3 + 0, 3 + j, QString::number(stats.max, 'f', 2), true);
			fillTableDataCell(table, 3 + i * 3 + 1, 3 + j, QString::number(stats.min, 'f', 2), true);
			fillTableDataCell(table, 3 + i * 3 + 2, 3 + j, QString::number(stats.rms, 'f', 2), true);
		}
	}
	skipTable();

	for (int i = 0; i < fpwcs.count(); i++)
	{
		const auto& wcname = _workingConditions[fpwcs[i]].name;
		const auto& wcdesp = _workingConditions[fpwcs[i]].description;
		setNormalSelectionStyle(ParagraphFormat::Level3Heading);
		_wordSelection->dynamicCall("TypeText(const QString&)", QString("(%1) %2").arg(QString::number(i + 1), wcdesp));
		_wordSelection->dynamicCall("TypeParagraph()");

		QDir exportRootDir(_rootDirPath + QString("/export/%1/脉动压力").arg(wcname));
		if (!exportRootDir.exists())
			exportRootDir.mkpath(".");
		auto exportRootPath = exportRootDir.absolutePath();
		_fpCharts[fpwcs[i]]->save(exportRootPath, 450, 170);
		_fpCharts[fpwcs[i]]->saveSeg(exportRootPath, 450, 170);

		setNormalSelectionStyle(ParagraphFormat::ChartCaption);
		for (size_t j = 0; j < sensorsName.count(); j++)
		{
			QString savepathts = QString("%1/测点%2_时域图.png").arg(exportRootPath, sensorsName[j]);
			insertImage(savepathts, 450, 170);
			setNormalSelectionStyle(ParagraphFormat::ChartCaption);
			addCaption("时域变化-P" + sensorsName[j], false);

			QString savepathfs = QString("%1/测点%2_频谱图.png").arg(exportRootPath, sensorsName[j]);
			insertImage(savepathfs, 450, 170);
			setNormalSelectionStyle(ParagraphFormat::ChartCaption);
			addCaption("频谱分析-P" + sensorsName[j], false);
		}
	}

	setNormalSelectionStyle(ParagraphFormat::Level2Heading);
	_wordSelection->dynamicCall("TypeText(const QString&)", "2. 分段时域频谱分析");
	_wordSelection->dynamicCall("TypeParagraph()");

	for (int i = 0; i < wcs.count(); i++)
	{
		auto wcdsp = wcs[i].description;
		QDir exportRootDir(_rootDirPath + QString("/export/%1/脉动压力").arg(wcs[i].name));
		if (!exportRootDir.exists())
			exportRootDir.mkpath(".");
		auto exportRootPath = exportRootDir.absolutePath();

		setNormalSelectionStyle(ParagraphFormat::Level3Heading);
		_wordSelection->dynamicCall("TypeText(const QString&)", QString("(%1) %2").arg(QString::number(i + 1), wcdsp));
		_wordSelection->dynamicCall("TypeParagraph()");
		_wordSelection->dynamicCall("TypeParagraph()");

		setNormalSelectionStyle(ParagraphFormat::ChartCaption);
		addCaption("脉动压力特征值-" + wcdsp, true);
		QStringList wcsSeg;
		auto segtable = createSegEigenvalueTable(wcs[i], wcsSeg, sensorsName);
		QMap<QString, QVector<double>> sensorMaxValue;
		QMap<QString, QVector<double>> sensorMinValue;
		QMap<QString, QVector<double>> sensorRmsValue;
		const auto& fpsegs = _fpData[wcs[i].name].segStatistics;
		for (int j = 0; j < sensorsName.count(); j++)
		{
			auto sensorname = sensorsName[j];
			for (auto k = 0; k < fpsegs.count(); k++)
			{
				auto stats = fpsegs[k][sensorname];
				fillTableDataCell(segtable, 3 + k * 3 + 0, 3 + j, QString::number(stats.max, 'f', 2), true);
				fillTableDataCell(segtable, 3 + k * 3 + 1, 3 + j, QString::number(stats.min, 'f', 2), true);
				fillTableDataCell(segtable, 3 + k * 3 + 2, 3 + j, QString::number(stats.rms, 'f', 2), true);
				sensorMaxValue[sensorname].push_back(stats.max);
				sensorMinValue[sensorname].push_back(stats.min);
				sensorRmsValue[sensorname].push_back(stats.rms);
			}
		}
		skipTable();

		auto chartMax = MagChart::paintMagChart("脉动压力最大值对比分析", "闸门开度", "脉动压力(kPA)", wcsSeg, sensorsName, sensorMaxValue);
		auto chartMin = MagChart::paintMagChart("脉动压力最小值对比分析", "闸门开度", "脉动压力(kPA)", wcsSeg, sensorsName, sensorMinValue);
		auto chartRms = MagChart::paintMagChart("脉动压力均方根对比分析", "闸门开度", "脉动压力(kPA)", wcsSeg, sensorsName, sensorRmsValue);
		auto chartMaxSavePath = QString("%1/工况%2_最大值对比.png").arg(exportRootPath, wcs[i].name);
		auto chartMinSavePath = QString("%1/工况%2_最小值对比.png").arg(exportRootPath, wcs[i].name);
		auto chartRmsSavePath = QString("%1/工况%2_均方根对比.png").arg(exportRootPath, wcs[i].name);
		chartMax->toPixmap(530, 400).save(chartMaxSavePath);
		chartMin->toPixmap(530, 400).save(chartMinSavePath);
		chartRms->toPixmap(530, 400).save(chartRmsSavePath);

		insertImage(chartMaxSavePath, 530, 400);
		setNormalSelectionStyle(ParagraphFormat::ChartCaption);
		addCaption("脉动压力最大值对比分析", false);

		insertImage(chartMinSavePath, 530, 400);
		setNormalSelectionStyle(ParagraphFormat::ChartCaption);
		addCaption("脉动压力最小值对比分析", false);

		insertImage(chartRmsSavePath, 530, 400);
		setNormalSelectionStyle(ParagraphFormat::ChartCaption);
		addCaption("脉动压力均方根对比分析", false);

		for (size_t j = 0; j < wcsSeg.count(); j++)
		{
			setNormalSelectionStyle(ParagraphFormat::Level3Heading);
			_wordSelection->dynamicCall("TypeText(const QString&)", QString("%1) 闸门开度%2").arg(QString::number(j + 1), wcsSeg[j]));
			_wordSelection->dynamicCall("TypeParagraph()");

			setNormalSelectionStyle(ParagraphFormat::ChartCaption);
			for (size_t k = 0; k < sensorsName.count(); k++)
			{
				QString savepathts = QString("%1/测点%2_时域图_段%3.png").arg(exportRootPath, sensorsName[k], QString::number(j));
				insertImage(savepathts, 450, 170);
				setNormalSelectionStyle(ParagraphFormat::ChartCaption);
				addCaption("时域变化-P" + sensorsName[k], false);

				QString savepathfs = QString("%1/测点%2_频谱图_段%3.png").arg(exportRootPath, sensorsName[k], QString::number(j));
				insertImage(savepathfs, 450, 170);
				setNormalSelectionStyle(ParagraphFormat::ChartCaption);
				addCaption("频谱分析-P" + sensorsName[k], false);
			}
		}
		setNormalSelectionStyle(ParagraphFormat::ChartCaption);
		addCaption("脉动压力特征时序变化-" + wcdsp, false);
	}

	return true;
}

QAxObject* ProjectData::createEigenvalueTable(WorkingConditionsList wcs, const QStringList& sensorsNames)
{
	int cols = sensorsNames.count() + 2;
	int rows = wcs.count() * 3 + 2;

	QAxObject* tables(_wordDocument->querySubObject("Tables"));
	QAxObject* table = tables->querySubObject("Add(Range*, int, int,QVariant,QVariant)"
		, _wordSelection->querySubObject("Range")->asVariant(), rows, cols, "1", "2");
	//table->dynamicCall("AutoFitBehavior(QVariant)", "2");
	//table->querySubObject("Range")->querySubObject("ParagraphFormat")->setProperty("Alignment", 1);

	// 填充水平表头
	fillTableHeaderCell(table, 1, 1, "试验工况");
	fillTableHeaderCell(table, 1, 3, "测点");
	for (auto i = 0; i < sensorsNames.count(); i++)
	{
		fillTableHeaderCell(table, 2, 3 + i, "P" + sensorsNames[i]);
	}
	// 水平表头合并
	mergeCells(table, 1, 1, 2, 1);
	mergeCells(table, 1, 2, 2, 2);
	mergeCells(table, 1, 1, 1, 2);
	for (int i = 0; i < sensorsNames.count() - 1; i++)
	{
		mergeCells(table, 1, 2, 1, 3);
	}

	//填充工况
	for (int i = 0; i < wcs.count(); i++)
	{
		fillTableHeaderCell(table, 3 + i * 3, 1, wcs[i].description);

		fillTableHeaderCell(table, 3 + i * 3, 2, "max");
		fillTableHeaderCell(table, 3 + i * 3 + 1, 2, "min");
		fillTableHeaderCell(table, 3 + i * 3 + 2, 2, "σ");
	}
	for (int i = 0; i < wcs.count(); i++)
	{
		mergeCells(table, 3 + i * 3, 1, 3 + i * 3 + 1, 1);
		mergeCells(table, 3 + i * 3, 1, 3 + i * 3 + 2, 1);
	}

	return table;
}

QAxObject* ProjectData::createSegEigenvalueTable(WorkingConditions wc, QStringList& wcsSeg, const QStringList& sensorsNames)
{
	WorkingConditionsList wcsTemp;
	auto start = wc.gateOpenStart;
	auto end = wc.gateOpenEnd;
	auto range = end - start;
	auto between = range / SEGMENT_COUNT;
	auto offset = between / 2;
	for (auto i = 0; i < SEGMENT_COUNT - 1; i++)
	{
		WorkingConditions wctemp;
		auto st = start + offset + between * i;
		auto et = st + between;
		wctemp.description = QString("闸门开度[%1~%2]").arg(QString::number(st), QString::number(et));
		wcsSeg.append(QString("[%1~%2]").arg(QString::number(st), QString::number(et)));
		wcsTemp.push_back(wctemp);
	}
	return createEigenvalueTable(wcsTemp, sensorsNames);
}

void ProjectData::fillTableDataCell(QAxObject* table, int row, int col, const QString& text, bool centerAlign)
{
	QScopedPointer<QAxObject> cell(table->querySubObject("Cell(int, int)", row, col));
	QScopedPointer<QAxObject> range(cell->querySubObject("Range"));
	if (range.isNull()) return;
	range->dynamicCall("SetText(const QString&)", text);
	QScopedPointer<QAxObject> font(range->querySubObject("Font"));
	font->setProperty("Name", "宋体");
	font->setProperty("Size", 10);
	font->setProperty("Bold", false);
	cell->setProperty("VerticalAlignment", 1); // 1 = wdCellAlignVerticalCenter
	range->querySubObject("ParagraphFormat")->setProperty("Alignment", centerAlign ? 1 : 0);
}

void ProjectData::fillTableHeaderCell(QAxObject* table, int row, int col, const QString& text)
{
	QScopedPointer<QAxObject> cell(table->querySubObject("Cell(int, int)", row, col));
	QScopedPointer<QAxObject> range(cell->querySubObject("Range"));
	range->dynamicCall("SetText(const QString&)", text);
	QScopedPointer<QAxObject> font(range->querySubObject("Font"));
	font->setProperty("Name", "宋体");
	font->setProperty("Size", 10);
	font->setProperty("Bold", true);
	range->querySubObject("ParagraphFormat")->setProperty("Alignment", 1);
	cell->setProperty("VerticalAlignment", 1); // 垂直居中
}

void ProjectData::mergeCells(QAxObject* table, int row1, int col1, int row2, int col2)
{
	QScopedPointer<QAxObject> cell1(table->querySubObject("Cell(int, int)", row1, col1));
	QScopedPointer<QAxObject> cell2(table->querySubObject("Cell(int, int)", row2, col2));
	cell1->dynamicCall("Merge(QAxObject*)", cell2->asVariant());
}

void ProjectData::skipTable()
{
	_wordSelection->dynamicCall("EndOf(QVariant, QVariant)", 15, 0);
	_wordSelection->dynamicCall("MoveRight(QVariant, QVariant)", QVariant(1), QVariant(1));
	_wordSelection->dynamicCall("TypeText(const QString&)", "");
	_wordSelection->dynamicCall("TypeParagraph()");
}

void ProjectData::insertImage(const QString& imagePath, int width, int height)
{
	qDebug() << imagePath;
	if (!_wordSelection || !QFile::exists(imagePath)) return;
	QAxObject* inlineShapes = _wordSelection->querySubObject("InlineShapes");
	QAxObject* image = inlineShapes->querySubObject(
		"AddPicture(const QString&, QVariant, QVariant, QVariant)",
		QDir::toNativeSeparators(imagePath),
		false,  // 不链接到文件
		true,   // 随文档保存
		QVariant() // 空范围
	);

	if (!image) {
		delete inlineShapes;
		return;
	}

	// 3. 转换为浮动图形并设置环绕
	QAxObject* shape = image->querySubObject("ConvertToShape()");
	QAxObject* wrapFormat = shape->querySubObject("WrapFormat");
	wrapFormat->setProperty("Type", 7);  // wdWrapInline
	wrapFormat->setProperty("Side", 0);  // wdWrapBoth

	// 4. 设置图片尺寸（像素转磅）
	shape->setProperty("LockAspectRatio", true);
	shape->setProperty("Width", width * 72.0f / 96.0);

	// 5. 水平居中
	shape->setProperty("RelativeHorizontalPosition", 1);  // wdRelativeHorizontalPositionPage
	shape->setProperty("Left", -999995);  // wdShapeCenter

	_wordSelection->dynamicCall("TypeParagraph()");

	// 释放资源
	delete wrapFormat;
	delete shape;
	delete image;
	delete inlineShapes;
}

bool numericCompare(const QString& a, const QString& b)
{
	bool ok1, ok2;
	int numA = a.toInt(&ok1), numB = b.toInt(&ok2);
	if (ok1 && ok2) return numA < numB;
	else if (ok1) return true;
	else if (ok2) return false;
	else return a < b;
};