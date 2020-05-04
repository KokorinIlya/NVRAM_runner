#include "gtest/gtest.h"
#include <thread>
#include "../code/blocking_queue/blocking_queue.h"
#include <unistd.h>

TEST(queue, base_correctness)
{
    blocking_queue<int> queue;
    queue.push(1);
    queue.push(3);
    EXPECT_EQ(queue.take(), 1);
    queue.push(3);
    queue.push(7);
    EXPECT_EQ(queue.take(), 3);
    EXPECT_EQ(queue.take(), 3);
    EXPECT_EQ(queue.take(), 7);
    EXPECT_EQ(queue.size(), 0);
}

TEST(queue, two_threads)
{
    int thread1_elem = -1;
    int thread2_elem = -1;
    blocking_queue<int> queue;
    std::thread t1(
            [&thread1_elem, &queue]()
            {
                thread1_elem = queue.take();
            }
    );
    std::thread t2(
            [&thread2_elem, &queue]()
            {
                thread2_elem = queue.take();
            }
    );
    sleep(1);
    queue.push(1);
    sleep(1);
    queue.push(2);
    t1.join();
    t2.join();
    EXPECT_TRUE((thread1_elem == 1 && thread2_elem == 2) ||
                (thread1_elem == 2 && thread2_elem == 1));
}
