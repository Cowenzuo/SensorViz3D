#pragma once

#include <QWidget>
#include "ui_SceneViewer.h"

#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Uniform>
#include <osgQOpenGL/osgQOpenGLWidget>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osgUtil/CullVisitor>

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
	osg::ref_ptr<osg::Node> _modelDispmentPreNode{ };
	std::vector<osg::Geode*> _sensorNodes{ };//默认最大20个
	osg::ref_ptr<osg::MatrixTransform> _modelMtx{};
public:
	SceneViewer(QWidget* parent = nullptr);
	~SceneViewer();

	AutosizeosgQt* getOstQt() {
		return _osgWidget;
	}

	osg::Group* getRootNode() {
		return _rootNode.get();
	}

	osg::Node* getModelNode() {
		return _modelNode.get();
	}

	float getModelNodeRadius();

	void setSensorPos(osg::Vec3Array* pos);

	void showDisplacementPreModel(bool visible);
protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	void loadSkyBox();
	void loadLand(int xcount, int ycount, float xstep, float ystep);

	void setNodeTransparent(osg::Node* node);
private slots:
	void on3DInitialized();

};
//一个变换类，使天空盒绕视点旋转
class MoveEarthySkyWithEyePointTransform : public osg::Transform
{
public:
	//局部矩阵计算成世界矩阵
	virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
	{
		osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
		if (cv)
		{
			osg::Vec3 eyePointLocal = cv->getEyeLocal();
			matrix.preMult(osg::Matrix::translate(eyePointLocal));
		}
		return true;
	}

	//世界矩阵计算为局部矩阵
	virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
	{
		osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
		if (cv)
		{
			osg::Vec3 eyePointLocal = cv->getEyeLocal();
			matrix.postMult(osg::Matrix::translate(-eyePointLocal));
		}
		return true;
	}
};

//更新立方体图纹理
struct TexMatCallback : public osg::NodeCallback
{
public:

	TexMatCallback(osg::TexMat& tm) :
		_texMat(tm)
	{
		//
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
		if (cv)
		{
			//得到模型视图矩阵并设置旋转角度
			const osg::Matrix& MV = *(cv->getModelViewMatrix());
			const osg::Matrix R = osg::Matrix::rotate(osg::DegreesToRadians(112.0f), 0.0f, 0.0f, 1.0f) *
				osg::Matrix::rotate(osg::DegreesToRadians(90.0f), 1.0f, 0.0f, 0.0f);

			osg::Quat q = MV.getRotate();
			const osg::Matrix C = osg::Matrix::rotate(q.inverse());

			//设置纹理矩阵
			_texMat.setMatrix(C * R);
		}

		traverse(node, nv);
	}

	//纹理矩阵
	osg::TexMat& _texMat;
};