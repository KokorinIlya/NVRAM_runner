#include "gtest/gtest.h"
#include "../../code/common/pmem_utils.h"
#include <random>
#include <chrono>
#include "test_utils.h"
#include "../../code/persistent_stack/persistent_stack.h"
#include "../../code/storage/thread_local_non_owning_storage.h"
#include <thread>
#include <functional>

TEST(pmem_utils, align)
{
    uint32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<uint64_t> k_distribution(0, 10000);
    std::uniform_int_distribution<uint64_t> b_distribution(0, CACHE_LINE_SIZE - 1);
    for (uint32_t i = 0; i < 1000; i++)
    {
        uint64_t k = k_distribution(generator);
        uint64_t b = b_distribution(generator);
        uint64_t addr = k * CACHE_LINE_SIZE + b;
        uint64_t ans = k * CACHE_LINE_SIZE;
        if (b != 0)
        {
            ans += CACHE_LINE_SIZE;
        }
        EXPECT_EQ(ans, get_cache_line_aligned_address(addr));
    }
}

TEST(pmem_utils, align_idempotency)
{
    uint32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<uint64_t> k_distribution(0, 10000);
    std::uniform_int_distribution<uint64_t> b_distribution(0, CACHE_LINE_SIZE - 1);
    for (uint32_t i = 0; i < 1000; i++)
    {
        uint64_t k = k_distribution(generator);
        uint64_t b = b_distribution(generator);
        uint64_t addr = k * CACHE_LINE_SIZE + b;
        uint64_t ans = get_cache_line_aligned_address(addr);
        EXPECT_EQ(ans, get_cache_line_aligned_address(ans));
    }
}

TEST(pmem_utils, address_in_stack_simple_test)
{
    temp_file file(get_temp_file_name("stack"));
    persistent_stack p_stack(file.file_name, false);
    thread_local_non_owning_storage<persistent_stack>::ptr = &p_stack;
    for (uint32_t i = 1; i < 100; i++)
    {
        EXPECT_FALSE(is_stack_address(p_stack.get_stack_ptr() - i));
    }
    for (uint32_t i = 0; i < PMEM_STACK_SIZE; i++)
    {
        EXPECT_TRUE(is_stack_address(p_stack.get_stack_ptr() + i));
    }
    for (uint32_t i = 0; i < 100; i++)
    {
        EXPECT_FALSE(is_stack_address(p_stack.get_stack_ptr() + PMEM_STACK_SIZE + i));
    }
}

TEST(pmem_utils, address_in_stack_multithreading_test)
{
    uint32_t number_of_threads = 4;
    std::vector<temp_file> files;
    std::vector<persistent_stack> stacks;
    for (uint32_t i = 0; i < number_of_threads; i++)
    {
        std::string file_name = get_temp_file_name("stack");
        files.emplace_back(file_name);
        stacks.emplace_back(file_name, false);
    }
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < number_of_threads; i++)
    {
        std::function<void()> thread_action = [i, &stacks]()
        {
            thread_local_non_owning_storage<persistent_stack>::ptr = &stacks[i];
            for (uint32_t j = 1; j < 100; j++)
            {
                EXPECT_FALSE(is_stack_address(stacks[i].get_stack_ptr() - j));
            }
            for (uint32_t j = 0; j < PMEM_STACK_SIZE; j++)
            {
                EXPECT_TRUE(is_stack_address(stacks[i].get_stack_ptr() + j));
            }
            for (uint32_t j = 0; j < 100; j++)
            {
                EXPECT_FALSE(
                        is_stack_address(stacks[i].get_stack_ptr() + PMEM_STACK_SIZE + j)
                );
            }
        };
        threads.emplace_back(thread_action);
    }
    for (std::thread& cur_thread: threads)
    {
        cur_thread.join();
    }
}