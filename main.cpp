#include <iostream>
#include "code/persistent_stack/call.h"
#include "code/debug_utils/stack_check.h"
#include "code/debug_utils/queue_check.h"

int main()
{
    persistent_stack persistent_stack("/home/ilyakoko/IFMO-diploma/stack-1");
    std::cout << "Stack address: " << (long long) persistent_stack.get_stack_ptr() << std::endl;
    ram_stack stack;
    check_stack_write(stack, persistent_stack);
    check_stack_read(persistent_stack);
    remove_frame(stack, persistent_stack);
    std::cout << "After removal:" << std::endl;
    check_stack_read(persistent_stack);
    std::cout << "Queue check:" << std::endl;
    check_queue();
    return 0;
}