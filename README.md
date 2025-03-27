# SensorViz3D
传感器采集数据图表可视化以及将数据渲染到三维模型上

# 依赖库
1. Qt			5.12.12		可视化界面以及基本数据类型
2. QXslx		1.5.0		输出带格式的.xslx文件
3. QCustomPlots 2.1.1		可视化绘制频谱图以及本地化保存
4. MatLab		R2024b		解析.mat文件
5. OSG			3.6.5		三维渲染
6. libfftw3		3.3.10		谱分析所使用的傅里叶变换
> Qt使用模块：core;opengl;gui;widgets
> 
> Matlab使用模块：mat数据解析模块，未使用任何引擎计算模块（新版本Engine模块使用需要用License，比较麻烦）

# 工程目录结构
> 当前工程使用vs的sln解决方案管理并编译
> 后续会考虑转为CMake工程或QMake工程，只是感觉可能没必要
1. 3rdprty				里面只存在QXslx、QCustomPlots、libfftw3这三个较为轻量级的库
2. docs					文档
3. SensorViz3D			源代码
4. SensorViz3D.sln		solution
> 3rdprty没有放全东西，主要是因为Matlab、OSG、Qt的本体太过于庞大，所以关于这些的依赖环境需要自行配置，
> 
> 内部提供了宏定义的预设，因此在配置时需要将MatLab的安装程序路径写到环境变量里面去
> 
> Qt暂不需要，因为采用vs的sln方式，有qt-vs-addin插件的存在，已经全部都有了
> 
> 例：
> 1. MatLabInstallDir =D:/_softwaves/MATLAB
> 2. OSGInstallDir =D:/_softwaves/OSG

# 编译依赖环境设置
1. 头文件路径引用
“C/C++”-“常规”-“附加包含目录”：$(SolutionDir)3rdprty\include;$(MatLabInstallDir)\R2024b\extern\include;$(OSGInstallDir)\include;%(AdditionalIncludeDirectories)
1. 库文件路径应用
“链接器”-“常规”-“附加库目录”：$(SolutionDir)3rdprty\lib;$(MatLabInstallDir)\R2024b\extern\lib、win64\microsoft;$(OSGInstallDir)\lib;%(AdditionalLibraryDirectories)
1. 库文件
“链接器”-“输入”-“附加依赖项”：
```
		osg.lib
		osgDB.lib
		osgGA.lib
		osgViewer.lib
		osgQOpenGL.lib
		OpenThreads.lib
		opengl32.lib
		osgUtil.lib
		dwmapi.lib
		libmat.lib
		libmx.lib
		mclmcr.lib
		mclmcrrt.lib
		qcustomplot2.lib
		libfftw3-3.lib
		libfftw3f-3.lib
		libfftw3l-3.lib
		QXlsxQt5.lib
```
1. 编译指令
> 由于在VS环境下是使用的Loacl system编码，所以需要在C++编译指令那边显式指定文件为utf-8格式
> 
> “C/C++”-“命令行”-“其他选项”：/utf-8

# 可执行程序输出目录
VS工程配置属性里面的“常规”-“输出目录”设置有修改：
> $(SolutionDir)$(Platform)Bin-$(Configuration)\

# 可执行运行依赖环境设置
VS工程配置属性里面的“调试”-“环境”设置：（取消左下角的父级继承）
> PATH=$(QtDllPath);$(SolutionDir)3rdprty\bin;$(MatLabInstallDir)\R2024b\extern\bin\win64;$(MatLabInstallDir)\R2024b\bin\win64;$(OSGInstallDir)\bin%PATH%
> 
> 预设了MatLabInstallDir、OSGInstallDir的bin运行时库的引用

# release-with-debug(可选配置)
1. “C/C++”-“优化”-“优化”：已禁用(/Od)
1. “链接器”-“调试”-“生成调试信息”：是