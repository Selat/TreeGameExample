#include <QApplication>
#include <ctime>

#include "mainwidget.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	app.setOverrideCursor(Qt::BlankCursor);

	srand(time(NULL));
	
	MainWidget mainWidget;
	mainWidget.show();
	
	return app.exec();
}