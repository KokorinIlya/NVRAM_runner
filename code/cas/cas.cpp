#include "cas.h"
#include <limits>
#include <cstring>
#include "../common/pmem_utils.h"

void write_answer(uint8_t* answer, uint8_t result)
{
    std::memcpy(answer, &result, 1);
    pmem_do_flush(answer, 1);
}

void cas_internal(uint64_t* var,
                  uint32_t expected_value,
                  uint32_t new_value,
                  uint32_t cur_thread_number,
                  uint32_t total_thread_number,
                  uint32_t* thread_matrix,
                  uint8_t* answer)
{
    uint64_t last_thread_number_and_cur_value = __atomic_load_n(var, __ATOMIC_SEQ_CST);
    const uint8_t* const last_thread_number_and_cur_value_ptr = (const uint8_t*) &last_thread_number_and_cur_value;
    uint32_t last_thread_number;
    std::memcpy(&last_thread_number, last_thread_number_and_cur_value_ptr, 4);
    uint32_t cur_value;
    std::memcpy(&cur_value, last_thread_number_and_cur_value_ptr + 4, 4);
    if (cur_value != expected_value)
    {
        write_answer(answer, 0x0);
        return;
    }
    if (last_thread_number != std::numeric_limits<uint32_t>::max())
    {
        uint32_t index = last_thread_number * total_thread_number + cur_thread_number;
        __atomic_store_n(thread_matrix + index, cur_value, __ATOMIC_SEQ_CST);
        pmem_do_flush(thread_matrix + index, 4);
    }
    uint64_t cur_thread_number_and_new_value;
    uint8_t* const cur_thread_number_and_new_value_ptr = (uint8_t*) &cur_thread_number_and_new_value;
    std::memcpy(cur_thread_number_and_new_value_ptr, &cur_thread_number, 4);
    std::memcpy(cur_thread_number_and_new_value_ptr + 4, &new_value, 4);
    bool result = __atomic_compare_exchange_n(
            var,
            &last_thread_number_and_cur_value,
            cur_thread_number_and_new_value,
            false,
            __ATOMIC_SEQ_CST,
            __ATOMIC_SEQ_CST
    );
    if (result)
    {
        pmem_do_flush(var, 4);
        write_answer(answer, 0x1);
    }
    else
    {
        write_answer(answer, 0x0);
    }
}

void cas_recover_internal(uint64_t* var,
                          uint32_t expected_value,
                          uint32_t new_value,
                          uint32_t cur_thread_number,
                          uint32_t total_thread_number,
                          uint32_t* thread_matrix,
                          uint8_t* answer)
{
    uint64_t last_thread_number_and_cur_value = __atomic_load_n(var, __ATOMIC_SEQ_CST);
    const uint8_t* const last_thread_number_and_cur_value_ptr = (const uint8_t*) &last_thread_number_and_cur_value;
    uint32_t last_thread_number;
    std::memcpy(&last_thread_number, last_thread_number_and_cur_value_ptr, 4);
    uint32_t cur_value;
    std::memcpy(&cur_value, last_thread_number_and_cur_value_ptr + 4, 4);

    if (last_thread_number == cur_thread_number && cur_value == new_value)
    {
        write_answer(answer, 0x1);
        return;
    }
    for (uint32_t other_thread_number = 0; other_thread_number < total_thread_number; other_thread_number++)
    {
        uint32_t index = cur_thread_number * total_thread_number + other_thread_number;
        uint32_t other_thread_value = __atomic_load_n(thread_matrix + index, __ATOMIC_SEQ_CST);
        if (other_thread_value == new_value)
        {
            write_answer(answer, 0x1);
            return;
        }
    }
    cas_internal(var, expected_value, new_value, cur_thread_number, total_thread_number, thread_matrix, answer);
}

void cas(const uint8_t* args)
{

}

void cas_recover(const uint8_t* args)
{

}
