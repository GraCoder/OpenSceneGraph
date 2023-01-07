#include <sstream>
#include <iomanip>
#include <stdio.h>

#include <osg/Geometry>
#include <osg/BlendEquation>
#include <osg/GLDefines>

#include <osgDB/FileUtils>

#include <osgViewer/Viewer>
#include <osgViewer/ImGuiHandler>
#include <osgViewer/Renderer>

#include <osgViewer/imgui/imgui.h>
#include "imgui/imgui_internal.h"
#include "imgui_impl_opengl3.hpp"

namespace osgViewer {

static ImGuiKey ConvertFromOSGKey(int key)
{
#define KEY osgGA::GUIEventAdapter::KeySymbol

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
    default:  // Not found
      return ImGuiKey_None;
  }
}

class ImGuiBegUpdate : public osg::Operation {
public:
  void operator()(osg::Object*) { ImGui::NewFrame(); }
};

class ImGuiEndUpdate : public osg::Operation {
public:
  void operator()(osg::Object*)
  {
    ImGui::EndFrame();
    ImGui::Render();
  }
};

class ImGuiRenderCallback : public osg::Camera::DrawCallback {
public:
  ImGuiRenderCallback(ImGuiHandler* handler) : _initialize(true), _handler(handler) {}

  void operator()(osg::RenderInfo& renderInfo) const override
  {
    auto ext = renderInfo.getState()->get<osg::GLExtensions>();
    auto extWrap = static_cast<GLWrapper*>(ext);
    if (_initialize) {
      extWrap->ImGui_ImplOpenGL3_Init(nullptr);
      _initialize = false;
    }

    extWrap->ImGui_ImplOpenGL3_NewFrame();

    auto frameNum = renderInfo.getState()->getFrameStamp()->getFrameNumber();
    frameNum = frameNum % 2;
    auto vp = static_cast<ImGuiViewportP*>(_handler->_imvp[frameNum]);
    auto data = &vp->DrawDataP;
    extWrap->ImGui_ImplOpenGL3_RenderDrawData(data);
  }

private:
  mutable bool _initialize;
  ImGuiHandler* _handler;
};

ImGuiHandler::ImGuiHandler() : _initialized(false), _imctx(0)
{
  OSG_INFO << "ImGuiHandler::ImGuiHandler()" << std::endl;

  _camera = new osg::Camera;
  _camera->setName("ImGuiCamera");
  _camera->setProjectionResizePolicy(osg::Camera::FIXED);

  _camera->setRenderOrder(osg::Camera::POST_RENDER, INT_MAX);
  _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
  _camera->setViewMatrix(osg::Matrix::identity());

  _camera->setViewport(0, 0, 800, 640);

  _camera->setClearMask(0);
  _camera->setAllowEventFocus(false);

  _begOp = new ImGuiBegUpdate;
  _endOp = new ImGuiEndUpdate;

  initImGui();
}

ImGuiHandler::~ImGuiHandler()
{
  auto vp = static_cast<ImGuiViewportP*>(_imvp[0]);
  if (_imvp[0])
    ImGui::GetCurrentContext()->Viewports[0] = vp;
  _imvp[0] = nullptr;
  IM_DELETE(_imvp[1]);
  _imvp[1] = nullptr;

  if (_imctx) {
    ImGui::DestroyContext(_imctx);
    _imctx = 0;
  }
}

