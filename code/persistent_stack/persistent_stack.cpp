#include "persistent_stack.h"
#include <cstring>
#include <utility>
#include "../common/pmem_utils.h"
#include "../storage/global_storage.h"
#include "../model/function_address_holder.h"
#include "../model/system_mode.h"
#include <cassert>

/**
 * Reads single frame from persistent memory.
 * @param stack_ptr - pointer to the beginning of mapping of persistent memory to the virtual memory.
 * @param frame_offset - offset of the frame, that should be read. Offset is calculated from the beginning of
 *        of mapping of persistent memory to the virtual memory. Therefore, address of beginning
 *        of current stack frame is stack_ptr + frame_offset.
 * @return pair. First element of pair is shack frame, that has just been read. Second element of pair is true,
 *         if current frame is the last frame in the stack (i.e. it is terminated with stack end marker),
 *         false otherwise (i.e. it is terminated with frame end marker).
 */
std::pair<stack_frame, bool> read_frame(const uint8_t* const stack_ptr, const uint64_t frame_offset)
{
    /*
    * Skip 8 bytes of answer
    */
    uint64_t cur_offset = frame_offset + 8;

    /*
     * Read 2 bytes of function name len
     */
    uint16_t function_name_len;
    std::memcpy(&function_name_len, stack_ptr + cur_offset, 2);
    cur_offset += 2;

    /*
     * Read function_name_len bytes of function name
     */
    std::string function_name(function_name_len, '#');
    std::memcpy(function_name.data(), stack_ptr + cur_offset, function_name_len);
    cur_offset += function_name_len;

    /*
     * Read 2 bytes of args len
     */
    uint16_t args_len;
    std::memcpy(&args_len, stack_ptr + cur_offset, 2);
    cur_offset += 2;

    /*
     * Read args_len bytes of args
     */
    std::vector<uint8_t> args(args_len);
    std::memcpy(args.data(), stack_ptr + cur_offset, args_len);
    cur_offset += args_len;

    /*
     * Read 1 byte of end marker
     */
    uint8_t end_marker;
    std::memcpy(&end_marker, stack_ptr + cur_offset, 1);
    const bool is_last = end_marker == STACK_END_MARKER;

    return std::make_pair(stack_frame(function_name, args), is_last);
}

ram_stack read_stack(const persistent_memory_holder& persistent_stack)
{
    ram_stack stack;
    uint64_t cur_offset = 0;
    const uint8_t* const stack_mem = persistent_stack.get_pmem_ptr();

    while (true)
    {
        const std::pair<stack_frame, bool> read_result = read_frame(stack_mem, cur_offset);
        const bool is_last = read_result.second;
        const positioned_frame pos_frame = positioned_frame(read_result.first, cur_offset);
        stack.add_frame(pos_frame);

        if (is_last)
        {
            return stack;
        }
        /*
         * Position of first byte of last frame + length of last frame = position of first free byte
         */
        cur_offset += pos_frame.get_frame().size();
        /*
         * Align beginning of next frame
         */
        cur_offset = get_cache_line_aligned_address(cur_offset);
    }
}

void add_new_frame(ram_stack& stack,
                   stack_frame const& frame,
                   persistent_memory_holder& persistent_stack,
                   std::optional<std::vector<uint8_t>> const& new_ans_filler)
{
    uint8_t* const stack_mem = persistent_stack.get_pmem_ptr();
    /*
     * First free byte of the stack
     */
    const uint64_t stack_end = stack.get_stack_end();
    /*
     * New frame beginning should be aligned
     */
    const uint64_t new_frame_offset = get_cache_line_aligned_address(stack_end);
    assert(new_frame_offset % CACHE_LINE_SIZE == 0);
    stack.add_frame(positioned_frame{frame, new_frame_offset});

    uint64_t cur_offset = new_frame_offset;

    if (new_ans_filler.has_value())
    {
        if (new_ans_filler->empty() || new_ans_filler->size() > 8)
        {
            throw std::runtime_error(
                    "Cannot write answer of size " +
                    std::to_string(new_ans_filler->size())
            );
        }
        /*
         * Write default answer
         */
        std::memcpy(
                persistent_stack.get_pmem_ptr() + cur_offset,
                new_ans_filler->data(),
                new_ans_filler->size()
        );
    }

    /*
     * Skip 8 bytes for answer
     */
    cur_offset += 8;

    /*
     * Write 2 bytes of function name len
     */
    const uint16_t function_name_len = frame.get_function_name().size();
    std::memcpy(stack_mem + cur_offset, &function_name_len, 2);
    cur_offset += 2;

    /*
     * Write function_name_len bytes of function name
     */
    std::memcpy(stack_mem + cur_offset, frame.get_function_name().data(), function_name_len);
    cur_offset += function_name_len;

    /*
     * Write 2 bytes of args len
     */
    const uint16_t args_len = frame.get_args().size();
    std::memcpy(stack_mem + cur_offset, &args_len, 2);
    cur_offset += 2;

    /*
     * Write args_len bytes of args len
     */
    std::memcpy(stack_mem + cur_offset, frame.get_args().data(), args_len);
    cur_offset += args_len;

    /*
     * Write 1 byte of stack end marker
     */
    std::memcpy(stack_mem + cur_offset, &STACK_END_MARKER, 1);

    /*
     * All new frame is flushed, even if new_ans_filler is empty.
     * Beginning of the frame is aligned by cache line size
     * (i.e. beginning of the frame + 8 is not aligned by cache line size).
     * Therefore, it is useless to start flushing from beginning of the frame + 8, since
     * flushing is done by cache lines and first 8 bytes of the frame will be
     * flushed in all cases.
     */
    pmem_do_flush(stack_mem + new_frame_offset, frame.size());

    if (stack_end != 0)
    {
        /*
         * Stack end marker is just before first free byte of the stack
         */
        std::memcpy(stack_mem + stack_end - 1, &FRAME_END_MARKER, 1);
        pmem_do_flush(stack_mem + stack_end - 1, 1);
    }
}

void remove_frame(ram_stack& stack, persistent_memory_holder& persistent_stack)
{
    if (stack.size() == 1)
    {
        throw std::runtime_error("Cannot remove first frame of the stack");
    }

    stack.remove_frame();

    uint8_t* const stack_mem = persistent_stack.get_pmem_ptr();
    /*
     * Stack end marker is just before first free byte of the stack
     */
    const uint64_t end_marker_offset = stack.get_stack_end() - 1;
    std::memcpy(stack_mem + end_marker_offset, &STACK_END_MARKER, 1);
    pmem_do_flush(stack_mem + end_marker_offset, 1);
}

