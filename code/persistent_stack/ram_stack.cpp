#include "ram_stack.h"

uint64_t get_frame_size(const stack_frame& frame)
{
    /*
     * 8 bytes of function name size
     * function name
     * 8 bytes of arguments size
     * arguments
     * 8 bytes for answer
     * 1 byte for end marker
     */
    return frame.function_name.size() + frame.args.size() + 25;
}

uint64_t get_stack_end(const ram_stack& stack)
{
    if (stack.empty())
    {
        return 0;
    }
    const positioned_frame& last_frame = stack.top();
    return last_frame.position + get_frame_size(last_frame.frame);
}