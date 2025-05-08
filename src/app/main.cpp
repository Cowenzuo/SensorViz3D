#include <QtWidgets/QApplication>
#include <QSurfaceFormat>

#include "Application.h"

//just for test
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
	setupSurface();

	Application app(argc, argv);
	app.showMainWindow(true);

	//just for test
	ProjectData projectData(nullptr);
	projectData.setDataPackage("../data/2025_03_22_Experiment", "../data/2025_03_22_Experiment/export", true);
	projectData.setDataPackage("../data/2025_03_23_Experiment", "../data/2025_03_23_Experiment/export", true);
	projectData.setDataPackage("../data/2025_03_24_Experiment", "../data/2025_03_24_Experiment/export", true);

	return app.exec();
}
