#pragma once

#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QObject>

#ifdef slots
#undef slots
#include <Python.h>
#define slots Q_SLOTS
#endif

namespace docx
{
	class Document;
}

#define WORKING_CONDITIONS_LINE_COUNT 10

#define FIRST_LEVEL_TITILE_FONT "黑体"
#define FIRST_LEVEL_TITILE_FONT_SIZE 24
#define FIRST_LEVEL_TITILE_FONT_BOLD true

#define SECOND_LEVEL_TITILE_FONT "黑体"
#define SECOND_LEVEL_TITILE_FONT_SIZE 18
#define SECOND_LEVEL_TITILE_FONT_BOLD true

#define THIRD_LEVEL_TITILE_FONT "黑体"
#define THIRD_LEVEL_TITILE_FONT_SIZE 16
#define THIRD_LEVEL_TITILE_FONT_BOLD false

#define NORMAL_TEXT_FONT "宋体"
#define NORMAL_TEXT_FONT_SIZE 12

#define NORMAL_FS_TS_IMAGE_WIDTH 450
#define NORMAL_FS_TS_IMAGE_HEIGHT 450

struct WorkingConditions
{
	QString name{ "" };					//工况名称	
	QString description{ "" };			//动作描述
	int type{ 0 };						//实验类型 0-静态实验 1-动态实验
	double upWaterLevelStart{ 0.0 };	//上游水位（起始） double 单位（米）
	double upWaterLevelEnd{ 0.0 };		//上游水位（终止） double 单位（米）
	double downWaterLevelStart{ 0.0 };	//下游水位（起始） double 单位（米）
	double downWaterLevelEnd{ 0.0 };	//下游水位（终止） double 单位（米）
	double gateOpenStart{ 0.0 };		//闸门开度（起始） double 单位（% [0~1]）
	double gateOpenEnd{ 0.0 };			//闸门开度（终止） double 单位（% [0~1]）
	double pistonOpenStart{ 0.0 };		//活塞杆开度（起始） double  单位（毫米）
	double pistonOpenEnd{ 0.0 };		//活塞杆开度（终止） double  单位（毫米）
};

struct Statistics {
	double max{ 0.0 };//最大值
	double min{ 0.0 };//最小值
	double rms{ 0.0 };//均方根
};

struct RawData
{
	QString wcname{ "" };					//从属于的工况名称
	int frequency{ 100 };					//采集频率
	int senseCount{ 0 };					//传感器数量
	int dataCount{ 0 };						//数据点数量
	QDateTime startTime{};					//开始时间(绝对时间)
	QMap<QString, double*>data{};			//原始数据<传感器编号，数值>
	QMap<QString, Statistics>statistics{};	//统计数据<传感器编号，数值>
};

//数据分段数量，将会有9个中间数据点，数据点前后各0.5*num个数量的数据进行重分析与统计
#define SEGMENT_COUNT 10
struct FPData :public RawData
{
	int dataCountEach{ 0 };						//数据点数量
	QVector<QMap<QString, double*>>segData{};
	QVector<QMap<QString, Statistics>>segStatistics{};
};

class FPChart;
class ProjectData : public QObject
{
	Q_OBJECT

public:
	ProjectData(QObject* parent);
	~ProjectData();

	bool loadDataPackage(const QString& dirPath);

	bool save(const QString& dirPath);
private:
	// 读取工况数据并保存
	bool loadWorkingConditions(const QString& dirPath);
	// 读取脉动压力数据列表
	bool loadFluctuationPressure(const QString& dirPath);

	bool saveWorkingConditionsToDocx(docx::Document* doc);

	bool saveWorkingConditionsToDocx(PyObject* doc);
private:
	QString _rootDirPath;
	QString _rootName;

	QMap<QString, WorkingConditions> _workingConditions;

	QMap<QString, FPData> _fpData;		//脉动压力数据<工况名,数据>
	QMap<QString, FPChart*> _fpCharts;	//脉动压力图表<工况名,图表>


