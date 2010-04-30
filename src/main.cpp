#include <QtGui/QApplication>
#include <iostream>
#include "main_window.h"

#include "client_configuration.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setApplicationName( "Cow player" ); //needed for Phonon/ DBUS 
    main_window w;

    w.show();

    return a.exec();
}
