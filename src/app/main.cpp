#include <iostream>

#include <QtWidgets/QApplication>
#include <QSurfaceFormat>

#include "Application.h"
#include "ProjectData.h"

void setupSurface()
{
	QSurfaceFormat format = QSurfaceFormat::defaultFormat();
	format.setVersion(4, 1);
	format.setProfile(QSurfaceFormat::CompatibilityProfile);
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setSamples(0);
	format.setDepthBufferSize(24);
	format.setAlphaBufferSize(8);
	format.setStencilBufferSize(8);
	// format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	// format.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char* argv[])
{
    Application app(argc, argv);  // 先创建应用对象

    // 参数校验（argv[0]是程序自身路径）
    if (argc != 1 && argc != 3) {
        std::cerr << "Usage: " << argv[0] << " [data_path export_path]" << std::endl;
        return 1;
    }

    if (argc >= 3) {
        // 带参数模式：加载数据包后启动
        setupSurface();
        ProjectData projectData(nullptr);
        projectData.setDataPackage(argv[1], argv[2], true);  // 使用命令行参数
        return 0;
    }
    // 无参数模式：直接启动界面
	setupSurface();
    app.showMainWindow(true);
    return app.exec();
}