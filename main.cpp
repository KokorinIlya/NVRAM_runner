#include <iostream>
#include "code/persistent_stack/call.h"
#include "code/debug_utils/stack_check.h"
#include "code/debug_utils/queue_check.h"

int main()
{
    persistent_stack stack_holder("/home/ilyakoko/IFMO-diploma/stack-1");
    std::cout << "Stack address: " << (long long) stack_holder.get_stack_ptr() << std::endl;
    ram_stack stack;
    check_stack_write(stack, stack_holder);
    check_stack_read(stack_holder);
    remove_frame(stack, stack_holder);
    std::cout << "After removal:" << std::endl;
    check_stack_read(stack_holder);
    std::cout << "Queue check:" << std::endl;
    check_queue();
    return 0;
}