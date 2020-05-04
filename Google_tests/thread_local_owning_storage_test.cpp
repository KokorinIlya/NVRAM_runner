#include "gtest/gtest.h"
#include "../code/globals/thread_local_owning_storage.h"
#include <functional>
#include <thread>

TEST(thread_local_owning_storage, get_without_set)
{
    EXPECT_THROW(thread_local_owning_storage<int>::get_object(),
                 std::runtime_error);
}

TEST(thread_local_owning_storage, get_after_set)
{
    thread_local_owning_storage<int>::set_object(1);
    EXPECT_EQ(thread_local_owning_storage<int>::get_object(), 1);
}

TEST(thread_local_owning_storage, get_and_change)
{
    thread_local_owning_storage<int>::set_object(1);
    EXPECT_EQ(thread_local_owning_storage<int>::get_object(), 1);
    thread_local_owning_storage<int>::get_object()++;
    EXPECT_EQ(thread_local_owning_storage<int>::get_object(), 2);
}

TEST(thread_local_owning_storage, multiple_sets)
{
    thread_local_owning_storage<int>::set_object(1);
    EXPECT_EQ(thread_local_owning_storage<int>::get_object(), 1);
    thread_local_owning_storage<int>::set_object(2);
    thread_local_owning_storage<int>::set_object(3);
    EXPECT_EQ(thread_local_owning_storage<int>::get_object(), 3);
}

TEST(thread_local_owning_storage, multiple_types)
{
    thread_local_owning_storage<int>::set_object(1);
    EXPECT_EQ(thread_local_owning_storage<int>::get_object(), 1);
    thread_local_owning_storage<std::string>::set_object("hello");
    EXPECT_EQ(thread_local_owning_storage<std::string>::get_object(), "hello");
}

namespace
{
    std::function<void()> get_thread_action(int x, bool& t_correct)
    {
        return [x, &t_correct]()
        {
            thread_local_owning_storage<int>::set_object(x);
            thread_local_owning_storage<int>::get_object()++;
            t_correct = thread_local_owning_storage<int>::get_const_object() == x + 1;
        };
    }

    struct test
    {
    private:
        int* t_correct;
        mutable bool should_delete = true;
    public:
        explicit test(int* b) : t_correct(b)
        {
        }

        test(test const& other) : t_correct(other.t_correct)
        {
            other.should_delete = false;
        };

        test& operator=(test const& other)
        {
            if (&other == this)
            {
                return *this;
            }
            t_correct = other.t_correct;
            other.should_delete = false;
            return *this;
        };

        ~test()
        {
            if (should_delete)
            {
                ++*t_correct;
            }
        }
    };
}

TEST(thread_local_owning_storage, multiple_threads)
{
    int a = 100;
    int b = 200;
    bool t1_correct = false;
    bool t2_correct = false;
    std::thread t1(get_thread_action(a, t1_correct));
    std::thread t2(get_thread_action(b, t2_correct));
    t1.join();
    t2.join();
    EXPECT_TRUE(t1_correct && t2_correct);
}

TEST(thread_local_owning_storage, owning)
{
    int t_correct = 0;

    std::thread t(
            [&t_correct]()
            {
                thread_local_owning_storage<test>::set_object(test(&t_correct));
            }
    );
    t.join();
    EXPECT_EQ(t_correct, 1);
}