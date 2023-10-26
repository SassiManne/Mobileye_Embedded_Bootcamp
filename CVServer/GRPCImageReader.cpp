#include "GRPCImageReader.h"
#include "../SharedComponents/SafeQueue.cpp"
#include "configJson.h"

using namespace std;


// Constructor initializes frame counter to 0
GRPCImageReader::GRPCImageReader() : frame_count_(0) {}

// This function processes incoming frames from a gRPC client
grpc::Status GRPCImageReader::ProcessFrame(grpc::ServerContext* context, grpc::ServerReader<cvservice::FrameRequest>* reader, cvservice::FrameResponse* response) {
    cvservice::FrameRequest request;
    bool firstimage = true;
    const std::string client_ip = context->peer();

    // Continuously read frames from the client
    while (reader->Read(&request)) {

        // If client sends a terminate signal, shutdown the server and return cancelled status
        if (request.terminate_session()) {
            server_->Shutdown();
            return grpc::Status::CANCELLED;  // Indicate that the session was terminated by client's request
        }

        // Convert incoming data into a format suitable for OpenCV
        std::vector<unsigned char> image_data(request.image_data().begin(), request.image_data().end());
        cv::Mat frame = ConvertBytesToMat(image_data);

         //Display the received frame
        //cv::imshow("Received Frame", frame);
        //cv::waitKey(1);

        // Push the received frame onto a queue
        frames_queue.push(Frame{ frame_count_, frame });

        // Increment the frame counter
        this->frame_count_++;
    }

    // Set the response status to indicate successful processing
    response->set_status(1);
    response->set_frames_received(this->frame_count_);

    // Close any open OpenCV windows
    cv::destroyAllWindows();

    // Reset the frame counter
    this->frame_count_ = 0;

    return grpc::Status::OK;
}

// This function sets up and starts the gRPC server
void GRPCImageReader::read(std::atomic<bool>* isRunning) {

    ConfigFactory* configFactory = ConfigFactory::getInstance();
    json serverConfig = configFactory->getConfig("server");

    std::string server_address(serverConfig["IP_address"]);

    grpc::ServerBuilder builder;

    // Specify server address and credentials
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // Register the service with the server
    builder.RegisterService(this);

    // Build and start the server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

// Utility function to convert bytes into OpenCV Mat format
cv::Mat GRPCImageReader::ConvertBytesToMat(const std::vector<unsigned char>& image_data) {
    return cv::imdecode(image_data, cv::IMREAD_COLOR);
}
