//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_CALL_H
#define DIPLOM_CALL_H

#include <string>
#include <vector>
#include "persistent_stack.h"
#include "ram_stack.h"
#include "../globals/thread_local_non_owning_storage.h"
#include "../globals/thread_local_owning_storage.h"

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

/**
 * Performs call of function with specified name and args. Performs sequence of actions:
 * <ul>
 *  <li>
 *      Puts new frame, corresponding to the function, being called, to the top of the stack.
 *  </li>
 *  <li>
 *      Calls function with specified name, using specified args as parameters of the call.
 *      Pointer to function is taken from the map with address of functions, therefore
 *      function, that is being called, must be registered in the map.
 *  </li>
 *  <li>
 *      Removes last frame from the top of the stack.
 *  </li>
 * </ul>
 * If call_recover is false (usually), calls ordinary version of the function. Otherwise,
 * calls recovery version.
 * @param function_name - name of the function to call. Must be a valid key of the map with
 *                        addresses of the function.
 * @param args - arguments of function to call with.
 * @param call_recover - specifies, which of the function should be called
 *                       (ordinary or recovery version).
 */
void do_call(std::string const& function_name,
             std::vector<uint8_t> const& args,
             bool call_recover = false);

// TODO: document
int write_answer(uint64_t answer);

uint64_t read_answer();

#endif //DIPLOM_CALL_H
