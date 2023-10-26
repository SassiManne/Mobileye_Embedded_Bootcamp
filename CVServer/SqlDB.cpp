#include"SqlDB.h"
#include "../SharedComponents/Logger.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include "../SharedComponents/SafeQueue.cpp"
#include"../SharedComponents/structs.h"
#include <thread>


SqlDB::SqlDB() {};
SqlDB::~SqlDB() {};


void SqlDB::writeDetectionQueue(SafeQueue<vector<DetectionResult>>& results, std::atomic<bool>* isRunning)
{
    Logger::getInfoInstance().logInfo("Saving ROI to SQLite");

    sqlite3* db;
    int rc = sqlite3_open("DETECTIONS_DB.db", &db);
    if (rc) {
        std::cout << "error";
    }
    else {
        std::cout << "The DB File Is Opened \n";
    }


    const char* recreateTableSql = "DROP TABLE IF EXISTS DETECTION_INFO;"
        "CREATE TABLE DETECTION_INFO("  \
        "FRAME_NUMBER   INT     NOT NULL," \
        "X_AXIS         INT     NOT NULL," \
        "Y_AXIS         INT     NOT NULL," \
        "HEIGHT         INT," \
        "WIDTH          INT," \
        "BLUE_AVRG      REAL," \
        "GREEN_AVRG     REAL," \
        "RED_AVRG       REAL );";

    rc = sqlite3_exec(db, recreateTableSql, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        std::cout << "Error creating table: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return;
    }


    while (true) {

        while (results.empty() && *isRunning)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (!*isRunning)
            break;

        vector<DetectionResult> resultsForFrame = results.front();
        results.pop();

        for (auto& detection : resultsForFrame) {
            char insertSql[256];
            snprintf(insertSql, sizeof(insertSql),
                "INSERT INTO DETECTION_INFO (FRAME_NUMBER, X_AXIS, Y_AXIS, HEIGHT, WIDTH, BLUE_AVRG, GREEN_AVRG, RED_AVRG) "
                "VALUES (%d, %d, %d, %d, %d, %f, %f, %f);",
                detection.frameNumber, detection.boundingBox.x, detection.boundingBox.y, detection.boundingBox.width, detection.boundingBox.height,
                detection.avgColor[0], detection.avgColor[1], detection.avgColor[2]);

            rc = sqlite3_exec(db, insertSql, nullptr, nullptr, nullptr);

            if (rc != SQLITE_OK) {
                std::cout << "Error inserting to table: " << sqlite3_errmsg(db) << "\n";
                sqlite3_close(db);
                return;
            }
        }
    }
    Logger::getInfoInstance().logInfo("Closing SQLite");
    sqlite3_close(db);
}