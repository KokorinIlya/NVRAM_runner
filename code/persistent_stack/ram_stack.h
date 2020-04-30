//
// Created by ilya on 24.04.2020.
//

#ifndef DIPLOM_RAM_STACK_H
#define DIPLOM_RAM_STACK_H

#include <stack>
#include <string>
#include <vector>

struct stack_frame
{
    std::string function_name;
    std::vector<uint8_t> args;
};

struct positioned_frame
{
    stack_frame frame;
    uint64_t position = 0; // offset of begin of the frame
};

using ram_stack = std::stack<positioned_frame>;

uint64_t get_frame_size(const stack_frame &frame); // in bytes

uint64_t get_stack_end(const ram_stack &stack); // globals of first free byte

#endif //DIPLOM_RAM_STACK_H
