# 'Google_test' is the subproject name
project(Google_tests)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# 'Google_Tests_run' is the target name
# 'test1.cpp tests2.cpp' are source files with tests
add_executable(
        Google_Tests_run
        queue_test.cpp
        test_persistent_stack.cpp
        test_utils.cpp
        function_address_holder_test.cpp
        thread_local_non_owning_storage_test.cpp
        thread_local_owning_storage_test.cpp
        persistent_stack_multithreading_test.cpp
        call_test.cpp
        call_multithreading_test.cpp
        ../code/persistent_stack/ram_stack.cpp
        ../code/globals/function_address_holder.cpp
        ../code/persistent_stack/persistent_stack.cpp
        ../code/common/pmem_utils.cpp
        ../code/persistent_stack/call.cpp
)
target_link_libraries(Google_Tests_run pmem gtest gtest_main)