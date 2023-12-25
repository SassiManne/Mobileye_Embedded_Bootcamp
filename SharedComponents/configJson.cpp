#include <iostream>
#include <fstream>
#include "configJson.h" 

using json = nlohmann::json;

ConfigFactory* ConfigFactory::instance = nullptr;

// Private constructor to prevent direct instantiation
ConfigFactory::ConfigFactory() {}

// Create default server configuration
json ConfigFactory::createServerConfig() {
	json serverConfig;

	// Define the objects list
    json objects = {
    {"0", "person"},
    {"1", "bicycle"},
    {"2", "car"},
    {"3", "motorcycle"},
    {"4", "airplane"},
    {"5", "bus"},
    {"6", "train"},
    {"7", "truck"},
    {"8", "boat"},
    {"9", "traffic light"},
    {"10", "fire hydrant"},
    {"11", "stop sign"},
    {"12", "parking meter"},
    {"13", "bench"},
    {"14", "bird"},
    {"15", "cat"},
    {"16", "dog"},
    {"17", "horse"},
    {"18", "sheep"},
    {"19", "cow"},
    {"20", "elephant"},
    {"21", "bear"},
    {"22", "zebra"},
    {"23", "giraffe"},
    {"24", "backpack"},
    {"25", "umbrella"},
    {"26", "handbag"},
    {"27", "tie"},
    {"28", "suitcase"},
    {"29", "frisbee"},
    {"30", "skis"},
    {"31", "snowboard"},
    {"32", "sports ball"},
    {"33", "kite"},
    {"34", "baseball bat"},
    {"35", "baseball glove"},
    {"36", "skateboard"},
    {"37", "surfboard"},
    {"38", "tennis racket"},
    {"39", "bottle"},
    {"40", "wine glass"},
    {"41", "cup"},
    {"42", "fork"},
    {"43", "knife"},
    {"44", "spoon"},
    {"45", "bowl"},
    {"46", "banana"},
    {"47", "apple"},
    {"48", "sandwich"},
    {"49", "orange"},
    {"50", "broccoli"},
    {"51", "carrot"},
    {"52", "hot dog"},
    {"53", "pizza"},
    {"54", "donut"},
    {"55", "cake"},
    {"56", "chair"},
    {"57", "couch"},
    {"58", "potted plant"},
    {"59", "bed"},
    {"60", "dining table"},
    {"61", "toilet"},
    {"62", "tv"},
    {"63", "laptop"},
    {"64", "mouse"},
    {"65", "remote"},
    {"66", "keyboard"},
    {"67", "cell phone"},
    {"68", "microwave"},
    {"69", "oven"},
    {"70", "toaster"},
    {"71", "sink"},
    {"72", "refrigerator"},
    {"73", "book"},
    {"74", "clock"},
    {"75", "vase"},
    {"76", "scissors"},
    {"77", "teddy bear"},
    {"78", "hair drier"},
    {"79", "toothbrush"}
    };

	// Add the objects list to the server configuration
	serverConfig["objects"] = objects;

	serverConfig["queueSize"] = 5,
	serverConfig["IP_address"] = "0.0.0.0:50050";
	return serverConfig;
}

// Create default client configuration
json ConfigFactory::createClientConfig() {
	json clientConfig;
	clientConfig["threshold"] = 0.5,
	clientConfig["queueSize"] = 5,
	clientConfig["server_address"] = "127.0.0.1:50050";
    clientConfig["video_path"] = "0"; // 0 for computer camera

	return clientConfig;
}

// Load JSON data from a given file
void ConfigFactory::loadFromFile(const std::string& filepath) {
	std::ifstream inFile(filepath);
	inFile >> configData;
}

// Save JSON data to a given file
void ConfigFactory::saveToFile(const std::string& filepath) {
	std::ofstream outFile(filepath);
	outFile << configData.dump(4);  // Indented with 4 spaces.
}



// Public method to access the single instance of the class
ConfigFactory* ConfigFactory::getInstance() {
	if (!instance) {
		instance = new ConfigFactory();
	}
	return instance;
}

// Factory method to get the configuration.
// If the file doesn't exist, it creates it with default values.
// Then loads the JSON data into the private member and returns it.
json ConfigFactory::getConfig(const std::string& context) {
    // If the configData is not null, return it.
    if(!this->configData.is_null()) return this->configData ;

	std::string filepath = (context == "server") ? "../../CVServer/serverConfig.json" : "../../CVClient/clientConfig.json";

	std::ifstream file(filepath);
	if (!file.good()) {  // If file doesn't exist, create it.
		configData = (context == "server") ? createServerConfig() : createClientConfig();
		saveToFile(filepath);
	}
	else {
		loadFromFile(filepath);
	}
	return configData;
}

// Update a configuration value and save it to the file.
void ConfigFactory::updateConfigValue(const std::string& key, const json& value, const std::string& context) {
    // Update the value in memory
    this->configData[key] = value;

    // Determine the file path based on the context
    std::string filepath = (context == "server") ? "../../CVServer/serverConfig.json" : "../../CVClient/clientConfig.json";

    // Save the updated configuration to the file
    saveToFile(filepath);
}






