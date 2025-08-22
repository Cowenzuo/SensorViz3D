#include "SceneViewer.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QHBoxLayout>

#include <osg/ComputeBoundsVisitor>
#include <osg/MatrixTransform> 
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/Depth>
#include <osg/TextureCubeMap>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/LineWidth>
#include <osg/PolygonOffset>

#include "CameraManipulator.h"

SceneViewer::SceneViewer(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::SceneViewerClass())
{
	ui->setupUi(this);

	_osgWidget = new AutosizeosgQt();
	_osgWidget->setObjectName("osgWidget");
	_osgWidget->setFocusPolicy(Qt::StrongFocus);
	connect(_osgWidget, &AutosizeosgQt::initialized, this, &SceneViewer::on3DInitialized);

	QHBoxLayout* pHLayout = new QHBoxLayout();
	pHLayout->addWidget(_osgWidget);
	pHLayout->setSpacing(0);
	pHLayout->setMargin(0);
	setLayout(pHLayout);
}

SceneViewer::~SceneViewer()
{
	delete ui;
}

float SceneViewer::getModelNodeRadius()
{
	if (_modelMtx.valid())
	{
		return _modelMtx->computeBound().radius();
	}
	return 0.0f;
}

void SceneViewer::setSensorPos(osg::Vec3Array* pos)
{
	int sensorSize = _sensorNodes.size();
	for (int i = 0; i < sensorSize; i++)
	{
		_modelMtx->removeChild(_sensorNodes[i]);
	}

	int posSize = pos->size();
	for (int i = 0; i < posSize; i++)
	{
		auto sensorNode = new osg::ShapeDrawable(new osg::Sphere(pos->at(i), 0.05f));
		sensorNode->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));
		osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		geode->addDrawable(sensorNode);
		_sensorNodes.push_back(geode.get());
		_modelMtx->addChild(geode.get());
	}
}

void SceneViewer::showDisplacementPreModel(bool visible)
{
	if (!_modelDispmentPreNode.valid())
	{
		return;
	}
	_modelDispmentPreNode->setNodeMask(visible ? 1 : 0);
}

void SceneViewer::resizeEvent(QResizeEvent* event)
{
	if (!_osgWidget || !_osgWidget->getOsgViewer() || !_osgWidget->getOsgViewer()->getCamera()) {
		return;
	}
	float aspectRatio = (_osgWidget->sizeHint().width() * 1.0) / _osgWidget->sizeHint().height();
	_osgWidget->getOsgViewer()->getCamera()->setProjectionMatrixAsPerspective(30.f, aspectRatio, 1.f, 100000.f);
}

void SceneViewer::loadSkyBox()
{
	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();

	//设置纹理映射方式，指定为替代方式，即纹理中的颜色代替原来的颜色
	osg::ref_ptr<osg::TexEnv> te = new osg::TexEnv;
	te->setMode(osg::TexEnv::REPLACE);
	stateset->setTextureAttributeAndModes(0, te.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	//自动生成纹理坐标，反射方式(REFLECTION_MAP)
	/*
	NORMAL_MAP　标准模式－立方图纹理
	REFLECTION_MAP　反射模式－球体纹理
	SPHERE_MAP　球体模型－球体纹理
	*/
	osg::ref_ptr<osg::TexGen> tg = new osg::TexGen;
	tg->setMode(osg::TexGen::NORMAL_MAP);
	stateset->setTextureAttributeAndModes(0, tg.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	//设置纹理矩阵
	osg::ref_ptr<osg::TexMat> tm = new osg::TexMat;
	stateset->setTextureAttribute(0, tm.get());

	//设置立方图纹理
	osg::ref_ptr<osg::TextureCubeMap> cubemap = new osg::TextureCubeMap;

	auto path = QCoreApplication::applicationDirPath();
	auto path0 = path + "/data/scene/skybox/posx.png";
	auto path1 = path + "/data/scene/skybox/negx.png";
	auto path2 = path + "/data/scene/skybox/posy.png";
	auto path3 = path + "/data/scene/skybox/negy.png";
	auto path4 = path + "/data/scene/skybox/posz.png";
	auto path5 = path + "/data/scene/skybox/negz.png";
	// 加载6个面的纹理图像
	cubemap->setImage(osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile(path0.toUtf8().data()));
	cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile(path1.toUtf8().data()));
	cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile(path3.toUtf8().data()));
	cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile(path2.toUtf8().data()));
	cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile(path4.toUtf8().data()));
	cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile(path5.toUtf8().data()));

	//设置纹理环绕模式
	cubemap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	cubemap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	cubemap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

	//设置滤波：线形和mipmap
	cubemap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	cubemap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	stateset->setTextureAttributeAndModes(0, cubemap.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	stateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

	//将深度设置为远平面
	osg::ref_ptr<osg::Depth> depth = new osg::Depth;
	depth->setFunction(osg::Depth::ALWAYS);
	depth->setRange(1.0, 1.0);//远平面   
	stateset->setAttributeAndModes(depth, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	//设置渲染顺序为-1，先渲染
	stateset->setRenderBinDetails(-1, "RenderBin");

	osg::ref_ptr<osg::Drawable> drawable = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 1));

	//把球体加入到叶节点
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->setCullingActive(false);
	geode->setStateSet(stateset.get());
	geode->addDrawable(drawable.get());

	//设置变换
	osg::ref_ptr<osg::Transform> transform = new MoveEarthySkyWithEyePointTransform();
	transform->setCullingActive(false);
	transform->addChild(geode.get());

	osg::ref_ptr<osg::ClearNode> clearNode = new osg::ClearNode;
	clearNode->setCullCallback(new TexMatCallback(*tm));
	clearNode->addChild(transform.get());
	_rootNode->addChild(clearNode);
}

