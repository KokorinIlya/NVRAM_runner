#ifndef DIPLOM_BLOCKING_QUEUE_H
#define DIPLOM_BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

/**
 * Queue, that stores element of some type.
 * Can be used for solving producer-consumer task in a multi-threading
 * environment. Stores all data in RAM. Synchronization is done
 * using single coarse-grained mutex.
 * @tparam T - type of elements, that will be stored in queue.
 */
template <typename T>
struct blocking_queue
{
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cv;
public:
    /**
     * Adds single element to the back of the queue.
     * @param elem - elem to add to queue.
     */
    void push(const T& elem);

    /**
     * Returns single element from the top of the queue and removes
     * element, that was returned. If there are no elements in the queue,
     * thread is blocked until at least one element is pushed in the queue.
     * @return - element from the top of the queue.
     */
    T take();

    /**
     * Returns size of the queue.
     * @return size of the queue.
     */
    uint32_t size();
};

template <typename T>
void blocking_queue<T>::push(const T& elem)
{
    std::unique_lock<std::mutex> lock(mutex);
    queue.push(elem);
    cv.notify_one();
}

template <typename T>
T blocking_queue<T>::take()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (queue.empty())
    {
        cv.wait(lock);
    }
    T result = queue.front();
    queue.pop();
    return result;
}

template <typename T>
uint32_t blocking_queue<T>::size()
{
    std::unique_lock<std::mutex> lock(mutex);
    return queue.size();
}

#endif //DIPLOM_BLOCKING_QUEUE_H
