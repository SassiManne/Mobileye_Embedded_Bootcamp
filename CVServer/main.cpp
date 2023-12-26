#include "../SharedComponents/Logger.h"
#include <opencv2/opencv.hpp>
#include "IBackend.hpp"
#include "yoloBackend.h"
#include "../SharedComponents/SafeQueue.cpp"
#include <sqlite3.h>
#include"SqlDB.h"
#include "ImageReader.hpp"
#include "GRPCImageReader.h"
#include <thread>


#ifdef _DEBUG
// the debug version that corresponds to the opencv_world420d.dll:
    #pragma comment(lib, "opencv_world480d.lib")
#else
// the release version that corresponds to the opencv_world420.dll:
    #pragma comment(lib, "opencv_world480.lib")
#endif

// Get the server configuration from the configuration file 
ConfigFactory* configFactory = ConfigFactory::getInstance();
json serverConfig = configFactory->getConfig("server");

std::atomic<bool> isRunning(true);  // Atomic boolean to control the running state of threads


// Function to print all available objects from the server configuration
void printAvailableObjects(const json& serverConfig) {
    // Check if the "objects" section exists in the server configuration
    if (serverConfig.contains("objects")) {
        // Retrieve the "objects" section from the server configuration
        json objects = serverConfig["objects"];
        std::cout << "Available objects:" << std::endl;
        // Iterate through each key-value pair in the "objects" section
        for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
            // Print the key (object ID) and value (object name) to the console
            std::cout << it.key() << ": " << it.value() << std::endl;
        }
    }
    else {
        std::cerr << "No 'objects' section found in the server configuration." << std::endl;
    }
}


int main()
{
    Logger::Init();  // Initialize the logger

    printAvailableObjects(serverConfig);  // Print available objects

    int idToDetect = -1;
    std::cout << "Enter the ID of the object you want to detect: ";
    std::cin >> idToDetect;
    std::cin.get();  // Clear the input buffer

    // Check if the entered number is within the allowed range
    if (serverConfig["objects"].contains(std::to_string(idToDetect))){     // The number is allowed
        // Create backend, reader, and database objects
        IBackend* backend = new YoloBackend(idToDetect);
        ImageReader* reader = new GRPCImageReader();
        IDB* dataBase = new SqlDB();
     
        // Launch threads for gRPC server, backend processing, and database writing
        std::thread grpcServerThread([&]() {reader->read(&isRunning); });
        std::thread backendThread([&]() {backend->RunDetection(reader->GetFramesQueue(), &isRunning); });
        std::thread saveToSqliteThread([&]() {dataBase->writeDetectionQueue(backend->GetResults(), &isRunning); });

        // Wait for user input to terminate the program
        cin.get();
        isRunning = false;  // Set the running state to false to terminate threads

        // Log the user termination event
        Logger::getErrorInstance().logError("User has ended\n");

        // Wait for all threads to complete
        grpcServerThread.join();
        backendThread.join();
        saveToSqliteThread.join();

        // Clean up dynamically allocated objects
        delete backend;
        delete reader;
    }
    else {
     std::cerr << "Invalid ID. Please enter a valid object ID." << std::endl;
    }

    return 0; 
}
