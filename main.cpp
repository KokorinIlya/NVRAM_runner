#include <iostream>
#include "code/cas/cas.h"
#include "code/common/constants_and_types.h"
#include <cstring>
#include <limits>
#include "code/common/pmem_utils.h"
#include "code/storage/global_storage.h"
#include "code/storage/global_non_owning_storage.h"
#include "code/storage/thread_local_owning_storage.h"
#include "code/storage/thread_local_non_owning_storage.h"
#include "code/model/system_mode.h"
#include "code/persistent_memory/persistent_memory_holder.h"
#include <vector>
#include "code/persistent_stack/persistent_stack.h"
#include "code/persistent_stack/ram_stack.h"
#include "code/frame/stack_frame.h"
#include <thread>
#include <functional>
#include "code/blocking_queue/blocking_queue.h"
#include "code/model/total_thread_count_holder.h"
#include "code/model/cur_thread_id_holder.h"
#include "code/model/tasks.h"
#include "code/allocation/pmem_allocator.h"
#include "code/model/function_address_holder.h"
#include "code/runtime/exec_task.h"
#include "code/runtime/restoration.h"
#include "code/runtime/call.h"
#include <variant>
#include "code/common/variant_utils.h"

void read_var(uint64_t var_offset)
{
    const uint8_t* pmem_start_address = global_non_owning_storage<persistent_memory_holder>::ptr->get_pmem_ptr();
    const uint64_t* var = (uint64_t*) (pmem_start_address + var_offset);
    uint64_t last_thread_number_and_cur_value = __atomic_load_n(var, __ATOMIC_SEQ_CST);
    const uint8_t* const last_thread_number_and_cur_value_ptr = (const uint8_t*) &last_thread_number_and_cur_value;
    uint32_t cur_value;
    std::memcpy(&cur_value, last_thread_number_and_cur_value_ptr + 4, 4);
    uint32_t cur_thread_id = thread_local_owning_storage<cur_thread_id_holder>::get_const_object().cur_thread_id;
    std::string msg = "register value = " + std::to_string(cur_value) +
                      ", cur thread id = " + std::to_string(cur_thread_id) + "\n";
    std::cerr << msg;
}

