#pragma once

#include <QWidget>
#include "ui_SceneViewer.h"

#include <osg/Node>
#include <osg/Group>
#include <osg/Uniform>
#include <osgQOpenGL/osgQOpenGLWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class SceneViewerClass; };
QT_END_NAMESPACE

class AutosizeosgQt : public osgQOpenGLWidget {
public:
	AutosizeosgQt(QWidget* parent = nullptr) : osgQOpenGLWidget(parent) {
	}
	QSize sizeHint() const override {
		return QSize(this->width(), this->height());
	}
};
class SceneViewer : public QWidget
{
	Q_OBJECT
private:
	Ui::SceneViewerClass* ui;
	AutosizeosgQt* _osgWidget;
	osg::ref_ptr<osg::Group> _rootNode{ new osg::Group };
	osg::ref_ptr<osg::Node> _modelNode{ };
public:
	SceneViewer(QWidget* parent = nullptr);
	~SceneViewer();

	AutosizeosgQt* getOstQt() {
		return _osgWidget;
	};
	osg::Group* getRootNode() {
		return _rootNode.get();
	}
	osg::Node* getModelNode() {
		return _modelNode.get();
	}

protected:
	void resizeEvent(QResizeEvent* event) override;

private slots:
	void on3DInitialized();

};
