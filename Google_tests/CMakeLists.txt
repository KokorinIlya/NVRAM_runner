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
        ../code/common/constants_and_types.cpp
        blocking_queue/queue_test.cpp
        persistent_stack/test_persistent_stack.cpp
        common/test_utils.cpp
        storage/thread_local_non_owning_storage_test.cpp
        storage/thread_local_owning_storage_test.cpp
        persistent_stack/persistent_stack_multithreading_test.cpp
        persistent_stack/call_test.cpp
        persistent_stack/call_multithreading_test.cpp
        persistent_stack/answer_test.cpp
        storage/global_storage_test.cpp
        persistent_stack/answer_multithreading_test.cpp
        common/pmem_utils_test.cpp
)
target_link_libraries(Google_Tests_run pmem gtest gtest_main)