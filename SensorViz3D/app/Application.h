#pragma once

#include <QApplication>

class MainWindow;
class ChartsViewer;
class ProjectData;

class Application : public QApplication
{
	Q_OBJECT

public:
	Application(int& argc, char** argv);
	virtual ~Application()override;

	static Application* instance();

	void showMainWindow(bool isfirst =false);

	ProjectData* getProjData();
	MainWindow* getMainWindow();
	ChartsViewer* getChartsViewer();
private:
	MainWindow* _mainWindow{};
	ChartsViewer* _chartsViewer{};
	ProjectData* _projectData{};
};
#define cApp Application::instance()