#include "../SharedComponents/Logger.h"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "yoloBackend.h"
#include <chrono>
#include"../SharedComponents/structs.h"
#include "../SharedComponents/SafeQueue.cpp"
#include <thread>

using namespace std;
using namespace cv;

#ifdef _DEBUG
// the debug version that corresponds to the opencv_world420d.dll:
    #pragma comment(lib, "opencv_world480d.lib")
#else
// the release version that corresponds to the opencv_world420.dll:
    #pragma comment(lib, "opencv_world480.lib")
#endif

#ifdef _WIN32
    std::string modelPath = "..\\..\\yolo_models\\yolov5s.onnx";
#elif __linux__ 
    std::string modelPath = "../../yolo_models/yolov5s.onnx";
    //std::string modelPath = "/app/yolo_models/yolov5s.onnx"; for aws
#endif


// Constants.
const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.5;
const float NMS_THRESHOLD = 0.45;
const float CONFIDENCE_THRESHOLD = 0.45;

// Text parameters.
const float FONT_SCALE = 0.7;
const int FONT_FACE = FONT_HERSHEY_SIMPLEX;
const int THICKNESS = 1;
const int NUMBER_OF_CLASSES = 80;

// Colors.
Scalar BLACK = Scalar(0, 0, 0);
Scalar BLUE = Scalar(255, 178, 50);
Scalar YELLOW = Scalar(0, 255, 255);
Scalar RED = Scalar(0, 0, 255);

//void sendImageToChannel(const cv::Mat& image, string frameNumber); //for aws

// initialize the parameters for the pre-processing of the image
YoloBackend::YoloBackend(int idToDetect)
{
    this->idToDetect = idToDetect;
    Logger::getInfoInstance().logInfo("Detecting id: " + std::to_string(this->idToDetect));

    // load the network
    this->net = cv::dnn::readNet(modelPath);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
}

// Preprocess image and get outputs from the network
void YoloBackend::RunDetection(SafeQueue<Frame>& frames, std::atomic<bool>* isRunning)
{
    Logger::getInfoInstance().logInfo("Running Yolo Detection\n");

    while (true){
    
        while (frames.empty() && *isRunning )
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

       if (!*isRunning)
           break;

        Frame currentFrame = frames.front();
        frames.pop();
        

        vector<Mat> preprocessed_image = pre_process(currentFrame.frame);
        //auto start = std::chrono::high_resolution_clock::now();
        vector<DetectionResult> boxes = post_process(currentFrame, preprocessed_image);
        //auto stop = std::chrono::high_resolution_clock::now();
        //auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        //std::cout << "Time taken by function: " << duration.count() << " microseconds" << std::endl;
        Logger::getInfoInstance().logInfo("Pre-Processing and Post-Processing Completed Sucessfully");

        if (boxes.empty()) continue;
        this->results.push(boxes);

        this->draw_boxes(currentFrame.frame, boxes);

        /*
        // for aws send photo evry 10 times
       if(currentFrame.frameNumber % 10 == 0)
       {
            //Sending the picture with the rainbows on the objects to the Telegram channel
            sendImageToChannel(currentFrame.frame, std::to_string(currentFrame.frameNumber));
       }
       */

        imshow("Output", currentFrame.frame);
        waitKey(1);
       
    }
    Logger::getInfoInstance().logInfo("Detection Complete");

}

// returns vector of detections
vector<Mat> YoloBackend::pre_process(Mat& input_image)
{
    // Convert to blob.
    Mat blob;
    cv::dnn::blobFromImage(input_image, blob, 1. / 255., Size(INPUT_WIDTH, INPUT_HEIGHT), Scalar(), true, false);

    net.setInput(blob);

    // Forward propagate.
    vector<Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    return outputs;
}

