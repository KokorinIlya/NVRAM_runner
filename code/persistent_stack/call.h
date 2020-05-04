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
#include <vector>

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
 * Removes single frame from the top of the stack. Frame is removed from both
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

/**
 * Write answer of the function to persistent memory. Answer can be of any size between 1 and 8 bytes.
 * Answer is flushed to persistent memory from cache before the function returns.
 * Answer is written to the previous stack frame (8 bytes just before end marker).
 * If there is no previous frame (i.e. frame, corresponding to the function, that is now being executed,
 * is the only frame in the stack) answer couldn't be written and function throws std::runtime_error.
 * Also, if the answer is bigger than 8 bytes or less than 1 byte, std::runtime_error is also thrown.
 * Otherwise (if answer has been written successfully) function returns normally.
 * Note, that on non-NVRAM systems (for example, on systems, on which HDD is used as persistent storage)
 * write of more than 1 byte can be non-atomic operation and system can crash when only part of the answer
 * has been flushed to persistent memory, while the rest of the answer hasn't been flushed yet.
 * Function has to deal with this situation itself.
 * For example, in the beginning of the function we can write some default value to all of the bytes
 * of the answer, then in recovery mode read current function answer using read_current_answer
 * and then check, which of 3 situations happened:
 * <ul>
 *  <li>
 *      All bytes of answer contain default values - it means, that answer hasn't been flushed to
 *      persistent memory.
 *  </li>
 *  <li>
 *      No bytes of answer contain default values - it means, that answer has been fully flushed
 *      to persistent memory.
 *  </li>
 *  <li>
 *      Some of bytes of answer (but not all bytes) contain default value - it means, that
 *      only part of answer was flushed to persistent memory. Answer should be rewritten
 *      to persistent memory.
 *  </li>
 * </ul>
 * @param answer - answer to save in persistent memory.
 * @throws std::runtime_error - if answer size not between 1 and 8 inclusively or current frame is the
 *                              only frame in the stack.
 */
void write_answer(std::vector<uint8_t> answer);

// TODO: document
std::vector<uint8_t> read_current_answer(uint8_t size);

std::vector<uint8_t> read_answer(uint8_t size);

#endif //DIPLOM_CALL_H
