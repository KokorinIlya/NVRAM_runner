#ifndef DIPLOM_TEST_UTILS_H
#define DIPLOM_TEST_UTILS_H

#include <string>

std::string get_temp_file_name(std::string const& path_prefix);

struct temp_file
{
    std::string file_name;

    explicit temp_file(std::string name);

    ~temp_file();
};

#endif //DIPLOM_TEST_UTILS_H
