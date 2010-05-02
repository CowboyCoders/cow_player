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

#ifndef ___configuration_h___
#define ___configuration_h___

#include <boost/lexical_cast.hpp>

#include <string>
#include <map>
#include <sstream>
#include <stdexcept>

#include "cow_player_types.h"

namespace cow_player {
namespace configuration {

namespace exceptions {
/**
 * Load config error that can be thrown when it's not possible
 * to load the configuration from file.
 */
class load_config_error : public std::runtime_error
{
public:
   /**
    * Throws the load config error.
    */
    load_config_error(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * Save config error that can be thrown when it's not possible
 * to save the configuration to file.
 */
class save_config_error : public std::runtime_error
{
public:
   /**
    * Throws the save config error.
    */
    save_config_error(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * Conversion error that can be thrown when a property can't
 * be converted.
 */
class conversion_error : public std::runtime_error
{
public:
   /**
    * Throws the conversion error.
    */
    conversion_error(const std::string& msg) : std::runtime_error(msg) {}
};

}

/**
 * This class is responsible for generating and retrieving a storable representation
 * for a value of type T. 
 */
template <typename T>
class serializer
{
public:
   /**
    * When we try to serialize a value of unknown type, we will throw an error.
    * @param value The value to try to serialize.
    * @return A pair variable that stores the original value.
    */
    static cow_player::property_value serialize(T value) {
        throw exceptions::conversion_error("Missing serializer for the specified type."); 
    }

   /**
    * When we try to deserialize a value of unknown type, we will throw an error.
    * @param prop_value The value to try to deserialize.
    * @return The original value of type T.
    */
    static T deserialize(const std::string& prop_value) { 
        throw exceptions::conversion_error("Missing deserializer for the specified type."); 
    }
};

/**
 * This class is responsible for generating and retrieving a storable representation
 * for a value of type string. 
 */
template <>
class serializer<std::string>
{
public:
   /**
    * Stores the specified value in a property_value representation.
    * @param value The value to serialize.
    * @return A pair variable that stores the original value.
    */
    static cow_player::property_value serialize(const std::string& value)
    {
        return std::make_pair("string", value);
    }

   /**
    * Retrieves the original value for the specified prop_value.
    * @param prop_value The value to try to deserialize.
    * @return The original value of type string.
    */
    static std::string deserialize(const std::string& prop_value)
    {
        return prop_value;
    }
};

/**
 * This class is responsible for generating and retrieving a storable representation
 * for a value of type int. 
 */
template <>
class serializer<int>
{
public:
   /**
    * Stores the specified value in a property_value representation.
    * @param value The value to serialize.
    * @return A pair variable that stores the original value.
    */
    static cow_player::property_value serialize(int value)
    {
        return std::make_pair("int", boost::lexical_cast<std::string>(value));
    }

   /**
    * Retrieves the original value for the specified prop_value.
    * @param prop_value The value to try to deserialize.
    * @return The original value of type int.
    */
    static int deserialize(const std::string& prop_value)
    {
        try {
            return boost::lexical_cast<int>(prop_value);
        } catch (boost::bad_lexical_cast) {
            throw exceptions::conversion_error("Failed to convert data to a integer value.");
        }
    }
};

/**
 * This class is responsible for generating and retrieving a storable representation
 * for a value of type bool. 
 */
template <>
class serializer<bool>
{
public:
   /**
    * Stores the specified value in a property_value representation.
    * @param value The value to serialize.
    * @return A pair variable that stores the original value.
    */
    static cow_player::property_value serialize(bool value)
    {
        return std::make_pair("bool", boost::lexical_cast<std::string>(value));
    }

   /**
    * Retrieves the original value for the specified prop_value.
    * @param prop_value The value to try to deserialize.
    * @return The original value of type bool.
    */
    static bool deserialize(const std::string& prop_value)
    {
        try {
            return boost::lexical_cast<bool>(prop_value);
        } catch (boost::bad_lexical_cast) {
            throw exceptions::conversion_error("Failed to convert data to a boolean value.");
        }
    }
};

#define CONFIGURATION_PROPERTY(name, type, default_value) \
    void set_##name(type value) { set_property(#name, value); } \
    type get_##name() const { return get_property<type>(#name, default_value); }




/**
 *  This class is a general configuration class that loads
 *  and saves configurations.
 */
class base_configuration
{
public:
    
    base_configuration();

   /**
    * Creates a new cowplayer::configuration::base_configuration object
    * and loads settings from the specified file.
    * @param fileName The file to load settings from.
    */
    base_configuration(const std::string& fileName);

   /**
    * Load a configuration from a file.
    * @param fileName The file to load settings from.
    */
    void load(const std::string& fileName);
   /**
    * Save the current configuration to file.
    * @param fileName The file to save settings to.
    */
    void save(const std::string& fileName) const;

   /**
    * Sets a property in the configuration.
    * @param name The name of the property to set.
    * @param value The value of the property to set.
    */
    template <typename T>
    void set_property(const std::string& name, T value)
    {
        property_map_[name] = serializer<T>::serialize(value);
    }

   /**
    * Tries to retrieve the value for the specified property name.
    * If no value is found, default_value will be returned instead.
    * @param name The name of the property to retrive.
    * @param default_value The default value to return if the property isn't found.
    * @return A value of type T.
    */
    template <typename T>
    T get_property(const std::string& name, T default_value) const
    {
        cow_player::property_map::const_iterator item = property_map_.find(name);
        if (item == property_map_.end())
            return default_value;

        try {
            return serializer<T>::deserialize(item->second.second);
        } catch (exceptions::conversion_error) {
            return default_value;
        }
    }

    /**
     * Returns the property_map (a key,value) map containing 
     * all the settings and their values
     *
     * @return A property_map with all the read settings
     */
    cow_player::property_map settings() const {
        return property_map_;
    }
    
    /**
     * Sets the property_map (a key,value) map containing 
     * all the settings and their values
     *
     * @param A property_map with all the settings
     */
    void settings(cow_player::property_map map){
        property_map_ = map;
    }


private:
    cow_player::property_map property_map_;
};

}
}

#endif // ___configuration_h___
