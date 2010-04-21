#include <QtGui/QApplication>
#include <iostream>
#include "main_window.h"

#include "client_configuration.h"

int main(int argc, char *argv[])
{
    cow_player::client_configuration config;

	try {
        config.load("client.xml");
	} catch (cow_player::configuration::exceptions::load_config_error e) {
		std::cerr << "warning: could not open config file!" << std::endl;
	}

    bool test = config.get_fullscreen();
    std::string x = config.get_hello();

    config.set_hello("woot");
    std::string t = config.get_hello();

    config.set_hello("yadayada");
    t = config.get_hello();


    config.save("client.xml");

    QApplication a(argc, argv);
	a.setApplicationName( "Cow player" ); //needed for Phonon/ DBUS 
    main_window w;
    w.show();

    return a.exec();
}
