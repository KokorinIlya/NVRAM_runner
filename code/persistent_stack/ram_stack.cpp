//
// Created by ilya on 24.04.2020.
//

#include "ram_stack.h"

uint64_t get_frame_size(const stack_frame& frame)
{
    return frame.function_name.size() + frame.args.size() + 17;
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