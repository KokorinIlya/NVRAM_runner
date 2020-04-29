#include <iostream>
#include "code/persistent_stack/call.h"

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cout << "run: ./Diplom <mode> <number of threads> <path to directory with stacks>" << std::endl;
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
    }
    else
    {
        // TODO
    }


    return 0;
}