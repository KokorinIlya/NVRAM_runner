#include "call.h"
#include "../persistent_stack/ram_stack.h"
#include "../persistent_memory/persistent_memory_holder.h"
#include "../storage/thread_local_owning_storage.h"
#include "../storage/global_storage.h"
#include "../storage/thread_local_non_owning_storage.h"
#include "../common/constants_and_types.h"
#include "../common/pmem_utils.h"
#include "../persistent_stack/persistent_stack.h"
#include "../model/function_address_holder.h"
#include "../model/system_mode.h"
#include <cassert>
#include <cstring>

void do_call(std::string const& function_name,
             std::vector<uint8_t> const& args,
             std::optional<std::vector<uint8_t>> const& ans_filler,
             std::optional<std::vector<uint8_t>> const& new_ans_filler,
             bool call_recover)
{
    ram_stack& r_stack = thread_local_owning_storage<ram_stack>::get_object();
    persistent_memory_holder* p_stack = thread_local_non_owning_storage<persistent_memory_holder>::ptr;
    if (ans_filler.has_value())
    {
        if (ans_filler->empty() || ans_filler->size() > 8)
        {
            throw std::runtime_error("Cannot write answer of size " + std::to_string(ans_filler->size()));
        }
        const __uint64_t last_frame_offset = r_stack.get_last_frame().get_position();
        assert(last_frame_offset % CACHE_LINE_SIZE == 0);
        std::memcpy(p_stack->get_pmem_ptr() + last_frame_offset, ans_filler->data(), ans_filler->size());
        pmem_do_flush(p_stack->get_pmem_ptr() + last_frame_offset, ans_filler->size());
    }
    add_new_frame(r_stack, stack_frame(function_name, args), *p_stack, new_ans_filler);
    function_ptr f_ptr;
    if (call_recover)
    {
        if (global_storage<system_mode>::get_const_object() == system_mode::RECOVERY)
        {
            f_ptr = global_storage<function_address_holder>::get_const_object().funcs.at(function_name).second;
        }
        else
        {
            throw std::runtime_error("Cannot call recovery function when system is running in execution mode");
        }
    }
    else
    {
        f_ptr = global_storage<function_address_holder>::get_const_object().funcs.at(function_name).first;
    }
    f_ptr(args.data());
    remove_frame(r_stack, *p_stack);
}