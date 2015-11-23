#include "stdafx.h"
#include "Widgets/MainWindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	gui::MainWindow w;
	w.show();
	return a.exec();
}
