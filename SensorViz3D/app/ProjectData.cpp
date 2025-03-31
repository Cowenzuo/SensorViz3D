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

bool ProjectData::setDataPackage(const QString& dirPath, const QString& savePath/* = QString()*/, bool save /*= false*/)
{
	QDir rootDir(dirPath);
	if (!rootDir.exists())
	{
		qDebug() << "Directory not exists: " << dirPath;
		return false;
	}
	_rootName = QFileInfo(dirPath).baseName();
	_rootDirPath = QDir(dirPath).absolutePath();

	if (!save)
		return true;

	return this->save(savePath, _rootName);
}

bool ProjectData::save(const QString& saveDir, const QString& filename)
{
	QDir exportRootDir(saveDir);
	if (!exportRootDir.exists())
	{
		exportRootDir.mkpath(".");
	}
	_saveDirPath = exportRootDir.absolutePath();

	QAxObject* writer = nullptr;
	QAxObject* doc = nullptr;
	QAxObject* selection = nullptr;
	if (!initWordDocment(writer, doc, selection))
	{
		qDebug() << "Init docx writer failed.";
		return false;
	}

	QMap<QString, WorkingConditions> wcs{};
	if (!loadWorkingConditions(getFullPathFromDirByAppointFolder("工况列表", _rootDirPath), wcs))
	{
		qDebug() << "Loading working conditions failed.";
		return false;
	}
	if (!saveWorkingConditionsToDocx(doc, selection, wcs))
	{
		qDebug() << "Save working conditions to docx failed.";
		return false;
	}
	qDebug() << "Save working conditions to docx succeed.";

	QVector<QPair<QString, ResType>>resFloderInfo;
	resFloderInfo.append({ "脉动压力",ResType::FP });
	resFloderInfo.append({ "加速度",ResType::VA });
	resFloderInfo.append({ "位移",ResType::VD });
	resFloderInfo.append({ "应变",ResType::Strain });
	resFloderInfo.append({ "油压",ResType::OP });
	QStringList digits = { "二", "三", "四","五", "六", "七", "八", "九" };//"一",固定被工况所使用
	for (auto i = 0;i < resFloderInfo.count(); ++i)
	{
		auto folder = resFloderInfo[i];
		auto folderFullpath = getFullPathFromDirByAppointFolder(folder.first, _rootDirPath);
		QMap<QString, ExtraData> exdatas;
		QMap<QString, BaseChart*> charts;
		if (!loadAnalyseDataFile(folderFullpath, wcs, exdatas, charts, folder.second))
		{
			qDebug() << "Loading resource data failed. file:" << folderFullpath;
			continue;
		}
		QString name;
		QString unit;
		int datacol;
		getResTypeInfo(folder.second, name, unit, datacol);
		if (!saveAnalyseDataToDocx(doc, selection, digits[i], name, unit, wcs, exdatas, charts))
		{
			qDebug() << "Save analyse data to docx failed. floder:" << folder.first;
			continue;
		}
		qDebug() << "Save analyse data to docx succeed. floder:" << folder.first;
	}

	QString filesavepath = QString("%1/%2.docx").arg(saveDir, filename);
	if (!saveAndFreeWordDocment(filesavepath, doc, writer))
	{
		qDebug() << "Writting docx failed. path:" << filesavepath;
	}
	qDebug() << "Writting docx succeed. path:" << filesavepath;
	qDebug() << "All done.";
	return true;
}

bool ProjectData::initWordDocment(
	QAxObject*& writer,
	QAxObject*& doc,
	QAxObject*& selection)
{
	writer = new QAxObject("Word.Application");
	if (!writer) {
		qWarning() << "Failed to initialize Word application";
		return false;
	}
	// 设置Word不可见（静默操作）
	writer->setProperty("Visible", false);
	// 获取文档集合
	QAxObject* documents = writer->querySubObject("Documents");
	if (!documents) {
		writer->dynamicCall("Quit()");
		delete writer;
		return false;
	}
	// 添加新文档
	doc = documents->querySubObject("Add()");
	if (!doc) {
		writer->dynamicCall("Quit()");
		delete writer;
		return false;
	}
	// 获取选区对象（即光标）
	selection = writer->querySubObject("Selection");
	if (!selection) {
		doc->dynamicCall("Close()");
		writer->dynamicCall("Quit()");
		delete writer;
		return false;
	}
	return true;
}

