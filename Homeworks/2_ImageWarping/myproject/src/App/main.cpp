#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <stdlib.h>  
#include <crtdbg.h>

int main(int argc, char *argv[])
{
	//_CrtSetBreakAlloc(6596);
	QApplication a(argc, argv);
	//_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	_CrtDumpMemoryLeaks();
	MainWindow w;
    w.setWindowTitle("Image warping");
    w.setWindowIcon(QIcon(":/MainWindow/Resources/images/title.png"));
	w.show();
	return a.exec();
}
