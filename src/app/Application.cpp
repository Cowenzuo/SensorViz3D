#include "Application.h"

#include <QIcon>
#include <QMenu>
#include <QDesktopWidget>
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
		+ QString(QScrollAreaStyle)
		+ QString(QCheckBoxStyle)
		+ QString(QLabelStyle)
		+ QString(QSliderStyle)
	);

	_projectData = new ProjectData();
	_mainWindow = new MainWindow();
	_chartsViewer = new ChartsViewer();
	connect(_chartsViewer, &ChartsViewer::wcSelectChangedSignal, _mainWindow, &MainWindow::wcSelectChangedSlot);
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

void Application::showMainWindow(bool isfirst)
{
	_mainWindow->show();
	if (isfirst)
	{
		_chartsViewer->show();
		_chartsViewer->resize(QSize(1600, 900));
		_mainWindow->resize(QSize(1600, 900));
		QRect screenGeometry = QApplication::desktop()->screenGeometry();
		int x = (screenGeometry.width() - _mainWindow->width()) / 2;
		int y = (screenGeometry.height() - _mainWindow->height()) / 2;
		_mainWindow->move(x, y);
		_chartsViewer->move(x, y);
		_chartsViewer->setVisible(false);
	}
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
