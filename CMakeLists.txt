cmake_minimum_required(VERSION 2.8)
project(Diplom)

set(CMAKE_CXX_STANDARD 17)
include_directories(/opt/sw/pmdk/pmdk.old/include)
include_directories(/opt/sw/gcc/9.1.0/include/c++/9.1.0)
link_directories(/opt/sw/pmdk/pmdk.old/lib)
add_compile_options(-std=c++17)

add_executable(
        Diplom
        main.cpp
        code/persistent_stack/persistent_stack.cpp
        code/persistent_stack/ram_stack.cpp
        code/persistent_stack/call.cpp
        code/common/pmem_utils.cpp
        code/model/function_address_holder.cpp
        code/common/constants_and_types.cpp
        code/frame/stack_frame.cpp
        code/frame/positioned_frame.cpp
        code/cas/cas.cpp
)
target_link_libraries(Diplom pmem pthread)
add_subdirectory(Google_tests)


