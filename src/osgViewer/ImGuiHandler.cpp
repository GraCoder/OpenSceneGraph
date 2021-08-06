#include <sstream>
#include <iomanip>
#include <stdio.h>

#include <osg/io_utils>

#include <osg/MatrixTransform>

#include <osgViewer/View>
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

ImGuiHandler::ImGuiHandler(osg::GraphicsContext* context)
{
	OSG_INFO << "ImGuiHandler::ImGuiHandler()" << std::endl;

	_camera = new osg::Camera;
	_camera->getOrCreateStateSet()->setGlobalDefaults();
	_camera->setRenderer(new Renderer(_camera.get()));
	_camera->setProjectionResizePolicy(osg::Camera::FIXED);

	osg::DisplaySettings::ShaderHint shaderHint = osg::DisplaySettings::instance()->getShaderHint();
	if (shaderHint == osg::DisplaySettings::SHADER_GL3 || shaderHint == osg::DisplaySettings::SHADER_GLES3) {

		OSG_INFO << "ImGuiHandler::ImGuiHandler() Setting up GL3 compatible shaders" << std::endl;

		osg::ref_ptr<osg::Program> program = new osg::Program;
		program->addShader(new osg::Shader(osg::Shader::VERTEX, gl3_StatsVertexShader));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, gl3_StatsFragmentShader));
		_camera->getOrCreateStateSet()->setAttributeAndModes(program.get());
	} else if (shaderHint == osg::DisplaySettings::SHADER_GL2 || shaderHint == osg::DisplaySettings::SHADER_GLES2) {

		OSG_INFO << "ImGuiHandler::ImGuiHandler() Setting up GL2 compatible shaders" << std::endl;

		osg::ref_ptr<osg::Program> program = new osg::Program;
		program->addShader(new osg::Shader(osg::Shader::VERTEX, gl2_StatsVertexShader));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, gl2_StatsFragmentShader));
		_camera->getOrCreateStateSet()->setAttributeAndModes(program.get());
	} else {
		OSG_INFO << "ImGuiHandler::ImGuiHandler() Fixed pipeline" << std::endl;
	}

	_camera->setGraphicsContext(context);

	_camera->setRenderOrder(osg::Camera::POST_RENDER, 999);

	_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	_camera->setViewMatrix(osg::Matrix::identity());

	// only clear the depth buffer
	_camera->setClearMask(0);
	_camera->setAllowEventFocus(false);
}

bool ImGuiHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	osgViewer::View* myview = dynamic_cast<osgViewer::View*>(&aa);
	if (!myview) return false;

	osgViewer::ViewerBase* viewer = myview->getViewerBase();

	if (ea.getHandled()) return false;

	switch (ea.getEventType()) {
		case(osgGA::GUIEventAdapter::KEYDOWN):
		{
		}
		case(osgGA::GUIEventAdapter::RESIZE):
			setWindowSize(ea.getWindowWidth(), ea.getWindowHeight());
			break;
		default: break;
	}
	return false;
}

void ImGuiHandler::setWindowSize(int width, int height)
{
	if (width <= 0 || height <= 0)
		return;

	//_camera->setViewport(0, 0, width, height);
	//if (fabs(height * _statsWidth) <= fabs(width * _statsHeight)) {
	//	_camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, width * _statsHeight / height, 0.0, _statsHeight));
	//} else {
	//	_camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, _statsWidth, _statsHeight - height * _statsWidth / width, _statsHeight));
	//}
}

}
