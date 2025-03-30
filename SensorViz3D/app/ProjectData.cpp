#include "ProjectData.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>


#include "minidocx.hpp"

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

	//docx::Document* doc = new docx::Document;
	//saveWorkingConditionsToDocx(doc);

	//Py_Initialize();  // 初始化 Python 解释器
	// 导入pyansys模块
	/*auto docxModule = PyImport_ImportModule("docx");
	if (!docxModule) {
		PyErr_Print();
		std::cerr << "Failed to load 'docx'" << std::endl;
		return false;
	}
	PyObject* pModule = PyImport_ImportModule("docx");
	if (!pModule) { PY_CHECK(); }

	PyObject* pDocxModule = PyImport_ImportModule("docx.document");
	PyObject* pStylesModule = PyImport_ImportModule("docx.shared");
	PyObject* pEnumModule = PyImport_ImportModule("docx.enum.text");
	if (!pDocxModule || !pStylesModule || !pEnumModule) { PY_CHECK(); }*/

	//saveWorkingConditionsToDocx(pDoc);
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
		iter.value()->save(nullptr, savedir, 450, 170);
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
	//doc->Save(filepath);
	//delete doc;

	saveWorkingConditionsToDocx(filepath);
	//auto save = PyObject_CallMethod(pDoc, "save", "s", filepath.toStdString().c_str());

	//Py_XDECREF(pDoc);
	//Py_XDECREF(documentClass);
	//Py_XDECREF(docxModule);
	//Py_Finalize();  // 关闭 Python 解释器
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

