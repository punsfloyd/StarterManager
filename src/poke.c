#include <hw/inout.h>
#include "rw_utils.h"
#include "poke.h"

/**
*  Does memory-mapped IO using MMAP
*  @param addr Address where data must be written
*  @param data Data to be written to the port
*  @return none
*/
int poke_mem(unsigned int addr, unsigned int data, int io_depth)
{
    // Map the address we want to write to.
    void* pMapped = mmap_device_memory(0, MMAP_LENGTH, PROT_READ | PROT_WRITE, 0, addr);

    // Check if the mapping has failed.
    if (pMapped == MAP_FAILED)
    {
        DBG_MSG("mmap failed for %d bytes at 0x%x, ", MMAP_LENGTH, addr);
        return -1;
    }

    // Chek the bus width to use
    switch (io_depth)
    {
    case IO_DEPTH_8BIT:
        *((unsigned char*) pMapped) = data;
        break;

    case IO_DEPTH_16BIT:
        *((unsigned short*) pMapped) = data;
        break;

    case IO_DEPTH_32BIT:
        *((unsigned int*) pMapped) = data;
        break;

    default:
        *((unsigned int*) pMapped) = data;
        break;
    }

    // Unmap the address
    munmap_device_memory(pMapped, MMAP_LENGTH);

    DBG_MSG("poke_mem: address [0x%x], value [0x%x]", addr, data);

    return 0;
}
