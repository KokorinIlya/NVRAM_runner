#include "ram_stack.h"
#include "../common/pmem_utils.h"

uint64_t get_frame_size(const positioned_frame& frame)
{
    /*
     * 8 bytes of function name size
     * function name
     * 8 bytes of arguments size
     * arguments
     * to get address of possible beginning of answer
     */
    const uint64_t base_answer_offset = frame.position +
                                        frame.frame.args.size() +
                                        frame.frame.function_name.size() +
                                        16;
    /*
     * Align beginning of answer by CACHE_LINE_SIZE
     */
    const uint64_t answer_offset = get_cache_line_aligned_address(base_answer_offset);
    /*
     * 8 bytes of answer
     * 1 byte of stack frame
     * Frame size is difference between two offsets (offset of first byte after frame
     * minus offset of first byte of frame)
     */
    return answer_offset + 9 - frame.position;
}

uint64_t get_stack_end(const ram_stack& stack)
{
    if (stack.empty())
    {
        return 0;
    }
    const positioned_frame& last_frame = stack.top();
    return last_frame.position + get_frame_size(last_frame);
}