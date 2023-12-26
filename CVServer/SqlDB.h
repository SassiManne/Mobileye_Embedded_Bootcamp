#pragma once
#include "IDB.hpp"
#include "../SharedComponents/SafeQueue.h"
#include"../SharedComponents/structs.h"
#include<vector>

using namespace std;

class SqlDB : public IDB{

public:

	SqlDB();
	~SqlDB();

	void writeDetectionQueue(SafeQueue<vector<DetectionResult>> &results ,std::atomic<bool>* isRunning) override;
};