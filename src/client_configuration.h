/*
Copyright 2010 CowboyCoders. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY COWBOYCODERS ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COWBOYCODERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of CowboyCoders.
*/

#ifndef ___client_configuration_h___
#define ___client_configuration_h___

#include "configuration.h"

#include <string>

namespace cow_player {
/**
 *  This class is responsible for storing client settings.
 */
class client_configuration
    : public configuration::base_configuration
{
public:
    client_configuration() {}
   /**
    * Creates a new cowplayer::client_configuration that loads
    * the configuration from a file.
    * @param fileName The file to load.
    */
    client_configuration(const std::string& fileName) 
        : configuration::base_configuration(fileName) {}

   /**
    * A configuration property for setting fullscreen mode.
    */
    CONFIGURATION_PROPERTY(fullscreen, bool, false);
   /**
    * A configuration property for setting the URL to the XML file that libcow::program_table will use.
    */
    CONFIGURATION_PROPERTY(program_table_url, std::string, "");
   /**
    * A configuration property for setting the port to use for BitTorrent downloads.
    */
    CONFIGURATION_PROPERTY(bittorrent_port, int, 55678);
   /**
    * A configuration property for setting the directory to download files to.
    */
    CONFIGURATION_PROPERTY(download_dir, std::string, ".");
   
   /**
    * A configuration property for setting the number of pieces in the critical window.
    */
    CONFIGURATION_PROPERTY(critical_window, int, 5);
   
   /**
    * A configuration property for setting the timeout of a piece in the critical window
    */
    CONFIGURATION_PROPERTY(critical_window_timeout, int, 3000);
};

}

#endif // ___client_configuration_h___
