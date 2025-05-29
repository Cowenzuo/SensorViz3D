#include "SceneViewer.h"

#include <QHBoxLayout>
#include <QFile>
#include <QDebug>
#include <QCoreApplication>

#include <osg/ComputeBoundsVisitor>
#include <osg/MatrixTransform>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

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

void SceneViewer::resizeEvent(QResizeEvent* event)
{
	if (!_osgWidget || !_osgWidget->getOsgViewer() || !_osgWidget->getOsgViewer()->getCamera()) {
		return;
	}
	float aspectRatio = (_osgWidget->sizeHint().width() * 1.0) / _osgWidget->sizeHint().height();
	_osgWidget->getOsgViewer()->getCamera()->setProjectionMatrixAsPerspective(30.f, aspectRatio, 1.f, 100000.f);
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
