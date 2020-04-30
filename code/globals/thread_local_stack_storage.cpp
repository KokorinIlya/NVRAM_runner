//
// Created by ilya on 30.04.2020.
//

#include "thread_local_stack_storage.h"
#include <stdexcept>

thread_local std::optional<ram_stack> thread_local_stack_storage::stack_opt =
        std::optional<ram_stack>();

void thread_local_stack_storage::set_stack(const ram_stack& stack)
{
    if (stack_opt)
    {
        throw std::runtime_error("Not empty stack");
    }
    stack_opt = std::optional<ram_stack>(stack);
}

ram_stack& thread_local_stack_storage::get_stack()
{
    return *stack_opt;
}

