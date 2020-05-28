#include <cassert>
#include "../model/function_address_holder.h"
#include "../storage/global_storage.h"
#include "../common/pmem_utils.h"
#include <cstring>
#include "answer.h"

void write_answer(std::vector<uint8_t> const& answer)
{
    if (answer.empty() || answer.size() > 8)
    {
        throw std::runtime_error("Cannot write answer of size " + std::to_string(answer.size()));
    }
    ram_stack const& r_stack = thread_local_owning_storage<ram_stack>::get_const_object();
    persistent_memory_holder* p_stack = thread_local_non_owning_storage<persistent_memory_holder>::ptr;
    if (r_stack.size() == 1)
    {
        throw std::runtime_error("Cannot return value from the first frame");
    }
    const uint64_t answer_offset = r_stack.get_answer_position();
    assert(answer_offset % CACHE_LINE_SIZE == 0);
    memcpy(p_stack->get_pmem_ptr() + answer_offset, answer.data(), answer.size());
    pmem_do_flush(p_stack->get_pmem_ptr() + answer_offset, answer.size());
}

std::vector<uint8_t> read_answer(uint8_t size)
{
    if (size < 1 || size > 8)
    {
        throw std::runtime_error("Cannot read answer of size " + std::to_string(size));
    }
    ram_stack const& r_stack = thread_local_owning_storage<ram_stack>::get_const_object();
    persistent_memory_holder* p_stack = thread_local_non_owning_storage<persistent_memory_holder>::ptr;
    /*
     * Function writes answer to the beginning of the previous frame.
     * First 8 bytes of previous frame can hold answer.
     */
    const uint64_t answer_offset = r_stack.get_last_frame().get_position();
    assert(answer_offset % CACHE_LINE_SIZE == 0);
    std::vector<uint8_t> answer(size);
    memcpy(answer.data(), p_stack->get_pmem_ptr() + answer_offset, size);
    return answer;
}

std::vector<uint8_t> read_current_answer(uint8_t size)
{
    if (size < 1 || size > 8)
    {
        throw std::runtime_error("Cannot read answer of size " + std::to_string(size));
    }
    ram_stack const& r_stack = thread_local_owning_storage<ram_stack>::get_const_object();
    persistent_memory_holder* p_stack = thread_local_non_owning_storage<persistent_memory_holder>::ptr;
    if (r_stack.size() == 1)
    {
        throw std::runtime_error("Cannot return value from the first frame");
    }
    const uint64_t answer_offset = r_stack.get_answer_position();
    assert(answer_offset % CACHE_LINE_SIZE == 0);
    std::vector<uint8_t> answer(size);
    memcpy(answer.data(), p_stack->get_pmem_ptr() + answer_offset, size);
    return answer;
}