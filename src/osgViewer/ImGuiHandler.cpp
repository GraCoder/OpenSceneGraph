#include <sstream>
#include <iomanip>
#include <stdio.h>

#include <osg/io_utils>

#include <osg/MatrixTransform>

#include <osgViewer/ImGuiHandler>
#include <osgViewer/Renderer>

#include <osg/PolygonMode>
#include <osg/Geometry>

namespace osgViewer {

#if (!defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GLES3_AVAILABLE))
#define GLSL_VERSION_STR "330 core"
#else
#define GLSL_VERSION_STR "300 es"
#endif

static const char* gl3_StatsVertexShader = {
	"#version " GLSL_VERSION_STR "\n"
	"// gl3_StatsVertexShader\n"
	"#ifdef GL_ES\n"
	"    precision highp float;\n"
	"#endif\n"
	"in vec4 osg_Vertex;\n"
	"in vec4 osg_Color;\n"
	"uniform mat4 osg_ModelViewProjectionMatrix;\n"
	"out vec4 vertexColor;\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
	"    vertexColor = osg_Color; \n"
	"}\n"
};

static const char* gl3_StatsFragmentShader = {
	"#version " GLSL_VERSION_STR "\n"
	"// gl3_StatsFragmentShader\n"
	"#ifdef GL_ES\n"
	"    precision highp float;\n"
	"#endif\n"
	"in vec4 vertexColor;\n"
	"out vec4 color;\n"
	"void main(void)\n"
	"{\n"
	"    color = vertexColor;\n"
	"}\n"
};

static const char* gl2_StatsVertexShader = {
	"// gl2_StatsVertexShader\n"
	"#ifdef GL_ES\n"
	"    precision highp float;\n"
	"#endif\n"
	"varying vec4 vertexColor;\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"    vertexColor = gl_Color;\n"
	"}\n"
};

static const char* gl2_StatsFragmentShader = {
	"// gl2_StatsFragmentShader\n"
	"#ifdef GL_ES\n"
	"    precision highp float;\n"
	"#endif\n"
	"varying vec4 vertexColor;\n"
	"void main(void)\n"
	"{\n"
	"    gl_FragColor = vertexColor;\n"
	"}\n"
};

ImGuiHandler::ImGuiHandler(osg::GraphicsContext*)
{
	//OSG_INFO << "ImGuiHandler::ImGuiHandler()" << std::endl;

	//_camera = new osg::Camera;
	//_camera->getOrCreateStateSet()->setGlobalDefaults();
	//_camera->setRenderer(new Renderer(_camera.get()));
	//_camera->setProjectionResizePolicy(osg::Camera::FIXED);

	//osg::DisplaySettings::ShaderHint shaderHint = osg::DisplaySettings::instance()->getShaderHint();
	//if (shaderHint == osg::DisplaySettings::SHADER_GL3 || shaderHint == osg::DisplaySettings::SHADER_GLES3) {

	//	OSG_INFO << "ImGuiHandler::ImGuiHandler() Setting up GL3 compatible shaders" << std::endl;

	//	osg::ref_ptr<osg::Program> program = new osg::Program;
	//	program->addShader(new osg::Shader(osg::Shader::VERTEX, gl3_StatsVertexShader));
	//	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, gl3_StatsFragmentShader));
	//	_camera->getOrCreateStateSet()->setAttributeAndModes(program.get());
	//} else if (shaderHint == osg::DisplaySettings::SHADER_GL2 || shaderHint == osg::DisplaySettings::SHADER_GLES2) {

	//	OSG_INFO << "ImGuiHandler::ImGuiHandler() Setting up GL2 compatible shaders" << std::endl;

	//	osg::ref_ptr<osg::Program> program = new osg::Program;
	//	program->addShader(new osg::Shader(osg::Shader::VERTEX, gl2_StatsVertexShader));
	//	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, gl2_StatsFragmentShader));
	//	_camera->getOrCreateStateSet()->setAttributeAndModes(program.get());
	//} else {
	//	OSG_INFO << "ImGuiHandler::ImGuiHandler() Fixed pipeline" << std::endl;
	//}
}

bool ImGuiHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	//osgViewer::View* myview = dynamic_cast<osgViewer::View*>(&aa);
	//if (!myview) return false;

