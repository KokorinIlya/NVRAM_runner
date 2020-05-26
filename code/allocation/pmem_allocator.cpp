#include "pmem_allocator.h"

#include <cstring>
#include <cassert>

pmem_allocator::pmem_allocator(uint8_t* _heap_ptr, uint32_t _block_size, uint64_t _max_border, bool init_new)
        : heap_ptr(_heap_ptr),
          block_size(_block_size),
          freed_blocks(),
          allocation_border(0),
          max_border(_max_border),
          mutex()
{
    if (init_new)
    {
        /*
         * Marks first block of heap as allocated. First block can never be given to user or freed.
         */
        std::memcpy(heap_ptr + get_block_end(0), &HEAP_END_MARKER, 1);
    }
    else
    {
        /*
         * Traverse heap from beginning to end
         */
        uint64_t cur_block_num = 0;
        while (true)
        {
            uint8_t cur_marker;
            std::memcpy(&cur_marker, heap_ptr + get_block_end(cur_block_num), 1);
            assert(cur_marker == HEAP_END_MARKER
                   || cur_marker == ALLOCATED_BLOCK_MARKER || cur_marker == FREED_BLOCK_MARKER);
            if (cur_marker == HEAP_END_MARKER)
            {
                /*
                 * Reached heap end, all further blocks are not allocated.
                 */
                allocation_border = cur_block_num;
                return;
            }
            else if (cur_marker == FREED_BLOCK_MARKER)
            {
                /*
                 * Current block is situated before heap end and is not allocated,
                 * adding it to set.
                 */
                freed_blocks.insert(cur_block_num);
            }
            cur_block_num++;
        }
    }
}

uint64_t pmem_allocator::get_block_start(uint64_t block_num) const
{
    assert(block_num >= 0);
    return block_num * (block_size + 1);
}

uint64_t pmem_allocator::get_block_end(uint64_t block_num) const
{
    return get_block_start(block_num) + block_size;
}

uint64_t pmem_allocator::get_block_num(uint64_t block_offset) const
{
    assert(block_offset % (block_size + 1) == 0);
    return block_offset / (block_size + 1);
}

uint8_t* pmem_allocator::pmem_alloc()
{
    std::unique_lock lock(mutex);
    if (!freed_blocks.empty())
    {
        /*
         * Remove arbitrary freed block from freed blocks set
         */
        const uint64_t freed_block_num = *freed_blocks.begin();
        freed_blocks.erase(freed_block_num);

        /*
         * Mark block as allocated
         */
        uint64_t freed_block_end = get_block_end(freed_block_num);
        std::memcpy(heap_ptr + freed_block_end, &ALLOCATED_BLOCK_MARKER, 1);
        pmem_do_flush(heap_ptr + freed_block_end, 1);

        /*
         * Return pointer to block
         */
        return heap_ptr + get_block_start(freed_block_num);
    }
    if (allocation_border == max_border)
    {
        throw std::runtime_error("Cannot perform allocation: all blocks have already been allocated");
    }

    /*
     * Retrieving new allocation border
     */
    const uint64_t new_allocation_border = allocation_border + 1;
    /*
     * Last allocated byte of heap (i.e. last byte of the last allocated block)
     */
    const uint64_t new_heap_end = get_block_end(new_allocation_border);
    /*
     * Before current allocation, this byte was the last allocated byte.
     */
    const uint64_t old_heap_end = get_block_end(allocation_border);
    /*
     * Marking new heap end as allocated block.
     */
    std::memcpy(heap_ptr + new_heap_end, &HEAP_END_MARKER, 1);
    pmem_do_flush(heap_ptr + new_heap_end, 1);
    /*
     * Marking previous heap end as ordinary allocated block, i.e. moving heap end forward.
     */
    std::memcpy(heap_ptr + old_heap_end, &ALLOCATED_BLOCK_MARKER, 1);
    pmem_do_flush(heap_ptr + old_heap_end, 1);

    /*
     * Increasing allocation border
     */
    allocation_border = new_allocation_border;

    return heap_ptr + get_block_start(new_allocation_border);
}

void pmem_allocator::pmem_free(uint8_t* ptr)
{
    std::unique_lock lock(mutex);
    uint64_t block_num = get_block_num(ptr - heap_ptr);
    assert(block_num > 0 && block_num <= allocation_border);

    if (block_num < allocation_border)
    {
        /*
         * Mark block as freed
         */
        freed_blocks.insert(block_num);
        uint64_t block_end = get_block_end(block_num);
        std::memcpy(heap_ptr + block_end, &FREED_BLOCK_MARKER, 1);
        pmem_do_flush(heap_ptr + block_end, 1);
        return;
    }

    /*
     * Freed block is last allocated block. Find previous allocate block.
     * Traversing from heap end to beginning.
     */
    uint64_t previous_allocated_block = block_num - 1;
    while (true)
    {
        uint8_t cur_marker;
        std::memcpy(&cur_marker, heap_ptr + get_block_end(previous_allocated_block), 1);
        assert(cur_marker == ALLOCATED_BLOCK_MARKER || cur_marker == FREED_BLOCK_MARKER);
        if (cur_marker == FREED_BLOCK_MARKER)
        {
            /*
             * Current block is freed. After moving allocation border to the left, ths block will be situated
             * after allocation border. Therefore, it should be removed from freed block set.
             */
            assert(freed_blocks.count(previous_allocated_block) == 1);
            freed_blocks.erase(previous_allocated_block);
            previous_allocated_block--;
        }
        else
        {
            /*
             * Current block is allocated and has ALLOCATED_BLOCK_MARKER. After moving allocation border to the left,
             * this block will become allocation border.
             */
            break;
        }
    }
    std::memcpy(heap_ptr + get_block_end(previous_allocated_block), &HEAP_END_MARKER, 1);
    pmem_do_flush(heap_ptr + get_block_end(previous_allocated_block), 1);
    allocation_border = previous_allocated_block;
}

bool pmem_allocator::is_allocated(uint8_t* ptr)
{
    std::unique_lock lock(mutex);
    uint64_t block_num = get_block_num(ptr - heap_ptr);
    if (block_num > allocation_border)
    {
        return false;
    }
    uint8_t cur_marker;
    std::memcpy(&cur_marker, heap_ptr + get_block_end(block_num), 1);
    assert(cur_marker == FREED_BLOCK_MARKER || cur_marker == HEAP_END_MARKER || cur_marker == ALLOCATED_BLOCK_MARKER);
    return cur_marker != FREED_BLOCK_MARKER;
}
