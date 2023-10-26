#pragma once
#include"SafeQueue.cpp"
#include "GRPCImageWriter.h"
#include "CVService.grpc.pb.cc"
#include "CVService.pb.cc"
#include "configJson.h"

GRPCImageWriter::GRPCImageWriter(int camera_id, std::string serverIpAddress)
{
	this->camera_id = camera_id; // Set the camera id
	std::string server_address;  // Server address to be used for creating the channel

	// Check if the serverIpAddress argument is empty
	if (serverIpAddress.empty())
	{
		// If serverIpAddress is empty, obtain the server address from the JSON configuration
		ConfigFactory* configFactory = ConfigFactory::getInstance();
		json clientConfig = configFactory->getConfig("client");
		server_address = clientConfig["server_address"];
	}
	else
	{
		// If serverIpAddress is not empty, use it as the server address
		server_address = serverIpAddress;
	}
	
	// Create channel to server
	std::shared_ptr<grpc::Channel> channel;

	try {
		channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
	}
	catch (const std::exception& e) {
		std::cout << "Error connecting to the server: " << e.what() << std::endl;
	}

	// Initialize the stub with the channel
	this->stub_ = cvservice::CVService::NewStub(channel);
}




bool GRPCImageWriter::write(SafeQueue<Frame>& queue, std::atomic<bool>* isRunning) {

	grpc::ClientContext context;
	cvservice::FrameResponse response;
	this->writer = stub_->ProcessFrame(&context, &response);

	while (true)
	{
		while (queue.empty())
		{
			if (!(*isRunning))
			{
				// Finish the call
				writer->WritesDone();
				grpc::Status status = writer->Finish();
				if (!status.ok()) {
					std::cerr << "RPC failed" << std::endl;
					return false;
				}
				return true;
			}
		}

		cvservice::FrameRequest request;

		Frame structFrame = queue.front();
		queue.pop();

		std::vector<unsigned char> image_data = ConvertMatToJpegBytes(structFrame.frame);
		request.set_image_data(image_data.data(), image_data.size());
		request.set_frame_id(this->frame_id++);
		request.set_camera_id(this->camera_id);
		request.set_timestamp(structFrame.timestamp);
		// Set other fields as needed ...

		if (!writer->Write(request)) {
			// Broken stream, finish the call
			std::cout << "Broken stream, frame " << request.frame_id() << " failed." << std::endl;
		}
		else
		{
			std::cout << "Sent frame " << request.frame_id() << std::endl;
		}


	}

}

std::vector<unsigned char> GRPCImageWriter::ConvertMatToJpegBytes(cv::Mat image)
{
	std::vector<unsigned char> array;
	if (!image.empty()) {
		cv::imencode(".jpg", image, array);
	}
	return array;
}