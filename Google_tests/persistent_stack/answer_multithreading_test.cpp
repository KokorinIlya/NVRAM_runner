#include "gtest/gtest.h"
#include "../../code/persistent_memory/persistent_memory_holder.h"
#include "../../code/persistent_stack/call.h"
#include "../../code/storage/global_storage.h"
#include "../common/test_utils.h"
#include <functional>
#include <thread>
#include "../../code/model/function_address_holder.h"

namespace
{
    void f_0(const uint8_t*)
    {
        do_call("g_0", std::vector<uint8_t>({4, 5, 6, 0}));
        std::vector<uint8_t> g_answer = read_answer(7);
        EXPECT_EQ(g_answer, std::vector<uint8_t>({1, 3, 5, 7, 9, 6, 0}));

        do_call("l_0", std::vector<uint8_t>({10, 11, 12, 0}));
        std::vector<uint8_t> l_answer = read_answer(3);
        EXPECT_EQ(l_answer, std::vector<uint8_t>({2, 10, 0}));

        throw std::runtime_error("Ha-ha, system crash goes brrrrr");
    }

    void g_0(const uint8_t* ptr)
    {
        do_call("h_0", std::vector<uint8_t>({7, 8, 9, 0}));
        std::vector<uint8_t> h_answer = read_answer(6);
        EXPECT_EQ(h_answer, std::vector<uint8_t>({1, 3, 3, 7, 8, 0}));
        uint8_t val = *(ptr + 2);
        write_answer(std::vector<uint8_t>({1, 3, 5, 7, 9, val, 0}));
    }

    void h_0(const uint8_t* ptr)
    {
        uint8_t val = *(ptr + 1);
        write_answer(std::vector<uint8_t>({1, 3, 3, 7, val, 0}));
    }

    void l_0(const uint8_t* ptr)
    {
        uint8_t val = *ptr;
        write_answer(std::vector<uint8_t>({2, val, 0}));
    }

    void f_1(const uint8_t*)
    {
        do_call("g_1", std::vector<uint8_t>({4, 5, 6, 1}));
        std::vector<uint8_t> g_answer = read_answer(7);
        EXPECT_EQ(g_answer, std::vector<uint8_t>({1, 3, 5, 7, 9, 6, 1}));

        do_call("l_1", std::vector<uint8_t>({10, 11, 12, 1}));
        std::vector<uint8_t> l_answer = read_answer(3);
        EXPECT_EQ(l_answer, std::vector<uint8_t>({2, 10, 1}));

        throw std::runtime_error("Ha-ha, system crash goes brrrrr");
    }

    void g_1(const uint8_t* ptr)
    {
        do_call("h_1", std::vector<uint8_t>({7, 8, 9, 1}));
        std::vector<uint8_t> h_answer = read_answer(6);
        EXPECT_EQ(h_answer, std::vector<uint8_t>({1, 3, 3, 7, 8, 1}));
        uint8_t val = *(ptr + 2);
        write_answer(std::vector<uint8_t>({1, 3, 5, 7, 9, val, 1}));
    }

    void h_1(const uint8_t* ptr)
    {
        uint8_t val = *(ptr + 1);
        write_answer(std::vector<uint8_t>({1, 3, 3, 7, val, 1}));
    }

    void l_1(const uint8_t* ptr)
    {
        uint8_t val = *ptr;
        write_answer(std::vector<uint8_t>({2, val, 1}));
    }

    void b_0(const uint8_t*)
    {
        write_answer(std::vector<uint8_t>({1, 3, 3, 7, 0}));
        std::vector<uint8_t> ans = read_current_answer(5);
        EXPECT_EQ(ans, std::vector<uint8_t>({1, 3, 3, 7, 0}));
    }

    void a_0(const uint8_t*)
    {
        do_call("b_0", std::vector<uint8_t>({4, 5, 6}));
        throw std::runtime_error("system crash");
    }

    void b_1(const uint8_t*)
    {
        write_answer(std::vector<uint8_t>({1, 3, 3, 7, 1}));
        std::vector<uint8_t> ans = read_current_answer(5);
        EXPECT_EQ(ans, std::vector<uint8_t>({1, 3, 3, 7, 1}));
    }

    void a_1(const uint8_t*)
    {
        do_call("b_1", std::vector<uint8_t>({4, 5, 6}));
        throw std::runtime_error("system crash");
    }

    void d_0(const uint8_t*)
    {
        std::vector<uint8_t> ans = read_current_answer(4);
        EXPECT_EQ(ans, std::vector<uint8_t>({7, 8, 9, 0}));
    }

    void c_0(const uint8_t*)
    {
        do_call(
                "d_0",
                std::vector<uint8_t>({4, 5, 6}),
                std::make_optional(std::vector<uint8_t>({7, 8, 9, 0}))
        );
        throw std::runtime_error("system crash");
    }

    void d_1(const uint8_t*)
    {
        std::vector<uint8_t> ans = read_current_answer(4);
        EXPECT_EQ(ans, std::vector<uint8_t>({7, 8, 9, 1}));
    }

