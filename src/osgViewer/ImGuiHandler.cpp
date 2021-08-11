#include <sstream>
#include <iomanip>
#include <stdio.h>

#include <osg/io_utils>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osg/Geometry>

#include <osgViewer/View>
#include <osgViewer/ImGuiHandler>
#include <osgViewer/Renderer>

#include <osgViewer/imgui/imgui.h>
#include "imgui/imgui_internal.h"
#include "imgui_impl_opengl3.cpp"

namespace osgViewer {

static int ConvertFromOSGKey(int key)
{
	using KEY = osgGA::GUIEventAdapter::KeySymbol;

	switch (key) {
		case KEY::KEY_Tab:
			return ImGuiKey_Tab;
		case KEY::KEY_Left:
			return ImGuiKey_LeftArrow;
		case KEY::KEY_Right:
			return ImGuiKey_RightArrow;
		case KEY::KEY_Up:
			return ImGuiKey_UpArrow;
		case KEY::KEY_Down:
			return ImGuiKey_DownArrow;
		case KEY::KEY_Page_Up:
			return ImGuiKey_PageUp;
		case KEY::KEY_Page_Down:
			return ImGuiKey_PageDown;
		case KEY::KEY_Home:
			return ImGuiKey_Home;
		case KEY::KEY_End:
			return ImGuiKey_End;
		case KEY::KEY_Delete:
			return ImGuiKey_Delete;
		case KEY::KEY_BackSpace:
			return ImGuiKey_Backspace;
		case KEY::KEY_Return:
			return ImGuiKey_Enter;
		case KEY::KEY_Escape:
			return ImGuiKey_Escape;
		case 22:
			return osgGA::GUIEventAdapter::KeySymbol::KEY_V;
		case 3:
			return osgGA::GUIEventAdapter::KeySymbol::KEY_C;
		default: // Not found
			return key;
	}
}

std::map<osg::GraphicsContext*, ImGuiContext*> s_context;

ImGuiViewportP* s_viewports[2];

class ImGuiRenderObject : public osg::Drawable {
public:
	ImGuiRenderObject()
		: _initialized(false)
	{
		setCullingActive(false);
	}

	void drawImplementation(osg::RenderInfo& renderInfo) const
	{
		auto ext = renderInfo.getState()->get<osg::GLExtensions>();
		auto extWrap = static_cast<GLWrapper*>(ext);
		auto ctx = renderInfo.getState()->getGraphicsContext();
		auto& imctx = s_context[ctx];

		if (!_initialized)
		{
			auto ctxTmp = ImGui::CreateContext();
			extWrap->ImGui_ImplOpenGL3_Init("#version 460");
			extWrap->ImGui_ImplOpenGL3_NewFrame();
			initImGui();
			auto* viewport = IM_NEW(ImGuiViewportP)();
			s_viewports[0] = ctxTmp->Viewports[0];
			s_viewports[1] = viewport;
			imctx = ctxTmp;
			_initialized = true;
			return;
		} 

		auto frameNum = renderInfo.getState()->getFrameStamp()->getFrameNumber();
		frameNum = frameNum % 2;
		auto data = &s_viewports[frameNum]->DrawDataP;
		extWrap->ImGui_ImplOpenGL3_RenderDrawData(data);
		data->Clear();
	}

	void initImGui() const
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = ImGuiKey_Tab;
		io.KeyMap[ImGuiKey_LeftArrow] = ImGuiKey_LeftArrow;
		io.KeyMap[ImGuiKey_RightArrow] = ImGuiKey_RightArrow;
		io.KeyMap[ImGuiKey_UpArrow] = ImGuiKey_UpArrow;
		io.KeyMap[ImGuiKey_DownArrow] = ImGuiKey_DownArrow;
		io.KeyMap[ImGuiKey_PageUp] = ImGuiKey_PageUp;
		io.KeyMap[ImGuiKey_PageDown] = ImGuiKey_PageDown;
		io.KeyMap[ImGuiKey_Home] = ImGuiKey_Home;
		io.KeyMap[ImGuiKey_End] = ImGuiKey_End;
		io.KeyMap[ImGuiKey_Delete] = ImGuiKey_Delete;
		io.KeyMap[ImGuiKey_Backspace] = ImGuiKey_Backspace;
		io.KeyMap[ImGuiKey_Enter] = ImGuiKey_Enter;
		io.KeyMap[ImGuiKey_Escape] = ImGuiKey_Escape;
		io.KeyMap[ImGuiKey_A] = osgGA::GUIEventAdapter::KeySymbol::KEY_A;
		io.KeyMap[ImGuiKey_C] = osgGA::GUIEventAdapter::KeySymbol::KEY_C;
		io.KeyMap[ImGuiKey_V] = osgGA::GUIEventAdapter::KeySymbol::KEY_V;
		io.KeyMap[ImGuiKey_X] = osgGA::GUIEventAdapter::KeySymbol::KEY_X;
		io.KeyMap[ImGuiKey_Y] = osgGA::GUIEventAdapter::KeySymbol::KEY_Y;
		io.KeyMap[ImGuiKey_Z] = osgGA::GUIEventAdapter::KeySymbol::KEY_Z;

		//ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::StyleColorsDark();
	}

private:
	mutable bool _initialized;
};

