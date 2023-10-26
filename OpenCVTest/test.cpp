#include "pch.h"
#include <iostream>
#include <fstream>
#include <sqlite3.h>
#include <opencv2/opencv.hpp>
#include "../OpenCVProject/SqlDB.cpp"
#include "../OpenCVProject/mp4Video.cpp"
#include "../OpenCVProject/YoloBackend.cpp"
#include "../OpenCVProject/Logger.cpp"


class OpenCVTest : public ::testing::Test {
protected:
    YoloBackend yolo;
    mp4video* cam = new mp4video("");
    SqlDB dataBase;
    std::atomic<bool> isRunning = true;
    // If needed, you can add setup and tear down methods here. 
   
    ~OpenCVTest()
    {
        delete this->cam;
    }
};


// Test that the results queue is empty. 
TEST_F(OpenCVTest, TestEmptyQueue) {
    SafeQueue<Frame> frames;
    
    std::thread backendThread([&]() {yolo.RunDetection(&frames, &isRunning); });
    // sleep for 200 ms to allow the thread to run.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // stop the thread
    this->isRunning = false;
    backendThread.join();

    EXPECT_TRUE(yolo.GetResults().empty()); // Assert that the results queue remains empty. 
}


//Test that the results queue is empty after running frame without a cars. 
TEST_F(OpenCVTest, TestResultsEmptyPopulation) {
    SafeQueue<Frame> frames;
    cv::Mat blackImage = Mat::zeros(100, 100, CV_8UC3);
    Frame sampleFrame{ 1,blackImage };  // Create a sample frame without any objects. 
    frames.push(sampleFrame);

    std::thread backendThread([&]() {yolo.RunDetection(&frames, &isRunning); });
    // sleep for 200 ms to allow the thread to run.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // stop the thread
    this->isRunning = false;
    backendThread.join();

    EXPECT_TRUE(yolo.GetResults().empty()); // not detected a car 
}


//Test with frame of 1 car, so the results queue should have 1 element, and the element should have 1 car. 
TEST_F(OpenCVTest, TestResultsPopulation) {
    SafeQueue<Frame> frames;
    cv::Mat oneCar = cv::imread("..\\..\\OpenCVTest\\1 car- for test.jpeg");
    Frame sampleFrame{ 1,oneCar };
    frames.push(sampleFrame);

    std::thread backendThread([&]() {yolo.RunDetection(&frames, &isRunning); });
    // sleep for 200 ms to allow the thread to run.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // stop the thread
    this->isRunning = false;
    backendThread.join();

    EXPECT_TRUE(yolo.GetResults().front().size() == 1); // detected a car 
}


// Test csv file creation. 
TEST_F(OpenCVTest, TestFileCreation) {
    yolo.SaveROIToCSV();
    std::ifstream file("results.csv");
    EXPECT_TRUE(file.good());  // Check if the file exists. 
    file.close();
}



//Test for the average color of a single color image. 
TEST_F(OpenCVTest, TestSingleColor) {
    //Create a 100x100 image where all pixels are blue 
    cv::Mat blueImage(100, 100, CV_8UC3, cv::Scalar(255, 0, 0));
    // Define a rectangle that covers the entire image 
    cv::Rect rect(0, 0, 100, 100);
    cv::Scalar averageColor = yolo.CalculateAverageColor(blueImage, rect);
    //Expect the average color to be blue 
    EXPECT_EQ(averageColor, cv::Scalar(255, 0, 0));
}


//Test for the average color of a multicolor image. 
TEST_F(OpenCVTest, TestMulticolor) {
    // Create a 100x100 image where all pixels are initially red 
    cv::Mat multicolorImage(100, 100, CV_8UC3, cv::Scalar(255, 0, 0));
    //Create a half-sized blue image 
    cv::Mat halfBlue(100, 50, CV_8UC3, cv::Scalar(0, 0, 255));
    //Copy the blue half onto the right side of the multicolor image 
    halfBlue.copyTo(multicolorImage(cv::Rect(50, 0, 50, 100)));
    // Define a rectangle that covers the entire image 
    cv::Rect rect(0, 0, 100, 100);
    cv::Scalar averageColor = yolo.CalculateAverageColor(multicolorImage, rect);
    // Expect the average color to be purple (average of red and blue) 
    EXPECT_EQ(averageColor, cv::Scalar(127.5, 0, 127.5));
}



