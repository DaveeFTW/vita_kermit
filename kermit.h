#ifndef __KERMIT_H__
#define __KERMIT_H__

typedef struct
{
	u32 cmd;			//0x0
	SceUID sema;		//0x4
	KermitPacket *self;	//0x8
	u32 unk_C;			//0xC
} KermitPacket;

#define KERMIT_MAX_ARGC		(13)

/*
	Issue a command to kermit.
	
	packet:				a kermit packet. Header followed by 64 bit words (LE) as arguements.
	cmd_mode:			a valid command mode type.
	cmd:				a valid command subtype of cmd_mode.
	argc:				the number of 64 bit arguements following the header. Max 13 arguements.
	allow_callback:		set non-zero to use callback permitting semaphore wait.
	resp:				64 bit word returned by the kermit call.
	
	returns 0 on success, else < 0 on error.
*/
int sceKermit_driver_4F75AA05(KermitPacket *packet, u32 cmd_mode, u32 cmd, u32 argc, u32 allow_callback, u64 *resp)

#define KERMIT_INPUT_MODE	(1)
#define KERMIT_OUTPUT_MODE	(2)

/*
	Apply IO to kermit packet.
	
	packet:				a kermit packet. Header followed by 64 bit words (LE) as arguements.
	argc:				the number of arguements in the packet. Max 13 arguements.
	buffer:				the input buffer containing the data to be sent or the output buffer to store data.
	buffer_size:		the size of the input data, else the size of the output buffer.
	io_mode:			KERMIT_INPUT_MODE for data input. KERMIT_OUTPUT_MODE for expecting output data.
*/
void sceKermitMemory_driver_AAF047AC(KermitPacket *packet, u32 argc, u8 *buffer, u32 buffer_size, u32 io_mode);

/*
	Send data to vita host.
	
	data:				pointer to the data to be sent to host.
	len:				the size of the data to be sent.
*/
void sceKermitMemory_driver_80E1240A(u8 *data, u32 len);

/*
	Recieve data from vita host.
	
	data:				pointer to buffer to store output data.
	len:				the size of the expected output data.
*/
void sceKermitMemory_driver_90B662D0(u8 *data, u32 data_size);

/* Kermit command modes */
#define KERMIT_MODE_MSFS		(0x3)
#define KERMIT_MODE_FLASHFS		(0x4)
#define KERMIT_MODE_AUDIO		(0x5)
#define KERMIT_MODE_LOWIO		(0x7)
#define KERMIT_MODE_PERIPHERAL	(0x9)
#define KERMIT_MODE_WLAN		(0xA)
#define KERMIT_MODE_USB			(0xC)
#define KERMIT_MODE_UTILITY		(0xD)

/* kermit KERMIT_MODE_PERIPHERAL commands */
#define KERMIT_CMD_RTC_GET_CURRENT_TICK		(0x0)
#define KERMIT_CMD_ID_STORAGE_LOOKUP		(0x1)
#define KERMIT_CMD_POWER_FREQUENCY			(0x2)
#define KERMIT_CMD_AUDIO_ROUTING			(0x3)
#define KERMIT_CMD_GET_CAMERA_DIRECTION		(0x5)
#define KERMIT_CMD_GET_IDPSC_ENABLE			(0x6)
#define KERMIT_CMD_REQUEST_SUSPEND			(0xB)
#define KERMIT_CMD_IS_FIRST_BOOT  			(0xC)

/* kermit KERMIT_MODE_MSFS commands */
#define KERMIT_CMD_INIT_MS   			(0x0)
#define KERMIT_CMD_EXIT_MS   			(0x1)
#define KERMIT_CMD_OPEN_MS   			(0x2)
#define KERMIT_CMD_CLOSE_MS   			(0x3)
#define KERMIT_CMD_READ_MS   			(0x4)
#define KERMIT_CMD_WRITE_MS   			(0x5)
#define KERMIT_CMD_SEEK_MS   			(0x6)
#define KERMIT_CMD_IOCTL_MS   			(0x7)
#define KERMIT_CMD_REMOVE_MS   			(0x8)
#define KERMIT_CMD_MKDIR_MS   			(0x9)
#define KERMIT_CMD_RMDIR_MS   			(0xA)
#define KERMIT_CMD_DOPEN_MS   			(0xB)
#define KERMIT_CMD_DCLOSE_MS   			(0xC)
#define KERMIT_CMD_DREAD_MS   			(0xD)
#define KERMIT_CMD_GETSTAT_MS   		(0xE)
#define KERMIT_CMD_CHSTAT_MS   			(0xF)
#define KERMIT_CMD_RENAME_MS   			(0x10)
#define KERMIT_CMD_CHDIR_MS   			(0x11)
#define KERMIT_CMD_DEVCTL   			(0x14)

/* kermit KERMIT_MODE_UTILITY commands */
#define KERMIT_CMD_OSK_START   (0x0)
#define KERMIT_CMD_OSK_SHUTDOWN   (0x1)
#define KERMIT_CMD_UTIL_UNK3   (0x3)

#endif /* __KERMIT_H__ */