class ImGuiUpdateOperation : public osg::Operation {
	void operator () (osg::Object*)
	{
		ImGui::EndFrame();
		ImGui::Render();
	}
};

ImGuiHandler::ImGuiHandler()
	: _initialized(false)
{
	OSG_INFO << "ImGuiHandler::ImGuiHandler()" << std::endl;

	_camera = new osg::Camera;
	//_camera->getOrCreateStateSet()->setGlobalDefaults();
	_camera->setRenderer(new Renderer(_camera.get()));
	_camera->setProjectionResizePolicy(osg::Camera::FIXED);

	osg::DisplaySettings::ShaderHint shaderHint = osg::DisplaySettings::instance()->getShaderHint();
	_camera->setRenderOrder(osg::Camera::POST_RENDER, 999);
	_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	_camera->setViewMatrix(osg::Matrix::identity());

	_camera->setViewport(0, 0, 800, 640);

	// only clear the depth buffer
	_camera->setClearMask(0);
	_camera->setAllowEventFocus(false);
	_camera->addChild(new ImGuiRenderObject);
}

bool ImGuiHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	osgViewer::View* myview = dynamic_cast<osgViewer::View*>(&aa);
	if (!myview) 
		return false;

	osgViewer::ViewerBase* viewer = myview->getViewerBase();

	std::vector<osg::GraphicsContext*> ctxs;
	viewer->getContexts(ctxs);
	if (ctxs.size() == 0)
		return false;
	int idx = 0;
	while (idx < ctxs.size())
	{
		if (ctxs[idx] == ea.getGraphicsContext())
			break;
		idx++;
	}
	
	osg::GraphicsContext* ctx = nullptr;
	if (idx == ctxs.size())
		ctx = ctxs[0];
	else
		ctx = ctxs[idx];

	_camera->setGraphicsContext(ctx);

	auto imctx = s_context[ctx];
	if (imctx == nullptr)
		return false;
	 else
		ImGui::SetCurrentContext(imctx);

	ImGuiIO& io = ImGui::GetIO();
	const bool wantCaptureMouse = io.WantCaptureMouse;
	const bool wantCaptureKeyboard = io.WantCaptureKeyboard;

	switch (ea.getEventType()) {
		case osgGA::GUIEventAdapter::KEYDOWN:
		case osgGA::GUIEventAdapter::KEYUP:
		{
			const bool isKeyDown = ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN;
			const int c = ea.getKey();

			// Always update the mod key status.
			io.KeyCtrl = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL;
			io.KeyShift = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT;
			io.KeyAlt = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT;
			io.KeySuper = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SUPER;


			const int imgui_key = ConvertFromOSGKey(c);
			if (imgui_key > 0 && imgui_key < 512) {
				//assert((imgui_key >= 0 && imgui_key < 512) && "ImGui KeysMap is an array of 512");
				io.KeysDown[imgui_key] = isKeyDown;
			}

			// Not sure this < 512 is correct here....
			if (isKeyDown && imgui_key >= 32 && imgui_key < 512) {
				io.AddInputCharacter((unsigned int)c);
			}

			return wantCaptureKeyboard;
		}
		case (osgGA::GUIEventAdapter::RELEASE):
		{
			io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
			_mousePressed[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
			_mousePressed[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
			_mousePressed[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;

			_mouseDoubleClicked[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
			_mouseDoubleClicked[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
			_mouseDoubleClicked[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
			return wantCaptureMouse;
		}
		case (osgGA::GUIEventAdapter::PUSH):
		{
			io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
			_mousePressed[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
			_mousePressed[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
			_mousePressed[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
			return wantCaptureMouse;
		}
		case (osgGA::GUIEventAdapter::DOUBLECLICK):
		{
			io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
			_mouseDoubleClicked[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
			_mouseDoubleClicked[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
			_mouseDoubleClicked[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
			return wantCaptureMouse;
		}
		case (osgGA::GUIEventAdapter::DRAG):
		case (osgGA::GUIEventAdapter::MOVE):
		{
			io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
			return wantCaptureMouse;
		}
		case (osgGA::GUIEventAdapter::SCROLL):
		{
			_mouseWheel = ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP ? 1.0 : -1.0;
			return wantCaptureMouse;
		}
		case(osgGA::GUIEventAdapter::FRAME):
		{
			auto frameNum = viewer->getViewerFrameStamp()->getFrameNumber();
			frameNum = frameNum % 2;
			imctx->Viewports[0] = s_viewports[frameNum];
			newFrame();
			viewer->addUpdateOperation(new ImGuiUpdateOperation);
			break;
		}
		default: break;
	}
	return false;
}

void ImGuiHandler::newFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(1800, 900);
	//if (w > 0 && h > 0)
	//	io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

	//double currentTime = renderInfo.getView()->getFrameStamp()->getSimulationTime();
	//io.DeltaTime = currentTime - time_ + 0.0000001;
	//time_ = currentTime;

	for (int i = 0; i < 3; i++) {
		io.MouseDown[i] = _mousePressed[i];
	}

	for (int i = 0; i < 3; i++) {
		io.MouseDoubleClicked[i] = _mouseDoubleClicked[i];
	}

	io.MouseWheel = _mouseWheel;
	_mouseWheel = 0.0f;
	
	ImGui::NewFrame();
}

}
