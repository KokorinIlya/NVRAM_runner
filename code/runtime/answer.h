#ifndef DIPLOM_ANSWER_H
#define DIPLOM_ANSWER_H

#include <string>
#include <vector>
#include <optional>
#include "../persistent_memory/persistent_memory_holder.h"
#include "../frame/positioned_frame.h"
#include "../storage/thread_local_non_owning_storage.h"
#include "../storage/thread_local_owning_storage.h"
#include "../persistent_stack/ram_stack.h"

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


#endif //DIPLOM_ANSWER_H
