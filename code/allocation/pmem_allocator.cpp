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
        std::memcpy(heap_ptr + get_block_end(0), &HEAP_END_MARKER, 1);
    }
    else
    {
        uint64_t cur_block_num = 0;
        while (true)
        {
            uint32_t cur_marker;
            std::memcpy(&cur_marker, heap_ptr + get_block_end(cur_block_num), 1);
            if (cur_marker == HEAP_END_MARKER)
            {
                allocation_border = cur_block_num;
                return;
            }
            else if (cur_marker == FREED_BLOCK_MARKER)
            {
                freed_blocks.insert(cur_block_num);
            }
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
}
