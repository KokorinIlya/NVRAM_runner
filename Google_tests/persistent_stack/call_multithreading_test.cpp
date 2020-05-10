#include "gtest/gtest.h"
#include "../../code/persistent_stack/persistent_stack.h"
#include "../../code/persistent_stack/frames.h"
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
    }

    void g_0(const uint8_t*)
    {
        do_call("h_0", std::vector<uint8_t>({7, 8, 9, 0}));
        throw std::runtime_error("Ha-ha, crash goes brrrrrr");
    }

    void h_0(const uint8_t*)
    {
    }

    void f_1(const uint8_t*)
    {
        do_call("g_1", std::vector<uint8_t>({4, 5, 6, 1}));
    }

    void g_1(const uint8_t*)
    {
        do_call("h_1", std::vector<uint8_t>({7, 8, 9, 1}));
        throw std::runtime_error("Ha-ha, crash goes brrrrrr");
    }

    void h_1(const uint8_t*)
    {
    }
}

TEST(call_multithreading, restoration_after_crash)
{
    uint32_t number_of_threads = 2;
    std::vector<temp_file> temp_files;
    for (uint32_t i = 0; i < number_of_threads; ++i)
    {
        temp_files.emplace_back(
                get_temp_file_name("stack-" + std::to_string(i))
        );
    }
    std::function<void()> execution = [number_of_threads, &temp_files]()
    {
        global_storage<function_address_holder>::set_object(function_address_holder());
        global_storage<function_address_holder>::get_object().funcs.clear();

        global_storage<function_address_holder>::get_object().funcs["f_0"] = std::make_pair(f_0, f_0);
        global_storage<function_address_holder>::get_object().funcs["g_0"] = std::make_pair(g_0, g_0);
        global_storage<function_address_holder>::get_object().funcs["h_0"] = std::make_pair(h_0, h_0);

        global_storage<function_address_holder>::get_object().funcs["f_1"] = std::make_pair(f_1, f_1);
        global_storage<function_address_holder>::get_object().funcs["g_1"] = std::make_pair(g_1, g_1);
        global_storage<function_address_holder>::get_object().funcs["h_1"] = std::make_pair(h_1, h_1);

        std::vector<persistent_stack> stacks;
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            stacks.emplace_back(temp_files[i].file_name, false);
        }
        std::vector<std::thread> threads;
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            std::function<void()> thread_action = [&stacks, i]()
            {
                uint8_t small_i = static_cast<uint8_t>(i);
                thread_local_owning_storage<ram_stack>::set_object(ram_stack());
                thread_local_non_owning_storage<persistent_stack>::ptr = &stacks[i];
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
    };
    execution();

    std::function<void()> restoration = [number_of_threads, &temp_files]()
    {
        std::vector<persistent_stack> stacks;
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            stacks.emplace_back(temp_files[i].file_name, true);
        }
        std::vector<std::thread> threads;
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            std::function<void()> thread_action = [&stacks, i]()
            {
                uint8_t small_i = static_cast<uint8_t>(i);
                thread_local_non_owning_storage<persistent_stack>::ptr =
                        &stacks[i];
                thread_local_owning_storage<ram_stack>::set_object(
                        read_stack(*thread_local_non_owning_storage<persistent_stack>::ptr)
                );
                ram_stack& r_stack = thread_local_owning_storage<ram_stack>::get_object();
                EXPECT_EQ(r_stack.size(), 2);

                stack_frame frame_2 = r_stack.get_last_frame().frame;
                r_stack.remove_frame();
                EXPECT_EQ(frame_2.function_name, "g_" + std::to_string(i));
                EXPECT_EQ(frame_2.args, std::vector<uint8_t>({4, 5, 6, small_i}));

                stack_frame frame_1 = r_stack.get_last_frame().frame;
                r_stack.remove_frame();
                EXPECT_EQ(frame_1.function_name, "f_" + std::to_string(i));
                EXPECT_EQ(frame_1.args, std::vector<uint8_t>({1, 2, 3, small_i}));
            };
            threads.emplace_back(std::thread(thread_action));
        }
        for (std::thread& cur_thread: threads)
        {
            cur_thread.join();
        }
    };
    restoration();
}