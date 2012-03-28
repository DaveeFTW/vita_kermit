#define REAL_ADDRESS(x)			((u32)x & 0x0FFFFFFF)
#define PHYSICAL_ADDRESS(x)		((u32)x & 0x03FFFFFF)
#define KERMIT_ADDRESS(x, io)	(((u32)x + 0xAA000000 + io) & 0x9FFFFFFF)

void sceKermitMemory_driver_90B662D0(u8 *data, u32 data_size)
{
	/* remove access flags */
	u32 io_base = 0;
	u32 data_addr = REAL_ADDRESS(data);
	
	/* check if in kernel memory (or lower) */
	if (data_addr < 0x04800000)
	{
		/* ensure not below bottom 80 KB (0x04000000 -> 0x04014000)  */
		if (data_addr >= 0x00014000) // +80KB?
		{
			return;
		}
		
		/* set base */
		io_base = 0x00400000;
	}
	else
	{
		/* kernel land */
		io_base = 0x00200000;
	}
	
	/* get the kermit address */
	u32 kermit_addr = KERMIT_ADDRESS(PHYSICAL_ADDRESS(data), io_base);
	
	/* invalidate data cache */
	sceKernelDcacheInvalidateRange(kermit_addr, data_size);
	
	/* if this is < 0x3000 bytes, dont bother with DMA. Just copy */
	if (data_size < 0x3000)
	{
		/* copy data */
		memmove(data, kermit_addr, data_size);
	}
	
	/* else we do a DMA copy */
	else
	{
		/* if our data is not block aligned */
		if (data_size % 64)
		{
			/* calculate the size needed to align */
			u32 align_size = 64 - (data_size % 64);
			
			/* copy over first part */
			memmove(data, kermit_addr, align_size);
			sceKernelDcacheWritebackRange(data, align_size);
			
			/* update pointers */
			data += align_size;
			kermit_addr += align_size;
			data_size -= align_size;
		}
		
		/* copy data using fast DMA */
		sceDmacMemcpy(data, kermit_addr, data_size);
	}
	
	/* ensure our data is written to memory */
	sceKernelDcacheWritebackRange(data, data_size);
}

void sceKermitMemory_driver_80E1240A(u8 *data, u32 data_size)
{
	/* remove access flags */
	u32 io_base = 0;
	u32 data_addr = REAL_ADDRESS(data);
	
	/* check if in kernel memory (or lower) */
	if (data_addr < 0x04800000)
	{
		/* ensure not below bottom 80 KB (0x04000000 -> 0x04014000)  */
		if (data_addr >= 0x00014000) // +80KB?
		{
			return;
		}
		
		/* set base */
		io_base = 0x00400000;
	}
	else
	{
		/* kernel land */
		io_base = 0x00200000;
	}
	
	/* get the kermit address */
	u32 kermit_addr = KERMIT_ADDRESS(PHYSICAL_ADDRESS(data), io_base);
	
	/* if this is < 0x3000 bytes, dont bother with DMA. Just copy */
	if (data_size < 0x3000)
	{
		/* copy data */
		memmove(kermit_addr, data, data_size);
		
		/* cpu based copy, ensure cache is written back */
		sceKernelDcacheWritebackInvalidateRange(kermit_addr, data_size);
	}
	
	/* else we do a DMA copy */
	else
	{
		/* if our data is not block aligned */
		if (data_size % 64)
		{
			/* calculate the size needed to align */
			u32 align_size = 64 - (data_size % 64);
			
			/* copy over first part */
			memmove(kermit_addr, data, align_size);
			
			/* write back cache to memory */
			sceKernelDcacheWritebackInvalidateRange(kermit_addr, align_size);
			
			/* update pointers */
			data += align_size;
			kermit_addr += align_size;
			data_size -= align_size;
		}
		
		/* writeback data cache for upto date values in memory */
		sceKernelDcacheWritebackRange(data, data_size);
		
		/* copy data using fast DMA */
		sceDmacMemcpy(kermit_addr, data, data_size);
	}
}

void sceKermitMemory_driver_AAF047AC(u8 *packet, u32 argc, u8 *buffer, u32 size, u32 io_mode)
{
	/* check if input flag is passed */
	if (io_mode & KERMIT_INPUT_MODE)
	{
		/* tell kermit to allocate this data in Vita RAM */
		sceKermitMemory_driver_80E1240A(buffer, size);
	}
	
	/* check argument count */
	if (argc < KERMIT_MAX_ARGC)
	{
		/* store active buffer */
		((u32 *)(packet + (argc * 8) + 0x10) = buffer;
	}
}

