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
#include "../../code/runtime/exec_task.h"

TEST(exec_task, cas_single_successful)
{
    temp_file heap_file(get_temp_file_name("heap"));
    temp_file stack_file(get_temp_file_name("stack"));
    persistent_memory_holder heap(heap_file.file_name, false, PMEM_HEAP_SIZE);
    persistent_memory_holder stack(stack_file.file_name, false, PMEM_STACK_SIZE);

    global_non_owning_storage<persistent_memory_holder>::ptr = &heap;
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs["exec_task"] = {exec_task, exec_task};
    global_storage<function_address_holder>::get_object().funcs["cas"] = {cas, cas};

    add_new_frame(
            thread_local_owning_storage<ram_stack>::get_object(),
            stack_frame("initial_frame", std::vector<uint8_t>()),
            stack
    );

    uint64_t* var = (uint64_t*) heap.get_pmem_ptr();
    uint32_t total_thread_number = 4;
    global_storage<total_thread_count_holder>::set_object(total_thread_count_holder(total_thread_number));
    uint32_t* thread_matrix = (uint32_t*) (heap.get_pmem_ptr() + 8);

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    std::vector<uint8_t> args(33);
    uint8_t task_type = 0x0;
    std::memcpy(args.data(), &task_type, 1);
    uint64_t answer_offset = 200;
    std::memcpy(args.data() + 1, &answer_offset, 8);
    uint64_t var_offset = 0;
    std::memcpy(args.data() + 9, &var_offset, 8);
    uint32_t expected_value = 42;
    std::memcpy(args.data() + 17, &expected_value, 4);
    uint32_t new_value = 24;
    std::memcpy(args.data() + 21, &new_value, 4);
    uint64_t thread_matrix_offset = 8;
    std::memcpy(args.data() + 25, &thread_matrix_offset, 8);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(1));
    do_call(
            "exec_task",
            args,
            std::optional<std::vector<uint8_t>>(),
            std::make_optional(std::vector<uint8_t>({0xFF}))
    );

    uint8_t result;
    std::memcpy(&result, heap.get_pmem_ptr() + answer_offset, 1);
    EXPECT_EQ(result, 0x1);

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

