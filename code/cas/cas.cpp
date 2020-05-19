#include "cas.h"
#include <limits>
#include <cstring>
#include "../common/pmem_utils.h"
#include <cassert>
#include "../storage/global_storage.h"
#include "../storage/global_non_owning_storage.h"
#include "../storage/thread_local_owning_storage.h"
#include "../persistent_memory/persistent_memory_holder.h"
#include "../model/cur_thread_id_holder.h"
#include "../model/total_thread_count_holder.h"
#include "../persistent_stack/call.h"
#include <unistd.h>

bool cas_internal(uint64_t* var,
                  uint32_t expected_value,
                  uint32_t new_value,
                  uint32_t cur_thread_number,
                  uint32_t total_thread_number,
                  uint32_t* thread_matrix)
{
    /*
     * Atomically load 8 bytes of <thread_id, value> in per-process local memory.
     * No other process can interfere this load, therefore only consistent 8 bytes can be read.
     */
    uint64_t last_thread_number_and_cur_value = __atomic_load_n(var, __ATOMIC_SEQ_CST);

    const uint8_t* const last_thread_number_and_cur_value_ptr = (const uint8_t*) &last_thread_number_and_cur_value;
    /*
     * Read thread_id (first 4 bytes). No other threads can interfere this load, because load is done
     * from per-thread local variable.
     */
    uint32_t last_thread_number;
    std::memcpy(&last_thread_number, last_thread_number_and_cur_value_ptr, 4);
    /*
     * Read value (last 4 bytes). No other threads can interfere this load, because load is done
     * from per-thread local variable.
     */
    uint32_t cur_value;
    std::memcpy(&cur_value, last_thread_number_and_cur_value_ptr + 4, 4);

    if (cur_value != expected_value)
    {
        /*
         * Failed CAS can be linearized at the moment of load of <thread_id, value>
         */
        return false;
    }

#ifdef CAS_TEST
    usleep(1000000);
#endif

    /*
     * If some other thread changed the value using CAS
     */
    if (last_thread_number != std::numeric_limits<uint32_t>::max())
    {
        /*
         * Notify thread, that performed last successful CAS, that it's CAS was successful.
         * Notification is done using SRSW register.
         */
        uint32_t index = last_thread_number * total_thread_number + cur_thread_number;
        /*
         * Atomically store 4 bytes and flush caches to NVRAM
         */
        __atomic_store_n(thread_matrix + index, cur_value, __ATOMIC_SEQ_CST);

        /*
         * [thread_matrix + index .. thread_matrix + index + 3] belongs to single cache line
         */
        assert(((uint64_t) thread_matrix + index) / CACHE_LINE_SIZE ==
               ((uint64_t) thread_matrix + index + 3) / CACHE_LINE_SIZE);
        pmem_do_flush(thread_matrix + index, 4);
    }

#ifdef CAS_TEST
    usleep(1000000);
#endif

    /*
     * Collects 8 bytes of <thread_id, value> from thread_id and value
     */
    uint64_t cur_thread_number_and_new_value;
    uint8_t* const cur_thread_number_and_new_value_ptr = (uint8_t*) &cur_thread_number_and_new_value;
    /*
     * First 4 bytes stores thread_id
     */
    std::memcpy(cur_thread_number_and_new_value_ptr, &cur_thread_number, 4);
    /*
     * Last 4 bytes stores value
     */
    std::memcpy(cur_thread_number_and_new_value_ptr + 4, &new_value, 4);

    /*
     * Atomically performs 8 byte CAS
     */
    bool result = __atomic_compare_exchange_n(
            var,
            &last_thread_number_and_cur_value, // will be overwritten if CAS fails
            cur_thread_number_and_new_value,
            false,
            __ATOMIC_SEQ_CST,
            __ATOMIC_SEQ_CST
    );
    if (result)
    {
        /*
         * Address of variable should be aligned by cache line size.
         * Note, that since result of malloc is aligned by page size,
         * alignment of address of variable is equivalent to alignment of
         * offset of variable (where offset is calculated from the beginning
         * of mapping). I.e. if var == PMEM_START_ADDRESS + offset,
         * var % CACHE_LINE_SIZE == offset % CACHE_LINE_SIZE.
         */
        assert((uint64_t) var % CACHE_LINE_SIZE == 0);
        /*
         * If CAS succeed, new pair <thread_id, value> was written.
         * Flush 8 bytes to NVRAM. Note, that for atomicity of flush,
         * CAS'ed variable should be aligned by cache line.
         */
        pmem_do_flush(var, 8);
        return true;
    }
    else
    {
        return false;
    }
}

