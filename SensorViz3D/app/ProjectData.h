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

enum class ResType { FP, VA, VD, Strain, OP, HC };
Q_DECLARE_METATYPE(ResType);

class ChartPainter;
class FPChart;

struct AnalyseData
{
	ExtraData exData;
	ChartPainter* charts;
};
class ProjectData : public QObject
{
	Q_OBJECT
private:
	QString _saveDirPath{};
	QString _rootDirPath{};
	QString _rootName{};

	QMap<QString, WorkingConditions> _workingConditions;
	QMap<ResType, QMap<QString, AnalyseData>> _analyseDatas;

public:
	ProjectData(QObject* parent = nullptr);
	~ProjectData();

	// 主要逻辑上是设置原始数据包的路径，如果savePath、save都给了，会自动执行读取、处理、存储操作
	// dirPath:	给入原始数据包的文件夹，这个文件夹应该包含"工况列表"、"应变"等Mat数据文件夹
	// savePath:给入希望保存到的文件夹路径，必须是文件夹
	// saveBackground:	是否保存
	bool setDataPackage(const QString& dirPath, const QString& savePath = QString(), bool save = false);

	// 必须后于setDataPackage执行，内部执行相应数据的读取、处理、存储操作
	// 额外注意点，这个接口是为了可视化的逻辑而准备的，会一次性把所有数据与处理好数据都加载到内存里
	// 以便快速查看，通常会达到几个G的占用，源数据文件越大，占用越多，会有个极限，暂时没有测出来
	// Tips：优化方案也有，暂时没时间改了，希望将saveBackground逻辑都合并起来
	bool loadForVisual();

	// 必须后于setDataPackage执行，内部执行相应数据的读取、处理、存储操作
	// 额外注意点，这个接口除了需要setDataPackage外，其他都不依赖，为了后台静默运行准备的
	// 这个按顺序执行，处理完一个维度就释放内存，以确保真的遇到了大量数据集，不会发生内存直接爆炸的情况
	// 不用按常规可视化逻辑，让每一个维度数据的内存都是常驻，不太合理
	// saveDir:		给入希望保存到的文件夹路径，必须是文件夹
	// filename:	最终docx的文件名，不用带后缀，过程性文件直接使用内置命名，暂时未开放接口干预操作
	//				若为空直接将会使用setDataPackage时候获取的数据包文件夹名字作为文件名
	bool saveBackground(const QString& saveDir, const QString& filename = QString{});

public:
	//获取当前数据包的所有分析维度名与枚举量（获取方如果需要后续查询，请保存这个枚举量）
	QVector<QPair<QString, ResType>> getDimNames();
	//通过枚举量获取当前分析维度下有多少个工况(以及是否带有分段数据)
	QVector<QPair<QString, bool>> geWorkingConditionsNames(ResType dimtype);
	//通过枚举量以及工况名，获取当前状态全部传感器的名字列表
	QStringList geSensorNames(ResType dimtype, const QString& wcname);
	//通过枚举量以及工况名，获取当前状态全部传感器的绘图
	ChartPainter* getCharts(ResType dimtype, const QString& wcname);
	//通过枚举量以及工况名，获取当前工况有没有分断数据
	bool hasSegData(ResType dimtype, const QString& wcname);
public:
	QString getRootDirpath();
	QString getRootName();
	QString getSaveDirpath();
	bool hasLoadData();
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
	void getResTypeInfo(ResType type, QString& name, QString& unit);
	/**
	* @brief 加载并分析数据文件
	*
	* 该函数负责从指定目录加载MAT格式的数据文件，解析传感器数据，并根据需要进行分段处理，
	* 最后创建图表绘制对象。
	*
	* @param dirPath 数据文件所在目录路径
	* @param allwcs 所有工况的映射表，用于验证数据文件是否有效
	* @param exdatas [out] 存储解析后的额外数据
	* @param charts [out] 存储创建的图表绘制对象
	* @param type 资源类型，决定如何处理数据
	* @return bool 是否成功加载和分析数据
	*/
	bool loadAnalyseDataFile(
		const QString& dirPath,
		const QMap<QString, WorkingConditions>& wc,
		QMap<QString, AnalyseData>& analyseData,
		ResType type
	);
	/**
	 * @brief 处理分段数据
	 *
	 * 如果需要分段处理，则计算每段数据的统计信息
	 *
	 * @param exdata [in/out] 要处理的额外数据
	 * @param wcName 工况名称
	 * @param segwcnames 需要分段的工况名称列表
	 * @param sensorNames 传感器名称列表
	 * @param sensorValid 传感器是否需要解析的标记位
	 */
	void processSegmentedData(
		ExtraData& exdata,
		const QString& wcName,
		const QStringList& segwcnames,
		const QStringList& sensorNames,
		const QStringList& sensorValid
	);

	//将各个维度的数据存到docx(非即时写入)
	bool saveAnalyseDataToDocx(
		QAxObject* doc,
		QAxObject* selection,
		const QString& titleSeq,
		const QString& titlename,
		const QString& unit,
		const QMap<QString, WorkingConditions>& wcs,
		QMap<QString, AnalyseData>& analyseData
	);
private:
	//辅助函数：纯定制，无通用性，只是为了方遍从一个rootDir中提取出文件夹名字为foldername的完整文件夹路径
	QString getFullPathFromDirByAppointFolder(const QString& foldername, QDir rootDir);
	void clearExtraData(ExtraData& extra);
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

};

//排序辅助函数，纯定制，且只能通过对比key的str转int后的升序排列
bool numericCompare(const QString& a, const QString& b);