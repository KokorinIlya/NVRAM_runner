//
// Created by ilya on 29.04.2020.
//

#include "stack_check.h"
#include <iostream>

void check_stack_write(persistent_stack& stack, stack_holder &holder)
{
    stack_frame frame_1 = stack_frame
            {
                    "some_function_name",
                    std::vector<uint8_t>({1, 3, 3, 7})
            };
    add_new_frame(stack, frame_1, holder);

    stack_frame frame_2 = stack_frame
            {
                    "another_function_name",
                    std::vector<uint8_t>({2, 5, 1, 7})
            };
    add_new_frame(stack, frame_2, holder);

    stack_frame frame_3 = stack_frame
            {
                    "one_more_function_name",
                    std::vector<uint8_t>({1, 3, 5, 7, 9})
            };
    add_new_frame(stack, frame_3, holder);
}

void check_stack_read(const stack_holder &holder)
{
    persistent_stack another_stack = read_stack(holder);
    std::cout << "Stack size = " << another_stack.size() << std::endl;
    while (!another_stack.empty())
    {
        positioned_frame cur_frame = another_stack.top();
        another_stack.pop();
        std::cout << "New frame: position = " << cur_frame.position << std::endl;
        std::cout << cur_frame.frame.function_name << std::endl;
        for (uint8_t const& cur_arg: cur_frame.frame.args)
        {
            std::cout << (int) cur_arg << " ";
        }
        std::cout << std::endl;
    }
}