vector<DetectionResult>YoloBackend::post_process(const Frame& input_frame, const vector<Mat>& outputs)
{
    // Initialize vectors to hold respective outputs while unwrapping detections.
    vector<float> confidences; //Levels of certainty that this is an object
    vector<float> classScores; //Levels of certainty that this is the desired object
    //vector<float> class_ids; // for the future if we will want to detect more than one class !!
    vector<Rect> boxes;
    cv::Mat input_image = input_frame.frame;

    // Resizing factor.
    float x_factor = input_image.cols / INPUT_WIDTH;
    float y_factor = input_image.rows / INPUT_HEIGHT;

    float* data = (float*)outputs[0].data;

    const int dimensions = 85;
    const int rows = 25200;

    // Iterate through 25200 detections in the first output(the last network).
    for (int i = 0; i < rows; ++i)
    {
        float confidence = data[4];
        // Discard bad detections and continue.
        if (confidence >= CONFIDENCE_THRESHOLD) // if it is object
        {
            float* classes_scores = data + 5; // the beginning of the scores of the classes
   
            // Perform minMaxLoc and acquire index of best class score.
            Point class_id;
            double max_class_score = 0;
            // find the max score and its index
            for (int i = 0; i < NUMBER_OF_CLASSES; i++)
            {
                if (classes_scores[i] > max_class_score)
                {
					max_class_score = classes_scores[i];
					class_id.x = i;
				}
              
			}

            // Continue if the class score is above the threshold.
            if (max_class_score > SCORE_THRESHOLD && class_id.x == this->idToDetect) // if the max score is a car 
            {
                confidences.push_back(confidence); // Store confidence in the pre - defined respective vectors 
                classScores.push_back(max_class_score); // the confidence of the class (The grade given to the requested object)
                //class_ids.push_back(class_id.x); for the future if we will want to detect more than one class !!

                // Center.
                float cx = data[0];
                float cy = data[1];
                // Box dimension.
                float w = data[2];
                float h = data[3];
                // Bounding box coordinates.
                int left = int((cx - 0.5 * w) * x_factor);
                int top = int((cy - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);

                // Ensure the bounding box does not exceed the image boundaries.
                left = std::max(0, left);
                top = std::max(0, top);
                width = std::min(input_image.cols - left, width);
                height = std::min(input_image.rows - top, height);

                // Store good detections in the boxes vector.
                boxes.push_back(Rect(left, top, width, height));
                // now we have the bounding box of the object and in confidences vector we have the confidence of the detection
            }

        }
        // Jump to the next column.
        data += 85;
    }

    // The indices vector will be filled with the indices of bounding boxes that are kept after the NMS.
    vector<int> indices;

    // NMSBoxes function performs the non-maxima suppression (NMS) given the boxes and their associated confidences.
    // It keeps only the most significant bounding boxes that are strong enough (greater than SCORE_THRESHOLD)
    // and sufficiently different from the others (using the NMS_THRESHOLD).
    // The result, i.e., the indices of the kept bounding boxes, is stored in the 'indices' vector.
    dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, indices);

    // A vector to store the bounding boxes after the NMS.
    vector<DetectionResult> final_boxes;

    // Loop over the indices vector.
    for (int idx : indices)
    {
        // For each index, add the corresponding bounding box from the 'boxes' vector
        // to the 'final_boxes' vector.
        final_boxes.push_back(DetectionResult{ this->idToDetect, classScores[idx] ,input_frame.frameNumber, boxes[idx],CalculateAverageColor(input_image, boxes[idx]) });
    }

    return final_boxes;
}

void YoloBackend::draw_boxes(Mat& input_image, const vector<DetectionResult>& boxes)
{
    ConfigFactory* configFactory = ConfigFactory::getInstance();
    json serverConfig = configFactory->getConfig("server");

    // Loop over all the bounding boxes.
    for (const auto& box : boxes)
    {
        // Get the bounding box coordinates.
        int left = box.boundingBox.x;
        int top = box.boundingBox.y;
        int width = box.boundingBox.width;
        int height = box.boundingBox.height;
        // Draw the bounding box.
        rectangle(input_image, Point(left, top), Point(left + width, top + height), YELLOW, THICKNESS);
        // Get the label for the class name and its confidence.
        string label = format("%.2f", box.confidence);
        // label = class_name[class_ids[idx]] + ":" + label; - This is to add a class with all the names in the future
        label = serverConfig["objects"][std::to_string(this->idToDetect)].dump() + label;
        // Display the label at the top of the bounding box.
        draw_label(input_image, label, left, top);

    }
}

void YoloBackend::draw_label(Mat& input_image, string label, int left, int top)
{
    // Display the label at the top of the bounding box.
    int baseLine;
    Size label_size = getTextSize(label, FONT_FACE, FONT_SCALE, THICKNESS, &baseLine);
    top = max(top, label_size.height);
    // Top left corner.
    Point tlc = Point(left, top);
    // Bottom right corner.
    Point brc = Point(left + label_size.width, top + label_size.height + baseLine);
    // Draw black rectangle.
    rectangle(input_image, tlc, brc, BLACK, FILLED);
    // Put the label on the black rectangle.
    putText(input_image, label, Point(left, top + label_size.height), FONT_FACE, FONT_SCALE, YELLOW, THICKNESS);
}

cv::Scalar YoloBackend::CalculateAverageColor(const cv::Mat& image, cv::Rect& rectangle)
{
    Logger::getInfoInstance().logInfo("Calculating Average Color");

    cv::Mat ROI = image(rectangle);
    cv::Scalar averageBGR = cv::mean(ROI);
    return averageBGR;
}

/*
// for aws
// A function that sends a frame to a Telegram channel
void sendImageToChannel(const cv::Mat& image, string frameNumber) {
    // Save the image to a file
    cv::imwrite("image.png", image);

    // Construct the command string with the frameNumber argument
    std::string command = "python3 send_image_to_channel.py image.png " + frameNumber;

    // Execute the Python script with the image path and frameNumber as arguments
    system(command.c_str());

}
*/
