//
// Created by ilya on 10.05.2020.
//

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
 */
struct ram_stack
{
public:
    /**
     * Returns address of stack end, i.e. offset from first free byte.
     * Offset is calculated from the beginning of memory-mapping of the stack.
     * @return address of stack end
     */
    uint64_t get_stack_end() const;

    uint32_t size() const;

    void add_frame(const positioned_frame& frame);

    void remove_frame();

    positioned_frame const& get_last_frame() const;

    uint64_t get_answer_position() const;
private:
    std::vector<positioned_frame> frames;
};

#endif //DIPLOM_RAM_STACK_H
