#ifndef ___client_configuration_h___
#define ___client_configuration_h___

#include "configuration.h"

#include <string>

namespace cow_player {

class client_configuration
    : public configuration::base_configuration
{
public:
    client_configuration() {}
    client_configuration(const std::string& fileName) 
        : configuration::base_configuration(fileName) {}

    CONFIGURATION_PROPERTY(fullscreen, bool, false);
    CONFIGURATION_PROPERTY(hello, std::string, "abc");
};

}

#endif // ___client_configuration_h___