#include "gtest/gtest.h"
#include "../code/globals/global_storage.h"
#include <functional>
#include <thread>

TEST(global_storage, get_without_set)
{
    EXPECT_THROW(global_storage<int>::get_object(),
                 std::runtime_error);
}

TEST(global_storage, get_after_set)
{
    global_storage<int>::set_object(1);
    EXPECT_EQ(global_storage<int>::get_object(), 1);
}

TEST(global_storage, get_and_change)
{
    global_storage<int>::set_object(1);
    EXPECT_EQ(global_storage<int>::get_const_object(), 1);
    global_storage<int>::get_object()++;
    EXPECT_EQ(global_storage<int>::get_const_object(), 2);
}

TEST(global_storage, multiple_sets)
{
    global_storage<int>::set_object(1);
    EXPECT_EQ(global_storage<int>::get_const_object(), 1);
    global_storage<int>::set_object(2);
    global_storage<int>::set_object(3);
    EXPECT_EQ(global_storage<int>::get_object(), 3);
}

TEST(global_storage, multiple_types)
{
    global_storage<int>::set_object(1);
    EXPECT_EQ(global_storage<int>::get_const_object(), 1);
    global_storage<std::string>::set_object("hello");
    EXPECT_EQ(global_storage<std::string>::get_const_object(), "hello");
}

TEST(global_storage, multiple_threads)
{
    int a = 100;
    int b = 200;
    global_storage<int>::set_object(1);
    std::function<void()> thread_action = []()
    {
        EXPECT_EQ(global_storage<int>::get_const_object(), 1);
    };
    std::thread t1(thread_action);
    std::thread t2(thread_action);
    t1.join();
    t2.join();
}