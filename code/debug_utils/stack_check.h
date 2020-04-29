//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_STACK_CHECK_H
#define DIPLOM_STACK_CHECK_H

#include "../persistent_stack/stack_holder.h"
#include "../persistent_stack/persistent_stack.h"

void check_stack_write(persistent_stack& stack, stack_holder& holder);
void check_stack_read(const stack_holder& holder);

#endif //DIPLOM_STACK_CHECK_H
