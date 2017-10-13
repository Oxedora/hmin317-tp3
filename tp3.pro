QT       += core gui widgets

TARGET = plane
TEMPLATE = app

SOURCES += main.cpp \
    calendrier.cpp

SOURCES += \
    mainwidget.cpp \
    geometryengine.cpp

HEADERS += \
    mainwidget.h \
    geometryengine.h \
    calendrier.h

RESOURCES += \
    shaders.qrc \
    textures.qrc

# install
target.path = .
INSTALLS += target
