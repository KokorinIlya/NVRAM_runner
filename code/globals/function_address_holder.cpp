//
// Created by ilya on 28.04.2020.
//

#include "function_address_holder.h"

std::unordered_map<std::string, std::pair<function_ptr, function_ptr>> function_address_holder::functions =
        std::unordered_map<std::string, std::pair<function_ptr, function_ptr>>();