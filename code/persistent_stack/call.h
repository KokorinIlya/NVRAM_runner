//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_CALL_H
#define DIPLOM_CALL_H

#include <string>
#include <vector>
#include "persistent_stack.h"
#include "ram_stack.h"

ram_stack read_stack(const persistent_stack& persistent_stack);

void add_new_frame(ram_stack& stack, stack_frame const& frame, persistent_stack& persistent_stack);

void remove_frame(ram_stack& stack, persistent_stack& persistent_stack);

#endif //DIPLOM_CALL_H
