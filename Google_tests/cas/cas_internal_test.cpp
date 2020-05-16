#include "gtest/gtest.h"
#include "../../code/cas/cas.h"
#include "../common/test_utils.h"
#include <fcntl.h>
#include "../../code/common/constants_and_types.h"
#include "../../code/common/pmem_utils.h"
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <limits>
#include "../../code/persistent_memory/persistent_memory_holder.h"

TEST(cas, single_successful)
{
    temp_file file(get_temp_file_name("heap"));
    int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (posix_fallocate(fd, 0, PMEM_HEAP_SIZE) != 0)
    {
        perror("allocate");
        exit(EXIT_FAILURE);
    }
    void* pmemaddr = mmap(nullptr, PMEM_HEAP_SIZE,
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    uint8_t* stack_ptr = static_cast<uint8_t*>(pmemaddr);
    uint64_t* var = (uint64_t*) stack_ptr;
    uint32_t total_thread_number = 4;
    uint32_t* thread_matrix = (uint32_t*) stack_ptr + 8;

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    bool result = cas_internal(var, 42, 24, 1,
                               total_thread_number, thread_matrix);
    EXPECT_TRUE(result);

    uint32_t thread_number;
    std::memcpy(&thread_number, stack_ptr, 4);
    uint32_t value;
    std::memcpy(&value, stack_ptr + 4, 4);
    EXPECT_EQ(thread_number, 1);
    EXPECT_EQ(value, 24);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }

    if (munmap(pmemaddr, PMEM_STACK_SIZE) < 0)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if (close(fd) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

TEST(cas, single_failed)
{
    temp_file file(get_temp_file_name("heap"));
    int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (posix_fallocate(fd, 0, PMEM_HEAP_SIZE) != 0)
    {
        perror("allocate");
        exit(EXIT_FAILURE);
    }
    void* pmemaddr = mmap(nullptr, PMEM_HEAP_SIZE,
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    uint8_t* stack_ptr = static_cast<uint8_t*>(pmemaddr);
    uint64_t* var = (uint64_t*) stack_ptr;
    uint32_t total_thread_number = 4;
    uint32_t* thread_matrix = (uint32_t*) stack_ptr + 8;

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    bool result = cas_internal(var, 24, 18, 1,
                               total_thread_number, thread_matrix);
    EXPECT_FALSE(result);

    uint32_t thread_number;
    std::memcpy(&thread_number, stack_ptr, 4);
    uint32_t value;
    std::memcpy(&value, stack_ptr + 4, 4);
    EXPECT_EQ(thread_number, std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(value, 42);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }

    if (munmap(pmemaddr, PMEM_STACK_SIZE) < 0)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if (close(fd) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

TEST(cas, two_successful)
{
    temp_file file(get_temp_file_name("heap"));
    int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (posix_fallocate(fd, 0, PMEM_HEAP_SIZE) != 0)
    {
        perror("allocate");
        exit(EXIT_FAILURE);
    }
    void* pmemaddr = mmap(nullptr, PMEM_HEAP_SIZE,
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    uint8_t* stack_ptr = static_cast<uint8_t*>(pmemaddr);
    uint64_t* var = (uint64_t*) stack_ptr;
    uint32_t total_thread_number = 4;
    uint32_t* thread_matrix = (uint32_t*) stack_ptr + 8;

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    bool result = cas_internal(var, 42, 24, 1,
                               total_thread_number, thread_matrix);
    EXPECT_TRUE(result);

    uint32_t thread_number;
    std::memcpy(&thread_number, stack_ptr, 4);
    uint32_t value;
    std::memcpy(&value, stack_ptr + 4, 4);
    EXPECT_EQ(thread_number, 1);
    EXPECT_EQ(value, 24);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }

    bool second_result = cas_internal(var, 24, 53, 2,
                                      total_thread_number, thread_matrix);
    EXPECT_TRUE(second_result);

    std::memcpy(&thread_number, stack_ptr, 4);
    std::memcpy(&value, stack_ptr + 4, 4);
    EXPECT_EQ(thread_number, 2);
    EXPECT_EQ(value, 53);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            if (thread_num == 1 && other_thread_num == 2)
            {
                EXPECT_EQ(cur_value, 24);
            }
            else
            {
                EXPECT_EQ(cur_value, 0);
            }
        }
    }

    if (munmap(pmemaddr, PMEM_STACK_SIZE) < 0)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if (close(fd) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

TEST(cas, failed_after_successful)
{
    temp_file file(get_temp_file_name("heap"));
    int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (posix_fallocate(fd, 0, PMEM_HEAP_SIZE) != 0)
    {
        perror("allocate");
        exit(EXIT_FAILURE);
    }
    void* pmemaddr = mmap(nullptr, PMEM_HEAP_SIZE,
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    uint8_t* stack_ptr = static_cast<uint8_t*>(pmemaddr);
    uint64_t* var = (uint64_t*) stack_ptr;
    uint32_t total_thread_number = 4;
    uint32_t* thread_matrix = (uint32_t*) stack_ptr + 8;

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    bool result = cas_internal(var, 42, 24, 1,
                               total_thread_number, thread_matrix);
    EXPECT_TRUE(result);

    uint32_t thread_number;
    std::memcpy(&thread_number, stack_ptr, 4);
    uint32_t value;
    std::memcpy(&value, stack_ptr + 4, 4);
    EXPECT_EQ(thread_number, 1);
    EXPECT_EQ(value, 24);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }

    bool second_result = cas_internal(var, 18, 53, 2,
                                      total_thread_number, thread_matrix);
    EXPECT_FALSE(second_result);

    std::memcpy(&thread_number, stack_ptr, 4);
    std::memcpy(&value, stack_ptr + 4, 4);
    EXPECT_EQ(thread_number, 1);
    EXPECT_EQ(value, 24);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }

    if (munmap(pmemaddr, PMEM_STACK_SIZE) < 0)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if (close(fd) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

TEST(cas, successful_after_failed)
{
    temp_file file(get_temp_file_name("heap"));
    int fd = open(file.file_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (posix_fallocate(fd, 0, PMEM_HEAP_SIZE) != 0)
    {
        perror("allocate");
        exit(EXIT_FAILURE);
    }
    void* pmemaddr = mmap(nullptr, PMEM_HEAP_SIZE,
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    uint8_t* stack_ptr = static_cast<uint8_t*>(pmemaddr);
    uint64_t* var = (uint64_t*) stack_ptr;
    uint32_t total_thread_number = 4;
    uint32_t* thread_matrix = (uint32_t*) stack_ptr + 8;

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    bool result = cas_internal(var, 24, 53, 1,
                               total_thread_number, thread_matrix);
    EXPECT_FALSE(result);

    uint32_t thread_number;
    std::memcpy(&thread_number, stack_ptr, 4);
    uint32_t value;
    std::memcpy(&value, stack_ptr + 4, 4);
    EXPECT_EQ(thread_number, std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(value, 42);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }

    bool second_result = cas_internal(var, 42, 53, 2,
                                      total_thread_number, thread_matrix);
    EXPECT_TRUE(second_result);

    std::memcpy(&thread_number, stack_ptr, 4);
    std::memcpy(&value, stack_ptr + 4, 4);
    EXPECT_EQ(thread_number, 2);
    EXPECT_EQ(value, 53);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }

    if (munmap(pmemaddr, PMEM_STACK_SIZE) < 0)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if (close(fd) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}