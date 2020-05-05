#include "pmem_utils.h"
#include <libpmem.h>

// TODO: remove dependency from PMDK using msync(2)
void pmem_do_flush(const void* ptr, size_t len)
{
    pmem_msync(ptr, len);
}
