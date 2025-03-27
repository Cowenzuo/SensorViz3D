#include <QVector>

#include <fftw3.h>
namespace PSDA {
	// FFTW资源管理
	struct FFTWGuard {
		fftw_plan plan;
		fftw_complex* out;
		double* in;
		~FFTWGuard() {
			fftw_destroy_plan(plan);
			fftw_free(out);
			fftw_free(in);
		}
	};

	/*
		 * @brief 计算运行阶次均值(ROM)和有效波动数据
		 * @param data 输入数据数组
		 * @param datacount 数据点总数
		 * @param romData 输出的分段均值
		 * @param order 分段阶次
		 * @param sigmaThreshold 标准差阈值(默认2σ)
		 * @return 有效波动数据数量，-1表示错误
		 */
	int preprocessData(
		const double* data,
		int datacount,
		QVector<double>& fluctuation,
		double& min,
		double& max,
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
	// 辅助函数：窗函数计算
	bool calculateWindowFunction(
		QVector<double>& window,
		double& windowEnergy,
		int windowType
	);
	// 辅助函数：异常值处理
	void processOutliers(
		QVector<double>& segmentPxx,
		double threshold,
		double minValidValue = 0.0
	);
} // namespace PSDA