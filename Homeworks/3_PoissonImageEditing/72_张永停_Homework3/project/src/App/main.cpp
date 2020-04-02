#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <stdlib.h>  
#include <crtdbg.h>

int main(int argc, char *argv[])
{
	_CrtSetBreakAlloc(149);
	QApplication a(argc, argv);
	_CrtDumpMemoryLeaks();
	MainWindow w;
    w.setWindowTitle("Poisson Image Edit");
    w.setWindowIcon(QIcon("../Resources/images/title.png"));
	w.show();
	return a.exec();
}
