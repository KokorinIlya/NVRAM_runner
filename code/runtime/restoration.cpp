#include "restoration.h"
#include "../persistent_stack/ram_stack.h"
#include "../persistent_stack/call.h"
#include "../storage/global_storage.h"
#include "../model/system_mode.h"
#include "../model/function_address_holder.h"

void do_restoration(persistent_memory_holder& persistent_stack)
{
    if(global_storage<system_mode>::get_const_object() != system_mode::RECOVERY)
    {
        throw std::runtime_error("Cannot perform system restoration, when system is not in recovery mode");
    }
    ram_stack r_stack = read_stack(persistent_stack);
    thread_local_owning_storage<ram_stack>::set_object(r_stack);
    while (r_stack.size() > 1)
    {
        stack_frame const& top_frame = r_stack.get_last_frame().get_frame();
        /*
         * Retrieve pointer to recovery version of function, using function name from persistent stack frame.
         */
        function_ptr f_recover = global_storage<function_address_holder>::get_const_object()
                .funcs
                .at(top_frame.get_function_name())
                .second;
        f_recover(top_frame.get_args().data());
        /*
         * reference to top_frame becomes dangling, but it isn't used anymore
         */
        remove_frame(r_stack, persistent_stack);
    }
}
