#pragma once
#pragma once

#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QObject>
#include <qaxobject.h>
#include <QString>
#include <QDir>

//工况数据解析存储结构
#define WORKING_CONDITIONS_LINE_COUNT 10
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
typedef QList<WorkingConditions> WorkingConditionsList;

//原始数据解析存储结构（含初步的统计计算）
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
	bool hasSegData{ false };				//是否存在时序分割数据
};
//数据分段数量，将会有9个中间数据点，数据点前后各0.5*num个数量的数据进行重分析与统计
#define SEGMENT_COUNT 10
struct ExtraData :public RawData
{
	int dataCountEach{ 0 };						//数据点数量
	QVector<QMap<QString, double*>>segData{};
	QVector<QMap<QString, Statistics>>segStatistics{};
};

class BaseChart;
class FPChart;
class ProjectData : public QObject
{
	Q_OBJECT
public:
	ProjectData(QObject* parent);
	~ProjectData();

	// 主要逻辑上是设置原始数据包的路径，如果savePath、save都给了，会自动执行读取、处理、存储操作
	// dirPath:	给入原始数据包的文件夹，这个文件夹应该包含"工况列表"、"应变"等Mat数据文件夹
	// savePath:给入希望保存到的文件夹路径，必须是文件夹
	// save:	是否保存
	bool setDataPackage(const QString& dirPath, const QString& savePath = QString(), bool save = false);
	// 必须后于setDataPackage执行，内部执行相应数据的读取、处理、存储操作
	// saveDir:		给入希望保存到的文件夹路径，必须是文件夹
	// filename:	最终docx的文件名，不用带后缀，过程性文件直接使用内置命名，暂时未开放接口干预操作
	bool save(const QString& saveDir, const QString& filename);

private:
	//初始化和Ms office的交互
	bool initWordDocment(
		QAxObject*& writer,
		QAxObject*& doc,
		QAxObject*& selection
	);
	bool saveAndFreeWordDocment(
		const QString& absoluteFilepath,
		QAxObject* doc,
		QAxObject* writer
	);
private:
	// 从dirPath文件夹路径读取工况数据到allwcs
	bool loadWorkingConditions(
		const QString& dirPath,
		QMap<QString, WorkingConditions>& allwcs
	);
	// 通过doc、selection保存工况数据wcs到docx(非即时写入)
	bool saveWorkingConditionsToDocx(
		QAxObject* doc,
		QAxObject* selection,
		const QMap<QString, WorkingConditions>& wcs
	);
private:
	//将各个维度的数据加载到内存
	enum class ResType { FP, VA, VD, Strain, OP };
	void getResTypeInfo(ResType type, QString& name, QString& unit, int& datacol);
	bool loadAnalyseDataFile(
		const QString& dirPath,
		const QMap<QString, WorkingConditions>& wc,
		QMap<QString, ExtraData>& exdata,
		QMap<QString, BaseChart*>& charts,
		ResType type
	);
	//将各个维度的数据存到docx(非即时写入)
	bool saveAnalyseDataToDocx(
		QAxObject* doc,
		QAxObject* selection,
		const QString& titleSeq,
		const QString& titlename,
		const QString& unit,
		const QMap<QString, WorkingConditions>& wcs,
		const QMap<QString, ExtraData>& exdatas,
		const QMap<QString, BaseChart*>& charts
	);
private:
	//辅助函数：纯定制，无通用性，只是为了方遍从一个rootDir中提取出文件夹名字为foldername的完整文件夹路径
	QString getFullPathFromDirByAppointFolder(const QString& foldername, QDir rootDir);

private:
	//以下为写入Docx时的辅助函数，纯定制，无通用性，只是为了该项目读写数据文件使用
	enum class ParagraphFormat {
		TextBody,          // 正文
		ChartCaption,      // 图表上下标
		Level1Heading,     // 1级标题
		Level2Heading,     // 2级标题
		Level3Heading      // 3级标题
	};
	//设置通用正文、标题字体布局样式的统一接口
	void setNormalSelectionStyle(QAxObject* selection, ParagraphFormat pf);
	//添加表、图题注(非具体表格或图片)统一接口
	void addCaption(QAxObject* selection, const QString& text, bool isTable = false);
	//添加图的统一接口
	void insertImage(QAxObject* selection, const QString& imagePath, int width, int height);
	//添加表的统一接口
	QAxObject* createEigenvalueTable(QAxObject* doc, QAxObject* selection, WorkingConditionsList wcs, const QStringList& sensorsNames);
	QAxObject* createSegEigenvalueTable(QAxObject* doc, QAxObject* selection, WorkingConditions wc, QStringList& wcsSeg, const QStringList& sensorsNames);
	void fillTableDataCell(QAxObject* table, int row, int col, const QString& text, bool centerAlign);
	void fillTableHeaderCell(QAxObject* table, int row, int col, const QString& text);
	void mergeCells(QAxObject* table, int row1, int col1, int row2, int col2);
	//当表格输入完成后，一定要调用一下这个函数，以使光标Selection脱离Table的区域
	void skipTable(QAxObject* selection);
private:
	QString _saveDirPath{};
	QString _rootDirPath{};
	QString _rootName{};
};

//排序辅助函数，纯定制，且只能通过对比key的str转int后的升序排列
bool numericCompare(const QString& a, const QString& b);