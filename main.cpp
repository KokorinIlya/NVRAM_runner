#include <iostream>
#include "code/persistent_stack/call.h"
#include "code/globals/thread_local_non_owning_storage.h"
#include "code/globals/thread_local_stack_storage.h"
#include <thread>
#include <unistd.h>
#include <functional>

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cout << "run: ./Diplom <mode> <number of threads> <path to directory with stacks>"
                  << std::endl;
        exit(0);
    }
    const std::string mode(argv[1]);
    const uint32_t number_of_threads = static_cast<uint32_t>(std::stoi(argv[2]));
    const std::string stacks_path(argv[3]);

    if (mode == "run")
    {
        std::vector<persistent_stack> stacks;
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            const std::string file_name = stacks_path + "/stack-" + std::to_string(i);
            stacks.emplace_back(file_name, false);
        }
        std::vector<std::thread> threads;
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            std::function<void()> thread_action = [&stacks, i]()
            {
                thread_local_stack_storage::set_stack(ram_stack());
                thread_local_non_owning_storage<persistent_stack>::ptr =
                        &stacks[i];
                sleep(2);
                persistent_stack* stack_ptr =
                        thread_local_non_owning_storage<persistent_stack>::ptr;
                std::string msg = "Thread " + std::to_string(i) + ", ptr = " +
                                  std::to_string((long long) stack_ptr->get_stack_ptr());
                std::cout << msg << std::endl;
                stack_frame frame_1 = stack_frame
                        {
                                "some_function_name_" + std::to_string(i),
                                std::vector<uint8_t>({1, 3, 3, 7})
                        };
                add_new_frame(
                        thread_local_stack_storage::get_stack(),
                        frame_1,
                        *thread_local_non_owning_storage<persistent_stack>::ptr
                );

                stack_frame frame_2 = stack_frame
                        {
                                "another_function_name_" + std::to_string(i),
                                std::vector<uint8_t>({2, 5, 1, 7})
                        };
                add_new_frame(
                        thread_local_stack_storage::get_stack(),
                        frame_2,
                        *thread_local_non_owning_storage<persistent_stack>::ptr
                );

                stack_frame frame_3 = stack_frame
                        {
                                "one_more_function_name_" + std::to_string(i),
                                std::vector<uint8_t>({1, 3, 5, 7, 9})
                        };
                add_new_frame(
                        thread_local_stack_storage::get_stack(),
                        frame_3,
                        *thread_local_non_owning_storage<persistent_stack>::ptr
                );
                remove_frame(
                        thread_local_stack_storage::get_stack(),
                        *thread_local_non_owning_storage<persistent_stack>::ptr
                );
            };
            threads.emplace_back(std::thread(thread_action));
        }
        for (std::thread& cur_thread: threads)
        {
            cur_thread.join();
        }
        std::cout << "Master thread exiting" << std::endl;
    }
    else
    {
        std::vector<persistent_stack> stacks;
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            const std::string file_name = stacks_path + "/stack-" + std::to_string(i);
            stacks.emplace_back(file_name, true);
        }
        std::vector<std::thread> threads;
        for (uint32_t i = 0; i < number_of_threads; ++i)
        {
            std::function<void()> thread_action = [&stacks, i]()
            {
                thread_local_non_owning_storage<persistent_stack>::ptr =
                        &stacks[i];
                sleep(2);
                persistent_stack* stack_ptr =
                        thread_local_non_owning_storage<persistent_stack>::ptr;
                std::string msg = "Thread " + std::to_string(i) + ", ptr = " +
                                  std::to_string((long long) stack_ptr->get_stack_ptr());
                std::cout << msg << std::endl;
                ram_stack read_ram_stack = read_stack(*stack_ptr);
                thread_local_stack_storage::set_stack(read_ram_stack);
                sleep(2);
                ram_stack ram_stack_from_mem = thread_local_stack_storage::get_stack();
                std::string stack_content = "Thread " + std::to_string(i) +
                                            ", stack size = " +
                                            std::to_string(ram_stack_from_mem.size()) + "\n";
                while (!ram_stack_from_mem.empty())
                {
                    positioned_frame cur_frame = ram_stack_from_mem.top();
                    ram_stack_from_mem.pop();
                    stack_content += "Frame: position = " + std::to_string(cur_frame.position) +
                                     "\n";
                    stack_content += "function_name = " + cur_frame.frame.function_name + "\n";
                    stack_content += "args:\n";
                    for (uint8_t cur_arg: cur_frame.frame.args)
                    {
                        stack_content += std::to_string((int) cur_arg) + " ";
                    }
                    stack_content += "\n\n";
                }
                std::cout << stack_content;
            };
            threads.emplace_back(std::thread(thread_action));
        }
        for (std::thread& cur_thread: threads)
        {
            cur_thread.join();
        }
        std::cout << "Master thread exiting" << std::endl;
    }


    return 0;
}