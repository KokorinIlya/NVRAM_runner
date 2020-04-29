//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_STACK_CHECK_H
#define DIPLOM_STACK_CHECK_H

#include "../persistent_stack/persistent_stack.h"
#include "../persistent_stack/ram_stack.h"

void check_stack_write(ram_stack& stack, persistent_stack& holder);
void check_stack_read(const persistent_stack& holder);

#endif //DIPLOM_STACK_CHECK_H
