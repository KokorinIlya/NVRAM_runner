#ifndef DIPLOM_CALL_H
#define DIPLOM_CALL_H

#include <string>
#include <vector>
#include "../persistent_memory/persistent_memory_holder.h"
#include "../frame/positioned_frame.h"
#include "../storage/thread_local_non_owning_storage.h"
#include "../storage/thread_local_owning_storage.h"
#include <vector>
#include <optional>
#include "ram_stack.h"

/**
 * Reads stack from persistent memory to RAM. This function can be used
 * just after the crash, to read stack of functions, that
 * were being executed, when the crash occurred.
 * @param persistent_stack - instance of class, that owns file,
 *                           in which persistent stack is stored.
 * @return representation of persistent stack, that is stored in RAM.
 */
ram_stack read_stack(const persistent_memory_holder& persistent_stack);

/**
 * Adds new frame to the top of the stack. Frame is added to both
 * persistent and RAM stack. Can write new_ans_filler to the beginning of new frame.
 * If new_ans_filler size is not between 1 and 8 bytes inclusively, std::runtime_error
 * will be thrown. This parameter can be used to write some default value
 * (that cannot be return value of the function) to a place, where
 * answer will be stored.
 * @param stack - stack, that is stored in RAM. Should be representation
 *                (i.e. contain the same data) of persistent stack.
 * @param frame - frame to add to the top of the stack.
 * @param persistent_stack - stack, that is stored in file.
 * @param new_ans_filler - if option contains value, it's value will be written to the beginning
 *                         of new stack frame. Otherwise, won't be used.
 * @throws std::runtime_error - if new_ans_filler size is not between 1 and 8 bytes inclusively.
 */
void add_new_frame(
        ram_stack& stack,
        stack_frame const& frame,
        persistent_memory_holder& persistent_stack,
        std::optional<std::vector<uint8_t>> const& new_ans_filler = std::optional<std::vector<uint8_t>>()
);

/**
 * Removes single frame from the top of the stack. Frame is removed from both
 * persistent and RAM stack. Note, that since removing stack frame from persistent stack
 * is just writing stack end marker to the penultimate stack frame, first frame of the stack
 * CANNOT be removed. std::runtime_error will be thrown, if the frame, that should be removed,
 * is the only frame in the stack. Because of this, main function of each thread should
 * NEVER return a value. Main function of each thread should wait in an infinite loop and
 * finish it's execution only by exception or system crash. But, since according to the
 * system architecture, each worker thread should take and execute tasks from
 * tasks queue in an infinite loop, this limitation shouldn't be considered a drawback.
 * @param stack - stack, that is stored in RAM. Should be representation
 *                (i.e. contain the same data) of persistent stack.
 * @param persistent_stack - stack, that is stored in file.
 * @throws std::runtime_error if the frame, that should be removed, is the only frame
 *                            in the stack.
 */
void remove_frame(ram_stack& stack, persistent_memory_holder& persistent_stack);

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
 * calls recovery version. Note, that recover version of the function can be called
 * only when the system is running in recovery mode (mode, in which the system is running,
 * is determined using global_storage<system_mode>). If system mode is not RECOVERY and
 * call_recover is true, std::runtime_error will be thrown.
 * Can write ans_filler to a memory, where function, that is being called, will write it's answer.
 * This memory is located in the last stack frame. If ans_filler size is not between
 * 1 and 8 bytes inclusively, std::runtime_error will be thrown. This parameter can be used to
 * write some default value (that cannot be return value of the function) to a place, where
 * return value of the function, that is being called, will be written.
 * Can write new_ans_filler to the beginning of new frame. If new_ans_filler size is not between 1 and 8
 * bytes inclusively, std::runtime_error will be thrown. This can be used to fill answer memory of new frame
 * with some default value.
 * Since first frame of the stack cannot be removed, main function of each thread should
 * NEVER return a value. Main function of each thread should wait in an infinite loop and
 * finish it's execution only by exception or system crash. But, since according to the
 * system architecture, each worker thread should take and execute tasks from
 * tasks queue in an infinite loop, this limitation shouldn't be considered a drawback.
 * @param function_name - name of the function to call. Must be a valid key of the map with
 *                        addresses of the function.
 * @param args - arguments of function to call with.
 * @param ans_filler - if option contains value, it's value will be written to an answer memory
 *                     of current stack frame. Otherwise, won't be used.
 * @param new_ans_filler - if option contains value, it's value will be written to answer memory of
 *                         new stack frame. Otherwise, won't be used.
 * @param call_recover - if true, recover version of function will be called. Otherwise, ordinary
 *                       version will be called.
 * @throws std::runtime_error - if ans_filler size is not between 1 and 8 bytes inclusively or
 *                              if new_ans_filler size is not between 1 and 8 bytes inclusively or
 *                              if call_recover is true and system is not running in recovery mode.
 */
void do_call(std::string const& function_name,
             std::vector<uint8_t> const& args,
             std::optional<std::vector<uint8_t>> const& ans_filler = std::optional<std::vector<uint8_t>>(),
             std::optional<std::vector<uint8_t>> const& new_ans_filler = std::optional<std::vector<uint8_t>>(),
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
 * write of more than 1 byte can be non-atomic operation and system can crash when only part of
 * the answer has been flushed to persistent memory, while the rest of the answer hasn't been
 * flushed yet. Function has to deal with this situation itself.
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
void write_answer(std::vector<uint8_t> const& answer);

/**
 * Reads size bytes of answer, that was written by function, that is currently being executed.
 * Can be used to discover, if crash event occurred before or after all the answer was written to
 * persistent memory (method of discovering is described above). Since function writes answer to
 * the previous stack frame, read_current_answer reads answer from previous stack frame.
 * Therefore, if stack frame, corresponding to the function, that is currently being executed,
 * is the only frame in the stack, std::runtime_error will be thrown (note, that in such case answer
 * also cannot be written). Also, std::runtime_error is thrown if size is not between
 * 1 and 8 inclusively.
 * @param size - size of answer to retrieve in bytes.
 * @return size bytes of answer.
 * @throws std::runtime_error - if answer size not between 1 and 8 inclusively or current frame
 *                              is the only frame in the stack.
 */
std::vector<uint8_t> read_current_answer(uint8_t size);

/**
 * Reads answer from current stack frame. Since function writes it's answer to the previous stack frame,
 * read_answer retrieves return value of a function, that has just returned.
 * For example, consider the following code:
 *
 * def f():
 *      g()
 *      read_answer(8) - reads answer, that was written by function g
 *      h()
 *      read_answer(8) - reads answer, that was written by function h
 *
 * This function can read from 1 up to 8 bytes. If previously returned function has written
 * N bytes of answer, then only N bytes can be retrieved correctly, N+1-th byte can contain
 * any uninitialized data.
 * @param size - size of answer to retrieve in bytes.
 * @return size bytes of answer.
 * @throws std::runtime_error if size not between 1 and 8 inclusively.
 */
std::vector<uint8_t> read_answer(uint8_t size);

#endif //DIPLOM_CALL_H
