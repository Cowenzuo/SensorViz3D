#include "Application.h"

#include <QIcon>
#include <QMenu>
#include <QTranslator>

#include "MainWindow.h"
#include "ChartsViewer.h"
#include "ProjectData.h"
#include "InlineStyleDef.h"

Application::Application(int& argc, char** argv) : QApplication(argc, argv)
{
	QApplication::setOrganizationDomain("cowenzuo12138.icu");
	QApplication::setOrganizationName("cowenzuo");
	QApplication::setApplicationName("SensorViz3D");
	QApplication::setApplicationDisplayName("实验平台");
	QApplication::setWindowIcon(QIcon(":/image/logo.png"));

	QTranslator* translator = new QTranslator(this);
	if (translator->load(QString("translations/qt_zh_CN.qm")))
	{
		installTranslator(translator);
	}

	setStyleSheet(
		QString(OverWriteStyle)
		+ QString(MenuStyle)
		+ QString(BaseDialogStyle)
		+ QString(MsgBoxStyle)
		+ QString(ComboBoxStyle)
	);

	_projectData = new ProjectData();
	_chartsViewer = new ChartsViewer();
	_mainWindow = new MainWindow();
}

Application::~Application()
{
	if (_mainWindow)
	{
		delete _mainWindow;
		_mainWindow = nullptr;
	}
	if (_projectData)
	{
		delete _projectData;
		_projectData = nullptr;
	}
}

Application* Application::instance()
{
	return static_cast<Application*>(QCoreApplication::instance());
}

void Application::showMainWindow()
{
	_mainWindow->show();
}

ProjectData* Application::getProjData()
{
	return _projectData;
}

MainWindow* Application::getMainWindow()
{
	return _mainWindow;
}

ChartsViewer* Application::getChartsViewer()
{
	return _chartsViewer;
}
