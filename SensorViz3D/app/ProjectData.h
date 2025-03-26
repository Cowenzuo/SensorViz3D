#pragma once

#include <QObject>
#include <QMap>

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

struct FluctuationPressure
{
	QString wcname{ "" };			//从属于的工况名称
	int frequency{ 100 };			//采集频率
	int senseCount{ 0 };			//传感器数量
	int dataCount{ 0 };				//数据点数量
	QMap<QString, double*>data{};	//数据
};
class ProjectData : public QObject
{
	Q_OBJECT

public:
	ProjectData(QObject* parent);
	~ProjectData();

	bool loadDataPackage(const QString& dirPath);

private:
	// 读取工况数据并保存
	bool loadWorkingConditions(const QString& dirPath);
	// 读取脉动压力数据列表
	bool loadFluctuationPressure(const QString& dirPath);

private:
	QString _rootDirPath;
	QString _rootName;

	QMap<QString, WorkingConditions> _workingConditions;
	QMap<QString, FluctuationPressure> _fluctuationPressures;
};