TEST(exec_task, cas_single_failed)
{
    temp_file heap_file(get_temp_file_name("heap"));
    temp_file stack_file(get_temp_file_name("stack"));
    persistent_memory_holder heap(heap_file.file_name, false, PMEM_HEAP_SIZE);
    persistent_memory_holder stack(stack_file.file_name, false, PMEM_STACK_SIZE);

    global_non_owning_storage<persistent_memory_holder>::ptr = &heap;
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs["exec_task"] = {exec_task, exec_task};
    global_storage<function_address_holder>::get_object().funcs["cas"] = {cas, cas};

    add_new_frame(
            thread_local_owning_storage<ram_stack>::get_object(),
            stack_frame("initial_frame", std::vector<uint8_t>()),
            stack
    );

    uint64_t* var = (uint64_t*) heap.get_pmem_ptr();
    uint32_t total_thread_number = 4;
    global_storage<total_thread_count_holder>::set_object(total_thread_count_holder(total_thread_number));
    uint32_t* thread_matrix = (uint32_t*) (heap.get_pmem_ptr() + 8);

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    std::vector<uint8_t> args(33);
    uint8_t task_type = 0x0;
    std::memcpy(args.data(), &task_type, 1);
    uint64_t answer_offset = 200;
    std::memcpy(args.data() + 1, &answer_offset, 8);
    uint64_t var_offset = 0;
    std::memcpy(args.data() + 9, &var_offset, 8);
    uint32_t expected_value = 24;
    std::memcpy(args.data() + 17, &expected_value, 4);
    uint32_t new_value = 18;
    std::memcpy(args.data() + 21, &new_value, 4);
    uint64_t thread_matrix_offset = 8;
    std::memcpy(args.data() + 25, &thread_matrix_offset, 8);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(1));
    do_call(
            "exec_task",
            args,
            std::optional<std::vector<uint8_t>>(),
            std::make_optional(std::vector<uint8_t>({0xFF}))
    );

    uint8_t result;
    std::memcpy(&result, heap.get_pmem_ptr() + answer_offset, 1);
    EXPECT_EQ(result, 0x0);

    uint32_t thread_number;
    std::memcpy(&thread_number, heap.get_pmem_ptr(), 4);
    uint32_t value;
    std::memcpy(&value, heap.get_pmem_ptr() + 4, 4);
    EXPECT_EQ(thread_number, std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(value, 42);

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

TEST(exec_task, cas_two_successful)
{
    temp_file heap_file(get_temp_file_name("heap"));
    temp_file stack_file(get_temp_file_name("stack"));
    persistent_memory_holder heap(heap_file.file_name, false, PMEM_HEAP_SIZE);
    persistent_memory_holder stack(stack_file.file_name, false, PMEM_STACK_SIZE);

    global_non_owning_storage<persistent_memory_holder>::ptr = &heap;
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs["exec_task"] = {exec_task, exec_task};
    global_storage<function_address_holder>::get_object().funcs["cas"] = {cas, cas};

    add_new_frame(
            thread_local_owning_storage<ram_stack>::get_object(),
            stack_frame("initial_frame", std::vector<uint8_t>()),
            stack
    );

    uint64_t* var = (uint64_t*) heap.get_pmem_ptr();
    uint32_t total_thread_number = 4;
    global_storage<total_thread_count_holder>::set_object(total_thread_count_holder(total_thread_number));
    uint32_t* thread_matrix = (uint32_t*) (heap.get_pmem_ptr() + 8);

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);


    /*
     * First CAS
     */
    std::vector<uint8_t> args(33);
    uint8_t task_type = 0x0;
    std::memcpy(args.data(), &task_type, 1);
    uint64_t answer_offset = 200;
    std::memcpy(args.data() + 1, &answer_offset, 8);
    uint64_t var_offset = 0;
    std::memcpy(args.data() + 9, &var_offset, 8);
    uint32_t expected_value = 42;
    std::memcpy(args.data() + 17, &expected_value, 4);
    uint32_t new_value = 24;
    std::memcpy(args.data() + 21, &new_value, 4);
    uint64_t thread_matrix_offset = 8;
    std::memcpy(args.data() + 25, &thread_matrix_offset, 8);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(1));
    do_call(
            "exec_task",
            args,
            std::optional<std::vector<uint8_t>>(),
            std::make_optional(std::vector<uint8_t>({0xFF}))
    );

    uint8_t result;
    std::memcpy(&result, heap.get_pmem_ptr() + answer_offset, 1);
    EXPECT_EQ(result, 0x1);

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

    /*
     * Second CAS
     */
    std::vector<uint8_t> second_args(33);
    uint8_t second_task_type = 0x0;
    std::memcpy(second_args.data(), &second_task_type, 1);
    uint64_t second_answer_offset = 400;
    std::memcpy(second_args.data() + 1, &second_answer_offset, 8);
    uint64_t second_var_offset = 0;
    std::memcpy(second_args.data() + 9, &second_var_offset, 8);
    uint32_t second_expected_value = 24;
    std::memcpy(second_args.data() + 17, &second_expected_value, 4);
    uint32_t second_new_value = 53;
    std::memcpy(second_args.data() + 21, &second_new_value, 4);
    uint64_t second_thread_matrix_offset = 8;
    std::memcpy(second_args.data() + 25, &second_thread_matrix_offset, 8);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(2));
    do_call(
            "exec_task",
            second_args,
            std::optional<std::vector<uint8_t>>(),
            std::make_optional(std::vector<uint8_t>({0xFF}))
    );

    uint8_t second_result;
    std::memcpy(&second_result, heap.get_pmem_ptr() + second_answer_offset, 1);
    EXPECT_EQ(second_result, 0x1);

    uint32_t second_thread_number;
    std::memcpy(&second_thread_number, heap.get_pmem_ptr(), 4);
    uint32_t second_value;
    std::memcpy(&second_value, heap.get_pmem_ptr() + 4, 4);
    EXPECT_EQ(second_thread_number, 2);
    EXPECT_EQ(second_value, 53);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            if (thread_num == 1 && other_thread_num == 2)
            {
                EXPECT_EQ(cur_value, 24);
            }
            else
            {
                EXPECT_EQ(cur_value, 0);
            }
        }
    }
}