    void c_1(const uint8_t*)
    {
        do_call(
                "d_1",
                std::vector<uint8_t>({4, 5, 6}),
                std::make_optional(std::vector<uint8_t>({7, 8, 9, 1}))
        );
        throw std::runtime_error("system crash");
    }
}

TEST(answer_multithreading, return_value)
{
    uint32_t number_of_threads = 2;
    std::vector<temp_file> temp_files;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        temp_files.emplace_back(
                get_temp_file_name("stack-" + std::to_string(i))
        );
    }
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs.clear();

    global_storage<function_address_holder>::get_object().funcs["f_0"] = std::make_pair(f_0, f_0);
    global_storage<function_address_holder>::get_object().funcs["g_0"] = std::make_pair(g_0, g_0);
    global_storage<function_address_holder>::get_object().funcs["h_0"] = std::make_pair(h_0, h_0);
    global_storage<function_address_holder>::get_object().funcs["l_0"] = std::make_pair(l_0, l_0);

    global_storage<function_address_holder>::get_object().funcs["f_1"] = std::make_pair(f_1, f_1);
    global_storage<function_address_holder>::get_object().funcs["g_1"] = std::make_pair(g_1, g_1);
    global_storage<function_address_holder>::get_object().funcs["h_1"] = std::make_pair(h_1, h_1);
    global_storage<function_address_holder>::get_object().funcs["l_1"] = std::make_pair(l_1, l_1);

    std::vector<persistent_memory_holder> stacks;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        stacks.emplace_back(temp_files[i].file_name, false, PMEM_STACK_SIZE);
    }
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        std::function<void()> thread_action = [&stacks, i]()
        {
            uint8_t small_i = static_cast<uint8_t>(i);
            thread_local_owning_storage<ram_stack>::set_object(ram_stack());
            thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stacks[i];
            try
            {
                do_call(
                        "f_" + std::to_string(i),
                        std::vector<uint8_t>({1, 2, 3, small_i})
                );
            } catch (...)
            {}
        };
        threads.emplace_back(std::thread(thread_action));
    }
    for (std::thread& cur_thread: threads)
    {
        cur_thread.join();
    }
}

TEST(answer_multithreading, read_own_value)
{
    uint32_t number_of_threads = 2;
    std::vector<temp_file> temp_files;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        temp_files.emplace_back(
                get_temp_file_name("stack-" + std::to_string(i))
        );
    }
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs.clear();

    global_storage<function_address_holder>::get_object().funcs["a_0"] = std::make_pair(a_0, a_0);
    global_storage<function_address_holder>::get_object().funcs["b_0"] = std::make_pair(b_0, b_0);

    global_storage<function_address_holder>::get_object().funcs["a_1"] = std::make_pair(a_1, a_1);
    global_storage<function_address_holder>::get_object().funcs["b_1"] = std::make_pair(b_1, b_1);

    std::vector<persistent_memory_holder> stacks;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        stacks.emplace_back(temp_files[i].file_name, false, PMEM_STACK_SIZE);
    }
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        std::function<void()> thread_action = [&stacks, i]()
        {
            uint8_t small_i = static_cast<uint8_t>(i);
            thread_local_owning_storage<ram_stack>::set_object(ram_stack());
            thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stacks[i];
            try
            {
                do_call(
                        "a_" + std::to_string(i),
                        std::vector<uint8_t>({1, 2, 3, small_i})
                );
            } catch (...)
            {}
        };
        threads.emplace_back(std::thread(thread_action));
    }
    for (std::thread& cur_thread: threads)
    {
        cur_thread.join();
    }
}

TEST(answer_multithreading, fill_answer)
{
    uint32_t number_of_threads = 2;
    std::vector<temp_file> temp_files;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        temp_files.emplace_back(
                get_temp_file_name("stack-" + std::to_string(i))
        );
    }
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs.clear();

    global_storage<function_address_holder>::get_object().funcs["c_0"] = std::make_pair(c_0, c_0);
    global_storage<function_address_holder>::get_object().funcs["d_0"] = std::make_pair(d_0, d_0);

    global_storage<function_address_holder>::get_object().funcs["c_1"] = std::make_pair(c_1, c_1);
    global_storage<function_address_holder>::get_object().funcs["d_1"] = std::make_pair(d_1, d_1);

    std::vector<persistent_memory_holder> stacks;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        stacks.emplace_back(temp_files[i].file_name, false, PMEM_STACK_SIZE);
    }
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        std::function<void()> thread_action = [&stacks, i]()
        {
            uint8_t small_i = static_cast<uint8_t>(i);
            thread_local_owning_storage<ram_stack>::set_object(ram_stack());
            thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stacks[i];
            try
            {
                do_call(
                        "a_" + std::to_string(i),
                        std::vector<uint8_t>({1, 2, 3, small_i})
                );
            } catch (...)
            {}
        };
        threads.emplace_back(std::thread(thread_action));
    }
    for (std::thread& cur_thread: threads)
    {
        cur_thread.join();
    }
}