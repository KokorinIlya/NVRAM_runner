#include "gtest/gtest.h"
#include "../../code/allocation/pmem_allocator.h"
#include "../../code/persistent_memory/persistent_memory_holder.h"
#include "../common/test_utils.h"

TEST(pmem_allocator, single_allocation)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t first_offset = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(first_offset, 2);
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + first_offset));
}

TEST(pmem_allocator, allocations)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t first_offset = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(first_offset, 2);

    uint64_t second_offset = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(second_offset, 4);

    uint64_t third_offset = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(third_offset, 6);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + first_offset));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + second_offset));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + third_offset));
}

TEST(pmem_allocator, free_last)
{
    temp_file file(get_temp_file_name("heap"));
    persistent_memory_holder heap(file.file_name, false, PMEM_HEAP_SIZE);
    pmem_allocator allocator(heap.get_pmem_ptr(), 1, 200, true);

    uint64_t first_offset = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(first_offset, 2);

    uint64_t second_offset = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(second_offset, 4);

    uint64_t third_offset = allocator.pmem_alloc() - heap.get_pmem_ptr();
    EXPECT_EQ(third_offset, 6);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + first_offset));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + second_offset));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + third_offset));

    allocator.pmem_free(heap.get_pmem_ptr() + third_offset);

    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + first_offset));
    EXPECT_TRUE(allocator.is_allocated(heap.get_pmem_ptr() + second_offset));
    EXPECT_FALSE(allocator.is_allocated(heap.get_pmem_ptr() + third_offset));
}