
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGQOPENGL_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()


################################################################################
## Quieten warnings that a due to optional code paths

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wextra flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

SET(LIB_NAME osgQOpenGL)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})

FIND_PACKAGE(Qt5 COMPONENTS Widgets Gui REQUIRED)

SET(TARGET_H
	#${HEADER_PATH}/GraphicsScene
	#${HEADER_PATH}/GraphicsWindowEx
	#${HEADER_PATH}/OSGRenderer
	#${HEADER_PATH}/RenderStageEx
	#${HEADER_PATH}/StateEx
	#${HEADER_PATH}/TestWidget

    ${HEADER_PATH}/Export
	${HEADER_PATH}/osgQOpenGLView
	${HEADER_PATH}/osgQOpenGLWidget
	${HEADER_PATH}/osgQOpenGLWindow
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
	CullVisitorEx.cpp
	GraphicsScene.cpp
	GraphicsWindowEx.cpp
	osgQOpenGLView.cpp
	osgQOpenGLWidget.cpp
	osgQOpenGLWindow.cpp
	OSGRenderer.cpp
	RenderStageEx.cpp
	StateEx.cpp
	TestWidget.cpp

	${OPENSCENEGRAPH_VERSIONINFO_RC}
)

set(moc_h
	OSGRenderer
	TestWidget
	${TARGET_H}
)

qt5_wrap_cpp(mocfiles ${moc_h})
source_group("moc" FILES ${mocfiles})

SET(TARGET_H_NO_MODULE_INSTALL
	GraphicsScene
	GraphicsWindowEx
	OSGRenderer
	RenderStageEx
	StateEx

	${mocfiles}
)

SET(TARGET_LIBRARIES
    osg
    osgUtil
	osgViewer
    OpenThreads

	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Qt library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})

