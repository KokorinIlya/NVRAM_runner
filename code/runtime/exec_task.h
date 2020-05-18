#ifndef DIPLOM_EXEC_TASK_H
#define DIPLOM_EXEC_TASK_H

#include <cstdint>

/**
 * Executes task of some type and writes it's result to NVRAM.
 * By now, only CAS is supported and can be executed, but in future, more type of tasks can be added.
 * Args has the following structure:
 * <ul>
 *  <li>
 *      1 byte, containing type of task, that should be executed. By now, only 0x0 (CAS) is supported
 *  </li>
 *  <li>
 *      8 bytes of result offset (i.e. offset of memory location, where answer of task should be written).
 *      Offset is calculated from the beginning of memory mapping of NVRAM to the virtual memory
 *  </li>
 * </ul>
 * If type of task is 0x0 (CAS), subsequent args has the following structure:
 * <ul>
 *  <li>
 *      8 bytes of variable address offset
 *  </li>
 *  <li>
 *      4 bytes of expected value
 *  </li>
 *  <li>
 *      4 bytes of new value
 *  </li>
 *  <li>
 *      8 bytes of thread matrix offset
 *  </li>
 * </ul>
 * @param args - arguments of function, marshalled to byte array.
 */
void exec_task(const uint8_t* args);

/**
 * Recover version of exec_task. This function receives the same arguments as exec_task, in the same order.
 * If task has already been finished (and written it's answer to stack), writes it's answer to the answer
 * location in the memory. Otherwise, executes the task and writes it's result to NVRAM.
 * @param args - arguments of function, marshalled to byte array.
 */
void exec_task_recover(const uint8_t* args);

#endif //DIPLOM_EXEC_TASK_H
