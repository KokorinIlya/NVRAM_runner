//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_PERSISTENT_STACK_H
#define DIPLOM_PERSISTENT_STACK_H

#include <cstdint>
#include <string>

struct persistent_stack
{
    explicit persistent_stack(std::string file_name, bool open_existing);

    persistent_stack(persistent_stack&& other) noexcept;

    persistent_stack(const persistent_stack& other) = delete;

    persistent_stack operator=(const persistent_stack& other) = delete;

    ~persistent_stack();

    const uint8_t* get_stack_ptr() const;

    uint8_t* get_stack_ptr();

private:
    int fd;
    uint8_t* stack_ptr;
    std::string file_name;
};

#endif //DIPLOM_PERSISTENT_STACK_H
