#ifndef DIPLOM_PMEM_ALLOCATOR_H
#define DIPLOM_PMEM_ALLOCATOR_H

#include <cstdint>
#include <unordered_set>
#include "../common/pmem_utils.h"
#include <mutex>

/**
 * Allocator, that allocates and frees blocks of fixed size in persistent memory heap.
 * Allocator doesn't own pointer to persistent memory heap. Also, it doesn't own file,
 * with which NVRAM is emulated.
 */
struct pmem_allocator
{
public:
    /**
     * Initializes allocator. If init_new is true, initializes new allocator from the ground up,
     * otherwise, reads allocation information and restores state of the allocator before the crash
     * (or end of the work).
     * @param _heap_ptr - pointer to the beginning of the heap.
     * @param _block_size - size of blocks to allocate (in bytes).
     * @param _max_border - maximal possible value of allocation border.
     * @param init_new - if true, initialized allocator from the ground up, otherwise restores allocator
     *                   state.
     */
    pmem_allocator(uint8_t* _heap_ptr, uint32_t _block_size, uint64_t _max_border, bool init_new);

    /**
     * Allocates single block and returns address to first byte of the block. If block cannot be allocated
     * (because allocation border is equal to it's maximal value and there are no free blocks) std::runtime_error
     * is thrown.
     * @return pointer to first byte of the block.
     * @throws std::runtime_error if block cannot be allocated.
     */
    uint8_t* pmem_alloc();

    /**
     * Frees single block.
     * @param ptr - pointer to the first byte of the block, that should be freed.
     */
    void pmem_free(uint8_t* ptr);

    /**
     * Returns true, if ptr is pointer to the beginning of block, that was allocated and hasn't been freed yet,
     * false otherwise. Parameter must be a valid pointer to beginning of some block (possibly not allocated),
     * otherwise behaviour of function is undefined.
     * @param ptr - pointer to the beginning of some block.
     * @return true, if ptr is pointer to the beginning of allocated block, false otherwise.
     */
    bool is_allocated(uint8_t* ptr);
private:
    /**
     * Retrieves offset of beginning of block. Offset is calculated from the beginning of
     * of mapping of persistent memory to the virtual memory. Block numbers start with 0.
     * @param block_num - number of block.
     * @return Offset of first byte of the block.
     */
    uint64_t get_block_start(uint64_t block_num) const;

    /**
     * Retrieves offset of end of the block. Block end is the last byte of the block, where
     * allocation information is held (either ALLOCATED_BLOCK_MARKER, HEAP_END_MARKER or FREED_BLOCK_MARKER).
     * Offset is calculated from the beginning ofof mapping of persistent memory to the virtual memory.
     * Block numbers start with 0.
     * @param block_num - number of block.
     * @return Offset of end byte of the block.
     */
    uint64_t get_block_end(uint64_t block_num) const;

    /**
     * Retrieves block number by offset of the first byte of the block.
     * @param block_offset - offset of the first bye of the block.
     * @return number of the block.
     */
    uint64_t get_block_num(uint64_t block_offset) const;

    /**
     * Block is allocated. There exists some other allocated blocks with bigger offsets
     * (and therefore bigger block numbers).
     */
    static const uint8_t ALLOCATED_BLOCK_MARKER = 0x0;
    /**
     * This block is the last allocated block, there doesn't exist some other allocated blocks with
     * bigger offsets (and therefore bigger block numbers)
     */
    static const uint8_t HEAP_END_MARKER = 0x1;

    /**
     * This block has been freed. There exists some allocated blocks with bigger offsets
     * (and therefore bigger block numbers).
     */
    static const uint8_t FREED_BLOCK_MARKER = 0x2;

    /**
     * Pointer to the beginning of heap
     */
    uint8_t* const heap_ptr;

    /**
     * Size of the block
     */
    const uint32_t block_size;

    /**
     * Blocks, that:
     * <ul>
     *  <li>
     *      Have been allocated
     *  </li>
     *  <li>
     *      Have been freed
     *  </li>
     *  <li>
     *      Have smaller offset (and therefore, smaller block numbers) than last allocated
     *      block of the heap (block with HEAP_END_MARKER)
     *  </li>
     * </ul>
     */
    std::unordered_set<uint64_t> freed_blocks;

    /**
     * Number of last allocate block (block with HEAP_END_MARKER)
     */
    uint64_t allocation_border;

    /**
     * Maximal possible value of allocation border
     */
    const uint64_t max_border;

    /**
     * Mutex, that prevents concurrent access to allocator and ensures linearizability of allocate and free
     * operations.
     */
    std::mutex mutex;
};

#endif //DIPLOM_PMEM_ALLOCATOR_H
