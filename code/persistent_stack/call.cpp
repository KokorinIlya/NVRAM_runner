#include "call.h"
#include "cstring"
#include <utility>
#include "../common/pmem_utils.h"
#include "../globals/global_storage.h"
#include "../common/constants_and_types.h"
#include "../model/function_address_holder.h"

std::pair<stack_frame, bool> read_frame(const uint8_t* frame_mem)
{
    uint64_t cur_offset = 0;

    /*
     * Read 8 bytes of function name len
     */
    uint64_t function_name_len;
    std::memcpy(&function_name_len, frame_mem + cur_offset, 8);
    cur_offset += 8;

    /*
     * Read function_name_len bytes of function name
     */
    std::string function_name(function_name_len, '#');
    std::memcpy(&function_name[0], frame_mem + cur_offset, function_name_len);
    cur_offset += function_name_len;

    /*
     * Read 8 bytes of args len
     */
    uint64_t args_len;
    std::memcpy(&args_len, frame_mem + cur_offset, 8);
    cur_offset += 8;

    /*
     * Read args_len bytes of args
     */
    std::vector<uint8_t> args(args_len);
    std::memcpy(args.data(), frame_mem + cur_offset, args_len);
    cur_offset += args_len;

    /*
     * Skip 8 bytes of answer
     */
    cur_offset += 8;

    /*
     * Read 1 byte of end marker
     */
    uint8_t end_marker;
    std::memcpy(&end_marker, frame_mem + cur_offset, 1);
    const bool is_last = end_marker == 0x1;

    return std::make_pair(stack_frame{function_name, args}, is_last);
}

ram_stack read_stack(const persistent_stack& persistent_stack)
{
    ram_stack stack;
    uint64_t cur_offset = 0;
    const uint8_t* const stack_mem = persistent_stack.get_stack_ptr();

    while (true)
    {
        const std::pair<stack_frame, bool> read_result = read_frame(stack_mem + cur_offset);
        const bool is_last = read_result.second;
        const positioned_frame pos_frame = positioned_frame{read_result.first, cur_offset};
        stack.push(pos_frame);

        if (is_last)
        {
            return stack;
        }
        cur_offset += get_frame_size(read_result.first);
    }
}

