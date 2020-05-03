//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_CALL_H
#define DIPLOM_CALL_H

#include <string>
#include <vector>
#include "persistent_stack.h"
#include "ram_stack.h"

/**
 * Reads stack from persistent emory to RAM. This function can be used
 * just after the crash, to read stack of functions, that
 * were being executed, when the crash occurred.
 * @param persistent_stack - instance of class, that owns file,
 *                           in which persistent stack is stored.
 * @return representation of persistent stack, that is stored in RAM.
 */
ram_stack read_stack(const persistent_stack& persistent_stack);

/**
 * Adds new frame to the top of the stack. Frame is added to both
 * persistent and RAM stack.
 * @param stack - stack, that is stored in RAM. Should be representation
 * (i.e. contain the same data) as persistent stack.
 * @param frame - frame to add to the top of the stack.
 * @param persistent_stack - stack, that is stored in file.
 */
void add_new_frame(ram_stack& stack, stack_frame const& frame, persistent_stack& persistent_stack);

/**
 * Adds single frame from the top of the stack. Frame is removed from both
 * persistent and RAM stack.
 * @param stack - stack, that is stored in RAM. Should be representation
 * (i.e. contain the same data) as persistent stack.
 * @param persistent_stack - stack, that is stored in file.
 */
void remove_frame(ram_stack& stack, persistent_stack& persistent_stack);

void do_call(std::string const& function_name, std::vector<uint8_t> const& args);

// TODO: write & read answer from stack

#endif //DIPLOM_CALL_H
