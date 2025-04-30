#pragma once
#include "ui/base/NativeBaseWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE
class ProjectData;

class MainWindow : public NativeBaseWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	virtual ~MainWindow();

protected:
	void changeEvent(QEvent* event) override;
	bool hitTestCaption(const QPoint& pos) override;

private slots:
	void headerMenuTriggered();

private slots:
	void importDataPkgTriggered();
	void generateReportTriggered();
	void viewTableImgTriggered(bool open);
	void saveToLoaclTriggered();

private:
	Ui::MainWindowClass* ui;
};
