PROJECT(UnityToOSG)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7)
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix for Debug mode, usually d on windows")

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
INCLUDE(CheckIncludeFiles)

IF(WIN32)
    IF(MSVC)
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    ENDIF(MSVC)
    
    IF(NOT CMAKE_CL_64)
        # /LARGEADDRESSAWARE enables 32-bit applications to use more than 2 GB RAM
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
    ENDIF(NOT CMAKE_CL_64)
ELSE(WIN32)
    SET(CMAKE_CXX_FLAGS "-W -Wall -Wno-unused")
ENDIF(WIN32)

FIND_PATH(OPENSCENEGRAPH_INCLUDE_DIR osg/Referenced
    PATHS
    $ENV{OSG_ROOT}/include
    $ENV{OSG_DIR}/include
    /usr/include 
    /usr/local/include
)

FIND_PATH(OPENSCENEGRAPH_LIB_DIR libosg.so osg.lib
    PATHS
    $ENV{OSG_ROOT}/lib
    $ENV{OSG_DIR}/lib
    /usr/lib 
    /usr/local/lib
)

FIND_PACKAGE(OpenGL)
INCLUDE_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
LINK_DIRECTORIES(${OPENSCENEGRAPH_LIB_DIR})

SET(SOURCE_FILES
    viewer.cpp
    shader_data.cpp
    terrain_data.cpp
    particle_data.cpp
    utilities.cpp
    user_data_classes.h
    
    Particle.h
    Particle.cpp
    spark/SPK_All.cpp
    spark/SPK_GL_All.cpp
)

SET(COMMON_SHADER_FILES
    Default.vert
    Default.frag
    Terrain.vert
    Terrain.frag
)

SET(LEGACY_SHADER_FILES
    Legacy_Shaders/Bumped_Diffuse.frag
)

ADD_EXECUTABLE(osgUnitySceneViewer ${SOURCE_FILES})
SET_TARGET_PROPERTIES(osgUnitySceneViewer PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

TARGET_LINK_LIBRARIES(osgUnitySceneViewer
    debug osg${CMAKE_DEBUG_POSTFIX}          optimized osg
    debug osgAnimation${CMAKE_DEBUG_POSTFIX} optimized osgAnimation
    debug osgParticle${CMAKE_DEBUG_POSTFIX}  optimized osgParticle
    debug osgShadow${CMAKE_DEBUG_POSTFIX}    optimized osgShadow
    debug osgDB${CMAKE_DEBUG_POSTFIX}        optimized osgDB
    debug osgGA${CMAKE_DEBUG_POSTFIX}        optimized osgGA
    debug osgText${CMAKE_DEBUG_POSTFIX}      optimized osgText
    debug osgUtil${CMAKE_DEBUG_POSTFIX}      optimized osgUtil
    debug osgViewer${CMAKE_DEBUG_POSTFIX}    optimized osgViewer
    debug OpenThreads${CMAKE_DEBUG_POSTFIX}  optimized OpenThreads
    ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS osgUnitySceneViewer RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
INSTALL(FILES ${COMMON_SHADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/shaders)
INSTALL(FILES ${LEGACY_SHADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/shaders/Legacy_Shaders)
