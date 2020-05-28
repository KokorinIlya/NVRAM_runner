#include "exec_task.h"

#include <vector>
#include <iostream>
#include <cstring>
#include "../persistent_stack/persistent_stack.h"
#include "../storage/global_non_owning_storage.h"
#include <cassert>
#include "../common/pmem_utils.h"
#include "answer.h"
#include "call.h"
#include <optional>

void exec_task_common(const uint8_t* args, bool call_recover)
{
    uint64_t cur_offset = 0;
    /*
     * Read 1 byte of task type
     */
    uint8_t task_type;
    std::memcpy(&task_type, args + cur_offset, 1);
    cur_offset += 1;

    switch (task_type)
    {
        /*
         * Task is CAS
         */
        case 0x0:
        {
            /*
             * Read 8 bytes of answer offset
             */
            uint64_t answer_offset;
            std::memcpy(&answer_offset, args + cur_offset, 8);
            cur_offset += 8;

            uint8_t* answer_address =
                    global_non_owning_storage<persistent_memory_holder>::ptr->get_pmem_ptr() + answer_offset;

            /*
             * System is running in recovery mode
             */
            if (call_recover)
            {
                std::vector<uint8_t> cas_answer = read_answer(1);
                assert(cas_answer.size() == 1 &&
                       (cas_answer[0] == 0x0 || cas_answer[0] == 0x1 || cas_answer[0] == 0xFF));
                /*
                 * If CAS has already finished it's execution, retrieve it's result.
                 * Otherwise, run CAS again.
                 */
                if (cas_answer[0] == 0x0 || cas_answer[0] == 0x1)
                {
                    std::memcpy(answer_address, &cas_answer[0], 1);
                    pmem_do_flush(answer_address, 1);
                    return;
                }
            }

            /*
             * ordinary (not recover) operation is called OR answer hasn't been written to pmem
             */

            std::vector<uint8_t> cas_args(24);
            /*
             * 8 bytes of var offset
             * 4 bytes of expected value
             * 4 bytes of new value
             * 8 bytes of thread matrix offset
             */
            std::memcpy(cas_args.data(), args + cur_offset, 24);

            do_call(
                    "cas",
                    cas_args,
                    std::optional<std::vector<uint8_t>>(),
                    std::optional<std::vector<uint8_t>>(),
                    call_recover
            );
            std::vector<uint8_t> cas_answer = read_answer(1);
            assert(cas_answer.size() == 1 && (cas_answer[0] == 0x0 || cas_answer[0] == 0x1));

            /*
             * Write answer to pmem
             */
            std::memcpy(answer_address, &cas_answer[0], 1);
            pmem_do_flush(answer_address, 1);

            break;
        }
        default:
        {
            std::cerr << "Cannot execute task of type " << (int) task_type << std::endl;
            break;
        }
    }
}

void exec_task(const uint8_t* args)
{
    exec_task_common(args, false);
}

void exec_task_recover(const uint8_t* args)
{
    exec_task_common(args, true);
}