	bool saveWorkingConditionsToDocx(const QString& filename) {
		Py_Initialize();

		// 设置Python模块搜索路径（如果需要）
		PyRun_SimpleString("import sys\nsys.path.append('./')");

		// 导入docx模块
		PyObject* pDocxModule = PyImport_ImportModule("docx");
		if (!pDocxModule) {
			PyErr_Print();
			Py_Finalize();
			return false;
		}

		// 创建Document对象
		PyObject* pDocumentClass = PyObject_GetAttrString(pDocxModule, "Document");
		PyObject* pDoc = PyObject_CallObject(pDocumentClass, nullptr);
		Py_DECREF(pDocumentClass);

		// 添加主标题
		addTitle(pDoc, "工况", 16, "宋体", true);

		// 添加表格标题
		addTableTitle(pDoc, "工况列表", 12, "宋体");

		// 创建表格
		int rowCount = _workingConditions.count() + 2;
		int colCount = 7;
		PyObject* pTable = createTable(pDoc, rowCount, colCount);
		setTableHeaders(pTable);
		mergeCells(pTable);

		// 填充表格数据
		fillTableData(pTable);

		// 保存文档
		saveDocument(pDoc, filename.toUtf8().constData());

		// 清理资源
		Py_DECREF(pDoc);
		Py_DECREF(pDocxModule);
		Py_Finalize();
		return true;
	}

	PyObject* createTable(PyObject* pDoc, int rows, int cols) {
		PyObject* pTables = PyObject_GetAttrString(pDoc, "tables");
		PyObject* pAddTable = PyObject_GetAttrString(pTables, "add_table");
		PyObject* pArgs = PyTuple_Pack(2, PyLong_FromLong(rows), PyLong_FromLong(cols));
		PyObject* pTable = PyObject_CallObject(pAddTable, pArgs);
		Py_DECREF(pArgs);
		Py_DECREF(pAddTable);
		Py_DECREF(pTables);
		return pTable;
	}

	void setTableHeaders(PyObject* pTable) {
		const char* headers[] = {
			"序号", "名称", "描述", "闸门开度", "闸门开度", "活塞杆开度", "活塞杆开度"
		};
		const char* subHeaders[] = {
			"", "", "", "起始", "终止", "起始", "终止"
		};

		// 设置第一行标题
		for (int i = 0; i < 7; ++i) {
			setCellText(pTable, 0, i, headers[i], 14, "宋体", true);
		}

		// 设置第二行子标题
		for (int i = 0; i < 7; ++i) {
			setCellText(pTable, 1, i, subHeaders[i], 14, "宋体", true);
		}
	}

	void mergeCells(PyObject* pTable) {
		// 合并单元格（示例合并第一列的三个单元格）
		PyObject* pCell = getTableCell(pTable, 0, 0);
		PyObject* pMerge = PyObject_GetAttrString(pTable, "merge_cells");
		PyObject* pArgs = PyTuple_Pack(2, pCell, pCell); // 这里需要实际合并逻辑
		PyObject_CallObject(pMerge, pArgs);
		Py_DECREF(pArgs);
		Py_DECREF(pMerge);
	}

	void fillTableData(PyObject* pTable) {
		QList<QString> keys = _workingConditions.keys();
		std::sort(keys.begin(), keys.end(), [](const QString& a, const QString& b) {
			return a.toInt() < b.toInt();
			});

		int row = 2;
		for (const auto& key : keys) {
			const WorkingConditions& wc = _workingConditions[key];

			// 设置序号
			setCellText(pTable, row, 0, QString::number(row - 1), 12, "宋体");

			// 设置名称
			setCellText(pTable, row, 1, "工况-" + wc.name, 12, "宋体");

			// 设置描述
			setCellText(pTable, row, 2, wc.description, 12, "宋体");

			// 设置数值数据
			setCellText(pTable, row, 3, QString::number(wc.gateOpenStart, 'f', 2), 12, "宋体");
			setCellText(pTable, row, 4, QString::number(wc.gateOpenEnd, 'f', 2), 12, "宋体");
			setCellText(pTable, row, 5, QString::number(wc.pistonOpenStart), 12, "宋体");
			setCellText(pTable, row, 6, QString::number(wc.pistonOpenEnd), 12, "宋体");

			row++;
		}
	}

