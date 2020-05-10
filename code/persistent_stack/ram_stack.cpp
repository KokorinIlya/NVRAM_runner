//
// Created by ilya on 10.05.2020.
//

#include "ram_stack.h"

uint64_t ram_stack::get_stack_end() const
{
    if (frames.empty())
    {
        return 0;
    }
    const positioned_frame& last_frame = frames.back();
    return last_frame.position + get_frame_size(last_frame.frame);
}

uint32_t ram_stack::size() const
{
    return frames.size();
}

void ram_stack::add_frame(const positioned_frame& frame)
{
    frames.push_back(frame);
}

uint64_t ram_stack::get_answer_position() const
{
    return frames[frames.size() - 2].position;
}

void ram_stack::remove_frame()
{
    frames.pop_back();
}

positioned_frame const& ram_stack::get_last_frame() const
{
    return frames.back();
}
