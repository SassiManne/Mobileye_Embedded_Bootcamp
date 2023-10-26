#include "SafeQueue.h"
#include "structs.h"
#include "configJson.h"


// Default constructor
template <typename T>
SafeQueue<T>::SafeQueue() {}

// Destructor
template <typename T>
SafeQueue<T>::~SafeQueue() {}

// Check if the queue is empty in a thread-safe manner
template <typename T>
bool SafeQueue<T>::empty() const {
    this->mtx.lock();
    bool result = this->queue.empty();
    this->mtx.unlock();
    return result;
}

// Get the size of the queue in a thread-safe manner
template <typename T>
size_t SafeQueue<T>::size() const {
    this->mtx.lock();
    size_t result = this->queue.size();
    this->mtx.unlock();
    return result;
}

// Push an item to the queue
template <typename T>
void SafeQueue<T>::push(const T& frame) {

    ConfigFactory* configFactory = ConfigFactory::getInstance();
    json config = configFactory->getConfig("");

    this->mtx.lock();

    // if the type is cv::frame and the queue size is max 5 frames only 
    // so if the queue is full (5) then pop the first frame (the older) and push the new one
	if (typeid(T) == typeid(Frame) && queue.size() >= config["queueSize"])
		queue.pop();	
  
    queue.push(frame);
    this->mtx.unlock();
}

// Pop an item from the queue in a thread-safe manner
// Returns true if the pop was successful, false otherwise
template <typename T>
bool SafeQueue<T>::pop() {
    this->mtx.lock();
    if (this->queue.empty()) {  // Checking if the queue is empty
        this->mtx.unlock();
        return false;
    }
    this->queue.pop();
    this->mtx.unlock();
    return true;
}

// Get the front item of the queue in a thread-safe manner
template <typename T>
T SafeQueue<T>::front() {
    this->mtx.lock();
    T result = this->queue.front();
    this->mtx.unlock();
    return result;
}

