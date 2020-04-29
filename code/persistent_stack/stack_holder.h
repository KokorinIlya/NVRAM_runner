//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_STACK_HOLDER_H
#define DIPLOM_STACK_HOLDER_H

#include <cstdint>
#include <string>

struct stack_holder
{
    explicit stack_holder(std::string file_name);

    ~stack_holder();

    const uint8_t* get_stack_ptr() const;

    uint8_t* get_stack_ptr();

private:
    int fd;
    uint8_t* stack_ptr;
    std::string file_name;
};

#endif //DIPLOM_STACK_HOLDER_H
