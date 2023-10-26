#pragma once
#include"IBackend.hpp"

extern std::string modelPath;

class YoloBackend : public IBackend
{
public:
    YoloBackend(int idToDetect);

    void RunDetection(SafeQueue<Frame>& frames, std::atomic<bool>* isRunning) override;
    cv::Scalar CalculateAverageColor(const cv::Mat& image, cv::Rect& rectangle) override; 

private:
    // Preprocess image and get outputs from the network
    vector<Mat> pre_process(Mat& input_image);
    // Postprocess outputs and get bounding boxes, detect Cars in default
    vector<DetectionResult> post_process(const Frame & input_frame, const vector<Mat>& outputs);
    void draw_boxes(Mat& sourceImg, const vector<DetectionResult>& boxes);
    void draw_label(Mat& input_image, string label, int left, int top);

};
