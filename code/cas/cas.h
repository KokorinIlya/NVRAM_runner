#ifndef DIPLOM_CAS_H
#define DIPLOM_CAS_H

#include <cstdint>

void cas_internal(uint64_t* var,
                  uint32_t expected_value,
                  uint32_t new_value,
                  uint32_t cur_thread_number,
                  uint32_t total_thread_number,
                  uint32_t* thread_matrix,
                  uint8_t* answer);

void cas_recover_internal(uint64_t* var,
                          uint32_t expected_value,
                          uint32_t new_value,
                          uint32_t cur_thread_number,
                          uint32_t total_thread_number,
                          uint32_t* thread_matrix,
                          uint8_t* answer);

void cas(const uint8_t* args);

void cas_recover(const uint8_t* args);

#endif //DIPLOM_CAS_H
