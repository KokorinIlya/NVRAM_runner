//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_PERSISTENT_STACK_H
#define DIPLOM_PERSISTENT_STACK_H

#include <cstdint>
#include <string>

/**
 * Persistent stack is stored in some file, and the file is mapped
 * in virtual memory. Objects of this class own opened file and mapped memory.
 */
struct persistent_stack
{
    /**
     * Depending on the boolean parameter, either:
     * <ul>
     *  <li>
     *      deletes existing file (if exists), creates new file,
     *      where persistent stack will be stored, opens it and
     *      allocates enough memory for new persistent stack
     *      (PMEM_STACK_SIZE bytes).
     *  <\li>
     *  <li>
     *      Opens existing file with persistent stack
     *  <\li>
     * <\ul>
     * After this, memory-maps opened file using mmap(2).
     * @param file_name - path to file for storing persistent stack
     * @param open_existing - if false, than existing file will be removed (if exists)
     *                        and new empty file will be created. Otherwise,
     *                        Existing file will be opened.
     */
    explicit persistent_stack(std::string file_name, bool open_existing);

    /**
     * Constructs persistent stack from other persistent stack,
     * destroying stack, from which it was constructed.
     * @param other - other persistent stack. After move constructor
     *                persistent stack, from which new stack was constructed,
     *                doesn't own any resources (file or memory).
     */
    persistent_stack(persistent_stack&& other) noexcept;

    /**
     * Since persistent stack is stored in some file,
     * copying is not permitted.
     * @param other
     */
    persistent_stack(const persistent_stack& other) = delete;

    /**
     * = operator is also deleted, because it is can be considered as a form
     * of copy constructor.
     * @param other
     * @return
     */
    persistent_stack operator=(const persistent_stack& other) = delete;

    /**
     * munmap(2)-s file, in which persistent stack is stored,
     * and closes the file. If some error occurs while trying
     * to close file/munmap, it is logged.
     * If persistent stack doesn't own any file (for example, it was just
     * used as an argument for move constructor), does nothing.
     */
    ~persistent_stack();

    /**
     * Returns constant pointer to beginning of the memory-mapping
     * of the file, in which stack is stored. Returned pointer can be used for
     * reading stack frames, but not for writing.
     * @return constant pointer to the beginning of the stack.
     */
    const uint8_t* get_stack_ptr() const;

    /**
     * Returns pointer to beginning of the memory-mapping
     * of the file, in which stack is stored. Returned pointer
     * can be used both for reading and writing stack frames.
     * @return pointer to the beginning of the stack.
     */
    uint8_t* get_stack_ptr();

private:
    int fd;
    uint8_t* stack_ptr;
    std::string file_name;
};

#endif //DIPLOM_PERSISTENT_STACK_H
