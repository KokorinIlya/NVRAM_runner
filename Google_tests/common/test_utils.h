#ifndef DIPLOM_TEST_UTILS_H
#define DIPLOM_TEST_UTILS_H

#include <string>

std::string get_temp_file_name(std::string const& path_prefix);

struct temp_file
{
    std::string file_name;
    bool valid;

    explicit temp_file(std::string name);

    temp_file(const temp_file& other) = delete;

    temp_file& operator=(temp_file const& other) = delete;

    temp_file(temp_file&& other) noexcept;

    ~temp_file();
};

#endif //DIPLOM_TEST_UTILS_H
