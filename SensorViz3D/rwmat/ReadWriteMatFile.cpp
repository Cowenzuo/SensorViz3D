#include "ReadWriteMatFile.h"

#include <mat.h>
#include <matrix.h>


bool RWMAT::readMatFile(const QString& filepath, const QStringList& sensorNames, RawData& fp)
{
	MATFile* pmat = matOpen(filepath.toStdString().c_str(), "r");
	if (pmat == nullptr) {
		return false;
	}

	//由于数据有问题，这边先不读取这些字段，代码保留
#ifdef NoExist
	mxArray* datasArray = matGetVariable(pmat, "Datas");
	mxArray* dataCountArray = matGetVariable(pmat, "DataCount");
	mxArray* lineCountArray = matGetVariable(pmat, "LineCount");
	mxArray* sampleFrequencyArray = matGetVariable(pmat, "SampleFrequency");  // Hz
	mxArray* sampleStartTimeArray = matGetVariable(pmat, "SampleStartTime");
	mxArray* datasArray = matGetVariable(pmat, "Datas");
	if (dataCountArray == nullptr || lineCountArray == nullptr || sampleFrequencyArray == nullptr || sampleStartTimeArray == nullptr || datasArray == nullptr) {
		qDebug() << tr("读取 MAT 文件中的变量失败: %1").arg(fileName);
		matClose(pmat);
		return false;
	}
	char* dataCountStr = mxArrayToString(dataCountArray);
	char* lineCountStr = mxArrayToString(lineCountArray);
	char* sampleFrequencyStr = mxArrayToString(sampleFrequencyArray);
	char* sampleStartTime = mxArrayToString(sampleStartTimeArray);
	fp.senseCount = std::stoi(dataCountStr);
	fp.dataCount = std::stoi(lineCountStr);
	fp.frequency = std::stod(sampleFrequencyStr);
	fp.starttime = QDateTime::fromString(sampleStartTime, "yyyy/MM/dd HH:mm:ss");
#else
	mxArray* datasArray = matGetVariable(pmat, "Datas");
	if (datasArray == nullptr) {
		matClose(pmat);
		return false;
	}
	fp.startTime = QDateTime::currentDateTime();
	mxArray* sampleFrequencyArray = matGetVariable(pmat, "SampleFrequency");  // Hz
	if (sampleFrequencyArray)
	{
		char* sampleFrequencyStr = mxArrayToString(sampleFrequencyArray);
		fp.frequency = std::stod(sampleFrequencyStr);
	}
	else
	{
		fp.frequency = 100;
	}
#endif  // NoExist

	fp.senseCount = mxGetN(datasArray);
	if (fp.senseCount != sensorNames.count())
	{
		return false;
	}
	fp.dataCount = mxGetM(datasArray);
	double* datas = mxGetPr(datasArray);
	for (auto col = 0; col < fp.senseCount; col++) {
		double* data = new double[fp.dataCount];
		for (auto row = 0; row < fp.dataCount; row++) {
			data[row] = datas[col * fp.dataCount + row];
			//统计最大值、最小值、均方根
			if (row == 0) {
				fp.statistics[sensorNames[col]].max = data[row];
				fp.statistics[sensorNames[col]].min = data[row];
				fp.statistics[sensorNames[col]].rms = data[row] * data[row];
			}
			else {
				if (data[row] > fp.statistics[sensorNames[col]].max) {
					fp.statistics[sensorNames[col]].max = data[row];
				}
				if (data[row] < fp.statistics[sensorNames[col]].min) {
					fp.statistics[sensorNames[col]].min = data[row];
				}
				fp.statistics[sensorNames[col]].rms += data[row] * data[row];
			}
		}
		fp.statistics[sensorNames[col]].rms = sqrt(fp.statistics[sensorNames[col]].rms / fp.dataCount);
		fp.data[sensorNames[col]] = data;
	}

	// 释放资源
#ifdef NoExist
	mxFree(sampleStartTime);
	mxDestroyArray(dataCountArray);
	mxDestroyArray(lineCountArray);
	mxDestroyArray(sampleFrequencyArray);
	mxDestroyArray(sampleStartTimeArray);
#endif  // NoExist
	mxDestroyArray(datasArray);
	matClose(pmat);

	return true;
}