bool ProjectData::saveWorkingConditionsToDocx(docx::Document* doc)
{
	docx::Paragraph pWc = doc->AppendParagraph();
	pWc.SetAlignment(docx::Paragraph::Alignment::Left);
	auto title = pWc.AppendRun("工况");
	title.SetFontSize(FIRST_LEVEL_TITILE_FONT_SIZE);
	title.SetFont(FIRST_LEVEL_TITILE_FONT);
	title.SetFontStyle(docx::Run::Bold);

	docx::Paragraph pWcList = doc->AppendParagraph();
	pWcList.SetAlignment(docx::Paragraph::Alignment::Centered);
	auto tabtletitle = pWcList.AppendRun("工况列表");
	tabtletitle.SetFontSize(NORMAL_TEXT_FONT_SIZE);
	tabtletitle.SetFont(NORMAL_TEXT_FONT);

	auto wcsize = _workingConditions.count();
	QStringList header0{ "序号", "名称","描述","闸门开度","闸门开度","活塞杆开度","活塞杆开度" };
	QStringList header1{ "序号", "名称","描述","起始","终止","起始","终止" };

	auto tbl = doc->AppendTable(wcsize + 2, header0.size());
	tbl.SetAlignment(docx::Table::Alignment::Centered);


	for (int i = 0; i < header0.size(); i++)
	{
		auto containStr = header0[i].toUtf8().data();
		auto cell = tbl.GetCell(0, i);
		cell.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		auto cellp = cell.FirstParagraph();
		cellp.SetAlignment(docx::Paragraph::Alignment::Centered);
		auto cellrun = cellp.AppendRun(containStr);
		cellrun.SetFontSize(THIRD_LEVEL_TITILE_FONT_SIZE);
		cellrun.SetFont(NORMAL_TEXT_FONT);
		cellrun.SetFontStyle(docx::Run::Bold);
	}

	for (int i = 0; i < header1.size(); i++)
	{
		auto containStr = header1[i].toUtf8().data();
		auto cell = tbl.GetCell(1, i);
		cell.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		auto cellp = cell.FirstParagraph();
		cellp.SetAlignment(docx::Paragraph::Alignment::Centered);
		auto cellrun = cellp.AppendRun(containStr);
		cellrun.SetFontSize(THIRD_LEVEL_TITILE_FONT_SIZE);
		cellrun.SetFont(NORMAL_TEXT_FONT);
		cellrun.SetFontStyle(docx::Run::Bold);
	}

	tbl.MergeCells(tbl.GetCell(0, 0), tbl.GetCell(1, 0));
	tbl.MergeCells(tbl.GetCell(0, 1), tbl.GetCell(1, 1));
	tbl.MergeCells(tbl.GetCell(0, 2), tbl.GetCell(1, 2));
	tbl.MergeCells(tbl.GetCell(0, 3), tbl.GetCell(0, 4));
	tbl.MergeCells(tbl.GetCell(0, 5), tbl.GetCell(0, 6));

	QList<QString> keys = _workingConditions.keys();
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

	for (int i = 0; i < wcsize; i++)
	{
		auto wc = _workingConditions[keys[i]];
		auto row = i + 2;
		auto cell0 = tbl.GetCell(row, 0);
		auto cell1 = tbl.GetCell(row, 1);
		auto cell2 = tbl.GetCell(row, 2);
		auto cell3 = tbl.GetCell(row, 3);
		auto cell4 = tbl.GetCell(row, 4);
		auto cell5 = tbl.GetCell(row, 5);
		auto cell6 = tbl.GetCell(row, 6);
		cell0.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		cell1.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		cell2.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		cell3.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		cell4.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		cell5.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		cell6.SetVerticalAlignment(docx::TableCell::Alignment::Center);
		auto cellp0 = cell0.FirstParagraph();
		auto cellp1 = cell1.FirstParagraph();
		auto cellp2 = cell2.FirstParagraph();
		auto cellp3 = cell3.FirstParagraph();
		auto cellp4 = cell4.FirstParagraph();
		auto cellp5 = cell5.FirstParagraph();
		auto cellp6 = cell6.FirstParagraph();
		cellp0.SetAlignment(docx::Paragraph::Alignment::Centered);
		cellp1.SetAlignment(docx::Paragraph::Alignment::Centered);
		cellp2.SetAlignment(docx::Paragraph::Alignment::Left);
		cellp3.SetAlignment(docx::Paragraph::Alignment::Centered);
		cellp4.SetAlignment(docx::Paragraph::Alignment::Centered);
		cellp5.SetAlignment(docx::Paragraph::Alignment::Centered);
		cellp6.SetAlignment(docx::Paragraph::Alignment::Centered);
		auto runcell0 = cellp0.AppendRun(QString::number(i + 1).toUtf8().data());
		auto runcell1 = cellp1.AppendRun(QString("工况-" + wc.name).toUtf8().data());
		auto runcell2 = cellp2.AppendRun(wc.description.toUtf8().data());
		auto runcell3 = cellp3.AppendRun(QString::number(wc.gateOpenStart, 'f', 2).toUtf8().data());
		auto runcell4 = cellp4.AppendRun(QString::number(wc.gateOpenEnd, 'f', 2).toUtf8().data());
		auto runcell5 = cellp5.AppendRun(QString::number((int)wc.pistonOpenStart).toUtf8().data());
		auto runcell6 = cellp6.AppendRun(QString::number((int)wc.pistonOpenEnd).toUtf8().data());
		runcell0.SetFontSize(NORMAL_TEXT_FONT_SIZE);
		runcell1.SetFontSize(NORMAL_TEXT_FONT_SIZE);
		runcell2.SetFontSize(NORMAL_TEXT_FONT_SIZE);
		runcell3.SetFontSize(NORMAL_TEXT_FONT_SIZE);
		runcell4.SetFontSize(NORMAL_TEXT_FONT_SIZE);
		runcell5.SetFontSize(NORMAL_TEXT_FONT_SIZE);
		runcell6.SetFontSize(NORMAL_TEXT_FONT_SIZE);
		runcell0.SetFont(NORMAL_TEXT_FONT);
		runcell1.SetFont(NORMAL_TEXT_FONT);
		runcell2.SetFont(NORMAL_TEXT_FONT);
		runcell3.SetFont(NORMAL_TEXT_FONT);
		runcell4.SetFont(NORMAL_TEXT_FONT);
		runcell5.SetFont(NORMAL_TEXT_FONT);
		runcell6.SetFont(NORMAL_TEXT_FONT);
	}

	return true;
}

bool ProjectData::saveWorkingConditionsToDocx(PyObject* pDoc)
{
	PyObject* pParagraph = PyObject_CallMethod(pDoc, "add_paragraph", "s", "工况");
	PyObject_CallMethod(pParagraph, "add_run", "s", "工况");

	return true;
}
