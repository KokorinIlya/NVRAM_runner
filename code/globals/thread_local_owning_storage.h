//
// Created by ilya on 30.04.2020.
//

#ifndef DIPLOM_THREAD_LOCAL_OWNING_STORAGE_H
#define DIPLOM_THREAD_LOCAL_OWNING_STORAGE_H

#include <optional>
#include "../persistent_stack/ram_stack.h"
#include <stdexcept>

/**
 * Stores instances of some object of type T as thread local singleton object.
 * Stored object is considered singleton object for each of the program threads,
 * and for different threads this storage can store different objects of the same type.
 * Storage owns object, that is stored inside, and when the thread finishes,
 * object is destroyed.
 * Also, even in one thread, instances of this class for different types
 * (for example, thread_local_owning_storage<int> and
 * thread_local_owning_storage<string>) store different objects
 * Initially, in each of the threads no object is stored, and if get_object
 * is called before set_object, an exception will be raised.
 * In each thread set_object should be called before any get_object calls.
 * In each thread multiple calls to set_object are allowed, each of the get_object calls
 * should return object, that was argument of the last set_object call.
 */
template <typename T>
struct thread_local_owning_storage
{
private:
    static thread_local std::optional<T> object_opt;

public:
    /**
     * Storage in the caller thread will store the object, that is passed as an argument.
     * @param object - object to store
     */
    static void set_object(const T& object);

    /**
     * Returns reference to the object, that is stored in the thread-local storage.
     * @return object, that was an argument of the last call to set_object.
     */
    static T& get_object();

    /**
     * Returns constant reference to the object, that is stored in the thread-local storage.
     * @return object, that was an argument of the last call to set_object.
     */
    static const T& get_const_object();
};

template <typename T>
thread_local std::optional<T> thread_local_owning_storage<T>::object_opt = std::optional<T>();

template <typename T>
void thread_local_owning_storage<T>::set_object(const T& object)
{
    object_opt = std::make_optional(object);
}

template <typename T>
T& thread_local_owning_storage<T>::get_object()
{
    if (object_opt.has_value())
    {
        return *object_opt;
    }
    else
    {
        throw std::runtime_error("get_object call before set_object call");
    }
}

template <typename T>
const T& thread_local_owning_storage<T>::get_const_object()
{
    if (object_opt.has_value())
    {
        return *object_opt;
    }
    else
    {
        throw std::runtime_error("get_const_object call before set_object call");
    }
}


#endif //DIPLOM_THREAD_LOCAL_OWNING_STORAGE_H
