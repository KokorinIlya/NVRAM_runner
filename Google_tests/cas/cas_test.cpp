#include "gtest/gtest.h"
#include "../../code/cas/cas.h"
#include "../common/test_utils.h"
#include "../../code/common/constants_and_types.h"
#include "../../code/common/pmem_utils.h"
#include <cstring>
#include <limits>
#include "../../code/persistent_memory/persistent_memory_holder.h"
#include "../../code/storage/global_storage.h"
#include "../../code/storage/global_non_owning_storage.h"
#include "../../code/storage/thread_local_non_owning_storage.h"
#include "../../code/storage/thread_local_owning_storage.h"
#include "../../code/model/function_address_holder.h"
#include "../../code/model/cur_thread_id_holder.h"
#include "../../code/model/total_thread_count_holder.h"
#include "../../code/persistent_stack/ram_stack.h"
#include "../../code/persistent_stack/call.h"

namespace
{
    void cas_run(uint8_t const* args)
    {
        std::vector<uint8_t> cas_args(24);
        std::memcpy(cas_args.data(), args, 24);
        uint8_t expected_result;
        std::memcpy(&expected_result, args + 24, 1);

        do_call("cas", cas_args);
        std::vector<uint8_t> response = read_answer(1);
        EXPECT_EQ(response.size(), 1);
        EXPECT_EQ(response[0], expected_result);
        throw std::runtime_error("ha-ha, system crash go brrrrr");
    }
}

TEST(cas, single_successful)
{
    temp_file heap_file(get_temp_file_name("heap"));
    temp_file stack_file(get_temp_file_name("stack"));
    persistent_memory_holder heap(heap_file.file_name, false, PMEM_HEAP_SIZE);
    persistent_memory_holder stack(stack_file.file_name, false, PMEM_STACK_SIZE);

    global_non_owning_storage<persistent_memory_holder>::ptr = &heap;
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs["cas_run"] = {cas_run, cas_run};
    global_storage<function_address_holder>::get_object().funcs["cas"] = {cas, cas};

    uint64_t* var = (uint64_t*) heap.get_pmem_ptr();
    uint32_t total_thread_number = 4;
    global_storage<total_thread_count_holder>::set_object(total_thread_count_holder(total_thread_number));
    uint32_t* thread_matrix = (uint32_t*) heap.get_pmem_ptr() + 8;

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    std::vector<uint8_t> args(25);
    uint64_t var_offset = 0;
    std::memcpy(args.data(), &var_offset, 8);
    uint32_t expected_value = 42;
    std::memcpy(args.data() + 8, &expected_value, 4);
    uint32_t new_value = 24;
    std::memcpy(args.data() + 12, &new_value, 4);
    uint64_t thread_matrix_offset = 8;
    std::memcpy(args.data() + 16, &thread_matrix_offset, 8);
    uint8_t expected_result = 0x1;
    std::memcpy(args.data() + 24, &expected_result, 1);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(1));
    try
    {
        do_call("cas_run", args);
    } catch (...)
    {}

    uint32_t thread_number;
    std::memcpy(&thread_number, heap.get_pmem_ptr(), 4);
    uint32_t value;
    std::memcpy(&value, heap.get_pmem_ptr() + 4, 4);
    EXPECT_EQ(thread_number, 1);
    EXPECT_EQ(value, 24);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }
}