#include "ReadWriteMatFile.h"

#include <mat.h>
#include <matrix.h>


bool RWMAT::readMatFile(
	const QString& filepath,
	const QStringList& sensorNames,
	const QStringList& sensorValid,
	int singleDataCols,
	RawData& fp
)
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

	quint64 valueCols = mxGetN(datasArray);
	if (valueCols != sensorNames.count() * singleDataCols)
	{
		return false;
	}
	fp.senseCount = sensorNames.count();
	quint64 valueRows = mxGetM(datasArray);
	quint64 removeSize = fp.frequency * 5;//前后各扔掉5秒
	fp.dataCount = valueRows - removeSize * 2;

	double* datas = mxGetPr(datasArray);
	for (auto seq = 0; seq < sensorNames.count(); ++seq) {
		if (sensorValid[seq] != "1")
		{
			fp.senseCount--;
			continue;
		}

		double* data = new double[fp.dataCount];
		for (auto row = removeSize; row < valueRows - removeSize; row++) {
			double dataValue = 0.0;
			for (auto col = 0; col < singleDataCols; col++)
			{
				dataValue += pow(datas[(seq * singleDataCols + col) * valueRows + row], 2);
			}
			int newseq = row - removeSize;
			data[newseq] = sqrt(dataValue);

			//统计最大值、最小值、均方根
			if (row == 0) {
				fp.statistics[sensorNames[seq]].max = data[newseq];
				fp.statistics[sensorNames[seq]].min = data[newseq];
				fp.statistics[sensorNames[seq]].rms = data[newseq] * data[newseq];
			}
			else {
				if (data[newseq] > fp.statistics[sensorNames[seq]].max) {
					fp.statistics[sensorNames[seq]].max = data[newseq];
				}
				if (data[newseq] < fp.statistics[sensorNames[seq]].min) {
					fp.statistics[sensorNames[seq]].min = data[newseq];
				}
				fp.statistics[sensorNames[seq]].rms += data[newseq] * data[newseq];
			}
		}
		fp.statistics[sensorNames[seq]].rms = sqrt(fp.statistics[sensorNames[seq]].rms / fp.dataCount);
		fp.data[sensorNames[seq]] = data;
	}

	// 释放资源
#ifdef NoExist
	mxFree(sampleStartTime);
	mxDestroyArray(dataCountArray);
	mxDestroyArray(lineCountArray);
	mxDestroyArray(sampleFrequencyArray);
	mxDestroyArray(sampleStartTimeArray);
#endif  // NoExist
	if (sampleFrequencyArray)
	{
		mxDestroyArray(sampleFrequencyArray);
	}
	mxDestroyArray(datasArray);
	matClose(pmat);

	return true;
}
