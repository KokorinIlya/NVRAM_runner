#ifndef DIPLOM_FRAMES_H
#define DIPLOM_FRAMES_H

#include <stack>
#include <string>
#include <vector>

/**
 * Single frame of the stack.
 */
struct stack_frame
{
    /**
     * Name of the function, that was called.
     */
    std::string function_name;
    /**
     * Args of the function, marshalled to array of bytes.
     */
    std::vector<uint8_t> args;
};

/**
 * Single frame of the stack with position in the stack.
 */
struct positioned_frame
{
    /**
     * Stack frame.
     */
    stack_frame frame;
    /**
     * Offset of beginning of the frame, i.e.
     * first byte of the frame. Offset is calculated from
     * the beginning of memory-mapping of the stack.
     */
    uint64_t position = 0;
};

/**
 * Calculates size of frame in bytes.
 * @param frame - frame, size of which will be calculated.
 * @return size of frame in bytes.
 */
uint64_t get_frame_size(const stack_frame &frame);

#endif //DIPLOM_FRAMES_H