bool ImGuiHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  auto* view = aa.asView();
  auto ctx = view->getCamera()->getGraphicsContext();

  ImGuiIO& io = ImGui::GetIO();
  if (_initialized) {
    if (ctx) {
      auto traits = ctx->getTraits();
      io.DisplaySize = ImVec2(traits->width, traits->height);
    } else
      io.DisplaySize = ImVec2(800, 640);
  } else {
    _initialized = true;
    io.DisplaySize = ImVec2(800, 640);
    view->addSlave(_camera, false);
    _camera->setGraphicsContext(ctx);
    _camera->setPostDrawCallback(new ImGuiRenderCallback(this));
  }

  const bool wantCaptureMouse = io.WantCaptureMouse;
  const bool wantCaptureKeyboard = io.WantCaptureKeyboard;

  switch (ea.getEventType()) {
    case osgGA::GUIEventAdapter::KEYDOWN:
    case osgGA::GUIEventAdapter::KEYUP: {
      const bool isKeyDown = ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN;
      const int c = ea.getKey();

      // Always update the mod key status.
      io.KeyCtrl = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL;
      io.KeyShift = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT;
      io.KeyAlt = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT;
      io.KeySuper = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SUPER;

      auto imgui_key = ConvertFromOSGKey(c);
      io.AddKeyEvent(imgui_key, isKeyDown);

      return wantCaptureKeyboard;
    }
    case (osgGA::GUIEventAdapter::CHAR): {
      unsigned int k = ea.getKey();
      io.AddInputCharacter(k);
    }
    case (osgGA::GUIEventAdapter::RELEASE): {
      io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
      if (ea.getButton() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
        io.AddMouseButtonEvent(0, false);
      if (ea.getButton() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
        io.AddMouseButtonEvent(1, false);
      if (ea.getButton() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        io.AddMouseButtonEvent(2, false);
      return wantCaptureMouse;
    }
    case (osgGA::GUIEventAdapter::PUSH): {
      io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
      if (ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
        io.AddMouseButtonEvent(0, true);
      else if (ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
        io.AddMouseButtonEvent(1, true);
      else if (ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        io.AddMouseButtonEvent(2, true);
      return wantCaptureMouse;
    }
    case (osgGA::GUIEventAdapter::DOUBLECLICK): {
      io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
      io.MouseDoubleClicked[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
      io.MouseDoubleClicked[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
      io.MouseDoubleClicked[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
      return wantCaptureMouse;
    }
    case (osgGA::GUIEventAdapter::DRAG):
    case (osgGA::GUIEventAdapter::MOVE): {
      io.AddMousePosEvent(ea.getX(), io.DisplaySize.y - ea.getY());
      return wantCaptureMouse;
    }
    case (osgGA::GUIEventAdapter::SCROLL): {
      io.AddMouseWheelEvent(0, ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP ? 1.0 : -1.0);
      return wantCaptureMouse;
    }
    case (osgGA::GUIEventAdapter::FRAME): {
      ImGui::SetCurrentContext(_imctx);
      auto viewer = dynamic_cast<osgViewer::ViewerBase*>(&aa);
      auto frameNum = view->getFrameStamp()->getFrameNumber();
      frameNum = frameNum % 2;

      auto vp = static_cast<ImGuiViewportP*>(_imvp[frameNum]);
      ImGui::GetCurrentContext()->Viewports[0] = vp;
      viewer->addPreUpdateOperation(_begOp);
      viewer->addPstUpdateOperation(_endOp);
      break;
    }
    case (osgGA::GUIEventAdapter::CLOSE_WINDOW): {
    }
    default:
      break;
  }
  return false;
}

void ImGuiHandler::initImGui()
{
  //--------------------create imgui context----------------------------------
  _imctx = ImGui::CreateContext();

  ImGui::StyleColorsDark();
  ImGuiIO& io = ImGui::GetIO();

  std::vector<std::string> fontFiles = {"C:\\Users\\t\\AppData\\Local\\Microsoft\\Windows\\Fonts\\SourceHanSansSC-Regular.otf",
                                        "C:\\Windows\\Fonts\\simhei.ttf"};
  for (auto& fontPath : fontFiles) {
    if (osgDB::fileExists(fontPath)) {
      ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
      IM_UNUSED(font);
      break;
    }
  }

  // auto* bd = GLWrapper::ImGui_ImplOpenGL3_GetBackendData();
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  _imvp[0] = ImGui::GetCurrentContext()->Viewports[0];
  _imvp[1] = IM_NEW(ImGuiViewportP);

  //{
  //	io.KeyMap[ImGuiKey_Tab] = ImGuiKey_Tab;
  //	io.KeyMap[ImGuiKey_LeftArrow] = ImGuiKey_LeftArrow;
  //	io.KeyMap[ImGuiKey_RightArrow] = ImGuiKey_RightArrow;
  //	io.KeyMap[ImGuiKey_UpArrow] = ImGuiKey_UpArrow;
  //	io.KeyMap[ImGuiKey_DownArrow] = ImGuiKey_DownArrow;
  //	io.KeyMap[ImGuiKey_PageUp] = ImGuiKey_PageUp;
  //	io.KeyMap[ImGuiKey_PageDown] = ImGuiKey_PageDown;
  //	io.KeyMap[ImGuiKey_Home] = ImGuiKey_Home;
  //	io.KeyMap[ImGuiKey_End] = ImGuiKey_End;
  //	io.KeyMap[ImGuiKey_Delete] = ImGuiKey_Delete;
  //	io.KeyMap[ImGuiKey_Backspace] = ImGuiKey_Backspace;
  //	io.KeyMap[ImGuiKey_Enter] = ImGuiKey_Enter;
  //	io.KeyMap[ImGuiKey_Escape] = ImGuiKey_Escape;
  //	io.KeyMap[ImGuiKey_A] = osgGA::GUIEventAdapter::KeySymbol::KEY_A;
  //	io.KeyMap[ImGuiKey_C] = osgGA::GUIEventAdapter::KeySymbol::KEY_C;
  //	io.KeyMap[ImGuiKey_V] = osgGA::GUIEventAdapter::KeySymbol::KEY_V;
  //	io.KeyMap[ImGuiKey_X] = osgGA::GUIEventAdapter::KeySymbol::KEY_X;
  //	io.KeyMap[ImGuiKey_Y] = osgGA::GUIEventAdapter::KeySymbol::KEY_Y;
  //	io.KeyMap[ImGuiKey_Z] = osgGA::GUIEventAdapter::KeySymbol::KEY_Z;

  //	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  //}
}

}  // namespace osgViewer
