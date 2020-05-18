#ifndef DIPLOM_CAS_H
#define DIPLOM_CAS_H

#include <cstdint>

/**
 * Performs CAS on RMW register var.
 * Thread_matrix consists of N * N SRSW registers (where N is a number of worker threads),
 * used to notify each of the worker threads, that it's CAS was successful. Matrix should be
 * located in memory row by row - from 0-th row to N-1-th row. There shouldn't be any empty
 * space in the matrix.
 * Note, that each of the SRSW registers in thread matrix should't cross cache line, i.e.
 * if single register is located in range [addr .. addr + 3], all range [addr .. addr + 3] should
 * belong to a single cache line.
 * @param var - pointer to the RMW register.
 * @param expected_value - if RMW register has value = expected_value, it's value will be changed to new_value.
 * @param new_value - if RMW register has value = expected_value, it's value will be changed to new_value.
 * @param cur_thread_number - id of te thread, that is executing CAS. must be from 0 to N - 1 inclusively.
 * @param total_thread_number - total number of worker threads in the system (N).
 * @param thread_matrix - pointer to the beginning of thread matrix.
 * @return true, if CAS was successful (RMW register had value = expected_value), false otherwise.
 */
bool cas_internal(uint64_t* var,
                  uint32_t expected_value,
                  uint32_t new_value,
                  uint32_t cur_thread_number,
                  uint32_t total_thread_number,
                  uint32_t* thread_matrix);

/**
 * Recover version of cas_internal.
 * @param var - pointer to the RMW register.
 * @param expected_value - if RMW register has value = expected_value, it's value will be changed to new_value.
 * @param new_value - if RMW register has value = expected_value, it's value will be changed to new_value.
 * @param cur_thread_number - id of te thread, that is executing CAS. must be from 0 to N - 1 inclusively.
 * @param total_thread_number - total number of worker threads in the system (N).
 * @param thread_matrix - pointer to the beginning of thread matrix.
 * @return true, if CAS was successful (RMW register had value = expected_value), false otherwise.
 */
bool cas_recover_internal(uint64_t* var,
                          uint32_t expected_value,
                          uint32_t new_value,
                          uint32_t cur_thread_number,
                          uint32_t total_thread_number,
                          uint32_t* thread_matrix);

/**
 * CAS that can be called by the system runtime using do_call.
 * Receives arguments from byte array.
 * Args has the following structure:
 * <ul>
 *  <li>
 *      8 bytes of variable address offset (offset is calculated from the beginning of memory mapping
 *      of NVRAM to the virtual memory)
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
void cas(const uint8_t* args);

/**
 * Recover version of cas, that can be called by system runtime, using do_call.
 * This function receives the same arguments, as cas, in the same order.
 * @param args - arguments of function, marshalled to byte array.
 */
void cas_recover(const uint8_t* args);

#endif //DIPLOM_CAS_H
