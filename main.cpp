#include <QApplication>
#include "Arranger.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	Arranger arranger;
	arranger.show();

	return app.exec();
}
