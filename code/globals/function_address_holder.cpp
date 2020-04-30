//
// Created by ilya on 28.04.2020.
//

#include "function_address_holder.h"

std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>> function_address_map =
        std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>();