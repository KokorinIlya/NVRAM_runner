#include "gtest/gtest.h"
#include "../../code/persistent_stack/persistent_stack.h"
#include "../../code/persistent_stack/call.h"
#include "../../code/storage/global_storage.h"
#include "../common/test_utils.h"
#include "../../code/model/function_address_holder.h"

namespace
{
    void f(const uint8_t*)
    {
        do_call("g", std::vector<uint8_t>({4, 5, 6}));
        std::vector<uint8_t> g_answer = read_answer(6);
        EXPECT_EQ(g_answer, std::vector<uint8_t>({1, 3, 5, 7, 9, 6}));

        do_call("l", std::vector<uint8_t>({10, 11, 12}));
        std::vector<uint8_t> l_answer = read_answer(2);
        EXPECT_EQ(l_answer, std::vector<uint8_t>({2, 10}));

        throw std::runtime_error("Ha-ha, system crash goes brrrrr");
    }

    void g(const uint8_t* ptr)
    {
        do_call("h", std::vector<uint8_t>({7, 8, 9}));
        std::vector<uint8_t> h_answer = read_answer(5);
        EXPECT_EQ(h_answer, std::vector<uint8_t>({1, 3, 3, 7, 8}));
        uint8_t val = *(ptr + 2);
        write_answer(std::vector<uint8_t>({1, 3, 5, 7, 9, val}));
    }

    void h(const uint8_t* ptr)
    {
        uint8_t val = *(ptr + 1);
        write_answer(std::vector<uint8_t>({1, 3, 3, 7, val}));
    }

    void l(const uint8_t* ptr)
    {
        uint8_t val = *ptr;
        write_answer(std::vector<uint8_t>({2, val}));
    }

    void b(const uint8_t*)
    {
        write_answer(std::vector<uint8_t>({1, 3, 3, 7}));
        std::vector<uint8_t> ans = read_current_answer(4);
        EXPECT_EQ(ans, std::vector<uint8_t>({1, 3, 3, 7}));
    }

    void a(const uint8_t*)
    {
        do_call("b", std::vector<uint8_t>({4, 5, 6}));
        throw std::runtime_error("system crash");
    }

    void d(const uint8_t*)
    {
        std::vector<uint8_t> ans = read_current_answer(3);
        EXPECT_EQ(ans, std::vector<uint8_t>({7, 8, 9}));
    }

    void c(const uint8_t*)
    {
        do_call(
                "d",
                std::vector<uint8_t>({4, 5, 6}),
                std::make_optional(std::vector<uint8_t>({7, 8, 9}))
        );
        throw std::runtime_error("system crash");
    }
}

TEST(answer, return_value)
{
    std::string file_name = get_temp_file_name("stack");
    temp_file file(file_name);
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs.clear();
    global_storage<function_address_holder>::get_object().funcs["f"] = std::make_pair(f, f);
    global_storage<function_address_holder>::get_object().funcs["g"] = std::make_pair(g, g);
    global_storage<function_address_holder>::get_object().funcs["h"] = std::make_pair(h, h);
    global_storage<function_address_holder>::get_object().funcs["l"] = std::make_pair(l, l);
    persistent_stack p_stack(file.file_name, false);
    thread_local_non_owning_storage<persistent_stack>::ptr = &p_stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    try
    {
        do_call("f", std::vector<uint8_t>({1, 2, 3}));
    } catch (...)
    {}
}

TEST(answer, read_own_value)
{
    std::string file_name = get_temp_file_name("stack");
    temp_file file(file_name);
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs.clear();
    global_storage<function_address_holder>::get_object().funcs["b"] = std::make_pair(b, b);
    global_storage<function_address_holder>::get_object().funcs["a"] = std::make_pair(a, a);
    persistent_stack p_stack(file.file_name, false);
    thread_local_non_owning_storage<persistent_stack>::ptr = &p_stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    try
    {
        do_call("a", std::vector<uint8_t>({1, 2, 3}));
    } catch (...)
    {}
}

TEST(answer, fill_answer)
{
    std::string file_name = get_temp_file_name("stack");
    temp_file file(file_name);
    global_storage<function_address_holder>::set_object(function_address_holder());
    global_storage<function_address_holder>::get_object().funcs.clear();
    global_storage<function_address_holder>::get_object().funcs["c"] = std::make_pair(c, c);
    global_storage<function_address_holder>::get_object().funcs["d"] = std::make_pair(d, d);
    persistent_stack p_stack(file.file_name, false);
    thread_local_non_owning_storage<persistent_stack>::ptr = &p_stack;
    thread_local_owning_storage<ram_stack>::set_object(ram_stack());
    try
    {
        do_call("c", std::vector<uint8_t>({1, 2, 3}));
    } catch (...)
    {}
}