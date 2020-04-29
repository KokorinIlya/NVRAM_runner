//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_THREAD_LOCAL_NON_OWNING_STORAGE_H
#define DIPLOM_THREAD_LOCAL_NON_OWNING_STORAGE_H

template <typename T>
struct thread_local_non_owning_storage
{
    static thread_local T* ptr;
};

template <typename T>
thread_local T* thread_local_non_owning_storage<T>::ptr = nullptr;

#endif //DIPLOM_THREAD_LOCAL_NON_OWNING_STORAGE_H
