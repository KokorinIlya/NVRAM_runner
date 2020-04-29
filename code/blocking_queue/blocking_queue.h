//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_BLOCKING_QUEUE_H
#define DIPLOM_BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
struct blocking_queue
{
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cv;
public:
    void push(const T& elem);

    T take();
};

template<typename T>
void blocking_queue<T>::push(const T& elem)
{
    std::unique_lock<std::mutex> lock(mutex);
    queue.push(elem);
    cv.notify_one();
}

template<typename T>
T blocking_queue<T>::take()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (queue.empty())
    {
        cv.wait(lock);
    }
    const T& result = queue.back();
    queue.pop();
    return result;
}

#endif //DIPLOM_BLOCKING_QUEUE_H
