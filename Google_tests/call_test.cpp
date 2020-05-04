//
// Created by ilya on 04.05.2020.
//

#include "gtest/gtest.h"
#include "../code/persistent_stack/persistent_stack.h"
#include "../code/persistent_stack/ram_stack.h"
#include "../code/persistent_stack/call.h"
#include "../code/globals/thread_local_non_owning_storage.h"
#include "../code/globals/thread_local_owning_storage.h"
#include "../code/globals/function_address_holder.h"
#include "test_utils.h"
#include <functional>

namespace
{
    void f(const uint8_t*)
    {
        do_call("g", std::vector<uint8_t>({4, 5, 6}));
    }

    void g(const uint8_t*)
    {
        do_call("h", std::vector<uint8_t>({7, 8, 9}));
        throw std::runtime_error("Ha-ha, crash goes brrrrrr");
    }

    void h(const uint8_t*)
    {
    }
}

TEST(call, restoration_after_crash)
{
    std::string file_name = get_temp_file_name("stack");
    std::function<void()> execution = [&file_name]()
    {
        function_address_holder::functions.clear();
        function_address_holder::functions["f"] = std::make_pair(f, f);
        function_address_holder::functions["g"] = std::make_pair(g, g);
        function_address_holder::functions["h"] = std::make_pair(h, h);
        persistent_stack p_stack(file_name, false);
        thread_local_non_owning_storage<persistent_stack>::ptr = &p_stack;
        thread_local_owning_storage<ram_stack>::set_object(ram_stack());
        do_call("f", std::vector<uint8_t>({1, 2, 3}));
    };
    try
    {
        execution();
    } catch (...)
    {

    }


    std::function<void()> restoration = [&file_name]()
    {
        persistent_stack p_stack(file_name, true);
        thread_local_non_owning_storage<persistent_stack>::ptr = &p_stack;
        ram_stack r_stack = read_stack(p_stack);
        thread_local_owning_storage<ram_stack>::set_object(r_stack);
        EXPECT_EQ(r_stack.size(), 2);

        stack_frame g_frame = r_stack.top().frame;
        r_stack.pop();
        EXPECT_EQ(g_frame.function_name, "g");
        EXPECT_EQ(g_frame.args, std::vector<uint8_t>({4, 5, 6}));

        stack_frame f_frame = r_stack.top().frame;
        r_stack.pop();
        EXPECT_EQ(f_frame.function_name, "f");
        EXPECT_EQ(f_frame.args, std::vector<uint8_t>({1, 2, 3}));
    };
    restoration();

    remove(file_name.c_str());
}