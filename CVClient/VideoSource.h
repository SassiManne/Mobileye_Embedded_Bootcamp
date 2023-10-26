#pragma once
#include"ICamera.hpp"


class VideoSource : public ICamera
{
    string path;


public:
    VideoSource(string path)
    {
        this->path = path;
    }

    void ReadImages(std::atomic<bool>* isRunning) override;
    void PushImage(cv::Mat& image, int frameNumber,double currentTimestamp) override;
    bool IsSimilarFrame(cv::Mat& currentFrame, cv::Mat& previousFrame, float percentage) ;

    ~VideoSource()
    {
        //delete this->frames;
    }
private:

 
    float getPercentageOfSimilarity(const cv::Mat& image1, const cv::Mat& image2);
};
