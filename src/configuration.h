#ifndef ___configuration_h___
#define ___configuration_h___

#include <boost/lexical_cast.hpp>

#include <string>
#include <map>
#include <sstream>
#include <stdexcept>

namespace cow_player {
namespace configuration {

namespace exceptions {
class load_config_error : public std::runtime_error
{
public:
    load_config_error(const std::string& msg) : std::runtime_error(msg) {}
};

class save_config_error : public std::runtime_error
{
public:
    save_config_error(const std::string& msg) : std::runtime_error(msg) {}
};

class conversion_error : public std::runtime_error
{
public:
    conversion_error(const std::string& msg) : std::runtime_error(msg) {}
};

}

// fst: type
// snd: value
typedef std::pair<std::string, std::string> property_value;

template <typename T>
class serializer
{
public:
    static property_value serialize(T value) {
        throw exceptions::conversion_error("Missing serializer for the specified type."); 
    }

    static T deserialize(const std::string& prop_value) { 
        throw exceptions::conversion_error("Missing deserializer for the specified type."); 
    }
};

template <>
class serializer<std::string>
{
public:
    static property_value serialize(const std::string& value)
    {
        return std::make_pair("string", value);
    }

    static std::string deserialize(const std::string& prop_value)
    {
        return prop_value;
    }
};

template <>
class serializer<bool>
{
public:
    static property_value serialize(bool value)
    {
        return std::make_pair("bool", boost::lexical_cast<std::string>(value));
    }

    static bool deserialize(const std::string& prop_value)
    {
        try {
            return boost::lexical_cast<bool>(prop_value);
        }
        catch (boost::bad_lexical_cast) {
            throw exceptions::conversion_error("Failed to convert data to a boolean value.");
        }
    }
};

#define CONFIGURATION_PROPERTY(name, type, default_value) \
    void set_##name(type value) { set_property(#name, value); } \
    type get_##name() const { return get_property<type>(#name, default_value); }




class base_configuration
{
public:
    base_configuration();
    base_configuration(const std::string& fileName);

    void load(const std::string& fileName);
    void save(const std::string& fileName) const;

    template <typename T>
    void set_property(const std::string& name, T value)
    {
        property_map_[name] = serializer<T>::serialize(value);
    }

    template <typename T>
    T get_property(const std::string& name, T default_value) const
    {
        property_map::const_iterator item = property_map_.find(name);
        if (item == property_map_.end())
            return default_value;

        return serializer<T>::deserialize(item->second.second);
    }

private:
    typedef std::map<std::string, property_value> property_map;

    property_map property_map_;
};

}
}

#endif // ___configuration_h___
