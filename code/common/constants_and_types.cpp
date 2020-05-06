#include "constants_and_types.h"
#include <unistd.h>

const uint32_t PMEM_STACK_SIZE = 2048;

const uint32_t CACHE_LINE_SIZE = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

const uint32_t PAGE_SIZE = getpagesize();