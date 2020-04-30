//
// Created by ilya on 30.04.2020.
//

#ifndef DIPLOM_THREAD_LOCAL_STACK_STORAGE_H
#define DIPLOM_THREAD_LOCAL_STACK_STORAGE_H

#include <optional>
#include "../persistent_stack/ram_stack.h"

struct thread_local_stack_storage
{
    static thread_local std::optional<ram_stack> stack_opt;

    static void set_stack(const ram_stack& stack);

    static ram_stack& get_stack();
};

#endif //DIPLOM_THREAD_LOCAL_STACK_STORAGE_H
