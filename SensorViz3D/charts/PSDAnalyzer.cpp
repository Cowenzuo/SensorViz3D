#include "PSDAnalyzer.h"

#include <algorithm>

#include <QDebug>
#include <QtMath>


bool PSDA::preprocessData(
	const double* data,
	int datacount,
	QVector<double>& resData,
	QVector<double>& romData,
	QVector<double>& fluctuation,
	double& resmin,
	double& resmax,
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

	resData.resize(numSegments * order);
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
			resData[startIdx + i] = val; // 直接赋值到resData
			if (0 == seg)
			{
				resmin = resmax = val;
			}
			else
			{
				resmin = qMin(resmin, val);
				resmax = qMax(resmax, val);
			}
			sum += val;
			sumSq += val * val;
		}

		const double mean = sum / order;
		romData[seg] = mean;
		segmentStdDevs[seg] = sqrt((sumSq - sum * sum / order) / order);
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

void PSDA::butterworthHighPass(const double* input, double* output, int count, double sampleRate, double cutoffFreq)
{
	const double nyquist = 0.5 * sampleRate;
	const double normalCutoff = cutoffFreq / nyquist;
	const double sqrt2 = 1.4142135623730951;

	// 滤波器系数
	double b0 = 1.0 / (1.0 + sqrt2 * normalCutoff + normalCutoff * normalCutoff);
	double b1 = -2.0 * b0;
	double b2 = b0;
	double a1 = 2.0 * (normalCutoff * normalCutoff - 1.0) * b0;
	double a2 = (1.0 - sqrt2 * normalCutoff + normalCutoff * normalCutoff) * b0;

	// 前向滤波
	std::vector<double> forward(count);
	double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
	for (int i = 0; i < count; ++i) {
		forward[i] = b0 * input[i] + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
		x2 = x1;
		x1 = input[i];
		y2 = y1;
		y1 = forward[i];
	}

	// 反向滤波（零相位）
	x1 = x2 = y1 = y2 = 0.0;
	for (int i = count - 1; i >= 0; --i) {
		output[i] = b0 * forward[i] + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
		x2 = x1;
		x1 = forward[i];
		y2 = y1;
		y1 = output[i];
	}
}

void PSDA::detrendSignal(double* data, int count)
{
	double sum = 0.0;
	for (int i = 0; i < count; ++i) {
		sum += data[i];
	}
	double mean = sum / count;

	for (int i = 0; i < count; ++i) {
		data[i] -= mean;
	}
}

void PSDA::calculateVD(
	const double* acc, 
	int accCount, 
	double sampleRate, 
	double cutoffFreq, 
	double* disp, 
	int& dispCount
)
{
	if (accCount < 10 || !acc || !disp) {
		dispCount = 0;
		return;
	}

	// 中间缓冲区
	std::vector<double> processed(accCount);
	std::vector<double> velocity(accCount);

	// 1. 去趋势
	std::copy(acc, acc + accCount, processed.begin());
	detrendSignal(processed.data(), accCount);

	// 2. 高通滤波
	butterworthHighPass(processed.data(), processed.data(), accCount, sampleRate, cutoffFreq);

	// 3. 第一次积分：加速度→速度
	double dt = 1.0 / sampleRate;
	velocity[0] = 0.0;
	for (int i = 1; i < accCount; ++i) {
		velocity[i] = velocity[i - 1] + (processed[i] + processed[i - 1]) * 0.5 * dt;
	}

	// 4. 二次积分：速度→位移
	disp[0] = 0.0;
	for (int i = 1; i < accCount; ++i) {
		disp[i] = disp[i - 1] + (velocity[i] + velocity[i - 1]) * 0.5 * dt;
	}

	// 5. 后处理：去除积分漂移
	detrendSignal(disp, accCount);
	dispCount = accCount;
}