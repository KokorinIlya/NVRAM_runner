#include <iostream>
#include "code/cas/cas.h"
#include <fcntl.h>
#include "code/common/constants_and_types.h"
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <limits>
#include "code/common/pmem_utils.h"

int main(int argc, char** argv)
{
    int fd = open("heap.nvram", O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (posix_fallocate(fd, 0, PMEM_HEAP_SIZE) != 0)
    {
        perror("allocate");
        exit(EXIT_FAILURE);
    }
    void* pmemaddr = mmap(nullptr, PMEM_HEAP_SIZE,
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    uint8_t* stack_ptr = static_cast<uint8_t*>(pmemaddr);
    uint64_t* var = (uint64_t*) stack_ptr;
    uint32_t thread_number = 4;
    uint32_t* thread_matrix = (uint32_t*) stack_ptr + 8;

    uint64_t initial_thread_number_and_initial_value;
    uint8_t* initial_thread_number_and_initial_value_ptr = (uint8_t*) &initial_thread_number_and_initial_value;
    uint32_t initial_thread_number = std::numeric_limits<uint32_t>::max();
    uint32_t initial_value = 42;
    std::memcpy(initial_thread_number_and_initial_value_ptr, &initial_thread_number, 4);
    std::memcpy(initial_thread_number_and_initial_value_ptr + 4, &initial_value, 4);

    std::memcpy(var, &initial_thread_number_and_initial_value, 8);
    pmem_do_flush(var, 8);

    bool result = cas_internal(var, 42, 24, 1, thread_number, thread_matrix);
    std::cout << (int) result << std::endl;

    if (munmap(pmemaddr, PMEM_STACK_SIZE) < 0)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if (close(fd) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;
}