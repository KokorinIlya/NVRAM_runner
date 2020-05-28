#ifndef DIPLOM_CALL_H
#define DIPLOM_CALL_H

#include <vector>
#include <optional>

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


#endif //DIPLOM_CALL_H
