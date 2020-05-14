#include "ram_stack.h"

uint64_t ram_stack::get_stack_end() const
{
    if (frames.empty())
    {
        /*
         * First frame of the stack has zero offset
         */
        return 0;
    }
    const positioned_frame& last_frame = frames.back();
    /*
     * Offset of first byte of last frame + size of last frame = offset of first free byte.
     */
    return last_frame.get_position() + last_frame.get_frame().size();
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
    /*
     * Function should write it's answer to the beginning of previous
     * stack frame. First 8 bytes of previous stack frame is a place, where
     * function, that is currently being executed, can store it's answer.
     */
    return frames[frames.size() - 2].get_position();
}

void ram_stack::remove_frame()
{
    frames.pop_back();
}

positioned_frame const& ram_stack::get_last_frame() const
{
    return frames.back();
}
