#pragma once
#pragma once

#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QObject>
#include <qaxobject.h>
#include <QString>

#define WORKING_CONDITIONS_LINE_COUNT 10

#define NORMAL_FS_TS_IMAGE_WIDTH 450
#define NORMAL_FS_TS_IMAGE_HEIGHT 450
class BaseChart;
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
	// 读取振动加速度数据列表
	// 读取脉动压力数据列表
	enum class ResType {
		FP,
		VA,
		VD,
		Strain,
		OP
	};
	bool loadResFile(const QString& dirPath, ResType type);

	bool initWordDocment();

	enum class ParagraphFormat {
		TextBody,          // 正文
		ChartCaption,      // 图表上下标
		Level1Heading,     // 1级标题
		Level2Heading,     // 2级标题
		Level3Heading      // 3级标题
	};
	//设置通用正文、标题字体布局样式的统一接口
	void setNormalSelectionStyle(ParagraphFormat pf);
	//添加表、图统一接口
	void addCaption(const QString& text, bool isTable = false);

private:
	bool saveWorkingConditionsToDocx();
	bool saveFluctuationPressureToDocx();
	bool saveVibrationAccelerationToDocx();
	bool saveVibrationDisplacementToDocx();
	bool saveStrainToDocx();
	bool saveOilPressureToDocx();
private:
	QAxObject* createEigenvalueTable(WorkingConditionsList wcs, const QStringList& sensorsNames);
	QAxObject* createSegEigenvalueTable(WorkingConditions wc, QStringList& wcsSeg, const QStringList& sensorsNames);
	void fillTableDataCell(QAxObject* table, int row, int col, const QString& text, bool centerAlign);
	void fillTableHeaderCell(QAxObject* table, int row, int col, const QString& text);
	void mergeCells(QAxObject* table, int row1, int col1, int row2, int col2);
	void skipTable();

	void insertImage(const QString& imagePath, int width, int height);
private:
	QString _rootDirPath;
	QString _rootName;

	QMap<QString, WorkingConditions> _workingConditions;

	QMap<QString, ExtraData> _fpData;		//脉动压力数据<工况名,数据>
	QMap<QString, BaseChart*> _fpCharts;	//脉动压力图表<工况名,图表>

	QMap<QString, ExtraData> _vaData;		//振动加速度数据<工况名,数据>
	QMap<QString, BaseChart*> _vaCharts;	//振动加速度图表<工况名,图表>

	QMap<QString, ExtraData> _vdData;		//振动位移数据<工况名,数据>
	QMap<QString, BaseChart*> _vdCharts;	//振动位移图表<工况名,图表>

	QMap<QString, ExtraData> _strainData;		//应变数据<工况名,数据>
	QMap<QString, BaseChart*> _strainCharts;	//应变图表<工况名,图表>

	QMap<QString, ExtraData> _opData;		//油压数据<工况名,数据>
	QMap<QString, BaseChart*> _opCharts;	//油压图表<工况名,图表>

	QAxObject* _wordWriter{};
	QAxObject* _wordDocument{};
	QAxObject* _wordSelection{};
};

bool numericCompare(const QString& a, const QString& b);