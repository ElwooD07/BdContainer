include($$PWD/../props/DBContainer.pri)

TARGET = DbContainer
TEMPLATE = app
CONFIG += qt
QT += core gui widgets

INCLUDEPATH += . \
    ./GeneratedFiles/ \
    ./../DbContainerLib/ \

LIBS += -L$$DESTDIR -lDbContainerLib
LIBS += -L$$SQLITE_LIB -L$$OPENSSL_LIB -lSQLite -llibeay32MD -lssleay32MD

SOURCES += \
    DbContainerModel.cpp \
    main.cpp \
    ModelUtils.cpp \
    stdafx.cpp \
    TreeItemDelegate.cpp \
    TreeNode.cpp \
    Widgets/ChooseContainerDialog.cpp \
    Widgets/ElementViewWidget.cpp \
    Widgets/FsTreeMenuElement.cpp \
    Widgets/FsTreeMenuTree.cpp \
    Widgets/FsTreeWidget.cpp \
    Widgets/MainWindow.cpp


HEADERS += \
    DbContainerModel.h \
    MainWindowView.h \
    ModelUtils.h \
    resource.h \
    TreeItemDelegate.h \
    TreeNode.h \
    Widgets/ChooseContainerDialog.h \
    Widgets/ElementViewWidget.h \
    Widgets/FsTreeMenuElement.h \
    Widgets/FsTreeMenuTree.h \
    Widgets/FsTreeWidget.h \
    Widgets/MainWindow.h

FORMS += \
    Ui/ChooseContainerDialog.ui \
    Ui/ElementViewWidget.ui \
    Ui/FsTreeWidget.ui \
    Ui/MainWindow.ui
