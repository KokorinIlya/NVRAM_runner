#include "tasks.h"

cas_task::cas_task(uint64_t _var_offset,
                   uint32_t _expected_value,
                   uint32_t _new_value,
                   uint64_t _answer_offset,
                   uint64_t _thread_matrix_offset) :
        var_offset(_var_offset),
        expected_value(_expected_value),
        new_value(_new_value),
        answer_offset(_answer_offset),
        thread_matrix_offset(_thread_matrix_offset)
{}

read_task::read_task(uint64_t _var_offset) : var_offset(_var_offset)
{}
