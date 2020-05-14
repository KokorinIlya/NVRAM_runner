#ifndef DIPLOM_STACK_FRAME_H
#define DIPLOM_STACK_FRAME_H

#include <string>
#include <vector>

/**
 * Single frame of the stack.
 */
struct stack_frame
{
private:
    /**
     * Name of the function, that was called.
     */
    std::string function_name;
    /**
     * Args of the function, marshalled to array of bytes.
     */
    std::vector<uint8_t> args;

public:
    /**
     * Calculates size of frame in bytes without taking offset into account.
     * Frame size is calculated starting from first payload byte of the frame
     * (i.e. starting from answer field)
     * @return size of frame in bytes.
     */
    [[nodiscard]] uint64_t size() const;

    stack_frame(std::string _function_name, std::vector<uint8_t> _args);

    stack_frame(stack_frame&& other) noexcept;

    stack_frame(stack_frame const& other);

    [[nodiscard]] const std::string& get_function_name() const;

    [[nodiscard]] const std::vector<uint8_t>& get_args() const;
};

#endif //DIPLOM_STACK_FRAME_H
