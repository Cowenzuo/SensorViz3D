#include "ProjectData.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

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

	_rootName = QFileInfo(dirPath).baseName();
	_rootDirPath = QDir(dirPath).absolutePath();
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
	QDir fpDir(dirPath);
	QStringList matFiles = fpDir.entryList(QStringList() << "*.mat", QDir::Files);
	foreach(const QString & matFile, matFiles) {
	}

	return false;
}
