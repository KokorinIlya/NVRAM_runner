#ifndef DIPLOM_PERSISTENT_STACK_H
#define DIPLOM_PERSISTENT_STACK_H

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

#endif //DIPLOM_PERSISTENT_STACK_H
