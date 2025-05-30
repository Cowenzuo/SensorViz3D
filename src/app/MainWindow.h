#pragma once
#include "ui/base/NativeBaseWindow.h"

#include "ProjectData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE
class SceneViewer;
class SceneViewerSettings;
class RendPlayer;
class SceneCtrl;

class MainWindow : public NativeBaseWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	virtual ~MainWindow();

	SceneViewer* getSceneViewer();

protected:
	void resizeEvent(QResizeEvent* event) override;
	void changeEvent(QEvent* event) override;
	bool hitTestCaption(const QPoint& pos) override;

public slots:
	void wcSelectChangedSlot(ResType type, QString wcname);

private slots:
	void headerMenuTriggered();
	void importDataPkgTriggered();
	void generateReportTriggered();
	void viewTableImgTriggered(bool open);
	void saveToLoaclTriggered();

	void handleTimestampChanged(int index);
	void handleWeightChanged(const QString& value);
	void handleMaxThresholdChanged(float value);
	void handleRadiationThresholdChanged(float value);
private:
	Ui::MainWindowClass* ui;

	SceneViewerSettings* _widgetSvs{ nullptr };
	RendPlayer* _widgetRp{ nullptr };

	SceneCtrl* _sceneCtrl{ nullptr };
	ResType _currentDimType{ ResType::FP };
	QString _currentWcname{""};
};
