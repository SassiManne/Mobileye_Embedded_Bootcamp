
#pragma once
#include <grpcpp/grpcpp.h>
#include <opencv2/opencv.hpp>
#include "CVService.pb.h"
#include "CVService.grpc.pb.h"
#include "SafeQueue.h"
#include "structs.h"
#include "ImageWriter.hpp"



class GRPCImageWriter : public ImageWriter {

public:

    GRPCImageWriter( int camera_id);

    // Write frames to the grpc server
    bool write(SafeQueue<Frame> &queue, std::atomic<bool>* isRunning) override;
    

private:
    std::unique_ptr<cvservice::CVService::Stub> stub_;
    std::unique_ptr<grpc::ClientWriter<cvservice::FrameRequest>> writer;
    int frame_id = 0;
    int camera_id;

    std::vector<unsigned char> ConvertMatToJpegBytes(cv::Mat image);

};

