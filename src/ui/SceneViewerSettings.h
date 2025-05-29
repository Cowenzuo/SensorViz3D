#pragma once

#include <QWidget>
#include "ui_SceneViewerSettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SceneViewerSettingsClass; };
QT_END_NAMESPACE

class SceneViewerSettings : public QWidget
{
	Q_OBJECT

public:
	SceneViewerSettings(QWidget *parent = nullptr);
	~SceneViewerSettings();

	QString getCurrentWeight();
private:
	Ui::SceneViewerSettingsClass *ui;
};
