#include "PSDAnalyzer.h"

#include <algorithm>

#include <QDebug>
#include <QtMath>


bool PSDA::preprocessData(
	const double* data,
	int datacount,
	QVector<double>& romData,
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
		return false;
	}
	// 计算完整段数,向下取整，多的直接不要了（不过在完整处理逻辑上，前面读取原始数据的时候就已经按order采样频率取整了）
	const int numSegments = datacount / order;
	if (numSegments == 0) {
		qDebug() << "Data size too small for segmentation";
		return false;
	}

	romData.resize(numSegments);
	fluctuation.resize(numSegments * order);

	// 预分配内存
	//QVector<double> romData(numSegments);
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

		const double mean = sum / order;
		romData[seg] = mean;
		segmentStdDevs[seg] = sqrt((sumSq - sum * sum / order) / order);
		if (0 == seg)
		{
			min = max = mean;
		}
		else
		{
			min = qMin(min, mean);
			max = qMax(max, mean);
		}
	}

	// 筛选有效波动数据
	for (int i = 0; i < fluctuation.count(); ++i) {
		const int seg = i / order;
		double residual = data[i] - romData[seg];
		const double dev = segmentStdDevs[seg];
		if (dev < 0 || std::abs(residual) > sigmaThreshold * dev) {
			residual = romData[seg];
		}

		fluctuation[i] = residual;
	}
	return true;
}

/**
 * @brief 计算功率谱密度(PSD)
 *
 * 使用Welch方法计算输入信号的功率谱密度，包括分段、加窗、FFT和平均处理。
 *
 * @param data 输入数据数组
 * @param datacount 数据点数
 * @param sampleFrequency 采样频率(Hz)
 * @param freqs [out] 输出频率向量(Hz)
 * @param pxx [out] 输出功率谱密度向量
 * @param maxFreqRatio 最大频率比例(0-1)，默认为0.4(即奈奎斯特频率的40%)
 * @param outlierThreshold 异常值阈值(未使用，保留供未来扩展)
 */
void PSDA::calculatePowerSpectralDensity(
	const double* data,
	int datacount,
	double sampleFrequency,
	QVector<double>& freqs,
	QVector<double>& pxx,
	double maxFreqRatio /*= 0.4*/,
	double outlierThreshold /*= 3.0*/)
{
	// 1. 参数校验
	if (!data || datacount <= 0 || sampleFrequency <= 0 || maxFreqRatio <= 0 || maxFreqRatio > 1.0) {
		qWarning() << "Invalid parameters in calculatePowerSpectralDensity:"
			<< "Data:" << datacount
			<< "| Fs:" << sampleFrequency
			<< "| Ratio:" << maxFreqRatio;
		return;
	}

	// 2. 确定FFT点数(优化性能与分辨率平衡)
	int nfft = qNextPowerOfTwo(datacount);
	nfft = qBound(256, qMin(nfft, datacount), 4096); // 更合理的范围限制
	const int overlap = nfft / 2; // 50%重叠
	const int numSegments = qMax(1, (datacount - overlap) / (nfft - overlap)); // 确保至少1段

	if (numSegments <= 0) {
		qCritical() << "Invalid segment calculation - datacount:" << datacount
			<< "nfft:" << nfft << "overlap:" << overlap;
		return;
	}

	// 3. 初始化输出向量
	const int outputSize = nfft / 2 + 1;
	freqs.resize(outputSize);
	pxx.resize(outputSize);
	std::fill(pxx.begin(), pxx.end(), 0.0);

	// 4. 预计算频率轴
	const double freqStep = sampleFrequency / nfft;
	for (int i = 0; i < outputSize; ++i) {
		freqs[i] = i * freqStep;
	}

	// 5. 分段处理
	double windowEnergySum = 0.0; // 用于归一化的窗函数能量总和
	std::vector<double> window(nfft); // 预计算窗函数
	for (int i = 0; i < nfft; ++i) {
		window[i] = 0.5 * (1 - cos(2 * M_PI * i / (nfft - 1))); // 汉宁窗
		windowEnergySum += window[i] * window[i];
	}
	const double scaleFactor = 1.0 / (sampleFrequency * windowEnergySum * nfft);

	// 6. 主处理循环
	for (int seg = 0; seg < numSegments; ++seg) {
		const int start = seg * (nfft - overlap);
		const int end = start + nfft;

		if (end > datacount) break;

		// 6.1 准备FFT输入数据(应用窗函数)
		std::vector<double> segment(nfft);
		for (int i = 0; i < nfft; ++i) {
			segment[i] = data[start + i] * window[i];
		}

		// 6.2 执行FFT
		fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * outputSize);
		fftw_plan plan = fftw_plan_dft_r2c_1d(nfft, segment.data(), out, FFTW_ESTIMATE);
		fftw_execute(plan);

		// 6.3 计算并累加功率谱
		for (int i = 0; i < outputSize; ++i) {
			const double real = out[i][0], imag = out[i][1];
			pxx[i] += (real * real + imag * imag) * scaleFactor;
		}

		// 6.4 清理FFTW资源
		fftw_destroy_plan(plan);
		fftw_free(out);
	}

	// 7. 后处理
	// 7.1 平均各段结果
	const double avgScale = 1.0 / numSegments;
	for (int i = 0; i < outputSize; ++i) {
		pxx[i] *= avgScale;
	}

	// 7.2 去除直流分量
	for (size_t i = 0; i < 3; i++)
	{
		pxx[i] = 0.0;
	}

	// 7.3 应用经验缩放因子(只是为了放大Y值，让画面更好看点)
	//const double empiricalScale = 10000.0; 
	//for (int i = 0; i < outputSize; ++i) {
	//    pxx[i] *= empiricalScale;
	//}

	// 7.4 限制输出频率范围
	const int maxFreqIndex = qMin(static_cast<int>(maxFreqRatio * outputSize), outputSize - 1);
	freqs.resize(maxFreqIndex + 1);
	pxx.resize(maxFreqIndex + 1);
}