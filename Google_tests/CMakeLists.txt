# 'Google_test' is the subproject name
project(Google_tests)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(
        Google_Tests_run
        ../code/persistent_memory/persistent_memory_holder.cpp
        ../code/persistent_stack/persistent_stack.cpp
        ../code/persistent_stack/ram_stack.cpp
        ../code/common/pmem_utils.cpp
        ../code/model/function_address_holder.cpp
        ../code/common/constants_and_types.cpp
        ../code/frame/stack_frame.cpp
        ../code/frame/positioned_frame.cpp
        ../code/cas/cas.cpp
        ../code/model/total_thread_count_holder.cpp
        ../code/model/cur_thread_id_holder.cpp
        ../code/runtime/exec_task.cpp
        ../code/runtime/restoration.cpp
        ../code/allocation/pmem_allocator.cpp
        ../code/model/tasks.cpp
        ../code/runtime/answer.cpp
        ../code/runtime/call.cpp
        blocking_queue/queue_test.cpp
        persistent_stack/test_persistent_stack.cpp
        common/test_utils.cpp
        storage/thread_local_non_owning_storage_test.cpp
        storage/thread_local_owning_storage_test.cpp
        persistent_stack/persistent_stack_multithreading_test.cpp
        runtime/call_test.cpp
        runtime/call_multithreading_test.cpp
        runtime/answer_test.cpp
        storage/global_storage_test.cpp
        runtime/answer_multithreading_test.cpp
        common/pmem_utils_test.cpp
        cas/cas_internal_test.cpp
        cas/cas_test.cpp
        runtime/exec_task_test.cpp
        runtime/restoration_test.cpp
        allocation/pmem_allocator_test.cpp
)
target_link_libraries(Google_Tests_run pmem gtest gtest_main)