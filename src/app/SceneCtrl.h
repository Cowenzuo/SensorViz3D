#pragma once

#include <QObject>

#include <osg/Node>

#include "ProjectData.h"
class SceneViewer;
class SceneCtrl :public QObject
{
	Q_OBJECT
public:
	SceneCtrl(SceneViewer* sv, QObject* parent=nullptr);
	~SceneCtrl();

	bool installSimRender(QVector<SensorPositon> sp,float max);

	bool uninstallSimRender();

private:
	SceneViewer* _sceneViewer;

	osg::ref_ptr<osg::Node> _model;

};

