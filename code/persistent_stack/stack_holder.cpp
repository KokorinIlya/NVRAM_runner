//
// Created by ilya on 29.04.2020.
//

#include "stack_holder.h"
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/mman.h>
#include "../common/constants_and_types.h"
#include <iostream>
#include <utility>

stack_holder::stack_holder(std::string stack_file_name)
        : fd(-1),
          stack_ptr(nullptr),
          file_name(std::move(stack_file_name))
{
    if ((fd = open(file_name.c_str(), O_CREAT | O_RDWR, 0666)) < 0)
    {
        throw std::runtime_error("Error while opening file " + file_name);
    }

    if ((errno = posix_fallocate(fd, 0, PMEM_STACK_SIZE)) != 0)
    {
        if (close(fd) == -1)
        {
            std::string error_text(
                    "Error while closing file "
                    "while trying to allocate memory in file " + file_name);
            throw std::runtime_error(error_text);
        }
        else
        {
            std::string error_text(
                    "Error while trying to allocate memory in file " + file_name);
            throw std::runtime_error(error_text);
        }
    }

    void* pmemaddr = mmap(nullptr, PMEM_STACK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (pmemaddr == nullptr)
    {
        if (close(fd) == -1)
        {
            std::string error_text(
                    "Error while closing file "
                    "while trying to mmap file " + file_name);
            throw std::runtime_error(error_text);
        }
        else
        {
            std::string error_text(
                    "Error while trying to mmap file " + file_name);
            throw std::runtime_error(error_text);
        }
    }
    stack_ptr = static_cast<uint8_t*>(pmemaddr);
}

stack_holder::~stack_holder()
{
    if (munmap(stack_ptr, PMEM_STACK_SIZE) == -1)
    {
        std::cerr << "Error while munmap file " << file_name << std::endl;
    }
    if (close(fd) == -1)
    {
        std::cerr << "Error while closing file " << file_name << std::endl;
    }
}

const uint8_t* stack_holder::get_stack_ptr() const
{
    return stack_ptr;
}

uint8_t* stack_holder::get_stack_ptr()
{
    return stack_ptr;
}
