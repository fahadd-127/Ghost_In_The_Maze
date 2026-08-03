TEMPLATE    = app
QT         += opengl 

HEADERS += src/MyForm.h src/BaseGLWidget.h src/MySceneWidget.h

SOURCES += src/main.cpp src/MyForm.cpp src/BaseGLWidget.cpp src/MySceneWidget.cpp Model/model.cpp

INCLUDEPATH += glm-master  src  Model

FORMS += ui/MyForm.ui

# =====================================
# Assimp library and helper classes
#
HEADERS += ./assimp/Mesh.h 
SOURCES +=  ./assimp/Mesh.cpp ./assimp/ogldev_texture.cpp ./assimp/ogldev_util.cpp ./assimp/3rdparty/stb_image.cpp

# In the event that the provided libraries do not work in your OS/computer, you must compile assimp to obtain the binaries.
# To do so, it is recommanded to build with vcpkg: 
# Build on all platforms using vcpkg
# You can download and install Assimp using the vcpkg dependency manager:
# 
# bash
# git clone https://github.com/Microsoft/vcpkg.git
# cd vcpkg
# ./bootstrap-vcpkg.sh
# ./vcpkg integrate install
# vcpkg install assimp
#
# (More instruccions are included in: https://the-asset-importer-lib-documentation.readthedocs.io/en/latest/about/quickstart.html)
#
# After the setup, you should copy the *.a files (see below) from  the folder [vcpkg]/installed/x64-linux/lib/ to [project folder]/assimp/lib.

INCLUDEPATH += ../assimp/include

LIBS += ../assimp/lib/libassimp.a
LIBS += ../assimp/lib/libdraco.a
LIBS += ../assimp/lib/libkubazip.a
LIBS += ../assimp/lib/libminizip.a
LIBS += ../assimp/lib/libpoly2tri.a
LIBS += ../assimp/lib/libpolyclipping.a
LIBS += ../assimp/lib/libpugixml.a
LIBS += ../assimp/lib/libz.a

# =====================================

QMAKE_CXXFLAGS += -Wno-ignored-qualifiers -Wno-type-limits -Wno-unused-parameter -fcompare-debug-second