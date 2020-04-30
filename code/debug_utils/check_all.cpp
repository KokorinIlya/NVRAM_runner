//
// Created by ilya on 29.04.2020.
//

#include "check_all.h"
#include "../persistent_stack/persistent_stack.h"
#include <iostream>
#include "../persistent_stack/ram_stack.h"
#include "stack_check.h"
#include "queue_check.h"
#include "../persistent_stack/call.h"

void check_all()
{
    persistent_stack persistent_stack("/home/ilyakoko/IFMO-diploma/stack_opt-1", false);
    std::cout << "Stack globals: " << (long long) persistent_stack.get_stack_ptr() << std::endl;
    ram_stack stack;
    check_stack_write(stack, persistent_stack);
    check_stack_read(persistent_stack);
    remove_frame(stack, persistent_stack);
    std::cout << "After removal:" << std::endl;
    check_stack_read(persistent_stack);
    std::cout << "Queue check:" << std::endl;
    check_queue();
}