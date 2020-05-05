# 'Google_test' is the subproject name
project(Google_tests)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(
        Google_Tests_run
        ../code/persistent_stack/ram_stack.cpp
        ../code/persistent_stack/persistent_stack.cpp
        ../code/common/pmem_utils.cpp
        ../code/persistent_stack/call.cpp
        ../code/model/function_address_holder.cpp
        queue_test.cpp
        test_persistent_stack.cpp
        test_utils.cpp
        thread_local_non_owning_storage_test.cpp
        thread_local_owning_storage_test.cpp
        persistent_stack_multithreading_test.cpp
        call_test.cpp
        call_multithreading_test.cpp
        answer_test.cpp
        global_storage_test.cpp
)
target_link_libraries(Google_Tests_run pmem gtest gtest_main)