void SceneViewer::loadLand(int xcount, int ycount, float xstep, float ystep)

{
	// 创建几何体对象
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

	// 创建顶点和颜色数组
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

	// 设置颜色（白色）
	colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	// 生成顶点数据
	float halfX = (xcount * xstep) / 2.0f;
	float halfY = (ycount * ystep) / 2.0f;

	for (int y = 0; y <= ycount; ++y) {
		for (int x = 0; x <= xcount; ++x) {
			float xPos = x * xstep - halfX;
			float yPos = y * ystep - halfY;
			vertices->push_back(osg::Vec3(xPos, yPos, 0.0f));
		}
	}

	// 设置顶点数组
	geom->setVertexArray(vertices.get());
	geom->setColorArray(colors.get(), osg::Array::BIND_OVERALL);

	// 创建图元集合（网格线）
	osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_LINES);

	// 水平线
	for (int y = 0; y <= ycount; ++y) {
		for (int x = 0; x < xcount; ++x) {
			int idx = y * (xcount + 1) + x;
			indices->push_back(idx);
			indices->push_back(idx + 1);
		}
	}

	// 垂直线
	for (int x = 0; x <= xcount; ++x) {
		for (int y = 0; y < ycount; ++y) {
			int idx = y * (xcount + 1) + x;
			indices->push_back(idx);
			indices->push_back(idx + (xcount + 1));
		}
	}

	// 添加图元到几何体
	geom->addPrimitiveSet(indices.get());

	// 创建Geode节点并添加几何体
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(geom.get());
	osg::ref_ptr<osg::Program> program = new osg::Program;

	// 顶点着色器（传递顶点位置到片段着色器）
	const char* vertSource = R"(
        #version 330
		layout(location = 0) in vec4 vertex;
		// osg build-in
		uniform mat4 osg_ModelViewProjectionMatrix;
		uniform mat4 osg_ModelViewMatrix;
		uniform mat4 osg_ViewMatrixInverse;
		out vec3 oWorldPos;
        void main() {
			oWorldPos = (osg_ViewMatrixInverse * osg_ModelViewMatrix * vertex).xyz;
			gl_Position = osg_ModelViewProjectionMatrix * vertex;
        }
    )";

	// 片段着色器（根据距离中心点的距离计算透明度）
	const char* fragSource = R"(
        #version 330
		in vec3 oWorldPos;
        void main() {
            // 计算到中心点(0,0,0)的距离
            float dist = length(oWorldPos.xy);
            
            // 归一化距离（假设网格最大半径是halfX和halfY中的较大者）
            float maxDist = max(abs(oWorldPos.x), abs(oWorldPos.y));
            
            // 计算透明度（距离越远越透明）
            float alpha = 1.0 - smoothstep(0.0, 90.0, dist);
            
            // 应用透明度
            gl_FragColor = vec4(1.0, 1.0, 1.0, alpha * 0.2); // 0.7是基础透明度
        }
    )";

	program->addShader(new osg::Shader(osg::Shader::VERTEX, vertSource));
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragSource));

	// 创建StateSet并设置Shader
	osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();
	stateset->setAttributeAndModes(program.get());

	// 启用混合（透明度需要）
	stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
	stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	_rootNode->addChild(geode);
}