int main(int argc, char** argv)
{
    if (argc != 6)
    {
        std::cerr << "Args: "
                     "<number of threads> "
                     "<exec/recover> "
                     "<init_heap/recover_heap> "
                     "<path to heap> "
                     "<path to stacks>" << std::endl;
        return EXIT_FAILURE;
    }
    uint32_t number_of_threads = std::stoi(argv[1]);
    std::string execution_mode = argv[2];
    std::string allocator_mode = argv[3];
    std::string path_to_heap = argv[4];
    std::string path_to_stacks = argv[5];


    /*
     * Write total number of threads
     */
    global_storage<total_thread_count_holder>::set_object(total_thread_count_holder(number_of_threads));

    /*
     * Init mapping: function name -> (function address, recover function address)
     */
    function_address_holder func_map;
    func_map.funcs["exec_task"] = {exec_task, exec_task_recover};
    func_map.funcs["cas"] = {cas, cas_recover};
    global_storage<function_address_holder>::set_object(func_map);

    /*
     * Get address of RMW register and thread matrix
     */
    const uint64_t var_offset = get_cache_line_aligned_address(2000);
    const uint64_t thread_matrix_offset = get_cache_line_aligned_address(3000);

    bool heap_exists;
    if (allocator_mode == "init_heap")
    {
        if (execution_mode == "recover")
        {
            /*
             * If mode is recover, heap should have been inited in previous execution
             */
            std::cerr << "system mode must is recover, but heap shouldn't be restored" << std::endl;
            return EXIT_FAILURE;
        }
        heap_exists = false;
    }
    else if (allocator_mode == "recover_heap")
    {
        heap_exists = true;
    }
    else
    {
        std::cerr << "allocator mode must be either init_heap or recover_heap" << std::endl;
        return EXIT_FAILURE;
    }

    /*
     * Init heap and allocator
     */
    persistent_memory_holder heap_holder(path_to_heap, heap_exists, PMEM_HEAP_SIZE);
    global_non_owning_storage<persistent_memory_holder>::ptr = &heap_holder;

    /*
     * If heap hasn't been initialized, init thread matrix and RMW register
     */
    if (!heap_exists)
    {
        uint64_t initial_thread_number_and_initial_value;
        uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
        uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
        uint32_t initial_value = 42;
        std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
        std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

        std::memcpy(heap_holder.get_pmem_ptr() + var_offset, &initial_thread_number_and_initial_value, 8);
        pmem_do_flush(heap_holder.get_pmem_ptr() + var_offset, 8);
    }

    /*
     * If heap doesn't exist (heap_exists == false), init new allocator (init_new = true)
     * If heap already exists (heap_exists == true), recover allocator state allocator (init_new = false)
     */
    pmem_allocator allocator(heap_holder.get_pmem_ptr(), 1, 1000, !heap_exists);

    if (execution_mode == "exec")
    {
        /*
         * Set system mode
         */
        global_storage<system_mode>::set_object(system_mode::EXECUTION);

        /*
         * Init persistent and ram stacks
         */
        std::vector<persistent_memory_holder> persistent_stacks;
        std::vector<ram_stack> ram_stacks;
        for (uint32_t cur_thread_number = 0; cur_thread_number < number_of_threads; cur_thread_number++)
        {
            std::string cur_stack_path = path_to_stacks + "/stack_" + std::to_string(cur_thread_number);
            persistent_stacks.emplace_back(cur_stack_path, false, PMEM_STACK_SIZE);
            ram_stacks.emplace_back();

            /*
             * Put beginning frame (corresponding to the main function of each worker thread) to the top of the stack
             */
            stack_frame first_frame = stack_frame("main_function", std::vector<uint8_t>());
            add_new_frame(ram_stacks.back(), first_frame, persistent_stacks.back());
        }

        /*
         * All stacks have been initialized
         */
        std::cerr << "Starting execution" << std::endl;

        /*
         * Init queue with tasks
         */
        blocking_queue<std::variant<cas_task, read_task>> tasks_queue;

        /*
         * Init worker threads
         */
        std::vector<std::thread> threads;
        for (uint32_t cur_thread_number = 0; cur_thread_number < number_of_threads; cur_thread_number++)
        {
            std::function<void()> main_thread_function = [
                    cur_thread_number,
                    &persistent_stacks,
                    &ram_stacks,
                    &tasks_queue
            ]()
            {
                thread_local_owning_storage<ram_stack>::set_object(ram_stacks[cur_thread_number]);
                thread_local_non_owning_storage<persistent_memory_holder>::ptr = &persistent_stacks[cur_thread_number];
                thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(cur_thread_number));

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
                /*
                 * Main loop: get task from queue and execute it
                 */
                while (true)
                {
                    std::variant<cas_task, read_task> cur_task = tasks_queue.take();
                    std::visit(
                            make_visitor(
                                    [](const cas_task& cur_cas_task)
                                    {
                                        /*
                                         * Serialize CAS args
                                         */
                                        std::vector<uint8_t> args(33);
                                        uint64_t cur_offset = 0;

                                        /*
                                         * Write 1 byte of task type
                                         */
                                        std::memcpy(args.data() + cur_offset, &cas_task::CAS_TYPE, 1);
                                        cur_offset += 1;

                                        /*
                                         * Write 8 bytes of answer offset
                                         */
                                        std::memcpy(args.data() + cur_offset, &cur_cas_task.answer_offset, 8);
                                        cur_offset += 8;

                                        /*
                                         * Write 8 bytes of variable offset
                                         */
                                        std::memcpy(args.data() + cur_offset, &cur_cas_task.var_offset, 8);
                                        cur_offset += 8;

                                        /*
                                         * Write 4 bytes of expected value
                                         */
                                        std::memcpy(args.data() + cur_offset, &cur_cas_task.expected_value, 4);
                                        cur_offset += 4;

                                        /*
                                         * Write 4 bytes of new value
                                         */
                                        std::memcpy(args.data() + cur_offset, &cur_cas_task.new_value, 4);
                                        cur_offset += 4;

                                        /*
                                         * Write 8 bytes of thread matrix offset
                                         */
                                        std::memcpy(
                                                args.data() + cur_offset,
                                                &cur_cas_task.thread_matrix_offset,
                                                8
                                        );

                                        /*
                                         * Wait for CAS completion and continue
                                         */
                                        do_call(
                                                "exec_task",
                                                args,
                                                std::optional<std::vector<uint8_t>>(),
                                                std::make_optional(std::vector<uint8_t>({0xFF}))
                                        );
                                    },
                                    [](const read_task& cur_read_task)
                                    {
                                        read_var(cur_read_task.var_offset);
                                    }
                            ),
                            cur_task
                    );
                }
#pragma clang diagnostic pop
            };
            threads.emplace_back(main_thread_function);
        }

        /*
         * In main thread: add some tasks to worker queue
         */
        std::vector<std::variant<cas_task, read_task>> tasks(
                {
                        cas_task(var_offset,
                                 42,
                                 24,
                                 allocator.pmem_alloc() - heap_holder.get_pmem_ptr(),
                                 thread_matrix_offset),
                        cas_task(var_offset,
                                 42,
                                 53,
                                 allocator.pmem_alloc() - heap_holder.get_pmem_ptr(),
                                 thread_matrix_offset),
                        cas_task(var_offset,
                                 24,
                                 117,
                                 allocator.pmem_alloc() - heap_holder.get_pmem_ptr(),
                                 thread_matrix_offset),
                        cas_task(var_offset,
                                 53,
                                 48,
                                 allocator.pmem_alloc() - heap_holder.get_pmem_ptr(),
                                 thread_matrix_offset),
                        read_task(var_offset),
                        read_task(var_offset)
                }
        );
        for (const std::variant<cas_task, read_task>& cur_task: tasks)
        {
            tasks_queue.push(cur_task);
        }


        /*
         * Wait for worker threads termination (i.e. wait infinitely)
         */
        for (std::thread& cur_thread: threads)
        {
            cur_thread.join();
        }
    }
    else if (execution_mode == "recover")
    {
        global_storage<system_mode>::set_object(system_mode::RECOVERY);

        /*
         * Init persistent stacks
         */
        std::vector<persistent_memory_holder> persistent_stacks;
        for (uint32_t cur_thread_number = 0; cur_thread_number < number_of_threads; cur_thread_number++)
        {
            std::string cur_stack_path = path_to_stacks + "/stack_" + std::to_string(cur_thread_number);
            persistent_stacks.emplace_back(cur_stack_path, true, PMEM_STACK_SIZE);
        }

        /*
         * All stacks have been initialized
         */
        std::cerr << "Starting restoration" << std::endl;

        /*
         * Init restoration threads
         */
        std::vector<std::thread> threads;
        for (uint32_t cur_thread_number = 0; cur_thread_number < number_of_threads; cur_thread_number++)
        {
            std::function<void()> restoration_thread_function = [cur_thread_number, &persistent_stacks]()
            {
                thread_local_non_owning_storage<persistent_memory_holder>::ptr = &persistent_stacks[cur_thread_number];
                thread_local_owning_storage<cur_thread_id_holder>::set_object(cur_thread_id_holder(cur_thread_number));
                do_restoration(persistent_stacks[cur_thread_number]);
            };
            threads.emplace_back(restoration_thread_function);
        }

        /*
         * Wait for worker threads termination (i.e. for the end of restoration)
         */
        for (std::thread& cur_thread: threads)
        {
            cur_thread.join();
        }
        return EXIT_SUCCESS;
    }
    else
    {
        std::cerr << "system mode must be either exec or recover" << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}