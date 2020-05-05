#include <iostream>
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


    return 0;
}