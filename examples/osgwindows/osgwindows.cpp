/* OpenSceneGraph example, osgwindows.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>


#include <iostream>

#include <osgViewer/ImGuiHandler>
#include <osgViewer/imgui/imgui.h>

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);

    auto geo = new osg::Geometry;
    auto vertx = new osg::Vec3Array;
    vertx->push_back({ -10, -10, 0 });
    vertx->push_back({  10, -10, 0 });
    vertx->push_back({  0,  10, 0 });
    geo->setVertexArray(vertx);
    geo->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 3));
    geo->setUseVertexArrayObject(false);
    loadedModel = geo;

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readRefNodeFile("cow.osgt");

    // if no model has been successfully loaded report failure.
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // construct the viewer.
    osgViewer::Viewer viewer;
    viewer.addEventHandler(new osgViewer::ImGuiHandler);

    class xxUpdate : public osg::NodeCallback {
    public:

        void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            auto fm = nv->getFrameStamp()->getFrameNumber();
            if (fm < 10)
                return;
            ImGui::Begin("hello world");
            ImGui::Text("This is some useful text.");
            ImGui::Button("Button");
            ImGui::SameLine();
            ImGui::Text("counter = ");
            ImGui::Text("Application average 3f ms/frame (1f FPS)");
            ImGui::End();
        }
    };

    auto xxNode = new osg::Geometry;
    auto grp = new osg::Group;
    grp->addChild(loadedModel);
    grp->addChild(xxNode);
    xxNode->setUpdateCallback(new xxUpdate);
    loadedModel = grp;

    int xoffset = 40;
    int yoffset = 40;

    // left window + left slave camera

    osg::ref_ptr<osg::GraphicsContext> gc;
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

        traits->x = xoffset + 0;
        traits->y = yoffset + 0;
        traits->width = 600;
        traits->height = 480;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        gc->realize();

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0, traits->width, traits->height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        // add this slave camera to the viewer, with a shift left of the projection matrix
        viewer.addSlave(camera.get(), osg::Matrixd::translate(1,0.0,0.0), osg::Matrixd());
    }

    // right window + right slave camera
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = xoffset + 600;
        traits->y = yoffset + 0;
        traits->width = 600;
        traits->height = 480;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = gc;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        osg::ref_ptr<osg::GraphicsContext> gc1 = osg::GraphicsContext::createGraphicsContext(traits.get());
        gc1->realize();

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc1.get());
        camera->setViewport(new osg::Viewport(0,0, traits->width, traits->height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        // add this slave camera to the viewer, with a shift right of the projection matrix
        viewer.addSlave(camera.get(), osg::Matrixd::translate(-1,0.0,0.0), osg::Matrixd());

        gc1->getState()->setCheckForGLErrors(osg::State::ONCE_PER_ATTRIBUTE);
    }


    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel);

    // set the scene to render
    viewer.setSceneData(loadedModel);

    return viewer.run();
}

