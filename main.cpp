#include <iostream>
#include "code/persistent_stack/call.h"
#include "code/debug_utils/stack_check.h"
#include "code/debug_utils/queue_check.h"
#include "code/address/threadlocal_test.h"
#include <thread>

int main()
{
    persistent_stack persistent_stack("/home/ilyakoko/IFMO-diploma/stack_opt-1");
    std::cout << "Stack address: " << (long long) persistent_stack.get_stack_ptr() << std::endl;
    ram_stack stack;
    check_stack_write(stack, persistent_stack);
    check_stack_read(persistent_stack);
    remove_frame(stack, persistent_stack);
    std::cout << "After removal:" << std::endl;
    check_stack_read(persistent_stack);
    std::cout << "Queue check:" << std::endl;
    check_queue();

    std::cout << "threadlocal test" << std::endl;
    std::thread t1(
            []()
            {
                test::x = 0;
                for (int i = 0; i < 10; ++i)
                {
                    ++test::x;
                    std::string msg = "Thread 1, x = " + std::to_string(test::x) + "\n";
                    std::cout << msg;
                }
            }
    );

    std::thread t2(
            []()
            {
                test::x = 0;
                for (int i = 0; i < 10; ++i)
                {
                    ++test::x;
                    std::string msg = "Thread 2, x = " + std::to_string(test::x) + "\n";
                    std::cout << msg;
                }
            }
    );

    t1.join();
    t2.join();


    return 0;
}