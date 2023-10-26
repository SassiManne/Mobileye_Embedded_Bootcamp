#pragma once
// Include guards to prevent double inclusion
#ifndef GRPCIMAGEREADER_H
#define GRPCIMAGEREADER_H


#include"../SharedComponents/grpc/CVService.grpc.pb.h"
#include"../SharedComponents/grpc/CVService.pb.h"
#include <opencv2/opencv.hpp>
#include <grpcpp/grpcpp.h>
#include "../SharedComponents/structs.h"
#include "../SharedComponents/SafeQueue.h"
#include "ImageReader.hpp"


class GRPCImageReader final : public cvservice::CVService::Service, public ImageReader {
private:

    grpc::Server* server_;
    int frame_count_;

    cv::Mat ConvertBytesToMat(const std::vector<unsigned char>& image_data);

public:

    GRPCImageReader();

    grpc::Status ProcessFrame(grpc::ServerContext* context, grpc::ServerReader<cvservice::FrameRequest>* reader, cvservice::FrameResponse* response) override;

    void read(std::atomic<bool>* isRunning) override;
};


#endif // CV_SERVICE_IMPL_H