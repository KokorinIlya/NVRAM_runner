#ifndef DIPLOM_RAM_STACK_H
#define DIPLOM_RAM_STACK_H

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
 * Representation of persistent stack, that is stored
 * in RAM. Can ne used for faster retrieval of stack frames
 * (without reading or writing persistent memory).
 */
using ram_stack = std::stack<positioned_frame>;

/**
 * Calculates size of frame in bytes.
 * @param frame - frame, size of which will be calculated.
 * @return size of frame in bytes.
 */
uint64_t get_frame_size(const positioned_frame &frame);

/**
 * Returns address of stack end, i.e. offset from first free byte.
 * Offset is calculated from the beginning of memory-mapping of the stack.
 * @param stack
 * @return
 */
uint64_t get_stack_end(const ram_stack &stack);

#endif //DIPLOM_RAM_STACK_H
