//
// Created by ilya on 28.04.2020.
//

#ifndef DIPLOM_FUNCTION_ADDRESS_HOLDER_H
#define DIPLOM_FUNCTION_ADDRESS_HOLDER_H

#include <unordered_map>
#include <string>
#include <utility>

class function_address_holder
{
    static std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>> function_address_map;
};

#endif //DIPLOM_FUNCTION_ADDRESS_HOLDER_H
