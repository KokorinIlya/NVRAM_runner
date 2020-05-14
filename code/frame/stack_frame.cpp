#include "stack_frame.h"

#include <utility>

uint64_t stack_frame::size() const
{
    /*
     * 8 bytes for answer
     * 2 bytes of function name size
     * function name
     * 2 bytes of arguments size
     * arguments
     * 1 byte of end marker
     */
    return args.size() + function_name.size() + 13;
}

stack_frame::stack_frame(std::string _function_name, std::vector<uint8_t> _args) :
        function_name(std::move(_function_name)),
        args(std::move(_args))
{}

const std::string& stack_frame::get_function_name() const
{
    return function_name;
}

const std::vector<uint8_t>& stack_frame::get_args() const
{
    return args;
}

stack_frame::stack_frame(stack_frame&& other) noexcept:
        function_name(std::move(other.function_name)),
        args(std::move(other.args))
{}

stack_frame::stack_frame(const stack_frame& other) = default;
