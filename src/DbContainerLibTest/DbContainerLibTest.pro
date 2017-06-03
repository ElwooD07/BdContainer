include($$PWD/../props/DBContainer.pri)

QT -= core gui

TARGET = DbContainerTest
TEMPLATE = app
CONFIG += console

INCLUDEPATH += . \
    ./../DbContainerLib/ \
    $$CONTAINER_LIB_INC \
    $$SQLITE_INC \
    $$OPENSSL_INC \
    $$GTEST_INC

LIBS += -L$$DESTDIR -lDbContainerLib
LIBS += -L$$SQLITE_LIB -L$$OPENSSL_LIB -L$$GTEST_LIB -lgtest -lSQLite -llibeay32MD -lssleay32MD

SOURCES += \
    CommonUtilsTest.cpp \
    CryptingTest.cpp \
    main.cpp \
    ShittyProgressObserver.cpp \
    stdafx.cpp \
    TestA.cpp \
    TestB.cpp \
    TestC.cpp \
    TestD.cpp \
    TestE.cpp \
    TestF.cpp \
    TestG.cpp \
    TestH.cpp \
    TestI.cpp \
    TestJ.cpp \
    TestK.cpp \
    Utils.cpp


HEADERS += \
    ShittyProgressObserver.h \
    stdafx.h \
    targetver.h \
    Utils.h

