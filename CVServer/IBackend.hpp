
#pragma once
#include<vector> 
#include"../SharedComponents/structs.h"
#include"../SharedComponents/SafeQueue.h"

using namespace std;
using namespace cv;


class IBackend
{
protected:
    cv::dnn::Net net;
    SafeQueue<vector<DetectionResult>> results;
    int idToDetect;


public:


    //Run object detection on image
    virtual void RunDetection(SafeQueue<Frame>& frames, std::atomic<bool>* isRunning) = 0;

    // Calculate average color per channel in the detected rectangle
    virtual cv::Scalar CalculateAverageColor(const cv::Mat& image, cv::Rect& rectangle) = 0;

    // Save results to DB file, get results for each frame
    SafeQueue<vector<DetectionResult>>& GetResults() { return this->results; }

};



