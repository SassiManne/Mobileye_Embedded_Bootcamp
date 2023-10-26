#include "Logger.h"
#include <fstream>
#include "VideoSource.h"
#include <chrono>
#include <thread>
#include "SafeQueue.cpp"
#include "configJson.h"

#define ESC 27


using namespace std;


#ifdef _DEBUG
// the debug version that corresponds to the opencv_world420d.dll:
    #pragma comment(lib, "opencv_world480d.lib")
#else
// the release version that corresponds to the opencv_world420.dll:
    #pragma comment(lib, "opencv_world480.lib")
#endif



void VideoSource::ReadImages(std::atomic<bool>* isRunning) {

    Logger::getInfoInstance().logInfo("Reading images from mp4 video\n");

    ConfigFactory* configFactory = ConfigFactory::getInstance();
    json clientConfig = configFactory->getConfig("client");

    cv::Mat currentFrame , previousFrame;
    cv::VideoCapture capture;

    if (this->path.empty()) {
        capture.open(0);  // Open default camera if path is empty
    }
    else {
        capture.open(path);  // Otherwise, open the camera (or video) specified by the path
    }

    capture.read(currentFrame); // read the first frame
    previousFrame = currentFrame.clone(); // clone the first frame
    this->PushImage(currentFrame, 1 , 0); // push the first frame to the queue

    if (!capture.isOpened())
    {
        //std::cerr << "Error opening video file\n";
        Logger::getErrorInstance().logError("Error opening video file\n");
        return;
    }

    Logger::getInfoInstance().logInfo("Pushing frames to queue");

    while (*isRunning)
    {
        capture.read(currentFrame);
        Logger::getInfoInstance().logInfo("Reading image");
        int currentFrameNumber = static_cast<int>(capture.get(cv::CAP_PROP_POS_FRAMES));
        double currentTimestamp = capture.get(cv::CAP_PROP_POS_MSEC);


        if (currentFrame.empty())
        {
            std::cout << "End of stream\n";
            break;
        }

        if (!this->IsSimilarFrame(currentFrame, previousFrame, clientConfig["threshold"]))
        {
            this->PushImage(currentFrame, currentFrameNumber , currentTimestamp);
            previousFrame = currentFrame.clone();

            //int windowWidth = 1200;
            //int windowHeight = 620;
            //cv::resize(currentFrame, currentFrame, cv::Size(windowWidth, windowHeight));
            //cv::imshow("output", currentFrame);
        }

        //int key = cv::waitKey(1);

        // Sleep for 1ms
        std::this_thread::sleep_for(chrono::milliseconds(250));
       

        
    }
    capture.release();
    Logger::getInfoInstance().logInfo("Frames pushed to queue successfully\n");

}


void VideoSource::PushImage(cv::Mat& image, int frameNumber, double currentTimestamp)
{
    this->frames.push(Frame{ frameNumber,image.clone(),currentTimestamp });
}


// private function
float VideoSource::getPercentageOfSimilarity(const cv::Mat& image1, const cv::Mat& image2)
{
    cv::Mat diff;
    cv::absdiff(image1, image2, diff);

    // Convert difference image to grayscale
    cv::cvtColor(diff, diff, cv::COLOR_BGR2GRAY);

    // Compute percentage of non-zero pixels (percentage of difference)
    float totalPixels = diff.rows * diff.cols;
    float nonZeroPixels = cv::countNonZero(diff);
    float percentageDifference = (nonZeroPixels / totalPixels);

    return 1.0f - percentageDifference;
}


/*
* public function
* return true if currentFrame and previousFrame are similar by percentage parameter
* otherwise return false
* example: percentage = 90% if frame1 and frame2 are 85% the same,
* IsSimilarFrame will return false
*/
bool VideoSource::IsSimilarFrame(cv::Mat& currentFrame, cv::Mat& previousFrame, float percentage)
{
    return this->getPercentageOfSimilarity(currentFrame, previousFrame) >= percentage;
}

