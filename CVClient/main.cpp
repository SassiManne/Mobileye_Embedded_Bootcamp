
#include "../SharedComponents/Logger.h"
#include <opencv2/opencv.hpp>
#include"VideoSource.h"
#include"ImageWriter.hpp"
#include "GRPCImageWriter.h"
#include "SafeQueue.cpp" 
#include <thread>





#ifdef _DEBUG
// the debug version that corresponds to the opencv_world420d.dll:
#pragma comment(lib, "opencv_world480d.lib")
#else
// the release version that corresponds to the opencv_world420.dll:
#pragma comment(lib, "opencv_world480.lib")
#endif

std::atomic<bool> isRunning(true);  // Atomic boolean to manage the running state of threads

int main()
{
    Logger::Init();  // Initialize the logger

    std::string videoPath, serverIpAddress;  // Variables to hold user input for video path and server IP address

    // Prompt user for video path, with default to computer camera if Enter is pressed
    std::cout << "Enter video path, press Enter for the computer camera: ";
    std::getline(std::cin, videoPath);

    // Prompt user for server IP address, with default to configuration file value if Enter is pressed
    std::cout << "Enter server ip address, (press Enter for the default address (from the config file): ";
    std::getline(std::cin, serverIpAddress);

    // Create camera and writer objects based on user input or default values
    ICamera* cam = new VideoSource(videoPath);
    ImageWriter* writer = new GRPCImageWriter(1, serverIpAddress);

    // Launch camera and server threads
    std::thread cameraThread([&]() {cam->ReadImages(&isRunning); });
    std::thread serverThread([&]() {writer->write(cam->GetFrames(), &isRunning); });

    cin.get();  // Wait for user input to end program

    isRunning = false;  // Update the running state to false, signaling threads to terminate

    // Log user termination event
    Logger::getErrorInstance().logInfo("User has ended\n");

    // Wait for camera and server threads to complete
    cameraThread.join();
    serverThread.join();

    // Clean up dynamically allocated
    delete cam;
    delete writer;

    return 0;  
}