	void saveDocument(PyObject* pDoc, const char* filename) {
		PyObject* pSaveMethod = PyObject_GetAttrString(pDoc, "save");
		PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(filename));
		PyObject_CallObject(pSaveMethod, pArgs);
		Py_DECREF(pArgs);
		Py_DECREF(pSaveMethod);
	}

	PyObject* getTableCell(PyObject* pTable, int row, int col) {
		PyObject* pRows = PyObject_GetAttrString(pTable, "rows");
		PyObject* pRow = PyList_GetItem(pRows, row);
		PyObject* pCells = PyObject_GetAttrString(pRow, "cells");
		PyObject* pCell = PyList_GetItem(pCells, col);
		Py_DECREF(pCells);
		Py_DECREF(pRow);
		Py_DECREF(pRows);
		return pCell;
	}

	void setCellText(PyObject* pTable, int row, int col, const QString& text,
		int fontSize, const char* fontName, bool bold = false) {
		PyObject* pCell = getTableCell(pTable, row, col);
		PyObject* pParagraphs = PyObject_GetAttrString(pCell, "paragraphs");
		PyObject* pParagraph = PyList_GetItem(pParagraphs, 0);

		// 添加文本
		PyObject* pAddRun = PyObject_GetAttrString(pParagraph, "add_run");
		PyObject* pText = PyUnicode_FromString(text.toUtf8().constData());
		PyObject* pRun = PyObject_CallObject(pAddRun, PyTuple_Pack(1, pText));
		Py_DECREF(pText);
		Py_DECREF(pAddRun);

		// 设置字体样式
		PyObject_CallMethod(pRun, "font.size", "i", fontSize);
		PyObject_CallMethod(pRun, "font.name", "s", fontName);
		if (bold) {
			PyObject_CallMethod(pRun, "bold", "");
		}

		Py_DECREF(pRun);
		Py_DECREF(pParagraph);
		Py_DECREF(pParagraphs);
	}

	void addTitle(PyObject* pDoc, const QString& text, int fontSize,
		const char* fontName, bool bold = false) {
		PyObject* pAddParagraph = PyObject_CallMethod(pDoc, "add_paragraph", NULL);
		//PyObject* pParagraph = PyObject_CallObject(pAddParagraph, nullptr);
		//Py_DECREF(pAddParagraph);

		PyObject* pRun = PyObject_CallMethod(pAddParagraph, "add_run", "s",
			text.toUtf8().constData());
		PyObject_CallMethod(pRun, "font.size", "i", fontSize);
		PyObject_CallMethod(pRun, "font.name", "s", fontName);
		if (bold) {
			PyObject_CallMethod(pRun, "bold", "");
		}
		PyObject_CallMethod(pAddParagraph, "alignment", "i", 1); // 左对齐

		Py_DECREF(pRun);
		Py_DECREF(pAddParagraph);
	}

	void addTableTitle(PyObject* pDoc, const QString& text, int fontSize,
		const char* fontName) {
		PyObject* pAddParagraph = PyObject_CallMethod(pDoc, "add_paragraph", NULL);
		//PyObject* pParagraph = PyObject_CallObject(pAddParagraph, nullptr);
		//Py_DECREF(pAddParagraph);

		PyObject* pRun = PyObject_CallMethod(pAddParagraph, "add_run", "s",
			text.toUtf8().constData());
		PyObject_CallMethod(pRun, "font.size", "i", fontSize);
		PyObject_CallMethod(pRun, "font.name", "s", fontName);
		PyObject_CallMethod(pAddParagraph, "alignment", "i", 0); // 居中

		Py_DECREF(pRun);
		Py_DECREF(pAddParagraph);
	}

};
