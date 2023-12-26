#pragma once

#include "../SharedComponents/SafeQueue.h"
#include"../SharedComponents/structs.h"
#include<vector>

using namespace std;

class IDB {
public:
	virtual void writeDetectionQueue(SafeQueue<vector<DetectionResult>>& results, std::atomic<bool>* isRunning) = 0;
};
