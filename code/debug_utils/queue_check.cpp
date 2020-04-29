//
// Created by ilya on 29.04.2020.
//

#include "queue_check.h"
#include <thread>
#include "../blocking_queue/blocking_queue.h"
#include <unistd.h>
#include <iostream>
#include <functional>

std::function<void()> get_thread_function(int number, blocking_queue<int>& queue)
{
    return [number, &queue]()
    {
        std::cout << "Thread " << number << " is going to take element" << std::endl;
        int elem = queue.take();
        std::cout << "Thread " << number << " has taken elem " << elem << ", exiting" << std::endl;
    };
}

void check_queue()
{
    blocking_queue<int> queue;
    std::thread t1(get_thread_function(1, queue));
    std::thread t2(get_thread_function(2, queue));
    sleep(2);
    queue.push(1);
    sleep(2);
    queue.push(2);
    t1.join();
    t2.join();
}
