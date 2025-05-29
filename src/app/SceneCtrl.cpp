#include "SceneCtrl.h"

#include <osgDB/ReadFile>

#include "3dviewer/SceneViewer.h"

SceneCtrl::SceneCtrl(SceneViewer* sv, QObject* parent) :_sceneViewer(sv), QObject(parent)
{
}

SceneCtrl::~SceneCtrl()
{
}

bool SceneCtrl::installSimRender(QVector<SensorPositon> sp, float max)
{
	return false;
}

bool SceneCtrl::uninstallSimRender()
{
	return false;
}
