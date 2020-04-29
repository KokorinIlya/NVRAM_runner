cmake_minimum_required(VERSION 2.8)
project(Diplom)

set(CMAKE_CXX_STANDARD 17)
include_directories(/opt/sw/pmdk/pmdk.old/include)
include_directories(/opt/sw/gcc/9.1.0/include/c++/9.1.0)
link_directories(/opt/sw/pmdk/pmdk.old/lib)
add_compile_options("-std=c++17")

add_executable(
        Diplom
        main.cpp
        code/persistent_stack/ram_stack.cpp
        code/address/pmem_address_holder.cpp
        code/address/function_address_holder.cpp
        code/persistent_stack/persistent_stack.cpp
        code/debug_utils/stack_check.cpp
        code/common/pmem_utils.cpp
        code/debug_utils/queue_check.cpp
        code/persistent_stack/call.cpp
        code/address/thread_local_non_owning_storage.h code/debug_utils/check_all.cpp code/debug_utils/check_all.h)
target_link_libraries(Diplom pmem pthread)


