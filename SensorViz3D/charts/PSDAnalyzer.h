#include <QVector>

#include <fftw3.h>
namespace PSDA {
	/*
		 * @brief 计算运行阶次均值(ROM)和有效波动数据
		 * @param data 输入数据数组
		 * @param datacount 数据点总数
		 * @param romData 输出的分段均值
		 * @param order 分段阶次
		 * @param sigmaThreshold 标准差阈值(默认2σ)
		 * @return 有效波动数据数量，-1表示错误
		 */
	bool preprocessData(
		const double* data,
		int datacount,
		QVector<double>& resData,
		QVector<double>& romData,
		QVector<double>& fluctuation,
		double& resmin,
		double& resmax,
		int order,
		double sigmaThreshold = 2.0
	);
	/**
	 * @brief 计算功率谱密度(PSD) Welch方法
	 * @param ydata 输入时域信号
	 * @param sampleFrequency 采样频率(Hz)
	 * @param freqs 输出频率向量(Hz)
	 * @param pxx 输出功率谱密度(kPa²/Hz)
	 * @param isLog 转换为dB/Hz
	 * @param windowType 窗函数类型(0:汉宁窗[默认])
	 * @param maxFreqRatio 最大显示频率比例(0-1)
	 * @param outlierThreshold 异常阈值(默认3σ)
	 */
	void calculatePowerSpectralDensity(
		const double* data,
		int datacount,
		double sampleFrequency,
		QVector<double>& freqs,
		QVector<double>& pxx,
		double maxFreqRatio = 0.5,
		double outlierThreshold = 3.0
	);
	/**
	* @brief 巴特沃斯高通滤波（二阶）
	* @param input 输入信号
	* @param output 输出信号
	* @param count 数据点数
	* @param sampleRate 采样率(Hz)
	* @param cutoffFreq 截止频率(Hz)
	*/
	void butterworthHighPass(
		const double* input,
		double* output,
		int count,
		double sampleRate,
		double cutoffFreq
	);
	/** @brief 去除线性趋势
	* @param data 输入信号
	* @param count 数据点数
	*/
	void detrendSignal(
		double* data,
		int count
	);
	/** @brief 主计算函数
	* @param acc 输入加速度数据
	* @param accCount 数据点数
	* @param sampleRate 采样率(Hz)
	* @param cutoffFreq 高通滤波截止频率(Hz)
	* @param disp 输出位移数据
	* @param dispCount 输出位移点数
	*/
	void calculateVD(
		const double* acc,
		int accCount,
		double sampleRate,
		double cutoffFreq,
		double* disp,
		int& dispCount
	);

} // namespace PSDA