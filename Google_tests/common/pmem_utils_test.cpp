#include "gtest/gtest.h"
#include "../../code/common/pmem_utils.h"
#include <random>
#include <chrono>
#include "test_utils.h"
#include "../../code/persistent_memory/persistent_memory_holder.h"
#include "../../code/storage/thread_local_non_owning_storage.h"
#include <thread>
#include <functional>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>

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
    persistent_memory_holder p_stack(file.file_name, false, PMEM_STACK_SIZE);
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &p_stack;
    for (uint32_t i = 1; i < 100; i++)
    {
        EXPECT_FALSE(is_stack_address(p_stack.get_pmem_ptr() - i));
    }
    for (uint32_t i = 0; i < PMEM_STACK_SIZE; i++)
    {
        EXPECT_TRUE(is_stack_address(p_stack.get_pmem_ptr() + i));
    }
    for (uint32_t i = 0; i < 100; i++)
    {
        EXPECT_FALSE(is_stack_address(p_stack.get_pmem_ptr() + PMEM_STACK_SIZE + i));
    }
}

TEST(pmem_utils, address_in_stack_multithreading_test)
{
    uint32_t number_of_threads = 4;
    std::vector<temp_file> files;
    std::vector<persistent_memory_holder> stacks;
    for (uint32_t i = 0; i < number_of_threads; i++)
    {
        std::string file_name = get_temp_file_name("stack");
        files.emplace_back(file_name);
        stacks.emplace_back(file_name, false, PMEM_STACK_SIZE);
    }
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < number_of_threads; i++)
    {
        std::function<void()> thread_action = [i, &stacks]()
        {
            thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stacks[i];
            for (uint32_t j = 1; j < 100; j++)
            {
                EXPECT_FALSE(is_stack_address(stacks[i].get_pmem_ptr() - j));
            }
            for (uint32_t j = 0; j < PMEM_STACK_SIZE; j++)
            {
                EXPECT_TRUE(is_stack_address(stacks[i].get_pmem_ptr() + j));
            }
            for (uint32_t j = 0; j < 100; j++)
            {
                EXPECT_FALSE(
                        is_stack_address(stacks[i].get_pmem_ptr() + PMEM_STACK_SIZE + j)
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

TEST(pmem_utils, flush_test_full)
{
    uint32_t test_count = 100;
    for (int i = 0; i < test_count; ++i)
    {
        temp_file file(get_temp_file_name("stack"));
        uint32_t end_offset = (PAGE_SIZE * 10) + 10;
        int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
        posix_fallocate(fd, 0, end_offset * sizeof(uint32_t));
        void* pmemaddr = mmap(nullptr, end_offset * sizeof(uint32_t),
                              PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        uint32_t* ptr = static_cast<uint32_t*>(pmemaddr);
        for (uint32_t j = 0; j < end_offset; j++)
        {
            *(ptr + j) = j;
        }
        pmem_do_flush(ptr, end_offset * sizeof(uint32_t));
        for (uint32_t j = 0; j < end_offset; j++)
        {
            EXPECT_EQ(*(ptr + j), j);
        }
        munmap(pmemaddr, end_offset * sizeof(uint32_t));
        close(fd);
    }
}

TEST(pmem_utils, flush_test_full_fork)
{
    uint32_t test_count = 100;
    for (int i = 0; i < test_count; ++i)
    {
        temp_file file(get_temp_file_name("stack"));
        uint32_t end_offset = (PAGE_SIZE * 10) + 10;
        pid_t pid_id = fork();
        if (pid_id == 0)
        {
            int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
            posix_fallocate(fd, 0, end_offset * sizeof(uint32_t));
            void* pmemaddr = mmap(nullptr, end_offset * sizeof(uint32_t),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            uint32_t* ptr = static_cast<uint32_t*>(pmemaddr);
            for (uint32_t j = 0; j < end_offset; j++)
            {
                *(ptr + j) = j;
            }
            pmem_do_flush(ptr, end_offset * sizeof(uint32_t));
            exit(EXIT_SUCCESS);
        }
        else
        {
            int status = 0;
            waitpid(pid_id, &status, 0);
            int fd = open(file.file_name.c_str(), O_RDWR, 0666);
            void* pmemaddr = mmap(nullptr, end_offset * sizeof(uint32_t),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            uint32_t* ptr = static_cast<uint32_t*>(pmemaddr);
            for (uint32_t j = 0; j < end_offset; j++)
            {
                EXPECT_EQ(*(ptr + j), j);
            }
            munmap(pmemaddr, end_offset * sizeof(uint32_t));
            close(fd);
        }
    }
}

TEST(pmem_utils, flush_test_partial)
{
    uint32_t test_count = 100;
    for (int i = 0; i < test_count; ++i)
    {
        temp_file file(get_temp_file_name("stack"));
        uint32_t end_offset = (PAGE_SIZE * 10) + 10;
        int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
        posix_fallocate(fd, 0, end_offset * sizeof(uint32_t));
        void* pmemaddr = mmap(nullptr, end_offset * sizeof(uint32_t),
                              PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        uint32_t* ptr = static_cast<uint32_t*>(pmemaddr);
        for (uint32_t j = 0; j < 10; j++)
        {
            for (uint32_t k = 0; k < PAGE_SIZE / 2; k++)
            {
                uint32_t cur_offset = j * PAGE_SIZE + k;
                *(ptr + cur_offset) = cur_offset;
            }
        }
        pmem_do_flush(ptr, end_offset * sizeof(uint32_t));
        for (uint32_t j = 0; j < 10; j++)
        {
            for (uint32_t k = 0; k < PAGE_SIZE / 2; k++)
            {
                uint32_t cur_offset = j * PAGE_SIZE + k;
                EXPECT_EQ(*(ptr + cur_offset), cur_offset);
            }
        }
        munmap(pmemaddr, end_offset * sizeof(uint32_t));
        close(fd);
    }
}

TEST(pmem_utils, flush_test_partial_fork)
{
    uint32_t test_count = 100;
    for (int i = 0; i < test_count; ++i)
    {
        temp_file file(get_temp_file_name("stack"));
        uint32_t end_offset = (PAGE_SIZE * 10) + 10;
        pid_t pid_id = fork();
        if (pid_id == 0)
        {
            int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
            posix_fallocate(fd, 0, end_offset * sizeof(uint32_t));
            void* pmemaddr = mmap(nullptr, end_offset * sizeof(uint32_t),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            uint32_t* ptr = static_cast<uint32_t*>(pmemaddr);
            for (uint32_t j = 0; j < 10; j++)
            {
                for (uint32_t k = 0; k < PAGE_SIZE / 2; k++)
                {
                    uint32_t cur_offset = j * PAGE_SIZE + k;
                    *(ptr + cur_offset) = cur_offset;
                }
            }
            pmem_do_flush(ptr, end_offset * sizeof(uint32_t));
            exit(EXIT_SUCCESS);
        }
        else
        {
            int status = 0;
            waitpid(pid_id, &status, 0);
            int fd = open(file.file_name.c_str(), O_RDWR, 0666);
            void* pmemaddr = mmap(nullptr, end_offset * sizeof(uint32_t),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            uint32_t* ptr = static_cast<uint32_t*>(pmemaddr);
            for (uint32_t j = 0; j < 10; j++)
            {
                for (uint32_t k = 0; k < PAGE_SIZE / 2; k++)
                {
                    uint32_t cur_offset = j * PAGE_SIZE + k;
                    EXPECT_EQ(*(ptr + cur_offset), cur_offset);
                }
            }
            munmap(pmemaddr, end_offset * sizeof(uint32_t));
            close(fd);
        }
    }
}