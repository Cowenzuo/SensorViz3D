#include "ReadWriteMatFile.h"

#include <cmath>
#include <memory>

#include <QScopeGuard>
#include <QDebug>

#include <mat.h>
#include <matrix.h>


bool RWMAT::readMatFile(
	RawData& fp,
	const QString& filepath,
	const QStringList& sensorNames,
	const QStringList& sensorValid,
	double minValue,
	double maxValue,
	ResType type
)
{
	// 1. 文件打开与基础校验
	MATFile* pmat = matOpen(filepath.toUtf8().constData(), "r");
	if (!pmat) {
		qWarning() << "Failed to open MAT file:" << filepath;
		return false;
	}

	// 使用智能指针管理MAT资源
	auto matCloser = qScopeGuard([&]() { matClose(pmat); });

	// 2. 读取数据数组
	mxArray* datasArray = matGetVariable(pmat, "Datas");
	if (!datasArray) {
		qWarning() << "Variable 'Datas' not found in MAT file";
		return false;
	}
	auto arrayDeleter = qScopeGuard([&]() { mxDestroyArray(datasArray); });

	// 3. 读取采样频率（带默认值）
	fp.frequency = 100; // 默认值
	mxArray* sampleFrequencyArray = matGetVariable(pmat, "SampleFrequency");
	if (sampleFrequencyArray) {
		auto freqDeleter = qScopeGuard([&]() { mxDestroyArray(sampleFrequencyArray); });
		if (char* freqStr = mxArrayToString(sampleFrequencyArray)) {
			auto strDeleter = qScopeGuard([&]() { mxFree(freqStr); });
			bool ok = false;
			fp.frequency = QString(freqStr).toDouble(&ok);
			if (!ok) {
				qWarning() << "Invalid SampleFrequency format, using default 100Hz";
				fp.frequency = 100;
			}
		}
	}

	// 4. 数据维度校验
	int singleDataCols = 1;
	if (type == ResType::HC)
	{
		singleDataCols = 2;
	}
	const size_t valueCols = mxGetN(datasArray);
	const size_t expectedCols = sensorNames.size() * singleDataCols;
	if (valueCols != expectedCols) {
		qWarning() << "Data columns mismatch. Expected:" << expectedCols
			<< "Actual:" << valueCols;
		return false;
	}

	// 5. 数据预处理(固定前后各丢弃五秒数据fp.frequency * 5，同时在末尾再移除不能被频率及分段频率整除的部分redundancy)
	const size_t valueRows = mxGetM(datasArray);
	int redundancy = valueRows % (fp.frequency * SEGMENT_COUNT);
	const size_t removeSize = std::min<size_t>(fp.frequency * 5, valueRows / 2);
	fp.dataCount = (int)valueRows - ((int)removeSize * 2) - redundancy;
	fp.startTime = QDateTime::currentDateTime();
	fp.senseCount = 0;

	// 6. 数据指针获取
	double* resValues = mxGetPr(datasArray);
	if (!resValues) {
		qWarning() << "Failed to get data pointer";
		return false;
	}

	// 7. 传感器数据处理
	for (int i = 0; i < sensorNames.size(); ++i) {
		if (sensorValid[i] != "1") {
			continue;
		}

		// 7.1 分配数据内存
		std::unique_ptr<double[]> newdata(new double[fp.dataCount]);
		Statistics stats;
		bool statsInitialized = false;

		// 7.2 处理每行数据
		auto startSeq = removeSize;
		auto endSeq = valueRows - removeSize - redundancy;
		for (size_t row = removeSize, newIdx = 0; row < endSeq; ++row, ++newIdx) {
			// 计算向量幅值
			double dataValue = 0.0;
			if (type == ResType::HC)
			{
				//预先计算
				// 油缸内径650mm，活塞杆直径360mm
				// 无杆腔做功面积=π*(650/2)²= 331830.718375mm²
				// 有杆腔做功面积=π*(650/2)²-π*(360/2)²=230043.118135mm²
				// 由于油压单位是MPa，面积单位是mm²,结果为N，转为KN需要除以1000
				// 最终结果单位是吨，需要给KN除以9.80665
				// 无杆腔计算因子=331830.718375/9806.65 = 33.83731
				// 有杆腔计算因子=230043.118135/9806.65	= 23.45787
				double val0 = resValues[(i + 0) * valueRows + row] * 33.83731;//resValues是#1 #2 无杆腔油压
				double val1 = resValues[(i + 2) * valueRows + row] * 23.45787;//resValues是#3 #4 有杆腔油压
				dataValue = std::abs(val0 - val1);
			}
			else
			{
				if (1 == singleDataCols)
				{
					dataValue = resValues[(i * singleDataCols) * valueRows + row];
				}
				else
				{
					for (int col = 0; col < singleDataCols; ++col) {
						double val = resValues[(i * singleDataCols + col) * valueRows + row];
						dataValue += val * val;
					}
					dataValue = std::sqrt(dataValue);
				}
			}

			if (dataValue<minValue || dataValue>maxValue)
			{
				dataValue = 0.0;
			}
			// 存储数据
			newdata[newIdx] = dataValue;

			// 更新统计信息
			if (!statsInitialized) {
				stats = { dataValue, dataValue, dataValue * dataValue };
				statsInitialized = true;
			}
			else {
				stats.max = std::max(stats.max, dataValue);
				stats.min = std::min(stats.min, dataValue);
				stats.rms += dataValue * dataValue;
			}
		}

		// 7.3 最终统计计算
		if (statsInitialized) {
			stats.rms = std::sqrt(stats.rms / fp.dataCount);
			fp.statistics[sensorNames[i]] = stats;
			fp.data[sensorNames[i]] = newdata.release();
			++fp.senseCount;
		}
	}
	return fp.senseCount > 0;
}