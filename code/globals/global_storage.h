#ifndef DIPLOM_GLOBAL_STORAGE_H
#define DIPLOM_GLOBAL_STORAGE_H

#include <optional>
#include <stdexcept>

/**
 * TODO: update docs
 * TODO: test
 * Holds mapping from function name to pair - address of the function and address of the
 * recover version of the function.
 * Since there should be only one such mapping in the system, it is stored as a static hash map.
 * All functions, that will be called during the execution, should be registered in the map.
 * Since the map is not protected by any lock, it should be correctly initialized from a single process
 * to be used in a multi-threading environment.
 * For example, it can be initialized using next sequence of steps:
 * <ul>
 *  <li>
 *      From a master thread, add all functions to the map
 *  </li>
 *  <li>
 *      Spawn N worker threads, each of which has read-only access to the map
 *  </li>
 * </ul>
 * Since there is happens-before relation between spawn of a thread and first action in the spawned thread,
 * worker threads can read from map correctly without locking. Note, that after worker threads spawn,
 * all threads in the system are forbidden to modify the map.
 *
 * Map is stored in the RAM and should be initialized after each system restart.
 */
template <typename T>
struct global_storage
{
private:
    static std::optional<T> object_opt;

public:
    static void set_object(const T& object);

    static T& get_object();

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
