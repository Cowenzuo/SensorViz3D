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

	// 遍历_fpCharts调用save
	for (auto iter = _fpCharts.begin(); iter != _fpCharts.end(); ++iter)
	{
		//构建一个文档对象
		auto savedir = exportRootDir.absolutePath() + "/" + iter.key() + "/脉动压力";
		QDir savedirpath(savedir);
		if (!savedirpath.exists())
		{
			savedirpath.mkpath(".");
		}
		iter.value()->save(savedir, 450, 170);
		//iter.value()->save(doc, savedir, 450, 170);
	}
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
	QString line = in.readLine();
	QStringList sensorNames = line.split(",");
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

bool ProjectData::saveWorkingConditionsToDocx()
{
	//章节标题
	setnNormalSelectionStyle(ParagraphFormat::Level1Heading);
	_wordSelection->dynamicCall("TypeText(const QString&)", "一、工况");
	_wordSelection->dynamicCall("TypeParagraph()");

	//表格标题
	setnNormalSelectionStyle(ParagraphFormat::ChartCaption);
	addCaption("工况列表", true);

	// 创建表格并设置剧中
	int wcsize = _workingConditions.count();
	int columns = 7; // 对应header0的大小
	QAxObject* tables = _wordDocument->querySubObject("Tables");
	QAxObject* range = _wordSelection->querySubObject("Range");
	QAxObject* table = tables->querySubObject("Add(Range*, int, int,QVariant,QVariant)"
		, range->asVariant(), wcsize + 2, columns, "0", "1");
	table->querySubObject("Range")->querySubObject("ParagraphFormat")->setProperty("Alignment", 1);

	// 填充表头
	QStringList header0{ "序号", "名称","描述","闸门开度","","活塞杆开度","" };
	//setnNormalSelectionStyle(ParagraphFormat::TableHeader);
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
	auto numericCompare = [](const QString& a, const QString& b) -> bool {
		bool ok1, ok2;
		int numA = a.toInt(&ok1), numB = b.toInt(&ok2);
		if (ok1 && ok2) return numA < numB;
		else if (ok1) return true;
		else if (ok2) return false;
		else return a < b;
		};
	std::sort(keys.begin(), keys.end(), numericCompare);
	for (int i = 0; i < wcsize; i++) {
		auto wc = _workingConditions[keys[i]];
		int row = i + 3; // Word表格行从1开始，前两行是表头
		// 填充各列数据
		fillTableCell(table, row, 1, QString::number(i + 1), true);
		fillTableCell(table, row, 2, "工况-" + wc.name, true);
		fillTableCell(table, row, 3, wc.description, false);
		fillTableCell(table, row, 4, QString::number(wc.gateOpenStart, 'f', 2), true);
		fillTableCell(table, row, 5, QString::number(wc.gateOpenEnd, 'f', 2), true);
		fillTableCell(table, row, 6, QString::number((int)wc.pistonOpenStart), true);
		fillTableCell(table, row, 7, QString::number((int)wc.pistonOpenEnd), true);
	}
	return true;
}

void ProjectData::fillTableCell(QAxObject* table, int row, int col, const QString& text, bool centerAlign)
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

void ProjectData::mergeCells(QAxObject* table, int row1, int col1, int row2, int col2)
{
	QScopedPointer<QAxObject> cell1(table->querySubObject("Cell(int, int)", row1, col1));
	QScopedPointer<QAxObject> cell2(table->querySubObject("Cell(int, int)", row2, col2));
	cell1->dynamicCall("Merge(QAxObject*)", cell2->asVariant());
}

void ProjectData::setnNormalSelectionStyle(ParagraphFormat pf)
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
