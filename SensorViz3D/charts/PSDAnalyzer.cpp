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
		//fluctuation.append(residual);
		const double dev = segmentStdDevs[seg];
		if (dev > 0 && std::abs(residual) <= sigmaThreshold * dev) {
			fluctuation.append(residual);
		}
		else
		{
			fluctuation.append(0.0);// ??添加0值可能影响后续分析
		}
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
	pxx[0] = 0.0;
	// 限制频率范围
	int maxFreqIndex = maxFreqRatio * nfft;
	if (maxFreqIndex >= freqs.size()) {
		maxFreqIndex = freqs.size() - 1;
	}
	freqs.resize(maxFreqIndex + 1);
	pxx.resize(maxFreqIndex + 1);
}

bool PSDA::calculateWindowFunction(QVector<double>& window, double& windowEnergy, int windowType)
{
	const int n = window.size();
	if (n <= 0) {
		qWarning() << "Window size must be positive";
		return false;
	}

	// 校验窗类型
	if (windowType < 0 || windowType > 2) {
		qWarning() << "Invalid window type, using Hanning as default";
		windowType = 0;
	}

	double windowSum = 0.0;
	const bool isFlatTop = (windowType == 2);
	const double norm = (n > 1) ? 1.0 / (n - 1) : 1.0; // 防除零

	// 预计算公共参数
	double cos1, cos2, cos3, cos4;
	double* windowPtr = window.data();

	for (int i = 0; i < n; ++i) {
		const double theta = 2 * M_PI * i * norm;

		switch (windowType) {
		case 1: // 海明窗
			*windowPtr = 0.54 - 0.46 * cos(theta);
			break;

		case 2: // 平顶窗(优化计算顺序)
			cos1 = cos(theta);
			cos2 = cos(2 * theta);
			cos3 = cos(3 * theta);
			cos4 = cos(4 * theta);
			*windowPtr = 0.21557895
				- 0.41663158 * cos1
				+ 0.277263158 * cos2
				- 0.083578947 * cos3
				+ 0.006947368 * cos4;
			break;

		default: // 汉宁窗
			*windowPtr = 0.5 * (1 - cos(theta));
		}

		windowSum += (*windowPtr) * (*windowPtr);
		++windowPtr;
	}
	windowEnergy = sqrt(windowSum);
	return true;
}

// 辅助函数：异常值处理
void PSDA::processOutliers(
	QVector<double>& segmentPxx,
	double threshold,
	double minValidValue /*= 0.0*/
)
{
	const int n = segmentPxx.size();
	if (n < 5) return;  // 数据太少不处理

	// 1. 快速选择算法计算分位数（O(n)）
	QVector<double> copy = segmentPxx;
	const int q1_pos = n / 4;
	const int q3_pos = 3 * n / 4;

	std::nth_element(copy.begin(), copy.begin() + q1_pos, copy.end());
	const double q1 = copy[q1_pos];

	std::nth_element(copy.begin(), copy.begin() + q3_pos, copy.end());
	const double q3 = copy[q3_pos];

	const double iqr = q3 - q1;
	const double lowerBound = q1 - threshold * iqr;
	const double upperBound = q3 + threshold * iqr;

	// 2. 物理约束检查
	const double actualLowerBound = qMax(lowerBound, minValidValue);

	// 3. 异常值替换（带边缘处理）
	for (int i = 0; i < n; ++i) {
		if (segmentPxx[i] < actualLowerBound || segmentPxx[i] > upperBound) {
			// 边界处理
			if (i == 0) {
				segmentPxx[i] = segmentPxx[i + 1];  // 只使用后邻点
			}
			else if (i == n - 1) {
				segmentPxx[i] = segmentPxx[i - 1];  // 只使用前邻点
			}
			else {
				// 中值滤波避免连续异常影响
				const double median = std::min(segmentPxx[i - 1],
					std::max(segmentPxx[i + 1],
						(segmentPxx[i - 1] + segmentPxx[i + 1]) / 2));
				segmentPxx[i] = median;
			}

			// 确保不违反物理约束
			segmentPxx[i] = qMax(segmentPxx[i], minValidValue);
		}
	}
}