TEST(exec_task, cas_failed_after_successful)
{
    temp_file heap_file(get_temp_file_name("heap"));
    temp_file stack_file(get_temp_file_name("stack"));
    persistent_memory_holder heap(heap_file.file_name, false, PMEM_HEAP_SIZE);
    persistent_memory_holder stack(stack_file.file_name, false, PMEM_STACK_SIZE);

    global_non_owning_storage<persistent_memory_holder>::ptr = &heap;
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs["exec_task"] = {exec_task, exec_task};
    global_storage<function_address_holder>::get_object().funcs["cas"] = {cas, cas};

    add_new_frame(
            thread_local_owning_storage<ram_stack>::get_object(),
            stack_frame("initial_frame", std::vector<uint8_t>()),
            stack
    );

    uint64_t* var = (uint64_t*) heap.get_pmem_ptr();
    uint32_t total_thread_number = 4;
    global_storage<total_thread_count_holder>::set_object(total_thread_count_holder(total_thread_number));
    uint32_t* thread_matrix = (uint32_t*) (heap.get_pmem_ptr() + 8);

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);


    /*
     * First CAS
     */
    std::vector<uint8_t> args(33);
    uint8_t task_type = 0x0;
    std::memcpy(args.data(), &task_type, 1);
    uint64_t answer_offset = 200;
    std::memcpy(args.data() + 1, &answer_offset, 8);
    uint64_t var_offset = 0;
    std::memcpy(args.data() + 9, &var_offset, 8);
    uint32_t expected_value = 42;
    std::memcpy(args.data() + 17, &expected_value, 4);
    uint32_t new_value = 24;
    std::memcpy(args.data() + 21, &new_value, 4);
    uint64_t thread_matrix_offset = 8;
    std::memcpy(args.data() + 25, &thread_matrix_offset, 8);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(1));
    do_call(
            "exec_task",
            args,
            std::optional<std::vector<uint8_t>>(),
            std::make_optional(std::vector<uint8_t>({0xFF}))
    );

    uint8_t result;
    std::memcpy(&result, heap.get_pmem_ptr() + answer_offset, 1);
    EXPECT_EQ(result, 0x1);

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

    /*
     * Second CAS
     */
    std::vector<uint8_t> second_args(33);
    uint8_t second_task_type = 0x0;
    std::memcpy(second_args.data(), &second_task_type, 1);
    uint64_t second_answer_offset = 400;
    std::memcpy(second_args.data() + 1, &second_answer_offset, 8);
    uint64_t second_var_offset = 0;
    std::memcpy(second_args.data() + 9, &second_var_offset, 8);
    uint32_t second_expected_value = 18;
    std::memcpy(second_args.data() + 17, &second_expected_value, 4);
    uint32_t second_new_value = 53;
    std::memcpy(second_args.data() + 21, &second_new_value, 4);
    uint64_t second_thread_matrix_offset = 8;
    std::memcpy(second_args.data() + 25, &second_thread_matrix_offset, 8);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(2));
    do_call(
            "exec_task",
            second_args,
            std::optional<std::vector<uint8_t>>(),
            std::make_optional(std::vector<uint8_t>({0xFF}))
    );

    uint8_t second_result;
    std::memcpy(&second_result, heap.get_pmem_ptr() + second_answer_offset, 1);
    EXPECT_EQ(second_result, 0x0);

    uint32_t second_thread_number;
    std::memcpy(&second_thread_number, heap.get_pmem_ptr(), 4);
    uint32_t second_value;
    std::memcpy(&second_value, heap.get_pmem_ptr() + 4, 4);
    EXPECT_EQ(second_thread_number, 1);
    EXPECT_EQ(second_value, 24);

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

