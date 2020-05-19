#include "constants_and_types.h"
#include <unistd.h>

const uint32_t PMEM_STACK_SIZE = 2048;

const uint64_t PMEM_HEAP_SIZE = 2L * 1024L * 1024L;

const uint32_t CACHE_LINE_SIZE = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

const uint32_t PAGE_SIZE = getpagesize();

const uint8_t STACK_END_MARKER = 0x0;

const uint8_t FRAME_END_MARKER = 0x1;