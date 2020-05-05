#ifndef DIPLOM_GLOBAL_STORAGE_H
#define DIPLOM_GLOBAL_STORAGE_H

#include <optional>
#include <stdexcept>

/**
 * Stores instances of some object of type T as global singleton object.
 * For all program threads, only one instance of T is stored, but instances of this class
 * for different types (for example, global_storage<int> and global_storage<string>)
 * store different objects.
 * Initially, no object is stored, and if get_object is called before set_object,
 * an exception will be raised, for correct behaviour set_object should be called before
 * any get_object calls.
 * Multiple calls to set_object are allowed, each of the get_object calls
 * should return object, that was argument of the last set_object call.
 * Since the stored object is visible by all program threads and access to the object is
 * not synchronized (by mutex, for example), user should himself ensure, that all threads access
 * shared object correctly (without race conditions, for example).
 * For example, shared object can be used according to next pattern:
 * <ul>
 *  <li>
 *      From a master thread, initialize global object and store it using set_object.
 *  </li>
 *  <li>
 *      Spawn N worker threads, each of which has read-only access to the global object
 *      using only get_const_object, and therefore, none of the worker threads can
 *      modify stored object.
 *  </li>
 * </ul>
 * Since there is happens-before relation between spawn of a thread and first action in the
 * spawned thread, worker threads can read stored correctly without locking. Note, that
 * after worker threads spawn, all threads in the system are forbidden to modify the stored object
 * in order to prevent race conditions.
 * However, user can use his own access protocols (for example, access to global object can be
 * synchronized using global_storage<std::mutex>).
 * @tparam T - class of object, which will be stored.
 */
template <typename T>
struct global_storage
{
private:
    static std::optional<T> object_opt;

public:
    /**
     * Global storage  will store the object, that is passed as an argument.
     * @param object - object to store
     */
    static void set_object(const T& object);

    /**
     * Returns reference to the object, that is stored in the global storage.
     * @return object, that was an argument of the last call to set_object.
     */
    static T& get_object();

    /**
     * Returns constant reference to the object, that is stored in the global storage.
     * @return object, that was an argument of the last call to set_object.
     */
    static const T& get_const_object();
};

template <typename T>
std::optional<T> global_storage<T>::object_opt = std::optional<T>();

template <typename T>
void global_storage<T>::set_object(const T& object)
{
    object_opt = std::make_optional(object);
}

template <typename T>
T& global_storage<T>::get_object()
{
    if (object_opt.has_value())
    {
        return *object_opt;
    }
    else
    {
        throw std::runtime_error("global storage: get_object call before set_object call");
    }
}

template <typename T>
const T& global_storage<T>::get_const_object()
{
    if (object_opt.has_value())
    {
        return *object_opt;
    }
    else
    {
        throw std::runtime_error("global storage: get_const_object call before set_object call");
    }
}

#endif //DIPLOM_GLOBAL_STORAGE_H
