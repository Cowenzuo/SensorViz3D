#include "ReadWriteMatFile.h"

#include <cmath>
#include <memory>

#include <QScopeGuard>
#include <QDebug>

#include <mat.h>
#include <matrix.h>


bool RWMAT::readMatFile(
	const QString& filepath,
	const QStringList& sensorNames,
	const QStringList& sensorValid,
	int singleDataCols,
	RawData& fp)
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
	const size_t valueCols = mxGetN(datasArray);
	const size_t expectedCols = sensorNames.size() * singleDataCols;
	if (valueCols != expectedCols) {
		qWarning() << "Data columns mismatch. Expected:" << expectedCols
			<< "Actual:" << valueCols;
		return false;
	}

	// 5. 数据预处理
	const size_t valueRows = mxGetM(datasArray);
	const size_t removeSize = std::min<size_t>(fp.frequency * 5, valueRows / 2);
	fp.dataCount = valueRows - removeSize * 2;
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
		for (size_t row = removeSize, newIdx = 0; row < valueRows - removeSize; ++row, ++newIdx) {
			// 计算向量幅值
			double dataValue = 0.0;
			for (int col = 0; col < singleDataCols; ++col) {
				const double val = resValues[(i * singleDataCols + col) * valueRows + row];
				dataValue += val * val;
			}
			dataValue = std::sqrt(dataValue);

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