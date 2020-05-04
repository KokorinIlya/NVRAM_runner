#include "../code/globals/function_address_holder.h"
#include "gtest/gtest.h"
#include <utility>
#include <thread>
#include <functional>

TEST(function_address_holder, init)
{
    function_address_holder::functions.clear();
    EXPECT_EQ(function_address_holder::functions.empty(), true);
}

namespace
{
    void f(const uint8_t*)
    {}

    void f_recover(const uint8_t*)
    {}

    void g(const uint8_t*)
    {}

    void g_recover(const uint8_t*)
    {}

    std::function<void()> get_thread_action(bool& t_correct)
    {
        return [&t_correct]()
        {
            t_correct = function_address_holder::functions["f"].first == f &&
                        function_address_holder::functions["f"].second == f_recover &&
                        function_address_holder::functions["g"].first == g &&
                        function_address_holder::functions["g"].second == g_recover;
        };
    }
}

TEST(function_address_holder, add)
{
    function_address_holder::functions.clear();
    function_address_holder::functions["f"] = std::make_pair(f, f_recover);
    EXPECT_EQ(function_address_holder::functions.size(), 1);
}

TEST(function_address_holder, threads)
{
    function_address_holder::functions.clear();
    function_address_holder::functions["f"] = std::make_pair(f, f_recover);
    function_address_holder::functions["g"] = std::make_pair(g, g_recover);
    bool t1_correct = false;
    bool t2_correct = false;
    std::thread t1(get_thread_action(t1_correct));
    std::thread t2(get_thread_action(t2_correct));
    t1.join();
    t2.join();
    EXPECT_TRUE(t1_correct && t2_correct);
}