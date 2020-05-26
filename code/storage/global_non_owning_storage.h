#ifndef DIPLOM_GLOBAL_NON_OWNING_STORAGE_H
#define DIPLOM_GLOBAL_NON_OWNING_STORAGE_H

/**
 * Stores pointers to instances of some object of type T as global singleton object.
 * For all program threads, only one instance of T is stored, but instances of this class
 * for different types (for example, global_non_owning_storage<int> and global_non_owning_storage<string>)
 * store pointers to different objects.
 * Storage doesn't own objects, pointer to which is stored in storage.
 * Since there is only one global object in the system and all system threads can access it,
 * user should himself ensure correctness of access to the stored object
 * (just like when using global_storage<T>).
 * @tparam T - class of object, pointer to which will be stored.
 */
template <typename T>
struct global_non_owning_storage
{
    /**
     * Pointer to the object
     */
    static T* ptr;
};

template <typename T>
T* global_non_owning_storage<T>::ptr = nullptr;

#endif //DIPLOM_GLOBAL_NON_OWNING_STORAGE_H
