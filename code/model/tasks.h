#ifndef DIPLOM_TASKS_H
#define DIPLOM_TASKS_H

#include <cstdint>

struct cas_task
{
public:
    cas_task(uint64_t _var_offset,
             uint32_t _expected_value,
             uint32_t _new_value,
             uint64_t _answer_offset,
             uint64_t _thread_matrix_offset);

    const uint64_t var_offset;

    const uint32_t expected_value;

    const uint32_t new_value;

    const uint64_t answer_offset;

    const uint64_t thread_matrix_offset;

    static const uint8_t CAS_TYPE = 0x0;
};

#endif //DIPLOM_TASKS_H
