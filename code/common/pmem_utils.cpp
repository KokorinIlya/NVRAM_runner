#include "pmem_utils.h"
#include <libpmem.h>

void pmem_do_flush(const void* ptr, size_t len)
{
    pmem_msync(ptr, len);
}
