#pragma once
#include"SafeQueue.h"
#include"structs.h"

class ImageWriter
{
	public:
	// Write frames to the destination
	virtual bool write(SafeQueue<Frame>& queue, std::atomic<bool>* isRunning) = 0;
};