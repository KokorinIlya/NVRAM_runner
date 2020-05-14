#ifndef DIPLOM_POSITIONED_FRAME_H
#define DIPLOM_POSITIONED_FRAME_H

#include "stack_frame.h"

/**
 * Single frame of the stack with position in the stack.
 */
struct positioned_frame
{
private:
    /**
     * Stack frame.
     */
    stack_frame frame;

    /**
     * Offset of beginning of the frame, i.e.
     * offset of first byte of the frame. Offset is calculated from
     * the beginning of memory-mapping of the stack.
     */
    uint64_t position;

public:
    positioned_frame(stack_frame _frame, uint64_t _position);

    positioned_frame(positioned_frame&& other) noexcept;

    positioned_frame(positioned_frame const& other);

    [[nodiscard]] const stack_frame& get_frame() const;

    [[nodiscard]] uint64_t get_position() const;
};

#endif //DIPLOM_POSITIONED_FRAME_H
