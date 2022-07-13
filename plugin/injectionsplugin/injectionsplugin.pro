# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# target name
TARGET = $$qtLibraryTarget(savedinjectionsplugin)

# plugin header files
HEADERS += \
    advancedoptions.h \
    form.h \
    injectionsplugin.h \
    newgroupdata.h

# plugin source files
SOURCES += \
    advancedoptions.cpp \
    form.cpp \
    injectionsplugin.cpp \
    newgroupdata.cpp

# plugin forms
FORMS += \
    advancedoptions.ui \
    form.ui \
    newgroupdata.ui
