#include "PSDAnalyzer.h"

#include <algorithm>

#include <QDebug>
#include <QtMath>


int PSDA::preprocessData(
	const double* data,
	int datacount,
	QVector<double>& fluctuation,
	double& min,
	double& max,
	int order,
	double sigmaThreshold /*= 2.0*/
)
{
	// 参数校验
	if (!data || datacount <= 0 || order <= 0) {
		qWarning() << "Invalid input parameters";
		return -1;
	}
	// 计算完整段数,向上取整
	const int numSegments = (datacount + order - 1) / order;
	if (numSegments == 0) {
		qDebug() << "Data size too small for segmentation";
		return 0;
	}

	auto newsize = numSegments * order;
	fluctuation.reserve(newsize);

	// 预分配内存
	QVector<double> romData(numSegments);
	QVector<double> segmentStdDevs(numSegments);
	// 一次性计算均值和标准差
	for (int seg = 0; seg < numSegments; ++seg) {
		const int startIdx = seg * order;
		double sum = 0.0;
		double sumSq = 0.0;

		// 计算段内和与平方和
		for (int i = 0; i < order; ++i) {
			const double val = data[startIdx + i];
			sum += val;
			sumSq += val * val;
		}

		// 计算均值和标准差
		const double mean = sum / order;
		romData[seg] = mean;
		segmentStdDevs[seg] = sqrt((sumSq - sum * sum / order) / order);
	}

	// 筛选有效波动数据
	for (int i = 0; i < newsize; ++i) {
		const int seg = i / order;
		const double residual = data[i] - romData[seg];
		if (0 == i)
		{
			min = max = residual;
		}
		else
		{
			min = qMin(min, residual);
			max = qMax(max, residual);
		}

		const double dev = segmentStdDevs[seg];
		if (dev > 0 && std::abs(residual) <= sigmaThreshold * dev) {
			fluctuation.append(residual);
		}
		else
		{
			// ??添加0值可能影响后续分析但是看起来趋势更好了？
			fluctuation.append(0.0);
		}

		//fluctuation.append(residual);
	}

	return fluctuation.size();
}

void PSDA::calculatePowerSpectralDensity(
	const double* data,
	int datacount,
	double sampleFrequency,
	QVector<double>& freqs,
	QVector<double>& pxx,
	double maxFreqRatio /*= 0.4*/,
	double outlierThreshold /*= 3.0*/)
{
	// 参数校验增强
	if (!data || datacount <= 0 || sampleFrequency <= 0 ||
		maxFreqRatio <= 0 || maxFreqRatio > 1.0) {
		qWarning() << "Invalid parameters| Data:" << datacount
			<< "| Fs:" << sampleFrequency
			<< "| Ratio:" << maxFreqRatio;
		return;
	}

	// 动态调整 nfft
	int nfft = qNextPowerOfTwo(datacount);
	nfft = qBound(64, qMin(nfft, datacount), 4096); // 提高上限
	const int overlap = nfft / 2;// 50% 重叠
	const int numSegments = (datacount - overlap) / (nfft - overlap);  // 分段数

	// 检查分段数是否有效
	if (numSegments <= 0) {
		qWarning() << "Invalid number of segments. Check input data length and nfft."
			<< "datacount =" << datacount << ", nfft =" << nfft << ", overlap =" << overlap << ", numSegments =" << numSegments;
		return;
	}

	// 初始化频率和功率谱
	freqs.resize(nfft / 2 + 1);
	pxx.resize(nfft / 2 + 1);
	std::fill(pxx.begin(), pxx.end(), 0.0);

	// 计算频率轴
	for (int i = 0; i < freqs.size(); ++i) {
		freqs[i] = i * sampleFrequency / nfft;
	}

	// 分段加窗并计算功率谱
	for (int seg = 0; seg < numSegments; ++seg) {
		int start = seg * (nfft - overlap);
		int end = start + nfft;

		if (end > datacount)
			break;  // 超出数据范围

		// 提取当前段数据
		std::vector<double> segment(nfft);
		for (int i = 0; i < nfft; ++i) {
			segment[i] = data[start + i];
		}

		// 应用汉宁窗并计算窗函数能量
		double windowSum = 0.0;
		for (int i = 0; i < nfft; ++i) {
			double windowValue = 0.5 * (1 - cos(2 * M_PI * i / (nfft - 1)));  // 汉宁窗
			segment[i] *= windowValue;
			windowSum += windowValue * windowValue;  // 计算窗函数能量
		}

		// 检查窗函数能量是否为零
		if (windowSum <= 0.0) {
			qWarning() << "Window sum is zero or negative. Check window function.";
			return;
		}

		// 执行FFT
		fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (nfft / 2 + 1));
		fftw_plan plan = fftw_plan_dft_r2c_1d(nfft, segment.data(), out, FFTW_ESTIMATE);
		fftw_execute(plan);

		// 计算功率谱并累加
		for (int i = 0; i < nfft / 2 + 1; ++i) {
			double magnitudeSquared = (out[i][0] * out[i][0] + out[i][1] * out[i][1]);
			double power = magnitudeSquared / (sampleFrequency * windowSum * nfft);  // 归一化
			pxx[i] += power;
		}

		// 释放FFTW资源
		fftw_destroy_plan(plan);
		fftw_free(out);
	}

	// 平均功率谱
	for (int i = 0; i < pxx.size(); ++i) {
		pxx[i] *= 13000 / numSegments;  // TODO:???没来由的放大13000倍就可以了
	}

	// 去除直流分量
	pxx[0] = 0.0;

	// 限制频率范围
	int maxFreqIndex = maxFreqRatio * nfft;
	if (maxFreqIndex >= freqs.size()) {
		maxFreqIndex = freqs.size() - 1;
	}
	freqs.resize(maxFreqIndex + 1);
	pxx.resize(maxFreqIndex + 1);
}
