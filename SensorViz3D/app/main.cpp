#include "MainWindow.h"
#include <QtWidgets/QApplication>

#include "ProjectData.h"

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	//just for test
	ProjectData projectData(nullptr);
	projectData.setDataPackage("../data/2025_03_22_Experiment", "../data/2025_03_22_Experiment/export", true);
	//projectData.save("../data/2025_03_22_Experiment");
	return a.exec();
}
