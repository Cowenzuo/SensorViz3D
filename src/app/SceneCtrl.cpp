#include "SceneCtrl.h"

#include <osgDB/ReadFile>
#include <osg/ShapeDrawable>

#include "3dviewer/SceneViewer.h"

SceneCtrl::SceneCtrl(SceneViewer* sv, QObject* parent) :_sceneViewer(sv), QObject(parent)
{
}

SceneCtrl::~SceneCtrl()
{
}

bool SceneCtrl::installSimRender(QVector<SensorPositon> sp)
{
	if (!_model.valid())
		_model = _sceneViewer->getModelNode();
	if (!_model.valid())
		return false;
	auto radius = _model->computeBound().radius();
	auto pStateSet = _model->getOrCreateStateSet();
	if (!_simRender.valid())
	{
		_simRender = new osg::Program;
		_simRender->addShader(new osg::Shader(osg::Shader::VERTEX, SHADER_VERT));
		_simRender->addShader(new osg::Shader(osg::Shader::FRAGMENT, SHADER_FRAG));
		osg::ref_ptr<osg::Uniform> valueNumUniform = new osg::Uniform(osg::Uniform::INT, "uValueNum");//有效点的数量
		osg::ref_ptr<osg::Uniform> posUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "uPositions", 20);//最大20个，不做动态的了，要和shader里面配对!
		osg::ref_ptr<osg::Uniform> valuesUniform = new osg::Uniform(osg::Uniform::FLOAT, "uValues", 20);
		osg::ref_ptr<osg::Uniform> dsUniform = new osg::Uniform(osg::Uniform::FLOAT, "uDispScale");
		osg::ref_ptr<osg::Uniform> wUniform = new osg::Uniform(osg::Uniform::INT, "uWeight");
		osg::ref_ptr<osg::Uniform> dUniform = new osg::Uniform(osg::Uniform::INT, "uDisplacement");
		osg::ref_ptr<osg::Uniform> mvUniform = new osg::Uniform(osg::Uniform::FLOAT, "uMinValue");
		osg::ref_ptr<osg::Uniform> rvUniform = new osg::Uniform(osg::Uniform::FLOAT, "uRangeValue");
		valueNumUniform->set(0);
		dsUniform->set(1.0f);
		wUniform->set(0);
		dUniform->set(0);
		for (int i = 0;i < 20;++i)
		{
			posUniform->setElement(i, osg::Vec3f(0.0f, 0.0f, 0.0f));
			valuesUniform->setElement(i, 0.0f);
		}

		pStateSet->addUniform(valueNumUniform.get());
		pStateSet->addUniform(posUniform.get());
		pStateSet->addUniform(valuesUniform.get());
		pStateSet->addUniform(dsUniform.get());
		pStateSet->addUniform(wUniform.get());
		pStateSet->addUniform(dUniform.get());
		pStateSet->addUniform(mvUniform.get());
		pStateSet->addUniform(rvUniform.get());
	}
	pStateSet->getUniform("uValueNum")->set(sp.count());
	auto vuniform = pStateSet->getUniform("uPositions");
	osg::Vec3Array* pos = new osg::Vec3Array;
	for (int i = 0; i < sp.count(); i++)
	{
		vuniform->setElement(i, osg::Vec3f(float(sp[i].x), float(sp[i].y), float(sp[i].z)));

		pos->push_back(osg::Vec3(sp[i].x, sp[i].y, sp[i].z));
		//osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		//geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(float(sp[i].x), float(sp[i].y), float(sp[i].z)), 0.1f)));
		//_sceneViewer->getRootNode()->addChild(geode.get());
	}
	_sceneViewer->setSensorPos(pos);
	pStateSet->setAttributeAndModes(_simRender, osg::StateAttribute::ON);
	return true;
}

bool  SceneCtrl::updateSimValues(QVector<float> values)
{
	if (!_simRender.valid() || !_model.valid())
		return false;

	auto pStateSet = _model->getOrCreateStateSet();
	int valueNum = 0;
	if (!(pStateSet->getUniform("uValueNum")->get(valueNum)))
		return false;
	if (valueNum != values.count())
		return false;
	auto vuniform = pStateSet->getUniform("uValues");
	for (int i = 0; i < valueNum; i++)
	{
		vuniform->setElement(i, values[i]);
	}
	return true;
}

bool SceneCtrl::updateDispmentScaled(float value)
{
	if (!_simRender.valid() || !_model.valid())
		return false;

	auto pStateSet = _model->getOrCreateStateSet();
	auto dsuniform = pStateSet->getUniform("uDispScale");
	dsuniform->set(value);
	return true;
}

bool SceneCtrl::updateWeight(int value)
{
	if (!_simRender.valid() || !_model.valid())
		return false;

	auto pStateSet = _model->getOrCreateStateSet();
	auto wuniform = pStateSet->getUniform("uWeight");
	wuniform->set(value);
	return true;
}

bool SceneCtrl::updateDisplacement(int value)
{
	if (!_simRender.valid() || !_model.valid())
		return false;

	auto pStateSet = _model->getOrCreateStateSet();
	auto duniform = pStateSet->getUniform("uDisplacement");
	duniform->set(value);
	return true;
}

bool SceneCtrl::updateDisplacementRange(float min, float range)
{
	if (!_simRender.valid() || !_model.valid())
		return false;

	auto pStateSet = _model->getOrCreateStateSet();
	auto mvUniform = pStateSet->getUniform("uMinValue");
	auto rvUniform = pStateSet->getUniform("uRangeValue");
	mvUniform->set(min);
	rvUniform->set(range);
	return true;
}

bool SceneCtrl::uninstallSimRender()
{
	if (!_simRender.valid() || !_model.valid())
		return false;
	auto pStateSet = _model->getOrCreateStateSet();
	pStateSet->getUniform("uValueNum")->set(0);
	return true;
}