	//osgViewer::ViewerBase* viewer = myview->getViewerBase();
	//if (viewer && _threadingModelText.valid() && viewer->getThreadingModel() != _threadingModel) {
	//	_threadingModel = viewer->getThreadingModel();
	//	updateThreadingModelText();
	//}


	//if (ea.getHandled()) return false;

	//switch (ea.getEventType()) {
	//	case(osgGA::GUIEventAdapter::KEYDOWN):
	//	{
	//		if (ea.getKey() == _keyEventTogglesOnScreenStats) {
	//			if (viewer && viewer->getViewerStats()) {
	//				if (!_initialized) {
	//					setUpHUDCamera(viewer);
	//					setUpScene(viewer);
	//				}

	//				++_statsType;

	//				if (_statsType == LAST) _statsType = NO_STATS;

	//				osgViewer::ViewerBase::Cameras cameras;
	//				collectWhichCamerasToRenderStatsFor(viewer, cameras);

	//				switch (_statsType) {
	//					case(NO_STATS):
	//					{
	//						viewer->getViewerStats()->collectStats("frame_rate", false);
	//						viewer->getViewerStats()->collectStats("event", false);
	//						viewer->getViewerStats()->collectStats("update", false);

	//						for (osgViewer::ViewerBase::Cameras::iterator itr = cameras.begin();
	//							itr != cameras.end();
	//							++itr) {
	//							osg::Stats* stats = (*itr)->getStats();
	//							if (stats) {
	//								stats->collectStats("rendering", false);
	//								stats->collectStats("gpu", false);
	//								stats->collectStats("scene", false);
	//							}
	//						}

	//						viewer->getViewerStats()->collectStats("scene", false);

	//						_camera->setNodeMask(0x0);
	//						_switch->setAllChildrenOff();
	//						break;
	//					}
	//					case(FRAME_RATE):
	//					{
	//						viewer->getViewerStats()->collectStats("frame_rate", true);

	//						_camera->setNodeMask(0xffffffff);
	//						_switch->setValue(_frameRateChildNum, true);
	//						break;
	//					}
	//					case(VIEWER_STATS):
	//					{
	//						ViewerBase::Scenes scenes;
	//						viewer->getScenes(scenes);
	//						for (ViewerBase::Scenes::iterator itr = scenes.begin();
	//							itr != scenes.end();
	//							++itr) {
	//							Scene* scene = *itr;
	//							osgDB::DatabasePager* dp = scene->getDatabasePager();
	//							if (dp && dp->isRunning()) {
	//								dp->resetStats();
	//							}
	//						}

	//						viewer->getViewerStats()->collectStats("event", true);
	//						viewer->getViewerStats()->collectStats("update", true);

	//						for (osgViewer::ViewerBase::Cameras::iterator itr = cameras.begin();
	//							itr != cameras.end();
	//							++itr) {
	//							if ((*itr)->getStats()) (*itr)->getStats()->collectStats("rendering", true);
	//							if ((*itr)->getStats()) (*itr)->getStats()->collectStats("gpu", true);
	//						}

	//						_camera->setNodeMask(0xffffffff);
	//						_switch->setValue(_viewerChildNum, true);
	//						break;
	//					}
	//					case(CAMERA_SCENE_STATS):
	//					{
	//						_camera->setNodeMask(0xffffffff);
	//						_switch->setValue(_cameraSceneChildNum, true);

	//						for (osgViewer::ViewerBase::Cameras::iterator itr = cameras.begin();
	//							itr != cameras.end();
	//							++itr) {
	//							osg::Stats* stats = (*itr)->getStats();
	//							if (stats) {
	//								stats->collectStats("scene", true);
	//							}
	//						}

