
#pragma once
#include"../SharedComponents/SafeQueue.h"
#include"../SharedComponents/structs.h"

using namespace std;
using namespace cv;

class ICamera
{
protected:
    SafeQueue<Frame>frames;


public:

    // Read image from video or webcam
    virtual void ReadImages(std::atomic<bool>* isRunning) = 0;

    // Push image to a queue
    virtual void PushImage(cv::Mat& image,int frameNumber, double currentTimestamp) = 0;

    // Get the queue of frames
    SafeQueue<Frame>& GetFrames() { return this->frames; }

};


