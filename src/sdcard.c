/*
 * sdcard.c
 *
 *  Created on: Sep 25, 2015
 *      Author: RHolal
 */

#include "rw_utils.h"

#define DRX62X_GPIO1_BASE				0x4804C000
#define PINCTRL_BASE_ADDR               0x48140800
#define PIN_OFFSET(x)   				(4 * ((x) - 1))
#define INTERRUPT_PIN(gpiopin) 			(gpiopin)
#define PINCTRL_MASK                    0x000F07FF
/* Pull-up settings */
#define PULLUPEN_ENABLE       			0
#define PULLTYPESEL_UP                	( 1 << 17 )
#define SLEW_SLOW                    	( 1 << 19 )

#define GPIO_OE              			0x134
#define GPIO_DATAOUT         			0x13c
#define GPIO_RISINGDETECT    			0x148
#define GPIO_FALLINGDETECT   			0x14c
#define GPIO_LEVELDETECT0    			0x140
#define GPIO_LEVELDETECT1    			0x144
#define DRA62X_GPX_MAP_MEM_SZ			1024

extern int gSigExit;

/**
* reads data input register value
*
* @param   vbase pointer to intr access structure
* @param   offs  register offset
* @return  register contents
*/
/*lint -e{950} inlining this function is not a must */
inline uint32_t
read32(const void *vbase, unsigned offs)
{
   if (vbase == NULL){
      return 0;
   }
   /*lint -save -esym(613,hw) do not recheck parameters of static functions */
   uint32_t data = *(uint32_t *)((unsigned char *)vbase + offs);
   return data;
   /*lint -restore */
}

/**
* writes 32 bit register value
*
* @param   hw    pointer to pps structure
* @param   offs  register offset
* @param   val   value to be written
*/
/*lint -e{950} inlining this function is not a must */
inline void
write32 (const void *vbase, unsigned offs, uint32_t val)
{
   /*lint -save -esym(613,hw) do not recheck parameters of static functions */
   *(uint32_t *)((unsigned char *)vbase + offs) = val;
   /*lint -restore */
}


/**
* sets a bit/bits in a 32 bit register
*
* @param   hw    pointer to pps structure
* @param   offs  register offset
* @param   msk   mask of bits to be set
* @return  register contents
*/
/*lint -e{950} inlining this function is not a must */
inline void
set32 (const void *vbase, unsigned offs, uint32_t msk)
{
   write32 (vbase, offs, read32 (vbase, offs) | msk);
}


/**
* clears a bit/bits in a 32 bit register
*
* @param   hw    pointer to HW pps structure
* @param   offs  register offset
* @param   msk   mask of bits to be cleared
* @return  register contents
*/
/*lint -e{950} inlining this function is not a must */
inline void
clr32 (const void *vbase, unsigned offs, uint32_t msk)
{
   write32 (vbase, offs, read32 (vbase, offs) & ~msk);
}


void InitSDCardHdlr ( void )
{
	 unsigned mode;
	 void*	vbase;
	 void*	vbase_pin;
	 ThreadCtl(_NTO_TCTL_IO, NULL);

	 if ((vbase_pin = mmap_device_memory(NULL, (128 * 1024)/*512*/, PROT_NOCACHE | PROT_READ | PROT_WRITE,
				 MAP_SHARED | MAP_PHYS, PINCTRL_BASE_ADDR)) == MAP_FAILED)
		 {
			 DBG_MSG("%s: mmap_device_memory() for PINCTL failed", __FUNCTION__);
			 return ;
		 }

	 if ((vbase = mmap_device_memory(NULL, DRA62X_GPX_MAP_MEM_SZ, PROT_NOCACHE | PROT_READ | PROT_WRITE,
			 MAP_SHARED | MAP_PHYS, DRX62X_GPIO1_BASE)) == MAP_FAILED)
	 {
		 DBG_MSG("%s: mmap_device_memory() for GPIO failed", __FUNCTION__);
		 return ;
	 }

	 //DBG_MSG("vbase_pin = %x, vbase = %x",vbase_pin, vbase);

	 mode = (0x080 | (1 << 17) | (1 << 18) | (1 << 19));
	 write32(vbase_pin,PIN_OFFSET(74),mode);
	 mode = (0x080 | (1 << 17) | (1 << 18)) ;
	write32(vbase_pin,PIN_OFFSET(80),mode);

#if 0
	*((uint32_t *)((unsigned char *)vbase_pin+0x924)) = 0x000e0080;
	delay(20);
	*((uint32_t *)((unsigned char *)vbase_pin+0x93c)) = 0x00060080;
	delay(20);
	*((uint32_t *)((unsigned char *)vbase+0x134)) = 0xbff7afd3;
	delay(20);
	*((uint32_t *)((unsigned char *)vbase+0x148)) = 0x00000050;
	delay(20);
	*((uint32_t *)((unsigned char *)vbase+0x14c)) = 0x00000040;
	delay(20);
	*((uint32_t *)((unsigned char *)vbase+0x13c)) = 0x4008100c;
	delay(100);
#else
	write32(vbase,0x134,0xbff7afd3);
	write32(vbase,0x148,0x00000050);
	write32(vbase,0x14c,0x00000040);
	write32(vbase,0x13c,0x4008100c);

#endif
	munmap_device_memory(vbase,DRA62X_GPX_MAP_MEM_SZ);
	munmap_device_memory(vbase_pin,(128 * 1024));

	return;
}

