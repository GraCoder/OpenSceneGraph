#include <osgQOpenGL/osgQOpenGLWindow>
#include "OSGRenderer"

#include <osgViewer/Viewer>
#include <osg/GL>

#include <QApplication>
#include <QKeyEvent>
#include <QInputDialog>
#include <QLayout>
#include <QScreen>
#include <QWindow>
#include <QDebug>


osgQOpenGLWindow::osgQOpenGLWindow()
    : QOpenGLWindow(QOpenGLWindow::NoPartialUpdate, nullptr)
{
    _renderer = nullptr;
    _osgWantsToRenderFrame = true;
    _arguments = nullptr;
    _isFirstFrame = true;
}

osgQOpenGLWindow::osgQOpenGLWindow(osg::ArgumentParser * arguments, QWindow * parent)
	: QOpenGLWindow(QOpenGLWindow::NoPartialUpdate, nullptr)
	, _arguments(arguments)
{
    _renderer = nullptr;
    _osgWantsToRenderFrame = true;
    _isFirstFrame = true;
}

osgQOpenGLWindow::~osgQOpenGLWindow()
{
}

osgViewer::Viewer* osgQOpenGLWindow::getOsgViewer()
{
    return _renderer;
}

OpenThreads::ReadWriteMutex* osgQOpenGLWindow::mutex()
{
    return &_osgMutex;
}


void osgQOpenGLWindow::initializeGL()
{
    // Initializes OpenGL function resolution for the current context.
    initializeOpenGLFunctions();
    createRenderer();
    emit initialized();
}

void osgQOpenGLWindow::resizeGL(int w, int h)
{
    Q_ASSERT(_renderer);
    double pixelRatio = screen()->devicePixelRatio();
    qDebug() << pixelRatio << "\n";
    _renderer->resize(w, h, pixelRatio);
}

void osgQOpenGLWindow::paintGL()
{
    OpenThreads::ScopedReadLock locker(_osgMutex);
	if (_isFirstFrame) {
		_isFirstFrame = false;
		_renderer->getCamera()->getGraphicsContext()->setDefaultFboId(defaultFramebufferObject());
	}
    _renderer->frame();
}

void osgQOpenGLWindow::keyPressEvent(QKeyEvent* event)
{
    Q_ASSERT(_renderer);

    if(event->key() == Qt::Key_F)
    {
    }
    else // not 'F' key
    {
        // forward event to renderer
        _renderer->keyPressEvent(event);
    }
}

void osgQOpenGLWindow::keyReleaseEvent(QKeyEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->keyReleaseEvent(event);
}

void osgQOpenGLWindow::mousePressEvent(QMouseEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->mousePressEvent(event);
}

void osgQOpenGLWindow::mouseReleaseEvent(QMouseEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->mouseReleaseEvent(event);
}

void osgQOpenGLWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->mouseDoubleClickEvent(event);
}

void osgQOpenGLWindow::mouseMoveEvent(QMouseEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->mouseMoveEvent(event);
}

void osgQOpenGLWindow::wheelEvent(QWheelEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->wheelEvent(event);
}

void osgQOpenGLWindow::setDefaultDisplaySettings()
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    ds->setNvOptimusEnablement(1);
    ds->setStereo(false);
}

void osgQOpenGLWindow::createRenderer()
{
    // call this before creating a View...
    setDefaultDisplaySettings();
	if (!_arguments) {
		_renderer = new OSGRenderer(this, enQGLWindow);
	}
	else {
		_renderer = new OSGRenderer(_arguments, this, enQGLWindow);
	}
    double pixelRatio = screen()->devicePixelRatio();
    _renderer->setupOSG(width(), height(), pixelRatio);
}
