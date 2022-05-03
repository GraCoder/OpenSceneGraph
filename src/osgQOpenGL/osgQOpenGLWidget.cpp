#include <osgQOpenGL/osgQOpenGLWidget>
#include "OSGRenderer"

#include <osgViewer/Viewer>
#include <osg/GL>

#include <QApplication>
#include <QKeyEvent>
#include <QInputDialog>
#include <QLayout>
#include <QMainWindow>
#include <QScreen>
#include <QWindow>
#include <QDebug>

osgQOpenGLWidget::osgQOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    _renderer = nullptr;
    _osgWantsToRenderFrame = true;
    _arguments = nullptr;
    _isFirstFrame = true;
}

osgQOpenGLWidget::osgQOpenGLWidget(osg::ArgumentParser* arguments,
                                   QWidget* parent) :
    QOpenGLWidget(parent),
    _arguments(arguments)
{
    _renderer = nullptr;
    _osgWantsToRenderFrame = true;
    _isFirstFrame = true;
}

osgQOpenGLWidget::~osgQOpenGLWidget()
{
}

osgViewer::Viewer* osgQOpenGLWidget::getOsgViewer()
{
    return _renderer;
}

OpenThreads::ReadWriteMutex* osgQOpenGLWidget::mutex()
{
    return &_osgMutex;
}


void osgQOpenGLWidget::initializeGL()
{
    // Initializes OpenGL function resolution for the current context.
    initializeOpenGLFunctions();
    createRenderer();
    emit initialized();
}

void osgQOpenGLWidget::resizeGL(int w, int h)
{
    Q_ASSERT(_renderer);
    QScreen* screen = windowHandle()
                      && windowHandle()->screen() ? windowHandle()->screen() :
                      qApp->screens().front();
    _renderer->resize(w, h, screen->devicePixelRatio());
}

void osgQOpenGLWidget::paintGL()
{
    OpenThreads::ScopedReadLock locker(_osgMutex);
	if (_isFirstFrame) {
		_isFirstFrame = false;
		_renderer->getCamera()->getGraphicsContext()->setDefaultFboId(defaultFramebufferObject());
	}
	_renderer->frame();
}

void osgQOpenGLWidget::keyPressEvent(QKeyEvent* event)
{
    Q_ASSERT(_renderer);

    if(event->key() == Qt::Key_F)
    {
        static QSize g;
        static QMargins sMargins;

        if(parent() && parent()->isWidgetType())
        {
            QMainWindow* _mainwindow = dynamic_cast<QMainWindow*>(parent());

            if(_mainwindow)
            {
                g = size();

                if(layout())
                    sMargins = layout()->contentsMargins();

                bool ok = true;

                // select screen
                auto screenList = qApp->screens();
                if(screenList.size() > 1)
                {
                    QMap<QString, QScreen*> screens;
                    int screenNumber = 0;

                    for(int i = 0; i < screenList.size(); i++)
                    {
                        QScreen* screen = screenList[i];

                        QString name = screen->name();

                        if(name.isEmpty())
                        {
                            name = tr("Screen %1").arg(screenNumber);
                        }

                        name += " (" + QString::number(screen->size().width()) + "x" + QString::number(
                                    screen->size().height()) + ")";
                        screens[name] = screen;
                        ++screenNumber;
                    }

                    QString selected = QInputDialog::getItem(this,
                                                             tr("Choose fullscreen target screen"), tr("Screen"), screens.keys(), 0, false,
                                                             &ok);

                    if(ok && !selected.isEmpty())
                    {
                        context()->setScreen(screens[selected]);
                        move(screens[selected]->geometry().x(), screens[selected]->geometry().y());
                        resize(screens[selected]->geometry().width(),
                               screens[selected]->geometry().height());
                    }
                }

                if(ok)
                {
                    // in fullscreen mode, a thiner (1px) border around
                    // viewer widget
                    if(layout())
                        layout()->setContentsMargins(1, 1, 1, 1);

                    setParent(0);
                    showFullScreen();
                }
            }
        }
        else
        {
            showNormal();
            setMinimumSize(g);
            QMainWindow* _mainwindow = dynamic_cast<QMainWindow*>(parent());
            _mainwindow->setCentralWidget(this);

            if(layout())
                layout()->setContentsMargins(sMargins);

            qApp->processEvents();
            setMinimumSize(QSize(1, 1));
        }
    }
    else // not 'F' key
    {
        // forward event to renderer
        _renderer->keyPressEvent(event);
    }
}

void osgQOpenGLWidget::keyReleaseEvent(QKeyEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->keyReleaseEvent(event);
}

void osgQOpenGLWidget::mousePressEvent(QMouseEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->mousePressEvent(event);
}

void osgQOpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->mouseReleaseEvent(event);
}

void osgQOpenGLWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->mouseDoubleClickEvent(event);
}

void osgQOpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->mouseMoveEvent(event);
}

void osgQOpenGLWidget::wheelEvent(QWheelEvent* event)
{
    Q_ASSERT(_renderer);
    // forward event to renderer
    _renderer->wheelEvent(event);
}

void osgQOpenGLWidget::setDefaultDisplaySettings()
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    ds->setNvOptimusEnablement(1);
    ds->setStereo(false);
}

void osgQOpenGLWidget::createRenderer()
{
    // call this before creating a View...
    setDefaultDisplaySettings();
	if (!_arguments) {
		_renderer = new OSGRenderer(this, enQGLWidget);
	} else {
		_renderer = new OSGRenderer(_arguments, this, enQGLWidget);
	}
	QScreen* screen = windowHandle()
                      && windowHandle()->screen() ? windowHandle()->screen() :
                      qApp->screens().front();
    _renderer->setupOSG(width(), height(), screen->devicePixelRatio());
}
