#ifndef DIPLOM_GLOBAL_NON_OWNING_STORAGE_H
#define DIPLOM_GLOBAL_NON_OWNING_STORAGE_H

// TODO: write doc
template <typename T>
struct global_non_owning_storage
{
    /**
     * Pointer to the object
     */
    static thread_local T* ptr;
};

template <typename T>
thread_local T* global_non_owning_storage<T>::ptr = nullptr;

#endif //DIPLOM_GLOBAL_NON_OWNING_STORAGE_H