TEST(exec_task, cas_successful_after_failed)
{
    temp_file heap_file(get_temp_file_name("heap"));
    temp_file stack_file(get_temp_file_name("stack"));
    persistent_memory_holder heap(heap_file.file_name, false, PMEM_HEAP_SIZE);
    persistent_memory_holder stack(stack_file.file_name, false, PMEM_STACK_SIZE);

    global_non_owning_storage<persistent_memory_holder>::ptr = &heap;
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs["exec_task"] = {exec_task, exec_task};
    global_storage<function_address_holder>::get_object().funcs["cas"] = {cas, cas};

    add_new_frame(
            thread_local_owning_storage<ram_stack>::get_object(),
            stack_frame("initial_frame", std::vector<uint8_t>()),
            stack
    );

    uint64_t* var = (uint64_t*) heap.get_pmem_ptr();
    uint32_t total_thread_number = 4;
    global_storage<total_thread_count_holder>::set_object(total_thread_count_holder(total_thread_number));
    uint32_t* thread_matrix = (uint32_t*) (heap.get_pmem_ptr() + 8);

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);


    /*
     * First CAS
     */
    std::vector<uint8_t> args(33);
    uint8_t task_type = 0x0;
    std::memcpy(args.data(), &task_type, 1);
    uint64_t answer_offset = 200;
    std::memcpy(args.data() + 1, &answer_offset, 8);
    uint64_t var_offset = 0;
    std::memcpy(args.data() + 9, &var_offset, 8);
    uint32_t expected_value = 24;
    std::memcpy(args.data() + 17, &expected_value, 4);
    uint32_t new_value = 53;
    std::memcpy(args.data() + 21, &new_value, 4);
    uint64_t thread_matrix_offset = 8;
    std::memcpy(args.data() + 25, &thread_matrix_offset, 8);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(1));
    do_call(
            "exec_task",
            args,
            std::optional<std::vector<uint8_t>>(),
            std::make_optional(std::vector<uint8_t>({0xFF}))
    );

    uint8_t result;
    std::memcpy(&result, heap.get_pmem_ptr() + answer_offset, 1);
    EXPECT_EQ(result, 0x0);

    uint32_t thread_number;
    std::memcpy(&thread_number, heap.get_pmem_ptr(), 4);
    uint32_t value;
    std::memcpy(&value, heap.get_pmem_ptr() + 4, 4);
    EXPECT_EQ(thread_number, std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(value, 42);

    for (uint32_t thread_num = 0; thread_num < total_thread_number; thread_num++)
    {
        for (uint32_t other_thread_num = 0; other_thread_num < total_thread_number; other_thread_num++)
        {
            uint32_t cur_offset = thread_num * total_thread_number + other_thread_num;
            uint32_t cur_value = *(thread_matrix + cur_offset);
            EXPECT_EQ(cur_value, 0);
        }
    }

    /*
     * Second CAS
     */
    std::vector<uint8_t> second_args(33);
    uint8_t second_task_type = 0x0;
    std::memcpy(second_args.data(), &second_task_type, 1);
    uint64_t second_answer_offset = 400;
    std::memcpy(second_args.data() + 1, &second_answer_offset, 8);
    uint64_t second_var_offset = 0;
    std::memcpy(second_args.data() + 9, &second_var_offset, 8);
    uint32_t second_expected_value = 42;
    std::memcpy(second_args.data() + 17, &second_expected_value, 4);
    uint32_t second_new_value = 53;
    std::memcpy(second_args.data() + 21, &second_new_value, 4);
    uint64_t second_thread_matrix_offset = 8;
    std::memcpy(second_args.data() + 25, &second_thread_matrix_offset, 8);

    thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(2));
    do_call(
            "exec_task",
            second_args,
            std::optional<std::vector<uint8_t>>(),
            std::make_optional(std::vector<uint8_t>({0xFF}))
    );

    uint8_t second_result;
    std::memcpy(&second_result, heap.get_pmem_ptr() + second_answer_offset, 1);
    EXPECT_EQ(second_result, 0x1);

    uint32_t second_thread_number;
    std::memcpy(&second_thread_number, heap.get_pmem_ptr(), 4);
    uint32_t second_value;
    std::memcpy(&second_value, heap.get_pmem_ptr() + 4, 4);
    EXPECT_EQ(second_thread_number, 2);
    EXPECT_EQ(second_value, 53);

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