#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include"../SharedComponents/SafeQueue.h"
#include"../SharedComponents/structs.h"


class ImageReader {
public:
    virtual void read(std::atomic<bool>* isRunning) = 0;

    //
    SafeQueue<Frame> frames_queue;

    // get the frames queue
    SafeQueue<Frame>& GetFramesQueue() { return this->frames_queue; };
    
};


#endif