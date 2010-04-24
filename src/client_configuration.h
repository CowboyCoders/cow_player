#ifndef ___client_configuration_h___
#define ___client_configuration_h___

#include "configuration.h"

#include <string>

namespace cowplayer {

class client_configuration
    : public configuration::base_configuration
{
public:
    client_configuration() {}
    client_configuration(const std::string& fileName) 
        : configuration::base_configuration(fileName) {}

    CONFIGURATION_PROPERTY(fullscreen, bool, false);
    CONFIGURATION_PROPERTY(program_table_url, std::string, "");
    CONFIGURATION_PROPERTY(bittorrent_port, int, 23454);
    CONFIGURATION_PROPERTY(download_dir, std::string, ".");
};

}

#endif // ___client_configuration_h___