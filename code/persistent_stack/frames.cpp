#include "frames.h"
#include "../common/pmem_utils.h"

uint64_t get_frame_size(const stack_frame& frame)
{
    // TODO: make lengths uint16_t
    /*
     * 8 bytes for answer
     * 8 bytes of function name size
     * function name
     * 8 bytes of arguments size
     * arguments
     * 1 byte of end marker
     */
    return frame.args.size() + frame.function_name.size() + 25;
}