	//						break;
	//					}
	//					case(VIEWER_SCENE_STATS):
	//					{
	//						_camera->setNodeMask(0xffffffff);
	//						_switch->setValue(_viewerSceneChildNum, true);

	//						viewer->getViewerStats()->collectStats("scene", true);

	//						break;
	//					}
	//					default:
	//						break;
	//				}

	//				aa.requestRedraw();
	//			}
	//			return true;
	//		}
	//		if (ea.getKey() == _keyEventPrintsOutStats) {
	//			if (viewer && viewer->getViewerStats()) {
	//				OSG_NOTICE << std::endl << "Stats report:" << std::endl;
	//				typedef std::vector<osg::Stats*> StatsList;
	//				StatsList statsList;
	//				statsList.push_back(viewer->getViewerStats());

	//				osgViewer::ViewerBase::Contexts contexts;
	//				viewer->getContexts(contexts);
	//				for (osgViewer::ViewerBase::Contexts::iterator gcitr = contexts.begin();
	//					gcitr != contexts.end();
	//					++gcitr) {
	//					osg::GraphicsContext::Cameras& cameras = (*gcitr)->getCameras();
	//					for (osg::GraphicsContext::Cameras::iterator itr = cameras.begin();
	//						itr != cameras.end();
	//						++itr) {
	//						if ((*itr)->getStats()) {
	//							statsList.push_back((*itr)->getStats());
	//						}
	//					}
	//				}

	//				for (unsigned int i = viewer->getViewerStats()->getEarliestFrameNumber(); i < viewer->getViewerStats()->getLatestFrameNumber(); ++i) {
	//					for (StatsList::iterator itr = statsList.begin();
	//						itr != statsList.end();
	//						++itr) {
	//						if (itr == statsList.begin()) (*itr)->report(osg::notify(osg::NOTICE), i);
	//						else (*itr)->report(osg::notify(osg::NOTICE), i, "    ");
	//					}
	//					OSG_NOTICE << std::endl;
	//				}

	//			}
	//			return true;
	//		}
	//		break;
	//	}
	//	case(osgGA::GUIEventAdapter::RESIZE):
	//		setWindowSize(ea.getWindowWidth(), ea.getWindowHeight());
	//		break;
	//	default: break;
	//}
	return false;
}

//void ImGuiHandler::setUpHUDCamera(osgViewer::ViewerBase* viewer)
//{
//	// Try GraphicsWindow first so we're likely to get the main viewer window
//	osg::GraphicsContext* context = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());
//
//	if (!context) {
//		osgViewer::Viewer::Windows windows;
//		viewer->getWindows(windows);
//
//		if (!windows.empty()) context = windows.front();
//		else {
//			// No GraphicsWindows were found, so let's try to find a GraphicsContext
//			context = _camera->getGraphicsContext();
//
//			if (!context) {
//				osgViewer::Viewer::Contexts contexts;
//				viewer->getContexts(contexts);
//
//				if (contexts.empty()) return;
//
//				context = contexts.front();
//			}
//		}
//	}
//
//	_camera->setGraphicsContext(context);
//
//	_camera->setRenderOrder(osg::Camera::POST_RENDER, 10);
//
//	_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
//	_camera->setViewMatrix(osg::Matrix::identity());
//	setWindowSize(context->getTraits()->width, context->getTraits()->height);
//
//	// only clear the depth buffer
//	_camera->setClearMask(0);
//	_camera->setAllowEventFocus(false);
//
//	_camera->setRenderer(new Renderer(_camera.get()));
//
//	_initialized = true;
//}

//void ImGuiHandler::setWindowSize(int width, int height)
//{
//	if (width <= 0 || height <= 0)
//		return;
//
//	_camera->setViewport(0, 0, width, height);
//	if (fabs(height * _statsWidth) <= fabs(width * _statsHeight)) {
//		_camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, width * _statsHeight / height, 0.0, _statsHeight));
//	} else {
//		_camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, _statsWidth, _statsHeight - height * _statsWidth / width, _statsHeight));
//	}
//}

}
