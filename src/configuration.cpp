#include "configuration.h"
#include "tinyxml.h"

const std::string configuration_property_tag_name = "property";
const std::string configuration_property_name_attribute = "name";
const std::string configuration_property_type_attribute = "type";

using namespace cowplayer::configuration;

base_configuration::base_configuration()
{
}

base_configuration::base_configuration(const std::string& fileName)
{
    load(fileName);
}

void base_configuration::load(const std::string& fileName)
{
    TiXmlDocument document;
    if (!document.LoadFile(fileName)) {
        throw exceptions::load_config_error("Failed to open the configuration file.");
    }

    TiXmlElement* root = document.RootElement();

    for (TiXmlElement* elem = root->FirstChildElement(); 
        elem != NULL; elem = elem->NextSiblingElement()) 
    {
        if (elem->ValueStr() == configuration_property_tag_name) {
            const std::string* property_name = elem->Attribute(configuration_property_name_attribute);
            if (!property_name) {
                throw exceptions::load_config_error("Property element is missing name attribute.");
            }

            const std::string* property_type = elem->Attribute(configuration_property_type_attribute);
            if (!property_type) {
                throw exceptions::load_config_error("Property element is missing type attribute.");
            }

            std::string property_value(elem->GetText());

            property_map_[*property_name] = std::make_pair(*property_type, property_value);
        }
    }
}

void base_configuration::save(const std::string& fileName) const
{
    TiXmlDocument document;
    TiXmlElement root("properties");

    document.InsertEndChild(TiXmlDeclaration("1.0", "utf-8", ""));

    property_map::const_iterator it = property_map_.begin();
    for (; it != property_map_.end(); ++it) {
        // Create a property element
        TiXmlElement property_node(configuration_property_tag_name);

        // Set property attributes
        property_node.SetAttribute(configuration_property_name_attribute, it->first);
        property_node.SetAttribute(configuration_property_type_attribute, it->second.first);

        // Add property value
        property_node.InsertEndChild(TiXmlText(it->second.second));

        // Insert property element in the tree
        root.InsertEndChild(property_node);
    }

    // Insert root element into the document
    document.InsertEndChild(root);

    if (!document.SaveFile(fileName)) {
        throw exceptions::save_config_error("Failed to save the configuration file to disk.");
    }
}
