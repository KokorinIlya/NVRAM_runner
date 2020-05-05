#ifndef DIPLOM_THREAD_LOCAL_NON_OWNING_STORAGE_H
#define DIPLOM_THREAD_LOCAL_NON_OWNING_STORAGE_H

/**
 * Stores pointer to object of some class T. This object is considered singleton
 * object for each of the program threads, and for different threads this storage
 * can store pointers to different objects.
 * Also, even in one thread, instances of this class for different types
 * (for example, thread_local_non_owning_storage<int> and
 * thread_local_non_owning_storage<string>) store pointers to different objects
 * Storage doesn't own object, pointer to which is stored and that object isn't destroyed,
 * when the thread finishes. The object should be allocated in master thread and
 * destroyed by master thread, when the thread, that could use this object, finally finishes
 * it's execution.
 * Since objects are thread-local, there is no need to synchronize access to them
 * by mutex or some other synchronization primitives.
 * @tparam T - class of object, pointer of which will be stored.
 */
template <typename T>
struct thread_local_non_owning_storage
{
    /**
     * Pointer to the object
     */
    static thread_local T* ptr;
};

template <typename T>
thread_local T* thread_local_non_owning_storage<T>::ptr = nullptr;

#endif //DIPLOM_THREAD_LOCAL_NON_OWNING_STORAGE_H
