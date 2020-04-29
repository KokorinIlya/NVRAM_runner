//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_CALL_H
#define DIPLOM_CALL_H

#include <string>
#include <vector>
#include "stack_holder.h"
#include "ram_stack.h"

ram_stack read_stack(const stack_holder& stack_holder);

void add_new_frame(ram_stack& stack, stack_frame const& frame, stack_holder& stack_holder);

void remove_frame(ram_stack& stack, stack_holder& stack_holder);

#endif //DIPLOM_CALL_H
