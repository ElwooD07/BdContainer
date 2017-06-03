include($$PWD/../../props/DBContainer.pri)

QT -= core gui

TARGET = DbContainerLib
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += . \
    .. \
    ./Utils/ \
    $$SQLITE_INC \
    $$OPENSSL_INC

SOURCES += \
    Connection.cpp \
    Container.cpp \
    ContainerAPI.cpp \
    ContainerDefragmenter.cpp \
    ContainerException.cpp \
    ContainerInfoImpl.cpp \
    ContainerResources.cpp \
    Crypto.cpp \
    DataStorageBinaryFile.cpp \
    DataUsagePreferences.cpp \
    DefragProxyProgressObserver.cpp \
    DirectLink.cpp \
    Element.cpp \
    ElementProperties.cpp \
    ElementsIterator.cpp \
    ElementsSyncKeeper.cpp \
    ErrorCodes.cpp \
    File.cpp \
    FileStreamsAllocator.cpp \
    FileStreamsManager.cpp \
    Folder.cpp \
    ProxyProgressObserver.cpp \
    SQLQuery.cpp \
    SymLink.cpp \
    TransactionGuard.cpp \
    Utils/CommonUtils.cpp \
    Utils/FileStreamsUtils.cpp \
    Utils/FsUtils.cpp \
    Utils/Logging.cpp

HEADERS += \
    ../ContainerAPI.h \
    ../ContainerException.h \
    ../ContainerResources.h \
    ../DataUsagePreferences.h \
    ../DirectLink.h \
    ../Element.h \
    ../ElementProperties.h \
    ../ElementsIterator.h \
    ../ErrorCodes.h \
    ../File.h \
    ../Folder.h \
    ../IContainer.h \
    ../IContainerInfo.h \
    ../IDataStorage.h \
    ../IDefragProgressObserver.h \
    ../IProgressObserver.h \
    ../Iterator.h \
    ../SymLink.h \
    ../Types.h \
    Connection.h \
    Container.h \
    ContainerDefragmenter.h \
    ContainerInfoImpl.h \
    ContainerResourcesImpl.h \
    Crypto.h \
    DataStorageBinaryFile.h \
    DefragProxyProgressObserver.h \
    ElementsSyncKeeper.h \
    FileStreamsAllocator.h \
    FileStreamsManager.h \
    IContainnerResources.h \
    ProxyProgressObserver.h \
    SQLQuery.h \
    StreamInfo.h \
    TransactionGuard.h \
    TypesInternal.h \
    Utils/CommonUtils.h \
    Utils/FileStreamsUtils.h \
    Utils/FsUtils.h \
    Utils/Logging.h