bool cas_recover_internal(uint64_t* var,
                          uint32_t expected_value,
                          uint32_t new_value,
                          uint32_t cur_thread_number,
                          uint32_t total_thread_number,
                          uint32_t* thread_matrix)
{
    /*
     * Atomically load 8 bytes of <thread_id, value> in per-process local memory.
     */
    uint64_t last_thread_number_and_cur_value = __atomic_load_n(var, __ATOMIC_SEQ_CST);

    const uint8_t* const last_thread_number_and_cur_value_ptr = (const uint8_t*) &last_thread_number_and_cur_value;
    /*
    * Read thread_id (first 4 bytes).
    */
    uint32_t last_thread_number;
    std::memcpy(&last_thread_number, last_thread_number_and_cur_value_ptr, 4);
    /*
    * Read value (last 4 bytes).
    */
    uint32_t cur_value;
    std::memcpy(&cur_value, last_thread_number_and_cur_value_ptr + 4, 4);

#ifdef CAS_TEST
    usleep(1000000);
#endif

    if (last_thread_number == cur_thread_number && cur_value == new_value)
    {
        /*
         * CAS was successful; there weren't any other successful CAS'es after
         * our CAS and before the crash.
         */
        return true;
    }

#ifdef CAS_TEST
    usleep(1000000);
#endif

    for (uint32_t other_thread_number = 0; other_thread_number < total_thread_number; other_thread_number++)
    {
        uint32_t index = cur_thread_number * total_thread_number + other_thread_number;
        uint32_t other_thread_value = __atomic_load_n(thread_matrix + index, __ATOMIC_SEQ_CST);
        /*
         * Checking SRSW register. Only thread with id = other_thread_number could write to the register,
         * if it saw that CAS, performed by current thread, was successful.
         * If such CAS was current CAS, current CAS was successful.
         */
        if (other_thread_value == new_value)
        {
            return true;
        }
    }

#ifdef CAS_TEST
    usleep(1000000);
#endif

    /*
     * No other threads have seen current CAS, it can be retried.
     */
    return cas_internal(var, expected_value, new_value, cur_thread_number, total_thread_number, thread_matrix);
}

void cas_common(const uint8_t* args, bool call_recover)
{
    uint64_t cur_offset = 0;

    /*
     * Read 8 bytes of variable address
     */
    uint64_t var_offset;
    std::memcpy(&var_offset, args + cur_offset, 8);
    cur_offset += 8;

    /*
     * Read 4 bytes of expected value
     */
    uint32_t expected_value;
    std::memcpy(&expected_value, args + cur_offset, 4);
    cur_offset += 4;

    /*
     * Read 4 bytes of new value
     */
    uint32_t new_value;
    std::memcpy(&new_value, args + cur_offset, 4);
    cur_offset += 4;

    /*
     * Read 8 bytes of matrix address
     */
    uint64_t thread_matrix_offset;
    std::memcpy(&thread_matrix_offset, args + cur_offset, 8);

    uint32_t total_thread_count = global_storage<total_thread_count_holder>::get_const_object().total_thread_count;
    uint32_t cur_thread_id = thread_local_owning_storage<cur_thread_id_holder>::get_const_object().cur_thread_id;

    uint8_t* pmem_start_address = global_non_owning_storage<persistent_memory_holder>::ptr->get_pmem_ptr();
    uint64_t* var = (uint64_t*) pmem_start_address + var_offset;
    uint32_t* thread_matrix = (uint32_t*) pmem_start_address + thread_matrix_offset;

    bool result;
    if (call_recover)
    {
        result = cas_internal(
                var,
                expected_value,
                new_value,
                cur_thread_id,
                total_thread_count,
                thread_matrix
        );
    }
    else
    {
        result = cas_recover_internal(
                var,
                expected_value,
                new_value,
                cur_thread_id,
                total_thread_count,
                thread_matrix
        );
    }

#ifdef CAS_TEST
    usleep(1000000);
#endif

    if (result)
    {
        write_answer(std::vector<uint8_t>({0x1}));
    }
    else
    {
        write_answer(std::vector<uint8_t>({0x0}));
    }
}

void cas(const uint8_t* args)
{
    cas_common(args, false);
}

void cas_recover(const uint8_t* args)
{
    cas_common(args, true);
}
