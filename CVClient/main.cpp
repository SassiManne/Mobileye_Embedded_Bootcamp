
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

// first argument is the path to the video file, if 0 is passed as an argument, the default camera will be used
// second argument is the ip address of the server. if 0 is passed as an argument, the default ip address will be used
// if no arguments are passed, the default values will be used from the config file
int main(int argc, char** argv)
{
    Logger::Init();  // Initialize the logger

    ConfigFactory* configFactory = ConfigFactory::getInstance();
    json clientConfig = configFactory->getConfig("client");

    // Check if the user has passed arguments to the program
    // If so, update the config file with the new values, and from now, use the new values even if the user doesn't pass arguments, because the config file will be updated
    // it is usful for docker containers, where the user can pass arguments to the program when running the container, and when the rp is restarted(turn on), the new values will be used
    if (argc > 2)
    {
        if(argv[1] != "0") configFactory->updateConfigValue("video_path", argv[1], "client");
        if(argv[2] != "0") configFactory->updateConfigValue( "server_address", argv[2], "client");
    }


    // Create camera and writer objects based on user input or default values
    ICamera* cam = new VideoSource();
    ImageWriter* writer = new GRPCImageWriter(1);

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
