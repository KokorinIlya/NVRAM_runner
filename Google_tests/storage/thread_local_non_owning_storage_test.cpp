#include "../../code/storage/thread_local_non_owning_storage.h"
#include "gtest/gtest.h"
#include <unistd.h>
#include "functional"
#include <thread>

namespace
{
    std::function<void()> get_thread_action(int x, bool& t_correct)
    {
        return [&x, &t_correct]()
        {
            thread_local_non_owning_storage<int>::ptr = &x;
            sleep(1);
            t_correct = thread_local_non_owning_storage<int>::ptr == &x;
        };
    }
}

TEST(thread_local_non_owning_storage, two_threads)
{
    int a = 1;
    int b = 2;
    bool t1_correct = false;
    bool t2_correct = false;
    std::thread t1(get_thread_action(a, t1_correct));
    std::thread t2(get_thread_action(b, t2_correct));
    t1.join();
    t2.join();
    EXPECT_TRUE(t1_correct && t2_correct);
}

TEST(thread_local_non_owning_storage, different_types)
{
    int a = 1;
    std::string s = "hello";
    thread_local_non_owning_storage<int>::ptr = &a;
    thread_local_non_owning_storage<std::string>::ptr = &s;
    EXPECT_TRUE(thread_local_non_owning_storage<int>::ptr == &a &&
                thread_local_non_owning_storage<std::string>::ptr == &s);
}

TEST(thread_local_non_owning_storage, non_owning)
{
    std::vector<int> v{1, 3, 3, 7};
    std::thread t1(
            [&v]()
            {
                thread_local_non_owning_storage<std::vector<int>>::ptr = &v;
            }
    );
    t1.join();
    EXPECT_EQ(v, std::vector<int>({1, 3, 3, 7}));
}