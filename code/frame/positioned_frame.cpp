#include "positioned_frame.h"

#include <utility>

positioned_frame::positioned_frame(stack_frame _frame, uint64_t _position) :
        frame(std::move(_frame)), position(_position)
{}

const stack_frame& positioned_frame::get_frame() const
{
    return frame;
}

uint64_t positioned_frame::get_position() const
{
    return position;
}

positioned_frame::positioned_frame(positioned_frame&& other) noexcept:
        frame(std::move(other.frame)), position(other.position)
{}

positioned_frame::positioned_frame(const positioned_frame& other) :
        frame(other.frame), position(other.position)
{}
