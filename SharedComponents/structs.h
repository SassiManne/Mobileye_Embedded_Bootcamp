#pragma once
#include <opencv2/opencv.hpp>

struct DetectionResult
{
    int objectType;
    float confidence;
    int frameNumber;
    cv::Rect boundingBox;
    cv::Scalar avgColor;
};

struct Frame
{
    int frameNumber;
    cv::Mat frame;
    double timestamp; // in milliseconds
};
