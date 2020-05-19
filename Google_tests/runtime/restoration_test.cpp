#include "gtest/gtest.h"
#include "../../code/persistent_stack/call.h"
#include "../../code/model/system_mode.h"
#include "../../code/model/function_address_holder.h"
#include "../../code/storage/global_storage.h"
#include "../common/test_utils.h"
#include "../../code/runtime/restoration.h"

namespace
{
    bool f_executed = false;
    bool g_executed = false;
    bool h_executed = false;

    void f(uint8_t const*)
    {
        do_call("g", std::vector<uint8_t>());
    }

    void g(uint8_t const*)
    {
        do_call("h", std::vector<uint8_t>());
    }

    void h(uint8_t const*)
    {
        throw std::runtime_error("ha-ha, system crash go brrrrr");
    }

    void f_recover(uint8_t const*)
    {
        f_executed = true;
    }

    void g_recover(uint8_t const*)
    {
        g_executed = true;
    }

    void h_recover(uint8_t const*)
    {
        h_executed = true;
    }

    /*
     * Repeatable crashed
     */

    int a_executed = 0;
    int b_executed = 0;
    int c_executed = 0;
    int d_executed = 0;
    int recovery_count = 0;

    void a(uint8_t const*)
    {
        do_call("b", std::vector<uint8_t>());
    }

    void b(uint8_t const*)
    {
        do_call("c", std::vector<uint8_t>());
    }

    void c(uint8_t const*)
    {
        do_call("d", std::vector<uint8_t>());
    }

    void d(uint8_t const*)
    {
        throw std::runtime_error("ha-ha, system crash go brrrrr");
    }


    void a_recover(uint8_t const*)
    {
        a_executed++;
    }

    void b_recover(uint8_t const*)
    {
        b_executed++;
    }

    void c_recover(uint8_t const*)
    {
        if (recovery_count == 0)
        {
            recovery_count++;
            throw std::runtime_error("Ha-ha, repeatable crash go brrrr");
        }
        c_executed++;
    }

    void d_recover(uint8_t const*)
    {
        d_executed++;
    }
}

TEST(restoration, simple)
{
    temp_file stack_file(get_temp_file_name("stack"));
    persistent_memory_holder stack(stack_file.file_name, false, PMEM_STACK_SIZE);

    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs["f"] = {f, f_recover};
    global_storage<function_address_holder>::get_object().funcs["g"] = {g, g_recover};
    global_storage<function_address_holder>::get_object().funcs["h"] = {h, h_recover};

    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stack;
    global_storage<system_mode>::set_object(system_mode::RECOVERY);

    try
    {
        do_call("f", std::vector<uint8_t>());
    }
    catch (...)
    {}

    do_restoration(stack);
    EXPECT_FALSE(f_executed);
    EXPECT_TRUE(g_executed);
    EXPECT_TRUE(h_executed);
}

TEST(restoration, repeatable_crashes)
{
    temp_file stack_file(get_temp_file_name("stack"));
    persistent_memory_holder stack(stack_file.file_name, false, PMEM_STACK_SIZE);

    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs["a"] = {a, a_recover};
    global_storage<function_address_holder>::get_object().funcs["b"] = {b, b_recover};
    global_storage<function_address_holder>::get_object().funcs["c"] = {c, c_recover};
    global_storage<function_address_holder>::get_object().funcs["d"] = {d, d_recover};

    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    thread_local_non_owning_storage<persistent_memory_holder>::ptr = &stack;
    global_storage<system_mode>::set_object(system_mode::RECOVERY);

    try
    {
        do_call("a", std::vector<uint8_t>());
    }
    catch (...)
    {}

    try
    {
        do_restoration(stack);
    }
    catch (...)
    {}

    do_restoration(stack);

    EXPECT_EQ(a_executed, 0);
    EXPECT_EQ(b_executed, 1);
    EXPECT_EQ(c_executed, 1);
    EXPECT_EQ(d_executed, 1);
}