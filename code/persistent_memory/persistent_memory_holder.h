#ifndef DIPLOM_PERSISTENT_MEMORY_HOLDER_H
#define DIPLOM_PERSISTENT_MEMORY_HOLDER_H

#include <cstdint>
#include <string>

/**
 * Persistent stack (or heap) is stored in some file, and the file is mapped
 * in virtual memory. Objects of this class own opened file and mapped memory.
 * Each object of this class owns single file, therefore, there should be N + 1 such objects in
 * a program, where N is the number of worker threads. N objects should contain persistent
 * stacks for each of worker threads, and the last object should contain pointer to the heap.
 */
struct persistent_memory_holder
{
    /**
     * Depending on the boolean parameter, either:
     * <ul>
     *  <li>
     *      deletes existing file (if exists), creates new file,
     *      where persistent memory will be stored, opens it and
     *      allocates enough memory for persistent memory
     *      (_size bytes).
     *  <\li>
     *  <li>
     *      Opens existing file with persistent memory
     *  <\li>
     * <\ul>
     * After this, memory-maps opened file using mmap(2).
     * @param _file_name - path to file for storing persistent memory
     * @param open_existing - if false, than existing file will be removed (if exists)
     *                        and new empty file will be created. Otherwise,
     *                        existing file will be opened.
     * @param _size - number of bytes in file. It is recommended to use PMEM_STACK_SIZE for
     *               stack and PMEM_HEAP_SIZE for heap.
     */
    persistent_memory_holder(std::string _file_name, bool open_existing, uint64_t _size);

    /**
     * Constructs persistent memory bolder from other persistent memory holder,
     * destroying other memory holder, from which it was constructed.
     * @param other - other persistent memory holder. After move constructor
     *                persistent memory holder, from which new instance was constructed,
     *                doesn't own any resources (file or memory).
     */
    persistent_memory_holder(persistent_memory_holder&& other) noexcept;

    /**
     * Since persistent memory is stored in some file,
     * copying is not permitted.
     * @param other
     */
    persistent_memory_holder(const persistent_memory_holder& other) = delete;

    /**
     * = operator is also deleted, because it is can be considered as a form
     * of copy constructor.
     * @param other
     * @return
     */
    persistent_memory_holder operator=(const persistent_memory_holder& other) = delete;

    /**
     * munmap(2)-s file, in which persistent memory is stored,
     * and closes the file. If some error occurs while trying
     * to close file/munmap, it is logged, but no error is thrown
     * (because error in destructor can lead to std::abort).
     * If persistent memory holder doesn't own any file (for example, it was just
     * used as an argument for move constructor), does nothing.
     */
    ~persistent_memory_holder();

    /**
     * Returns constant pointer to beginning of the memory-mapping
     * of the file, in which persistent memory is stored. Returned pointer can be used for
     * reading persistent memory, but not for writing.
     * @return constant pointer to the beginning of the persistent memory mapping.
     */
    [[nodiscard]] const uint8_t* get_pmem_ptr() const;

    /**
     * Returns pointer to beginning of the memory-mapping
     * of the file, in which persistent memory is stored. Returned pointer
     * can be used both for reading and writing persistent memory.
     * @return pointer to the beginning of the persistent memory mapping.
     */
    [[nodiscard]] uint8_t* get_pmem_ptr();

private:
    int fd;
    uint8_t* pmem_ptr;
    uint64_t size;
    std::string file_name;
};

#endif //DIPLOM_PERSISTENT_MEMORY_HOLDER_H
