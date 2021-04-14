# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# target name
TARGET = $$qtLibraryTarget(testingframeworkplugin)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    actionlistprocessor.cpp \
    testcase.cpp \
    testingframeworkplugin.cpp \
    testingframeworkpluginform.cpp

HEADERS += \
    actionlistprocessor.h \
    testcase.h \
    testingframeworkplugin.h \
    testingframeworkpluginform.h

FORMS += \
    testingframeworkpluginform.ui
