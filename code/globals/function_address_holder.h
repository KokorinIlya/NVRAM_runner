#ifndef DIPLOM_FUNCTION_ADDRESS_HOLDER_H
#define DIPLOM_FUNCTION_ADDRESS_HOLDER_H

#include <unordered_map>
#include <string>
#include <utility>
#include "../common/constants_and_types.h"

/**
 * Holds mapping from function name to pair - address of the function and address of the
 * recover version of the function.
 * Since there should be only one such mapping in the system, it is stored as a static hash map.
 * All functions, that will be called during the execution, should be registered in the map.
 * Since the map is not protected by any lock, it should be correctly initialized from a single process
 * to be used in a multi-threading environment.
 * For example, it can be initialized using next sequence of steps:
 * <ul>
 *  <li>
 *      From a master thread, add all functions to the map
 *  </li>
 *  <li>
 *      Spawn N worker threads, each of which has read-only access to the map
 *  </li>
 * </ul>
 * Since there is happens-before relation between spawn of a thread and first action in the spawned thread,
 * worker threads can read from map correctly without locking. Note, that after worker threads spawn,
 * all threads in the system are forbidden to modify the map.
 *
 * Map is stored in the RAM and should be initialized after each system restart.
 */
struct function_address_holder
{
    /**
     * Global map, containing address of Op and Op.Recover for every function in the system.
     */
    static std::unordered_map<std::string, std::pair<function_ptr, function_ptr>> functions;
};

#endif //DIPLOM_FUNCTION_ADDRESS_HOLDER_H
