#ifndef _POKE_H_
#define _POKE_H_

#define IO_DEPTH_8BIT   1
#define IO_DEPTH_16BIT  2
#define IO_DEPTH_32BIT  4
#define MMAP_LENGTH     256

int poke_mem(unsigned int addr, unsigned int data, int io_depth);

#endif /* _POKE_H_ */