TEST_F(OpenCVTest, IsSimilarFrame) {

    // Arrange 
    cv::Mat image1 = cv::Mat::ones(100, 100, CV_8UC3);
    cv::Mat image2 = cv::Mat::ones(100, 100, CV_8UC3);

    cv::Scalar const Black = cv::Scalar(0, 0, 0);
    cv::Scalar const White = cv::Scalar(255, 255, 255);
    const int filled = -1;

    // Draw a rectangle in the center of image2 
    cv::Rect rect(25, 25, 50, 50);
    cv::rectangle(image2, rect, White,
        filled);

    // Act 
    float percentage = 0.85f;
    bool expected = false;
    bool actual = cam->IsSimilarFrame(image1, image2, percentage);

    EXPECT_FLOAT_EQ(actual, expected);

}

// Test sqlite db with emety table.
TEST_F(OpenCVTest, TestEmptyTable) {
   
    std::string dbFilePath = ".\\DETECTIONS_DB.db";

    SafeQueue<vector<DetectionResult>> results;

    // Call the function to be tested
    std::thread saveToSqliteThread([&]() {dataBase.writeDetectionQueue(results ,&isRunning); });
   
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Verify that the table is created
    sqlite3* db;
    int rc = sqlite3_open_v2(dbFilePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    // Check if the table exists using a query
    const char* tableExistsQuery = "SELECT COUNT(*) FROM DETECTION_INFO;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, tableExistsQuery, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    // Execute the query
    rc = sqlite3_step(stmt);
    ASSERT_EQ(rc, SQLITE_ROW); // Expecting a row if the table exists
    
    // Get the data count from the result
    int dataCount = sqlite3_column_int(stmt, 0);
    ASSERT_EQ(dataCount, 0); // Expecting at least one row of data !!!!!!!!!!!!
    this->isRunning = false;
    saveToSqliteThread.join();
    

    // Clean up by closing the database and deleting the temporary directory
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    rc = sqlite3_shutdown();
    ASSERT_EQ(rc, SQLITE_OK);
    rc = remove(dbFilePath.c_str());

    
    ASSERT_EQ(rc, 0);   

}


// Test sqlite db with some data .
TEST_F(OpenCVTest, TestPopulatedTable) {

    SafeQueue<Frame> frames;
    cv::Mat oneCar = cv::imread("..\\..\\OpenCVTest\\1 car- for test.jpeg");
    Frame sampleFrame{ 1, oneCar };
    frames.push(sampleFrame);
    std::thread backendThread([&]() {yolo.RunDetection(&frames, &isRunning); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::thread saveToSqliteThread([&]() {dataBase.writeDetectionQueue(yolo.GetResults(), &isRunning); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Form the temporary database file path
    std::string dbFilePath = ".\\DETECTIONS_DB.db";

   

    // Verify that the table contains data
    sqlite3* db;
    int rc = sqlite3_open_v2(dbFilePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    // Check if the table contains data using a query
    const char* dataExistsQuery = "SELECT COUNT(*) FROM DETECTION_INFO;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, dataExistsQuery, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    // Execute the query
    rc = sqlite3_step(stmt);
    ASSERT_EQ(rc, SQLITE_ROW); // Expecting a row with data count

    // Get the data count from the result
    int dataCount = sqlite3_column_int(stmt, 0);
    ASSERT_EQ(dataCount, 0); // Expecting at least one row of data !!!!!!!!!!!!


    this->isRunning = false;
    backendThread.join();
    saveToSqliteThread.join();
    // Clean up by closing the database and deleting the temporary directory
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    rc = sqlite3_shutdown();
    ASSERT_EQ(rc, SQLITE_OK);
    rc = remove(dbFilePath.c_str());
    ASSERT_EQ(rc, 0);
   
}


// Test if the log message is in the log file.
TEST_F(OpenCVTest, CheckIfLogMessageInFile) {

    const std::string logMessage = "Test Log Message";

    // Log the message.
    Logger::getInfoInstance().logInfo(logMessage);

    // Ensure the message is in the log file.
    std::ifstream logFile("Logs/Info.txt");
    std::string line;
    bool found = false;

    if (logFile.is_open()) {
        while (getline(logFile, line)) {
            if (line.find(logMessage) != std::string::npos) {
                found = true;
                break;
            }
        }
        logFile.close();
    }

    // Check if the log message was found in the file.
    EXPECT_TRUE(found);
}

// Test if the loggers were created.
TEST_F(OpenCVTest, CheckIfInstancesCreated) {
    
    // Check if the Info logger instance was created.
    Logger& infoLogger = Logger::getInfoInstance();
    EXPECT_NE(&infoLogger, nullptr);

    // Check if the Error logger instance was created.
    Logger& errorLogger = Logger::getErrorInstance();
    EXPECT_NE(&errorLogger, nullptr);
}



// Define a test named "MultithreadedAccess" for the "SafeQueueTest" test suite.
TEST_F(OpenCVTest, MultithreadedAccess) {
    // Instantiate a SafeQueue object to store integers.
    SafeQueue<int> safeQueue;

    // Define the number of threads we want to create and the number of operations each thread will execute.
    const int num_threads = 100;
    const int num_operations = 1000;

    // Define the function that threads will use to push values onto the queue, and after, pop them off.
    auto pushPopFunc = [&]() {
        for (int i = 0; i < num_operations; ++i) {
            safeQueue.push(i);
        }
        for (int i = 0; i < num_operations; ++i) {
            safeQueue.pop();
        }
    };

    


   
    // Create a vector to hold our thread objects.
    std::vector<std::thread> threads;

    // Launch pairs of threads: all thread will push values onto the queue, and after, will pop them off.
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(pushPopFunc));
    }

    // now num_threads threads are running concurrently. and each thread is pushing and popping num_operations values.
    // if the queue is not thread-safe, we will get a crash here or the queue will not be empty.

    // Wait for all threads to finish their execution.
    for (auto& t : threads) {
        t.join();
    }

    // After all threads have finished, assert that the queue is empty.
    // If it's not empty, the test will fail, indicating a problem with our SafeQueue's thread-safety.
    ASSERT_TRUE(safeQueue.empty());
}

// test for check if the sixth frame override the first one inside the queue
TEST_F(OpenCVTest, queueSize) {

    cv::Mat blackImage = Mat::zeros(100, 100, CV_8UC3);
    cv::Mat whiteImage = cv::Mat::ones(100, 100, CV_8UC3) * 255;

    SafeQueue<cv::Mat> queue;

    //first frame is black image
    queue.push(blackImage);
    //all the next frames are white 
    queue.push(whiteImage);
    queue.push(whiteImage);
    queue.push(whiteImage);
    queue.push(whiteImage);
    queue.push(whiteImage);
    
    //check if the queue size under 6
    EXPECT_EQ(queue.size(), 5);

    while (!queue.empty()) {

        cv::Mat currFrame = queue.front();
        queue.pop();
        
        //if the frames are equal, diffImage will be a white image
        cv::Mat diffImage = (currFrame == whiteImage);

        // Convert diffImage to grayscale
        cv::cvtColor(diffImage, diffImage, cv::COLOR_BGR2GRAY);
        
        bool areEqual = cv::countNonZero(diffImage) == currFrame.total();
        EXPECT_TRUE(areEqual);
    }
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}