bool ProjectData::saveAndFreeWordDocment(const QString& absoluteFilepath, QAxObject* doc, QAxObject* writer)
{
	//如果filepath文件存在，则先删除
	QFile docFile(absoluteFilepath);
	if (docFile.exists()) {
		if (!docFile.remove()) {
			qDebug() << "Failed to remove existing file: " << absoluteFilepath;
			return false;
		}
	}
	doc->dynamicCall("SaveAs(const QString&)", QFileInfo(docFile).absoluteFilePath());
	doc->dynamicCall("Close()");
	writer->dynamicCall("Quit()");
	delete writer;
	return true;
}

bool ProjectData::loadWorkingConditions(const QString& dirPath, QMap<QString, WorkingConditions>& allwcs)
{
	allwcs.clear();
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

		allwcs[wc.name] = wc;
		file.close();
	}
	return true;
}

bool ProjectData::saveWorkingConditionsToDocx(
	QAxObject* doc,
	QAxObject* selection,
	const QMap<QString, WorkingConditions>& wcs
)
{
	//章节标题
	setNormalSelectionStyle(selection, ParagraphFormat::Level1Heading);
	selection->dynamicCall("TypeText(const QString&)", "一、 工况");
	selection->dynamicCall("TypeParagraph()");

	// 创建表格
	setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
	addCaption(selection, "工况列表", true);
	int wcsize = wcs.count();
	int columns = 7; // 对应header0的大小
	QAxObject* tables = doc->querySubObject("Tables");
	QAxObject* table = tables->querySubObject(
		"Add(Range*, int, int,QVariant,QVariant)"
		, selection->querySubObject("Range")->asVariant(), wcsize + 2, columns, "1", "2");
	//table->dynamicCall("AutoFitBehavior(QVariant)", "2");
	//table->querySubObject("Range")->querySubObject("ParagraphFormat")->setProperty("Alignment", 1);

	// 填充表头
	QStringList header0{ "序号", "名称","描述","闸门开度","","活塞杆开度","" };
	//setNormalSelectionStyle(selection,ParagraphFormat::TableHeader);
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
	QList<QString> keys = wcs.keys();
	std::sort(keys.begin(), keys.end(), &numericCompare);

	for (int i = 0; i < wcsize; i++) {
		auto wc = wcs[keys[i]];
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
	skipTable(selection);
	return true;
}

void ProjectData::getResTypeInfo(ResType type, QString& name, QString& unit, int& datacol)
{
	switch (type)
	{
	case ProjectData::ResType::FP:
	{
		datacol = 1;
		name = "脉动压力";
		unit = "KPa";
		break;
	}
	case ProjectData::ResType::VA:
	{
		datacol = 3;
		name = "振动加速度";
		unit = "m/s²";
		break;
	}
	case ProjectData::ResType::VD:
	{
		datacol = 3;
		name = "振动位移";
		unit = "m";
		break;
	}
	case ProjectData::ResType::Strain:
	{
		datacol = 1;
		name = "应变";
		unit = "MPa";
		break;
	}
	case ProjectData::ResType::OP:
	{
		datacol = 1;
		name = "油压";
		unit = "MPa";
		break;
	}
	default:
		break;
	}
}

bool ProjectData::loadAnalyseDataFile(
	const QString& dirPath,
	const QMap<QString, WorkingConditions>& allwcs,
	QMap<QString, ExtraData>& exdatas,
	QMap<QString, BaseChart*>& charts,
	ResType type
)
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

	QString resTitle = "";
	QString resUnit = "";
	int singleDataCols = 1;
	getResTypeInfo(type, resTitle, resUnit, singleDataCols);

	QDir resDir(dirPath);
	QStringList matFiles = resDir.entryList(QStringList() << "*.mat", QDir::Files);
	foreach(const QString & matFile, matFiles) {
		//通过文件名在工况列表中查找对应的工况
		QString wcName = QFileInfo(matFile).baseName();
		if (!allwcs.contains(wcName) /*|| _workingConditions[wcName].type == 0*/) {
			continue;
		}
		qDebug() << "Loading mat file: " << resDir.absoluteFilePath(matFile);
		ExtraData exdata;
		exdata.wcname = wcName;

		if (!RWMAT::readMatFile(resDir.filePath(matFile), sensorNames, sensorValid, singleDataCols, exdata))
		{
			qWarning() << "Failed to load mat file: " << matFile;
			continue;
		}

		exdata.hasSegData = segwcnames.contains(wcName);

		const int segmentSize = exdata.dataCount / SEGMENT_COUNT;
		if (segmentSize < 1) {
			qWarning() << "Data count too small for segmentation in file:" << matFile;
			continue;
		}
		const int offset = qMax(0, (segmentSize / 2) - 1);  // 确保offset非负
		exdata.dataCountEach = offset;

		for (int i = 0; i < SEGMENT_COUNT - 1; i++)
		{
			QMap<QString, double*>segData{};
			QMap<QString, Statistics>segStatistics{};
			for (auto& sensorName : sensorNames)
			{
				if (!exdata.data.contains(sensorName))
				{
					continue;
				}
				double* data = new double[segmentSize];
				for (int j = 0; j < segmentSize; j++)
				{
					data[j] = exdata.data[sensorName][i * segmentSize + j + offset];
					if (exdata.hasSegData)
					{
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
				}
				if (exdata.hasSegData)
					segStatistics[sensorName].rms = sqrt(segStatistics[sensorName].rms / segmentSize);
				segData[sensorName] = data;
			}
			exdata.segData.append(segData);
			if (exdata.hasSegData)
				exdata.segStatistics.append(segStatistics);
		}
		BaseChart* chart = new BaseChart(resTitle, resUnit);
		chart->setData(exdata);
		exdatas[wcName] = exdata;
		charts[wcName] = chart;
	}
	return true;
}

bool ProjectData::saveAnalyseDataToDocx(
	QAxObject* doc,
	QAxObject* selection,
	const QString& titleSeq,
	const QString& titlename,
	const QString& unit,
	const QMap<QString, WorkingConditions>& wcs,
	const QMap<QString, ExtraData>& exdatas,
	const QMap<QString, BaseChart*>& charts
)
{
	if (exdatas.isEmpty() || charts.isEmpty())
	{
		return false;
	}

	//章节标题
	setNormalSelectionStyle(selection, ParagraphFormat::Level1Heading);
	selection->dynamicCall("TypeText(const QString&)", QString("%1、%2").arg(titleSeq, titlename));
	selection->dynamicCall("TypeParagraph()");
	setNormalSelectionStyle(selection, ParagraphFormat::Level2Heading);
	selection->dynamicCall("TypeText(const QString&)", "1. 全过程时域频谱分析");
	selection->dynamicCall("TypeParagraph()");
	//表格标题
	setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
	addCaption(selection, titlename + "特征值", true);

	QStringList sensorsName;
	WorkingConditionsList dataWcs;
	QStringList dataWcNames = exdatas.keys();
	std::sort(dataWcNames.begin(), dataWcNames.end(), &numericCompare);
	for (int i = 0; i < dataWcNames.count(); i++) {
		dataWcs.push_back(wcs[dataWcNames[i]]);
		if (0 == i)
		{
			const auto& fps = exdatas[dataWcNames[i]].statistics;
			sensorsName = fps.keys();
			std::sort(sensorsName.begin(), sensorsName.end(), &numericCompare);
		}
	}
	auto table = createEigenvalueTable(doc, selection, dataWcs, sensorsName);
	for (int i = 0; i < dataWcNames.count(); i++)
	{
		const auto& fps = exdatas[dataWcNames[i]].statistics;
		for (int j = 0; j < sensorsName.count(); j++)
		{
			auto sensorname = sensorsName[j];
			auto stats = fps[sensorname];
			fillTableDataCell(table, 3 + i * 3 + 0, 3 + j, QString::number(stats.max, 'f', 2), true);
			fillTableDataCell(table, 3 + i * 3 + 1, 3 + j, QString::number(stats.min, 'f', 2), true);
			fillTableDataCell(table, 3 + i * 3 + 2, 3 + j, QString::number(stats.rms, 'f', 2), true);
		}
	}
	skipTable(selection);

	for (int i = 0; i < dataWcNames.count(); i++)
	{

		const auto& wcname = wcs[dataWcNames[i]].name;
		const auto& wcdesp = wcs[dataWcNames[i]].description;
		setNormalSelectionStyle(selection, ParagraphFormat::Level3Heading);
		selection->dynamicCall("TypeText(const QString&)", QString("(%1) %2").arg(QString::number(i + 1), wcdesp));
		selection->dynamicCall("TypeParagraph()");

		QDir exportRootDir(QString("%1/%2/%3").arg(_saveDirPath, wcname, titlename));
		if (!exportRootDir.exists())
			exportRootDir.mkpath(".");
		auto exportRootPath = exportRootDir.absolutePath();
		charts[dataWcNames[i]]->save(exportRootPath, 450, 170);
		charts[dataWcNames[i]]->saveSeg(exportRootPath, 450, 170);

		setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
		for (int j = 0; j < sensorsName.count(); j++)
		{
			QString savepathts = QString("%1/测点%2_时域图.png").arg(exportRootPath, sensorsName[j]);
			insertImage(selection, savepathts, 450, 170);
			setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
			addCaption(selection, "时域变化-P" + sensorsName[j], false);

			QString savepathfs = QString("%1/测点%2_频谱图.png").arg(exportRootPath, sensorsName[j]);
			insertImage(selection, savepathfs, 450, 170);
			setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
			addCaption(selection, "频谱分析-P" + sensorsName[j], false);
		}
	}

	setNormalSelectionStyle(selection, ParagraphFormat::Level2Heading);
	selection->dynamicCall("TypeText(const QString&)", "2. 分段时域频谱分析");
	selection->dynamicCall("TypeParagraph()");

	for (int i = 0, seq = 0; i < dataWcs.count(); i++)
	{
		if (!exdatas[dataWcs[i].name].hasSegData)
		{
			continue;
		}
		auto wcdsp = dataWcs[i].description;
		QDir exportRootDir(QString("%1/%2/%3").arg(_saveDirPath, dataWcs[i].name, titlename));
		if (!exportRootDir.exists())
			exportRootDir.mkpath(".");
		auto exportRootPath = exportRootDir.absolutePath();

		setNormalSelectionStyle(selection, ParagraphFormat::Level3Heading);
		selection->dynamicCall("TypeText(const QString&)", QString("(%1) %2").arg(QString::number(++seq), wcdsp));
		selection->dynamicCall("TypeParagraph()");
		selection->dynamicCall("TypeParagraph()");

		setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
		addCaption(selection, titlename + "特征值-" + wcdsp, true);
		QStringList wcsSeg;
		auto segtable = createSegEigenvalueTable(doc, selection, dataWcs[i], wcsSeg, sensorsName);
		QMap<QString, QVector<double>> sensorMaxValue;
		QMap<QString, QVector<double>> sensorMinValue;
		QMap<QString, QVector<double>> sensorRmsValue;
		const auto& fpsegs = exdatas[dataWcs[i].name].segStatistics;
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
		skipTable(selection);

		auto chartMax = MagChart::paintMagChart(titlename + "最大值对比分析", "闸门开度", QString("%1(%2)").arg(titlename, unit), wcsSeg, sensorsName, sensorMaxValue);
		auto chartMin = MagChart::paintMagChart(titlename + "最小值对比分析", "闸门开度", QString("%1(%2)").arg(titlename, unit), wcsSeg, sensorsName, sensorMinValue);
		auto chartRms = MagChart::paintMagChart(titlename + "均方根对比分析", "闸门开度", QString("%1(%2)").arg(titlename, unit), wcsSeg, sensorsName, sensorRmsValue);
		auto chartMaxSavePath = QString("%1/工况%2_最大值对比.png").arg(exportRootPath, dataWcs[i].name);
		auto chartMinSavePath = QString("%1/工况%2_最小值对比.png").arg(exportRootPath, dataWcs[i].name);
		auto chartRmsSavePath = QString("%1/工况%2_均方根对比.png").arg(exportRootPath, dataWcs[i].name);
		chartMax->toPixmap(530, 400).save(chartMaxSavePath);
		chartMin->toPixmap(530, 400).save(chartMinSavePath);
		chartRms->toPixmap(530, 400).save(chartRmsSavePath);

		insertImage(selection, chartMaxSavePath, 530, 400);
		setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
		addCaption(selection, titlename + "最大值对比分析", false);

		insertImage(selection, chartMinSavePath, 530, 400);
		setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
		addCaption(selection, titlename + "最小值对比分析", false);

		insertImage(selection, chartRmsSavePath, 530, 400);
		setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
		addCaption(selection, titlename + "均方根对比分析", false);

		for (int j = 0; j < wcsSeg.count(); j++)
		{
			setNormalSelectionStyle(selection, ParagraphFormat::Level3Heading);
			selection->dynamicCall("TypeText(const QString&)", QString("%1) 闸门开度%2").arg(QString::number(j + 1), wcsSeg[j]));
			selection->dynamicCall("TypeParagraph()");

			setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
			for (int k = 0; k < sensorsName.count(); k++)
			{
				QString savepathts = QString("%1/测点%2_时域图_段%3.png").arg(exportRootPath, sensorsName[k], QString::number(j));
				insertImage(selection, savepathts, 450, 170);
				setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
				addCaption(selection, "时域变化-P" + sensorsName[k], false);

				QString savepathfs = QString("%1/测点%2_频谱图_段%3.png").arg(exportRootPath, sensorsName[k], QString::number(j));
				insertImage(selection, savepathfs, 450, 170);
				setNormalSelectionStyle(selection, ParagraphFormat::ChartCaption);
				addCaption(selection, "频谱分析-P" + sensorsName[k], false);
			}
		}
	}
	return true;
}

QString ProjectData::getFullPathFromDirByAppointFolder(const QString& foldername, QDir rootDir)
{
	if (!rootDir.exists())
	{
		qDebug() << "Directory not exists: " << rootDir.absolutePath();
		return QString();
	}
	QStringList folders = rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (const QString& folder : folders)
	{
		if (folder == foldername)
		{
			return rootDir.absoluteFilePath(folder);
		}
	}
	return QString();
}

void ProjectData::setNormalSelectionStyle(QAxObject* selection, ParagraphFormat pf)
{
	QScopedPointer<QAxObject> paragraphFormat(selection->querySubObject("ParagraphFormat"));
	QScopedPointer<QAxObject> font(selection->querySubObject("Font"));

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

void ProjectData::addCaption(QAxObject* selection, const QString& text, bool isTable)
{
	selection->dynamicCall("TypeText(const QString&)", isTable ? "表" : "图");

	QAxObject* fields = selection->querySubObject("Fields");
	QAxObject* range1 = selection->querySubObject("Range");
	fields->dynamicCall("Add(QAxObject*,QVariant, QVariant, QVariant)",
		range1->asVariant(),
		"12",  // wdFieldSequence
		isTable ? "table \\# \"0.\"" : "figure \\# \"0.\"",
		true
	);
	fields->dynamicCall("Update()");

	selection->dynamicCall("TypeText(const QString&)", text);
	selection->dynamicCall("TypeParagraph()");
}

QAxObject* ProjectData::createEigenvalueTable(QAxObject* doc, QAxObject* selection, WorkingConditionsList wcs, const QStringList& sensorsNames)
{
	int cols = sensorsNames.count() + 2;
	int rows = wcs.count() * 3 + 2;

	QAxObject* tables(doc->querySubObject("Tables"));
	QAxObject* table = tables->querySubObject("Add(Range*, int, int,QVariant,QVariant)"
		, selection->querySubObject("Range")->asVariant(), rows, cols, "1", "2");
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

QAxObject* ProjectData::createSegEigenvalueTable(QAxObject* doc, QAxObject* selection, WorkingConditions wc, QStringList& wcsSeg, const QStringList& sensorsNames)
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
	return createEigenvalueTable(doc, selection, wcsTemp, sensorsNames);
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

void ProjectData::skipTable(QAxObject* selection)
{
	selection->dynamicCall("EndOf(QVariant, QVariant)", 15, 0);
	selection->dynamicCall("MoveRight(QVariant, QVariant)", QVariant(1), QVariant(1));
	selection->dynamicCall("TypeText(const QString&)", "");
	selection->dynamicCall("TypeParagraph()");
}

void ProjectData::insertImage(QAxObject* selection, const QString& imagePath, int width, int height)
{
	qDebug() << imagePath;
	if (!selection || !QFile::exists(imagePath)) return;
	QAxObject* inlineShapes = selection->querySubObject("InlineShapes");
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

	selection->dynamicCall("TypeParagraph()");

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