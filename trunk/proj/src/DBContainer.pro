TEMPLATE = subdirs
SUBDIRS += DbContainerLib DbContainerLibTest DbContainer

DbContainerLib.file = DbContainerLib/impl/DbContainerLib.pro
DbContainerLibTest.file = DbContainerLibTest/DbContainerLibTest.pro
DbContainer.file = DbContainer/DbContainer.pro

DbContainerLibTest.depends = DbContainerLib
DbContainer.depends = DbContainerLib
