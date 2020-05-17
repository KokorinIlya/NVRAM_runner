#ifndef DIPLOM_CUR_THREAD_ID_HOLDER_H
#define DIPLOM_CUR_THREAD_ID_HOLDER_H

#include <cstdint>

/**
 * Stores id of current thread. It is recommended to use this class with some thread_local storage
 * to be able to retrieve id of thread from each of the threads. If this class is used with thread_local
 * storage, each of the system threads will have it's own id.
 */
struct cur_thread_id_holder
{
    /**
     * Id of current thread.
     */
    const uint32_t cur_thread_id;

    cur_thread_id_holder(uint32_t id);
};

#endif //DIPLOM_CUR_THREAD_ID_HOLDER_H
