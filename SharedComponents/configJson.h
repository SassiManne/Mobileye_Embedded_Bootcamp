#ifndef CONFIGFACTORY_H
#define CONFIGFACTORY_H

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigFactory {
private:
    json configData ;  // Private member to store the JSON data

    static ConfigFactory* instance;  // Pointer to the single instance of the class

    // Private constructor to prevent direct instantiation
    ConfigFactory();

    // Create default server configuration
    static json createServerConfig();

    // Create default client configuration
    static json createClientConfig();

    // Load JSON data from a given file
    void loadFromFile(const std::string& filepath);

    // Save JSON data to a given file
    void saveToFile(const std::string& filepath);

public:
    // Public method to access the single instance of the class
    static ConfigFactory* getInstance();

    // Factory method to get the configuration.
    json getConfig(const std::string& context);

    // Update a configuration value and save it to the file.
    void ConfigFactory::updateConfigValue(const std::string& key, const json& value, const std::string& context);
};

#endif // CONFIGFACTORY_H