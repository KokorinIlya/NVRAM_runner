#include "gtest/gtest.h"
#include "../../code/allocation/pmem_allocator.h"
#include "../../code/persistent_memory/persistent_memory_holder.h"
#include "../common/test_utils.h"
#include <functional>

TEST(pmem_allocator, single_allocation)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t offset_1 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_1, 2);
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
}

TEST(pmem_allocator, multiple_allocations)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t offset_1 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_1, 2);

    uint64_t offset_2 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_2, 4);

    uint64_t offset_3 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_3, 6);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));
}

TEST(pmem_allocator, free_last)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t offset_1 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_1, 2);

    uint64_t offset_2 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_2, 4);

    uint64_t offset_3 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_3, 6);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));

    allocator.pmem_free(heap.get_pmem_ptr() + offset_3);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));
}

TEST(pmem_allocator, free_last_and_allocate)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t offset_1 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_1, 2);

    uint64_t offset_2 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_2, 4);

    uint64_t offset_3 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_3, 6);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));

    allocator.pmem_free(heap.get_pmem_ptr() + offset_3);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));

    uint64_t offset_4 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_4, 6);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_4));
}

TEST(pmem_allocator, free_middle)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t offset_1 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_1, 2);

    uint64_t offset_2 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_2, 4);

    uint64_t offset_3 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_3, 6);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));

    allocator.pmem_free(heap.get_pmem_ptr() + offset_2);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));
}

TEST(pmem_allocator, free_middle_and_allocate)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t offset_1 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_1, 2);

    uint64_t offset_2 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_2, 4);

    uint64_t offset_3 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_3, 6);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));

    allocator.pmem_free(heap.get_pmem_ptr() + offset_2);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));

    uint64_t offset_4 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_4, 4);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_4));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));
}

TEST(pmem_allocator, border_shift)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t offset_1 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_1, 2);

    uint64_t offset_2 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_2, 4);

    uint64_t offset_3 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_3, 6);

    uint64_t offset_4 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_4, 8);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_4));

    allocator.pmem_free(heap.get_pmem_ptr() + offset_2);
    allocator.pmem_free(heap.get_pmem_ptr() + offset_3);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_4));

    allocator.pmem_free(heap.get_pmem_ptr() + offset_4);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_2));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_4));

    uint64_t offset_5 = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(offset_5, 4);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_1));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + offset_5));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_3));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + offset_4));
}

TEST(pmem_allocator, persistence)
{
    temp_file file(get_temp_file_name("heap"));

    std::function<void()> execution = [&file]()
    {
        persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
        pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

        uint64_t offset_1 = allocator.pmem_alloc() - heap.get_pmem_ptr();
        EXPECT_EQ(offset_1, 2);

        uint64_t offset_2 = allocator.pmem_alloc() - heap.get_pmem_ptr();
        EXPECT_EQ(offset_2, 4);

        uint64_t offset_3 = allocator.pmem_alloc() - heap.get_pmem_ptr();
        EXPECT_EQ(offset_3, 6);

        uint64_t offset_4 = allocator.pmem_alloc() - heap.get_pmem_ptr();
        EXPECT_EQ(offset_4, 8);

        allocator.pmem_free(heap.get_pmem_ptr() + offset_2);
        allocator.pmem_free(heap.get_pmem_ptr() + offset_3);
        allocator.pmem_free(heap.get_pmem_ptr() + offset_4);

        uint64_t offset_5 = allocator.pmem_alloc() - heap.get_pmem_ptr();
        EXPECT_EQ(offset_5, 4);

        uint64_t offset_6 = allocator.pmem_alloc() - heap.get_pmem_ptr();
        EXPECT_EQ(offset_6, 6);

        allocator.pmem_free(heap.get_pmem_ptr() + offset_5);
    };
    execution();

    std::function<void()> recovery = [&file]()
    {
        persistent_memory_holder heap(file.file_name, true, PMEM_HEAP_SIZE);
        pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, false);

        EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + 2));
        EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + 4));
        EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + 6));
        EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + 8));
        EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + 10));
    };
    recovery();
}