//
// Created by ilya on 24.04.2020.
//

#ifndef DIPLOM_PERSISTENT_STACK_H
#define DIPLOM_PERSISTENT_STACK_H

#include "../common/constants_and_types.h"
#include "stack_holder.h"
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

using persistent_stack = std::stack<positioned_frame>;

uint64_t get_frame_size(const stack_frame &frame); // in bytes

uint64_t get_stack_end(const persistent_stack &stack); // address of first free byte

persistent_stack read_stack(const stack_holder& stack_holder);

void add_new_frame(persistent_stack& stack, stack_frame const& frame, stack_holder& stack_holder);

void remove_frame(persistent_stack& stack, stack_holder& stack_holder);

#endif //DIPLOM_PERSISTENT_STACK_H
