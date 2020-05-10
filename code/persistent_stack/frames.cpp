#include "frames.h"
#include "../common/pmem_utils.h"

uint64_t get_frame_size(const stack_frame& frame)
{
    /*
     * 8 bytes for answer
     * 2 bytes of function name size
     * function name
     * 2 bytes of arguments size
     * arguments
     * 1 byte of end marker
     */
    return frame.args.size() + frame.function_name.size() + 13;
}