void add_new_frame(ram_stack& stack, stack_frame const& frame, persistent_stack& persistent_stack)
{
    uint8_t* const stack_mem = persistent_stack.get_stack_ptr();
    const uint64_t new_frame_offset = get_stack_end(stack);
    stack.push(positioned_frame{frame, new_frame_offset});

    uint64_t cur_offset = new_frame_offset;

    /*
     * Write 8 bytes of function name len
     */
    const uint64_t function_name_len = frame.function_name.size();
    std::memcpy(stack_mem + cur_offset, &function_name_len, 8);
    cur_offset += 8;

    /*
     * Write function_name_len bytes of function name
     */
    std::memcpy(stack_mem + cur_offset, frame.function_name.data(), function_name_len);
    cur_offset += function_name_len;

    /*
     * Write 8 bytes of args len
     */
    const uint64_t args_len = frame.args.size();
    std::memcpy(stack_mem + cur_offset, &args_len, 8);
    cur_offset += 8;

    /*
     * Write args_len bytes of args len
     */
    std::memcpy(stack_mem + cur_offset, frame.args.data(), args_len);
    cur_offset += args_len;

    /*
     * TODO: align by cache line
     * Skip 8 bytes for answer, do not write anything
     */
    cur_offset += 8;

    /*
     * Write 1 byte of stack end marker
     */
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

void remove_frame(ram_stack& stack, persistent_stack& persistent_stack)
{
    if (stack.size() == 1)
    {
        throw std::runtime_error("Cannot remove first frame of the stack");
    }
    uint8_t* const stack_mem = persistent_stack.get_stack_ptr();
    const positioned_frame last_frame = stack.top();
    stack.pop();
    const uint64_t offset = last_frame.position;
    const uint8_t stack_end_marker = 0x1;
    std::memcpy(stack_mem + offset - 1, &stack_end_marker, 1);
    pmem_do_flush(stack_mem + offset - 1, 1);
}

// TODO: allow call_recover = true only if system is in recovery mode
// TODO: add global_holder<T> and write global_storage using it
void do_call(const std::string& function_name, const std::vector<uint8_t>& args, bool call_recover)
{
    add_new_frame(
            thread_local_owning_storage<ram_stack>::get_object(),
            stack_frame{function_name, args},
            *thread_local_non_owning_storage<persistent_stack>::ptr
    );
    function_ptr f_ptr;
    if (call_recover)
    {
        f_ptr = global_storage<function_address_holder>::get_const_object()
                .funcs.at(function_name).second;
    }
    else
    {
        f_ptr = global_storage<function_address_holder>::get_const_object()
                .funcs.at(function_name).first;
    }
    f_ptr(args.data());
    remove_frame(
            thread_local_owning_storage<ram_stack>::get_object(),
            *thread_local_non_owning_storage<persistent_stack>::ptr
    );
}

// TODO: test
void write_answer(std::vector<uint8_t> answer)
{
    if (answer.empty() || answer.size() > 8)
    {
        throw std::runtime_error("Cannot write answer of size " + std::to_string(answer.size()));
    }
    ram_stack const& r_stack = thread_local_owning_storage<ram_stack>::get_const_object();
    persistent_stack* p_stack = thread_local_non_owning_storage<persistent_stack>::ptr;
    if (r_stack.size() == 1)
    {
        throw std::runtime_error("Cannot return value from the first frame");
    }
    const uint64_t last_frame_offset = r_stack.top().position;
    /*
     * 8 bytes of answer + 1 byte of end marker
     */
    const uint64_t answer_offset = last_frame_offset - 9;
    std::memcpy(p_stack->get_stack_ptr() + answer_offset, answer.data(), answer.size());
    pmem_do_flush(p_stack->get_stack_ptr() + answer_offset, answer.size());
}

std::vector<uint8_t> read_answer(uint8_t size)
{
    if (size < 1 || size > 8)
    {
        throw std::runtime_error("Cannot read answer of size " + std::to_string(size));
    }
    ram_stack const& r_stack = thread_local_owning_storage<ram_stack>::get_const_object();
    persistent_stack* p_stack = thread_local_non_owning_storage<persistent_stack>::ptr;
    const uint64_t last_frame_offset = r_stack.top().position;
    /*
     * Skip
     * 8 bytes of function name len
     * function_name_len bytes of function name
     * 8 bytes of args len
     * args_len bytes of args
     * to retrieve beginning of answer
     */
    const uint64_t answer_offset = last_frame_offset + 16 +
                                   r_stack.top().frame.args.size() +
                                   r_stack.top().frame.function_name.size();
    std::vector<uint8_t> answer(size);
    std::memcpy(answer.data(), p_stack->get_stack_ptr() + answer_offset, size);
    return answer;
}

std::vector<uint8_t> read_current_answer(uint8_t size)
{
    if (size < 1 || size > 8)
    {
        throw std::runtime_error("Cannot read answer of size " + std::to_string(size));
    }
    ram_stack const& r_stack = thread_local_owning_storage<ram_stack>::get_const_object();
    persistent_stack* p_stack = thread_local_non_owning_storage<persistent_stack>::ptr;
    if (r_stack.size() == 1)
    {
        throw std::runtime_error("Cannot return value from the first frame");
    }
    const uint64_t last_frame_offset = r_stack.top().position;
    /*
     * 8 bytes of answer + 1 byte of end marker
     */
    const uint64_t answer_offset = last_frame_offset - 9;
    std::vector<uint8_t> answer(size);
    std::memcpy(answer.data(), p_stack->get_stack_ptr() + answer_offset, size);
    return answer;
}