void SceneViewer::setNodeTransparent(osg::Node* node)
{
	osg::ref_ptr<osg::StateSet> stateset = node->getOrCreateStateSet();
	osg::ref_ptr<osg::PolygonMode> wirePolyMode = new osg::PolygonMode;
	wirePolyMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	stateset->setAttributeAndModes(wirePolyMode);
	osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth(1.0f);
	stateset->setAttribute(lineWidth);
	osg::ref_ptr<osg::PolygonOffset> polyOffset = new osg::PolygonOffset;
	polyOffset->setFactor(-0.1f);
	polyOffset->setUnits(-0.1f);
	stateset->setAttributeAndModes(polyOffset);
}

void SceneViewer::on3DInitialized()
{
	QSurfaceFormat format = QSurfaceFormat::defaultFormat();
	format.setVersion(2, 0);
	format.setProfile(QSurfaceFormat::CompatibilityProfile);
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setOption(QSurfaceFormat::DebugContext);
	format.setSwapInterval(0);
	format.setDepthBufferSize(24);
	format.setSamples(8);
	format.setStencilBufferSize(8);
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	QSurfaceFormat::setDefaultFormat(format);

	// 默认相机操作器
	osg::ref_ptr<CameraManipulator> cameraManipulator = new CameraManipulator;
	cameraManipulator->SetDragMouse(osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON);
	cameraManipulator->SetRotateModKeyMask(osgGA::GUIEventAdapter::MODKEY_ALT);
	cameraManipulator->SetRotateMouse(osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON);
	auto viewer = _osgWidget->getOsgViewer();
	viewer->setCameraManipulator(cameraManipulator);
	//viewer->setCameraManipulator(new osgGA::TrackballManipulator());
	viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	viewer->addEventHandler(new osgViewer::ThreadingHandler);
	viewer->addEventHandler(new osgViewer::WindowSizeHandler);
	viewer->addEventHandler(new osgViewer::StatsHandler);

	_rootNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	//_rootNode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
	//_rootNode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	auto modelpath = QCoreApplication::applicationDirPath() + QString("/data/models/jq.obj");
	_modelNode = osgDB::readNodeFile(modelpath.toUtf8().data());
	_modelDispmentPreNode = osgDB::readNodeFile(modelpath.toUtf8().data());
	setNodeTransparent(_modelDispmentPreNode.get());
	showDisplacementPreModel(false);
	_modelMtx = new osg::MatrixTransform;
	_modelMtx->setMatrix(osg::Matrix::scale(osg::Vec3f(1.0f, 1.0f, 1.0f)));
	_modelMtx->addChild(_modelNode.get());
	_modelMtx->addChild(_modelDispmentPreNode.get());
	loadSkyBox();
	loadLand(300, 300, 1.0f, 1.0f);

	_rootNode->addChild(_modelMtx.get());
	auto pos = _modelMtx->computeBound().center();
	float radius = _modelMtx->computeBound().radius();

	// 计算斜上方45度的相机位置
	// 使用球坐标计算：距离为半径的2倍，方位角45度，俯仰角45度
	float distance = radius * 3;
	float azimuth = osg::DegreesToRadians(45.0f); // 水平角度
	float elevation = osg::DegreesToRadians(45.0f); // 垂直角度

	// 计算相机位置
	osg::Vec3 eye(
		pos.x() + distance * cos(elevation) * cos(azimuth),
		pos.y() + distance * cos(elevation) * sin(azimuth),
		pos.z() + distance * sin(elevation)
	);

	cameraManipulator->setHomePosition(eye, pos, osg::Vec3(0.0, 0.0, 1.0));

	viewer->setSceneData(_rootNode.get());

	auto state = viewer->getCamera()->getGraphicsContext()->getState();
	state->setUseModelViewAndProjectionUniforms(true);
	state->setUseVertexAttributeAliasing(true);
	state->resetVertexAttributeAlias(false);

	viewer->getCamera()->setClearColor(osg::Vec4(0.06, 0.12, 0.19, 1.0));
	float aspectRatio = (_osgWidget->sizeHint().width() * 1.0) / _osgWidget->sizeHint().height();
	viewer->getCamera()->setProjectionMatrixAsPerspective(30.f, aspectRatio, 1.f, 100000.f);
	viewer->setRunMaxFrameRate(120.0f);
	viewer->setRunFrameScheme(osgViewer::ViewerBase::CONTINUOUS);
}
