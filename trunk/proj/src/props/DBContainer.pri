CONFIG(debug, debug|release) {
	CONFIGNAME=Debug
} else {
	CONFIGNAME=Release
}

SRC_ROOT=$$PWD/../
DESTDIR=$$SRC_ROOT/../bin/$$CONFIGNAME/
LIBDIR=$$SRC_ROOT/../lib/$$CONFIGNAME/
OBJECTS_DIR=$$SRC_ROOT/../obj/$$TARGET/$$CONFIGNAME

DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/$$CONFIGNAME
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles

win32 {
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_RELEASE -= -MT
    QMAKE_CXXFLAGS_DEBUG   += -MDd
    QMAKE_CXXFLAGS_DEBUG   -= -MTd
    QMAKE_CFLAGS_RELEASE += -MD
    QMAKE_CFLAGS_RELEASE -= -MT
    QMAKE_CFLAGS_DEBUG   += -MDd
    QMAKE_CFLAGS_DEBUG   -= -MTd
}

CONFIG += precompile_header
PRECOMPILED_HEADER = stdafx.h
win32-msvc* {
PRECOMPILED_SOURCE = stdafx.cpp
}

include($$SRC_ROOT/../../../externals/props/externals.pri)
