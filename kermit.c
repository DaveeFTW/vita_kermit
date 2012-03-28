typedef struct
{
	u32 cmd;			//0x0
	SceUID sema;		//0x4
	KermitPacket *self;	//0x8
	u32 unk_C;			//0xC
} KermitPacket;

int sub_00000A98(u8 *packet, u32 packet_size)
{
	/* remove access flags */
	u32 io_base = 0x00200000;
	u32 packet_addr = REAL_ADDRESS(packet);

	/* check if in kernel memory (or lower) */
	if (packet_addr < 0x04800000)
	{
		/* ensure not below bottom 80 KB (0x04000000 -> 0x04014000)  */
		if (data_addr >= 0x00014000) // +80KB?
		{
			/* error? */
			return packet;
		}
		
		/* set base */
		io_base = 0x00400000;
	}
	
	/* copy data to kermit io */
	u32 kermit_addr = KERMIT_ADDRESS(PHYSICAL_ADDRESS(packet), io_base);
	
	/* copy the data, and flush the cache back to RAM */
	memmove(kermit_addr, packet, packet_size);
	sceKernelDcacheWritebackInvalidateRange(kermit_addr, packet_size);
	
	/* return address to kermit memory */
	return kermit_addr;
}

typedef struct
{
	u32 cmd_num;
	u32 kermit_addr;
} kermitCommand;
	
static kermitCommand *g_command = (kermitCommand *)0xBFC008000;

int sceKermit_driver_4F75AA05(u8 *data, u32 cmd_mode, u32 cmd, u32 argc, u32 allow_callback, u8 *resp)
{
	/* check if we are not accepting kermit calls */
	if (!g_enable_kermit)
	{
		/* wait 10ms */
		sceKernelDelayThread(10 * 1000);
		return 0;
	}
	
	/* lock the mutex, no timeout  */
	sceKernelLockMutex(g_mutex_id, 1, NULL);
	
	/* update counter */
	g_active_connections++;
	
	/* release the mutex */
	sceKernelUnlockMutex(g_mutex_id, 1);
	
	/* use specific ID on modes KERMIT_MODE_AUDIO and mode 6. This is to improve parallelism and async */
	if 		(cmd_mode == KERMIT_MODE_AUDIO) proc_n = 1;
	else if (cmd_mode == 6)					proc_n = 2;
	else									proc_n = 0;
	
	/* construct sema timeout of 5 seconds */
	u32 timeout = 5 * 1000 * 1000;
	
	/* wait on sema */
	int res = sceKernelWaitSema(g_access_sema[proc_n], 1, &timeout);
	
	/* check if we error'd */
	if (res != 0)
	{
		/* go the the clean up code */
		goto exit;
	}
	
	/* read the message pipe */
	res = sceKernelReceiveMsgPipe(g_pipe_id, &sema_id, sizeof(SceUID), 0, 0, 0);
	
	/* check if error occured */
	if (res != 0)
	{
		/* error, clean up and exit */
		goto exit;
	}
	
	/* now set the command number */
	g_command[proc_n].cmd_type = (cmd_mode << 16) | cmd;
	
	/* DMA align the arg count. Max = 16 args */
	u32 packet_size = ((argc + sizeof(u64) + 1) & 0xFFFFFFF8) * sizeof(u64);
	
	/* store packet info */
	packet.cmd = cmd;
	packet.sema_id = sema_id;
	packet.self = packet;
	
	/* send data to kermit */
	g_command[proc_n].kermit_addr = sub_00000A98(packet, packet_size);
	
	/* wait? */
	sub_00000908();
	
	/* lock the power, prevent shutdown */
	res = sceKernelPowerLock(0);
	
	/* check if error occured */
	if (res != 0)
	{
		/* error, clean up and exit */
		goto exit;
	}
	
	/* suspend cpu interrupts */
	int intr = sceKernelCpuSuspendIntr();
	
	/* signal low, then high for the process */
	PIN_LOW(0xBC300050, proc_n + 4);
	PIN_HIGH(0xBC300050, proc_n + 4);
	
	/* resume the CPU interrupts */
	sceKernelCpuResumeIntr(intr);
	
	/* check for callback permitting process */
	if (allow_callback) sceKernelWaitSemaCB(sema_id, 1, NULL);
	else				sceKernelWaitSema(sema_id, 1, NULL);
	
	/* send sema id back into pipe, act as a circular queue */
	sceKernelSendMsgPipe(g_pipe_id, &sema_id, sizeof(SceUID), 0, 0, 0);
	
	/* now, check if there is a response */
	if (resp)
	{
		/* copy data from packet to response */
		((u64 *)resp)[0] = ((u64 *)packet)[0];
	}
	
	/* unlock power */
	res = sceKernelPowerUnlock(0);
	
	/* exit and cleanup code */
exit:
	/* lock mutex for exclusive access, no timeout */
	sceKernelLockMutex(g_mutex_id, 1, NULL);
	
	/* update counter */
	g_active_connections--;
	
	/* release the mutex */
	sceKernelUnlockMutex(g_mutex_id, 1);
	
	/* check result */
	if (res >= 0) res = 0;
	
	/* return result */
	return res;
}

int sceKermit_driver_9160841C(int pin_n, int allow_callbacks)
{
	/* lock the power source, no shutdown */
	int res = sceKernelPowerLock(0);
	
	/* check if valid */
	if (res == 0)
	{
		/* wait? */
		sub_00000908();
		
		/* suspend interrupts */
		int intr = sceKernelCpuSuspendIntr();
		
		PIN_HIGH(0xBC300038, pin_n);
		PIN_LOW(0xBC300050, pin_n);
		PIN_HIGH(0xBC300050, pin_n);
		
		/* resume all the interrupts */
		sceKernelCpuResumeIntr(intr);
		
		/* wait on work sema */
		if (allow_callbacks)	sceKernelWaitSemaCB(g_work_sema[pin_n], 1, NULL);
		else					sceKernelWaitSema(g_work_sema[pin_n], 1, NULL);
		
		/* unlock power */
		res = sceKernelPowerUnlock(0);
		
		/* resolve +ve res to 0 */
		if (res > 0) res = 0;
	}
	
	/* return the result */
	return res;
}
