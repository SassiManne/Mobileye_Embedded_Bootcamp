#pragma once

#include <queue>
#include <mutex>

template <typename T>
class SafeQueue {
public:
    SafeQueue();
    ~SafeQueue();

    bool empty() const;
    size_t size() const;
    void push(const T& frame);
    bool pop();
    T front();

private:
    mutable std::mutex mtx;
    std::queue<T> queue;
};

