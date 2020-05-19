#ifndef DIPLOM_RESTORATION_H
#define DIPLOM_RESTORATION_H

#include "../persistent_memory/persistent_memory_holder.h"

/**
 * Runs restoration procedure - traverses persistent stack from the end to the beginning,
 * running recovery operation for each frame of persistent stack.
 * Since, according to the system architecture, first frame of persistent stack should correspond
 * to a function, that doesn't read or write NVRAM and never returns, recovery function
 * for the first frame isn't called.
 * Persistent stack should contain at least one frame. If persistent stack doesn't contain
 * any frames, behaviour of function is undefined.
 * Restoration can be started if only system is running in recovery mode.
 * @param persistent_stack - object, that holds file with persistent stack.
 * @throws std::runtime error - if system is not running in restoration mode
 */
void do_restoration(persistent_memory_holder& persistent_stack);

#endif //DIPLOM_RESTORATION_H
