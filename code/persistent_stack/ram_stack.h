#ifndef DIPLOM_RAM_STACK_H
#define DIPLOM_RAM_STACK_H

#include <vector>
#include "frames.h"

/**
 * Representation of persistent stack, that is stored
 * in RAM. Can ne used for faster retrieval of stack frames
 * (without reading or writing persistent memory).
 * Should be representation (i.e. contain the same data)
 * of the persistent stack. Therefore, should be updated
 * synchronously to the persistent stack.
 * Can be retrieved from persistent stack using read_stack.
 */
struct ram_stack
{
public:
    /**
     * Returns address of stack end, i.e. offset of first free byte.
     * Offset is calculated from the beginning of memory-mapping of the stack.
     * @return address of stack end
     */
    [[nodiscard]] uint64_t get_stack_end() const;

    /**
     * Returns size of the stack, i.e. count of stack frames in the stack.
     * @return count of frames in the stack.
     */
    [[nodiscard]] uint32_t size() const;

    /**
     * Adds new frame to the top of the stack.
     * @param frame - new frame to add to the top of the stack.
     */
    void add_frame(const positioned_frame& frame);

    /**
     * Removes single frame from the top of the stack.
     */
    void remove_frame();

    /**
     * Returns last frame of the stack (i.e. frame, corresponding to the function,
     * that was called last).
     * @return last frame of the stack.
     */
    [[nodiscard]] positioned_frame const& get_last_frame() const;

    /**
     * Retrieves position, where function, that is currently being executed, should
     * write it's answer. Since function, that was called first, cannot return answer,
     * result of calling this function, when there is only a single frame in the stack,
     * is undefined. Position is meant to be offset from the beginning of the stack.
     * @return position for writing the answer.
     */
    [[nodiscard]] uint64_t get_answer_position() const;
private:
    std::vector<positioned_frame> frames;
};

#endif //DIPLOM_RAM_STACK_H
