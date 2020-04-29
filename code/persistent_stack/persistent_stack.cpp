//
// Created by ilya on 24.04.2020.
//

#include "persistent_stack.h"
#include "cstring"
#include <utility>
#include "../common/pmem_utils.h"

uint64_t get_frame_size(const stack_frame& frame)
{
    return frame.function_name.size() + frame.args.size() + 17;
}

uint64_t get_stack_end(const persistent_stack& stack)
{
    if (stack.empty())
    {
        return 0;
    }
    const positioned_frame& last_frame = stack.top();
    return last_frame.position + get_frame_size(last_frame.frame);
}

std::pair<stack_frame, bool> read_frame(const uint8_t* frame_mem)
{
    uint64_t function_name_len;
    std::memcpy(&function_name_len, frame_mem, 8);
    std::string function_name(function_name_len, '#');
    std::memcpy(&function_name[0], frame_mem + 8, function_name_len);

    uint64_t args_len;
    std::memcpy(&args_len, frame_mem + 8 + function_name_len, 8);
    std::vector<uint8_t> args(args_len);
    std::memcpy(args.data(), frame_mem + 16 + function_name_len, args_len);

    uint8_t end_marker;
    std::memcpy(&end_marker, frame_mem + 16 + function_name_len + args_len, 1);
    const bool is_last = end_marker == 0x1;

    return std::make_pair(stack_frame{function_name, args}, is_last);
}

persistent_stack read_stack(const stack_holder& stack_holder)
{
    persistent_stack stack;
    uint64_t cur_offset = 0;
    const uint8_t* const stack_mem = stack_holder.get_stack_ptr();

    while (true)
    {
        const std::pair<stack_frame, bool> read_result = read_frame(stack_mem + cur_offset);
        const stack_frame& cur_frame = read_result.first;
        const bool is_last = read_result.second;
        const positioned_frame pos_frame = positioned_frame{cur_frame, cur_offset};
        stack.push(pos_frame);

        if (is_last)
        {
            return stack;
        }
        cur_offset += get_frame_size(cur_frame);
    }
}

void add_new_frame(persistent_stack& stack, stack_frame const& frame, stack_holder& stack_holder)
{
    uint8_t* const stack_mem = stack_holder.get_stack_ptr();
    const uint64_t new_frame_offset = get_stack_end(stack);
    stack.push(positioned_frame{frame, new_frame_offset});

    uint64_t cur_offset = new_frame_offset;

    const uint64_t function_name_len = frame.function_name.size();
    std::memcpy(stack_mem + cur_offset, &function_name_len, 8);
    cur_offset += 8;

    std::memcpy(stack_mem + cur_offset, frame.function_name.data(), function_name_len);
    cur_offset += function_name_len;

    const uint64_t args_len = frame.args.size();
    std::memcpy(stack_mem + cur_offset, &args_len, 8);
    cur_offset += 8;

    std::memcpy(stack_mem + cur_offset, frame.args.data(), args_len);
    cur_offset += args_len;

    const uint8_t stack_end_marker = 0x1;
    std::memcpy(stack_mem + cur_offset, &stack_end_marker, 1);
    pmem_do_flush(stack_mem + new_frame_offset, get_frame_size(frame));

    if (new_frame_offset != 0)
    {
        const uint8_t frame_end_marker = 0x0;
        std::memcpy(stack_mem + new_frame_offset - 1, &frame_end_marker, 1);
        pmem_do_flush(stack_mem + new_frame_offset - 1, 1);
    }
}

void remove_frame(persistent_stack& stack, stack_holder& stack_holder)
{
    uint8_t* const stack_mem = stack_holder.get_stack_ptr();
    const positioned_frame last_frame = stack.top();
    stack.pop();
    const uint64_t offset = last_frame.position;
    const uint8_t stack_end_marker = 0x1;
    std::memcpy(stack_mem + offset - 1, &stack_end_marker, 1);
    pmem_do_flush(stack_mem + offset - 1, 1);
}
