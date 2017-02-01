/* ---------------------------------------------------------------------------------------------------------
 * Module : StarterManager
 * This utility controls the starting of packages in TML X104_HIGH system.
 * ---------------------------------------------------------------------------------------------------------*/

#include "rw_utils.h"
#include "starter_dcmd.hpp"
#include "starter_strings.h"
#include "config_defines.h"
#include <hw/inout.h>
#include <sys/pps.h>
#include "poke.h"
#include <unistd.h>
#include <fcntl.h>

/* ---------------------------------------------------------------------
 * CAUTION : This macro is applicable only for J5 MAN/SCANIA variants
 * 559 is no longer used, but let it be there for backward compatibility
 */
#define VARIANT_MAN(n)	(n == 555 || n == 556 )
#define VARIANT_SCANIA(n)	(n == 557 || n == 558 || n == 559 )
extern void InitSDCardHdlr ( void );

#define EOL_EN_TELNET		0x2
#define EOL_EN_USB_ETHERNET	0x4
#define PRINT_CONSOLE
#ifdef PRINT_CONSOLE
#define OutputToConsole(args...) fflush(stdout); printf("Line %d : ", __LINE__); printf("STMGR - "); printf(args); printf("\n"); fflush(stdout)
#else
#define OutputToConsole(args...) printf();
#endif

tStarterObjState	gObjState;
pthread_cond_t		gCondVar;
pthread_mutex_t     gMutexCond;

sMyBool				g_bSignalled 		= FALSE;
unsigned int		g_nPackagesLoaded 	= 0;
sMyBool				b_AntiTheft_state	= FALSE;

pthread_cond_t		gSWDLCondVar;
pthread_mutex_t		gSWDLMutexCond;

struct MS_bootInfo
{
	unsigned char bootImage;
	unsigned char screen;
	unsigned char system_lineIn;
	unsigned char defLedIllumination;
	unsigned char wakup_reason;
	unsigned char bolo;
	unsigned char lastMode;
	unsigned char error_reset_counter;
	unsigned char dab_usb_bt;
	unsigned char boardInfo_tft;
	unsigned char sample_info;
	unsigned char variant_info;
	unsigned char tuner_apple_dev;
	unsigned char system_sound;
	unsigned char checksum1;
	unsigned char checksum2;
};

enum E_DescriptionDataFromIOC
{
	IPCSTARTUP_BOOT_IMAGE,
	IPCSTARTUP_SPLASH_SCREEN,
	IPCSTARTUP_SYSTEM_LINEIN,
	IPCSTARTUP_DEF_LED_ILLUMINATION,
	IPCSTARTUP_WAKEUP_REASON,
	IPCSTARTUP_BOOTLOADER_INDICATION,
	IPCSTARTUP_PERSISTENT_OPERATION_MODE,
	IPCSTARTUP_ERROR_RESET_COUNTER,
	IPCSTARTUP_DAB_USB_BT,
	IPCSTARTUP_BOARD_INFO,
	IPCSTARTUP_SAMPLE_INFO,
	IPCSTARTUP_VARIANT_INFO,
	IPCSTARTUP_TUNER_APPLE_DEV,
	IPCSTARTUP_SYSTEM_SOUND,
	IPCSTARTUP_CHECKSUM_1,
	IPCSTARTUP_CHECKSUM_2
};


enum HwSample
{
	HW_SAMPLE_A2,
	HW_SAMPLE_B0,
	HW_SAMPLE_C0,
	HW_SAMPLE_C1,
	HW_SAMPLE_C1_5,
	HW_SAMPLE_C2,
	HW_SAMPLE_C3,
	HW_SAMPLE_D0,
	HW_SAMPLE_D1,
	HW_SAMPLE_D2,
	HW_SAMPLE_D3,
	HW_UNKNOWN
};

enum HwVariant
   {
   	A563,
   	A562,
   	A561,
   	A560,
   	A559,
   	A558,
   	A557,
   	A556,
   	A555,
   	HW_VARIANT_INVALID = 0xFF,
   };

/*WakeUp Reason Scania */
enum WUR_SCANIA
{
	SCANIA_WUR_UNKNOWN,
	SCANIA_WUR_BOOTLOADER,
	SCANIA_WUR_WATCHDOG_RESET,
	SCANIA_WUR_SYSTEM_RESET,
	SCANIA_WUR_ECU_RESET,
	SCANIA_WUR_FWK_ACTIVE,
	SCANIA_WUR_U15_ACTIVE,
	SCANIA_WUR_CAM1_ACTIVE,
	SCANIA_WUR_WUP3_ACTIVE,
	SCANIA_WUR_STP,
};

/*WakeUp Reason MAN */
enum WUR_MAN
{
	MAN_WUR_UNKNOWN,
	MAN_WUR_BOOTLOADER,
	MAN_WUR_WATCHDOG_RESET,
	MAN_WUR_SYSTEM_RESET,
	MAN_WUR_ECU_RESET,
	MAN_WUR_FWK_ACTIVE,
	MAN_WUR_U15_ACTIVE,
	MAN_WUR_CAM1_ACTIVE,
	MAN_WUR_CAM2_ACTIVE,
	MAN_WUR_STP,
};

enum MAN_LOGO
{
	MAN_DEF_LOGO,
	MAN_TRUCK_LOGO,
	MAN_NEOPLAN_BUS_LOGO,
	MAN_BUS_LOGO,
	MAN_TRUCK_LOGO_2018,
	MAN_TRUCK_IAA,
	RESERVED2,
	RESERVED3,
};

enum LAST_MODE
{
	LM_TUNER,
	LM_MEDIA,
	LM_AUX,
	LM_BT,
};
int gSigExit = 0;
int System_DAB = 0;
int System_SoundOutput = 0;
int System_USB1 = 0;
int System_Bluetooth = 0;
int System_Bluetooth_ph1 = 0;
int System_Bluetooth_ph2 = 0;
int Function_BTRemote = 0;
int System_TunerFrequencySettings = 0;
int System_Function_AppleDeviceSupport =0;
int System_Debug10 = 0;
int System_AM_FM_FM2 = 1;
int System_SoundOptimizationSettings = 0;
int g_SystemLineIn_1 = 0;
int g_SystemLineIn_2 = 0;
int System_SpeakerConfiguration = 0;
unsigned int 		g_nlastMode 	= LM_TUNER;
sMyBool b_rvcengage = FALSE;
/* EOL Parameters Default Value */
int g_CarPlayAvailability = 1;		/* Default */
int g_GPSAntennaAvailable = 1;		/* Default */
int g_RVCSupported = 0;				/* Default */
int g_VoiceAlertFeature = 1;		/* Default */

int BT_ID[] = {0,0,0,0,0,0};
int oemvariant = 4;

extern int MAN_Bluetooth_Arguments(void);
extern int Scania_Bluetooth_Arguments(void);
extern void System_BT_ID ( void );

char audiocfg_files[][73] =
{
"AudioCtrlSvc_0.conf"	,
"AudioCtrlSvc_1.conf"	,
"AudioCtrlSvc_2.conf"	,
"AudioCtrlSvc_3.conf"	,
"AudioCtrlSvc_4.conf"	,
"AudioCtrlSvc_5.conf"	,
"AudioCtrlSvc_6.conf"	,
"AudioCtrlSvc_7.conf"	,
"AudioCtrlSvc_8.conf"	,
"AudioCtrlSvc_9.conf"	,
"AudioCtrlSvc_10.conf"	,
"AudioCtrlSvc_11.conf"	,
"AudioCtrlSvc_12.conf"	,
"AudioCtrlSvc_13.conf"	,
"AudioCtrlSvc_14.conf"	,
"AudioCtrlSvc_15.conf"	,
"AudioCtrlSvc_16.conf"	,
"AudioCtrlSvc_17.conf"	,
"AudioCtrlSvc_18.conf"	,
"AudioCtrlSvc_19.conf"	,
"AudioCtrlSvc_20.conf"	,
"AudioCtrlSvc_21.conf"	,
"AudioCtrlSvc_22.conf"	,
"AudioCtrlSvc_23.conf"	,
"AudioCtrlSvc_24.conf"	,
"AudioCtrlSvc_25.conf"	,
"AudioCtrlSvc_26.conf"	,
"AudioCtrlSvc_27.conf"	,
"AudioCtrlSvc_28.conf"	,
"AudioCtrlSvc_29.conf"	,
"AudioCtrlSvc_30.conf"	,
"AudioCtrlSvc_31.conf"	,
"AudioCtrlSvc_32.conf"	,
"AudioCtrlSvc_33.conf"	,
"AudioCtrlSvc_34.conf"	,
"AudioCtrlSvc_35.conf"	,
"AudioCtrlSvc_36.conf"	,
"AudioCtrlSvc_37.conf"	,
"AudioCtrlSvc_38.conf"	,
"AudioCtrlSvc_39.conf"	,
"AudioCtrlSvc_40.conf"	,
"AudioCtrlSvc_41.conf"	,
"AudioCtrlSvc_42.conf"	,
"AudioCtrlSvc_43.conf"	,
"AudioCtrlSvc_44.conf"	,
"AudioCtrlSvc_45.conf"	,
"AudioCtrlSvc_46.conf"	,
"AudioCtrlSvc_47.conf"	,
"AudioCtrlSvc_48.conf"	,
"AudioCtrlSvc_49.conf"	,
"AudioCtrlSvc_50.conf"	,
"AudioCtrlSvc_51.conf"	,
"AudioCtrlSvc_52.conf"	,
"AudioCtrlSvc_53.conf"	,
"AudioCtrlSvc_54.conf"	,
"AudioCtrlSvc_55.conf"	,
"AudioCtrlSvc_56.conf"	,
"AudioCtrlSvc_57.conf"	,
"AudioCtrlSvc_58.conf"	,
"AudioCtrlSvc_59.conf"	,
"AudioCtrlSvc_60.conf"	,
"AudioCtrlSvc_61.conf"	,
"AudioCtrlSvc_62.conf"	,
"AudioCtrlSvc_63.conf"	,
"AudioCtrlSvc_64.conf"	,
"AudioCtrlSvc_65.conf"	,
"AudioCtrlSvc_66.conf"	,
"AudioCtrlSvc_67.conf"	,
"AudioCtrlSvc_68.conf"	,
"AudioCtrlSvc_69.conf"	,
"AudioCtrlSvc_70.conf"	,
"AudioCtrlSvc_71.conf"	,
"AudioCtrlSvc_72.conf"
};

void funTokenVal(int *d, char *s, int len)
{
	const char x[2] = ",";
	char *str;
	char *token = NULL;
	int i = 0;

	str = strchr(s,'[');
	if(str == NULL){
		return;
	}
	str++;
	if(str != NULL) {
		token = strtok(str, x);
	}
	while( token != NULL ){
		if(i < len){
			d[i] = atoi(token);
			i++;
			token = strtok(NULL, x);
		}
	}
	return;
}

int readEOLParameters()
{
	char* pData = NULL;
	char  ppsBuffer[5120];
	int   fdEol;
	int   nRead;

	pps_status_t status;
	pps_attrib_t info;

	// Open the EOL settigns.
	fdEol = open("/pps/Diag/EOL", O_RDONLY);

	// Check if open failed.
	if (fdEol < 0)
	{
		DBG_MSG("Error opening /pps/Diag/EOL");
		return -1;
	}

	// Read the entire EOL settings at once.
	nRead = read(fdEol, (void*) ppsBuffer, sizeof(ppsBuffer) - 1);

	// Check for error.
	if (nRead <= 0)
	{
		close(fdEol);
		return -1;
	}

	// End with null char.
	ppsBuffer[nRead] = '\0';
	pData = ppsBuffer;

	// Clear the pps node info struct.
	memset(&info, 0, sizeof(info));

	// Go through all EOL parameters.
	// Look for the following.
	// 	1. System_Line_in1
	//	2. System_Line_in2
	while ((status = ppsparse(&pData, NULL, NULL, &info, 0)) != PPS_END)
	{
		if (status == PPS_ATTRIBUTE)
		{
			if (!strcmp(info.attr_name, "System_Line_in1"))
			{
				// Save AUX1 setting.
				g_SystemLineIn_1 = atoi(info.value);
			}
			else if (!strcmp(info.attr_name, "System_Line_in2"))
			{
				// Save AUX2 setting.
				g_SystemLineIn_2 = atoi(info.value);
			}
            else if (!strcmp(info.attr_name, "Debug10"))
            {
				System_Debug10 = atoi(info.value);
            }
			else if(! strcmp(info.attr_name, "BT_ID"))
			{
				funTokenVal(&BT_ID[0], info.value, 6);
				DBG_MSG("BT_ID: Parameter1:: %d Parameter2:: %d Parameter3:: %d Parameter4:: %d Parameter5:: %d Parameter6:: %d", BT_ID[0],BT_ID[1],BT_ID[2],BT_ID[3],BT_ID[4],BT_ID[5]);
			}
 		}
	}

	// Close pps handler.
	close(fdEol);

	return 0;
}


static const char * sample_to_string(unsigned int id_sample)
{
	static const char* const SAMPLE[] =
	{
			[HW_SAMPLE_A2] = "A2",
			[HW_SAMPLE_B0] = "B0",
			[HW_SAMPLE_C0] = "C0",
			[HW_SAMPLE_C1] = "C1",
			[HW_SAMPLE_C1_5] = "C1_5",
			[HW_SAMPLE_C2] = "C2",
			[HW_SAMPLE_C3] = "C3",
			[HW_SAMPLE_D0] = "D0",
			[HW_SAMPLE_D1] = "D1",
			[HW_SAMPLE_D2] = "D2",
			[HW_SAMPLE_D3] = "D3",
			[HW_UNKNOWN] = "Unknown"
	};

	if(id_sample < HW_UNKNOWN)
	{
		return SAMPLE[id_sample];
	}
	else
	{
		return SAMPLE[HW_UNKNOWN];
	}
}

static inline sMyBool get_variant_from_bm_info(unsigned long address, struct MS_bootInfo *ms_boot_info)
{
	void * ptr = mmap_device_memory (0,16,PROT_READ,0,address);


	if(ptr)
	{
		unsigned int *iptr = (unsigned int *)ptr;
		DBG_MSG("IOCREAD : %x %x %x %x", iptr[0], iptr[1], iptr[2], iptr[3]);

		if(	((0x334e == iptr[0]) || (0 == iptr[0])) && (0 == iptr[1]) && (0 == iptr[2]) && (0 == iptr[3]))
		{
			DBG_MSG("Invalid 16 Byte BootInfo received, Restarting in APP mode again.");
			return FALSE;
		}

		char * bootm = (char*) ptr;
		ms_boot_info->bootImage				= bootm[IPCSTARTUP_BOOT_IMAGE];
		ms_boot_info->screen 				= bootm[IPCSTARTUP_SPLASH_SCREEN];
		ms_boot_info->system_lineIn 		= bootm[IPCSTARTUP_SYSTEM_LINEIN];
		ms_boot_info->defLedIllumination 	= bootm[IPCSTARTUP_DEF_LED_ILLUMINATION];
		ms_boot_info->wakup_reason 			= bootm[IPCSTARTUP_WAKEUP_REASON] & 0x7f;
		ms_boot_info->bolo 					= bootm[IPCSTARTUP_BOOTLOADER_INDICATION];
		ms_boot_info->lastMode 				= bootm[IPCSTARTUP_PERSISTENT_OPERATION_MODE];
		ms_boot_info->error_reset_counter 	= bootm[IPCSTARTUP_ERROR_RESET_COUNTER];
		ms_boot_info->dab_usb_bt 			= bootm[IPCSTARTUP_DAB_USB_BT];
		ms_boot_info->boardInfo_tft 		= bootm[IPCSTARTUP_BOARD_INFO];
		ms_boot_info->sample_info 			= bootm[IPCSTARTUP_SAMPLE_INFO];
		ms_boot_info->variant_info 			= bootm[IPCSTARTUP_VARIANT_INFO];
		ms_boot_info->tuner_apple_dev 		= bootm[IPCSTARTUP_TUNER_APPLE_DEV];
		ms_boot_info->system_sound 			= bootm[IPCSTARTUP_SYSTEM_SOUND];
		ms_boot_info->checksum1 			= bootm[IPCSTARTUP_CHECKSUM_1];
		ms_boot_info->checksum2 			= bootm[IPCSTARTUP_CHECKSUM_2];

	}
	else
	{
		DBG_MSG("Unable to map memory:%lu for accessing BM_INFO", address);
		// No BootInfo available, restart to app mode as recovery.
		return FALSE;
	}

	return TRUE;
}

static inline int get_bm_info(const char * str, unsigned long *address, int *length )
{
	int res = 0;
	if( str && address && length) {
		if (sscanf(str,"IOC_BM_INFO=0x%lx,0x%x",address,length) != 2)
		{
			res = -1;
			DBG_MSG("Unable to parse IOC_BM_INFO");
		}
	} else {
		DBG_MSG("Invalid arguments");
		res = -1;
	}

	return res;
}

static inline const char * find_string(const char * data, int * skip_length)
{
	const char * str_start = data + 4; //skip first 4 bytes.
	if (skip_length) {
		*skip_length = strlen(str_start) + 1; //add the NULL character.
	}
	return str_start;
}

sMyBool variant_sample_ids_from_syspage(struct MS_bootInfo *ms_boot_info)
{
	char * raw = SYSPAGE_ENTRY(typed_strings)->data;
	int length = SYSPAGE_ENTRY_SIZE(typed_strings);
	int skip_length = 0;
	const char * str;
	const char * str_bm_info;
	char * iter = raw;

	if(length > 4)
	{
		while( (str = find_string(iter, &skip_length)) <  (raw + length)  )
		{
			if ((str_bm_info = strstr(str,"IOC_BM_INFO"))) {
					unsigned long address;
					int length;
					if(get_bm_info(str_bm_info,&address,&length))
					{
						DBG_MSG("get_bm_info failed");
						return FALSE;
					}
					else
					{
						DBG_MSG("Reading 16 Byte BootInfo from Address[0x%lx], Length[%d]", address, length);
						if((length <  BOOT_INFO_LENGTH) || (FALSE == get_variant_from_bm_info(address, ms_boot_info)))
						{
							return FALSE;
						}
					}
				}
			iter += skip_length;
		}
	}
	else
	{
		// Seems Syspage entry is corrupted, try restart in APP mode
		return FALSE;
	}

	return TRUE;
}

int executor (struct MS_bootInfo *ms_boot_info, int variantNum)
{
	char  		*scania = "Scania" ;
	char  		*man = "MAN" ;
	int 		variant = 4;
	int 		bolo = 0;
	char  		*oem = 0;
	FILE 		*fptr = 0;
	FILE 		*can_fptr = 0;
	FILE 		*mcNet_fptr = 0;
	FILE 		*bootinfo_fptr = 0;
	const char 	*sample_info = NULL;
	int 		SD1 = 1;
	int 		AUX1 = 0;
	int 		AUX2 = 0;
	int 		audio_mute = 1;

	// identify whether you are boot mode or application mode
	bolo = ms_boot_info->bolo & 0x01;

	fptr = fopen ("/tmp/illumination_args","w");
	if(fptr==NULL)
	{
		DBG_MSG("Error creating illumination args!");
		exit(1);
	}

	// You dont need a file operation here, remove it.
	if (VARIANT_MAN (variantNum))
	{
		oem 		= man;
		variant 	= 2;
		oemvariant 	= 2;
		fprintf(fptr,"-v %d -t MAN\n",variantNum);
	}
	else // defaulting it to scania, bad though
	{
		oem 		= scania;
		variant 	= 4;
		oemvariant 	= 4;
		fprintf(fptr,"-v %d -t Scania\n",variantNum);
	}

	fclose(fptr); fptr = NULL;

	if (oemvariant == 4)
	{
		System_DAB 							= (ms_boot_info->dab_usb_bt & 0x1);
		System_USB1 						= ((ms_boot_info->dab_usb_bt >> 1) & 0x1);
		System_Bluetooth 					= ((ms_boot_info->dab_usb_bt >> 2) & 0x1);
		System_Bluetooth_ph1 				= ((ms_boot_info->dab_usb_bt >> 3) & 0x3);
		System_Bluetooth_ph2 				= ((ms_boot_info->dab_usb_bt >> 5) & 0x3);
		Function_BTRemote 					= ((ms_boot_info->dab_usb_bt >> 7) & 0x1);
		System_TunerFrequencySettings 		= (ms_boot_info->tuner_apple_dev & 0xf);
		System_Function_AppleDeviceSupport 	= ((ms_boot_info->tuner_apple_dev >> 4) & 0x3);
		System_SoundOptimizationSettings 	= (ms_boot_info->system_sound & 0x7f);
		System_SoundOutput 					= ((ms_boot_info->tuner_apple_dev >> 6) & 0x3);
		System_SoundOutput					|= ((ms_boot_info->system_lineIn & 0x8) >> 0x1);
		System_SpeakerConfiguration			= ((ms_boot_info->system_lineIn >> 5) & 0x7);
	}
	else
	{
		System_DAB 							= (ms_boot_info->dab_usb_bt & 0x1);
		System_USB1 						= ((ms_boot_info->dab_usb_bt >> 1) & 0x1);
		System_Bluetooth 					= ((ms_boot_info->dab_usb_bt >> 2) & 0x1);
		System_Bluetooth_ph1 				= ((ms_boot_info->dab_usb_bt >> 3) & 0x3);
		System_Bluetooth_ph2 				= ((ms_boot_info->dab_usb_bt >> 5) & 0x3);
		Function_BTRemote 					= ((ms_boot_info->dab_usb_bt >> 7) & 0x1);
		System_Function_AppleDeviceSupport 	= ((ms_boot_info->tuner_apple_dev >> 4) & 0x3);
		System_TunerFrequencySettings 		= (ms_boot_info->tuner_apple_dev & 0xf);
		audio_mute 							= ((ms_boot_info->system_sound >> 7) & 0x1);;
		System_SoundOptimizationSettings 	= (ms_boot_info->system_sound & 0x7f);
		DBG_MSG("ms_boot_info->tuner_apple_dev = [0x%x]",ms_boot_info->tuner_apple_dev);
		DBG_MSG("System_Function_AppleDeviceSupport = [%d]",System_Function_AppleDeviceSupport);
	}

	bootinfo_fptr=fopen("/tmp/bootInfo","w");
	if(bootinfo_fptr==NULL)
	{
		DBG_MSG("Error creating /tmp/bootInfo!");
		exit(1);
	}
	sample_info = sample_to_string((unsigned int)ms_boot_info->sample_info);
	DBG_MSG("Sample Detection : [%s]", sample_info);

	fprintf(bootinfo_fptr,"%s%d\n","VARIANT::",variantNum);
	fprintf(bootinfo_fptr,"%s%s\n","SAMPLE::",sample_info);
	fprintf(bootinfo_fptr,"%s%d\n","LED_BRIGHT::",ms_boot_info->defLedIllumination);
	fprintf(bootinfo_fptr,"%s%d\n","DISP_BRIGHT::",(ms_boot_info->boardInfo_tft >> 1));
	fprintf(bootinfo_fptr,"%s%s\n","TARGET::",(ms_boot_info->boardInfo_tft & 1? "Scania":"MAN"));
	fprintf(bootinfo_fptr,"%s%d\n","BOLO::",(ms_boot_info->bolo & 1));
	fprintf(bootinfo_fptr,"%s%d\n","BOL_PIN::",((ms_boot_info->bolo >> 1) & 1));
	fprintf(bootinfo_fptr,"%s%d\n","SWUPDT_PROGRESS::",((ms_boot_info->bolo >> 2) & 1));
	fprintf(bootinfo_fptr,"%s%d\n","LAST_ON_STATE::",((ms_boot_info->bolo >> 3) & 1));
	fprintf(bootinfo_fptr,"%s%d\n","WAKEUP_REASON::",ms_boot_info->wakup_reason);
	fprintf(bootinfo_fptr,"%s%d\n","LAST_MODE::",ms_boot_info->lastMode);
	fprintf(bootinfo_fptr,"%s%d\n","ERR_RESET_CNT::",ms_boot_info->error_reset_counter);

	fclose(bootinfo_fptr);

	can_fptr=fopen("/tmp/can_args","w");
	if(can_fptr==NULL)
	{
		DBG_MSG("Error creating /tmp/can_args!");
		exit(1);
	}

	mcNet_fptr=fopen("/tmp/mcnet_args","w");
	if(mcNet_fptr==NULL)
	{
		DBG_MSG("Error creating /tmp/mcnet_args!");
		exit(1);
	}

	if (oemvariant == 4)
	{
		fprintf(can_fptr,"%s ","SCANIA J5ECO 12");
		fprintf(mcNet_fptr,"%s ","SCANIA J5ECO");
	}
	else
	{
		fprintf(can_fptr,"%s ","MAN J5ECO 12");
		fprintf(mcNet_fptr,"%s ","MAN J5ECO");
	}

	fptr=fopen("/tmp/audio_args","w");
	if(fptr==NULL)
	{
		DBG_MSG("Error creating /tmp/audio_args!");
		exit(1);
	}

	if (oemvariant == 4)
	{
		if (System_SoundOutput == 0)
		{
			if(System_SpeakerConfiguration >= 4)
			{
				fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000000,suppress,clockenable=1\n");
			}
			else
			{
				fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000008,suppress,clockenable=1\n");
			}
		}
		else if (System_SoundOutput == 1)
		{
			fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000001,suppress,clockenable=1\n");
		}
		else if (System_SoundOutput == 2)
		{
			fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000002,suppress,clockenable=1\n");
		}
		else if (System_SoundOutput == 3)
		{
			fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000003,suppress,clockenable=1\n");
		}
		else if (System_SoundOutput == 4)
		{
			fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000004,suppress,clockenable=1\n");
		}
		else if (System_SoundOutput == 5)
		{
			fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000005,suppress,clockenable=1\n");
		}
		else if (System_SoundOutput == 6)
		{
			fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000006,suppress,clockenable=1\n");
		}
		else if (System_SoundOutput == 7)
		{
			if(System_SpeakerConfiguration >= 4)
			{
				fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000007,suppress,clockenable=1\n");
			}
			else
			{
				fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000009,suppress,clockenable=1\n");
			}
		}
		else
		{
			fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x00000000,suppress,clockenable=1\n");
		}
	}
	else
	{
		fprintf(fptr,"%s","-ddra6xx,int=77,intprio=20,base=0x8F000000,memsize=0x1000000,dspstartupparam=0x100,suppress,clockenable=1\n");
	}
	fclose(fptr);

	fptr = fopen("/tmp/audioCtrl_args","w");
	if (fptr==NULL)
	{
		DBG_MSG("Error creating /tmp/audioCtrl_args!");
		exit(1);
	}
	if ((System_SoundOptimizationSettings < 0) || (System_SoundOptimizationSettings > 72))
	{
		DBG_MSG("System_SoundOptimizationSettings out of range, setting default = 0");
		System_SoundOptimizationSettings = 0;
	}
	if (oemvariant == 4)
	{
		fprintf(fptr,"--tp=/fs/etfs/dsp/manScaniaAudioCtrlSrv.hbtc -c /fs/etfs/dsp/Scania/%s -r 13\n", audiocfg_files[System_SoundOptimizationSettings]);
	}
	else
	{
		fprintf(fptr,"--tp=/fs/etfs/dsp/manScaniaAudioCtrlSrv.hbtc -c /fs/etfs/dsp/Man/%s -r 13\n", audiocfg_files[System_SoundOptimizationSettings]);
	}
	fclose(fptr);

	if (System_Function_AppleDeviceSupport == 0)
	{
		fptr=fopen("/tmp/DisableAppleSupport","w");
		if(fptr==NULL)
		{
			DBG_MSG("Error!");
			exit(1);
		}
		fprintf(fptr,"%s","DisableAppleSupport\n");
		fclose(fptr);
	}
	

	fptr = fopen("/tmp/tuner_args","w");
	if (fptr == NULL)
	{
		DBG_MSG("Error!");
		exit(1);
	}
	fprintf(fptr,"%s", oem);
	fprintf(fptr,"%s", " ");
	if (555 ==  variantNum || 557 == variantNum)
	{
		fprintf(fptr,"%s", "DAB_OFF");
	}
	else if (System_DAB == 0)
	{
		fprintf(fptr,"%s", "DAB_OFF");
	}
	else
	{
		fprintf(fptr,"%s", "DAB_ON");
		fprintf(can_fptr,"%s ","DAB");
		fprintf(mcNet_fptr,"%s ","DAB");
		variant = variant + 8;
	}
	fprintf(fptr,"%s", " ");
	fprintf(fptr,"%d", System_TunerFrequencySettings);
	fprintf(fptr,"%s", " ");
	fprintf(fptr,"%s", "--tp=/tunerApp/Tuner.hbtc\n");

	fclose(fptr);

//  Nobody is using this runtime variant, hence Commenting
//	if((HW_SAMPLE_C0 == ms_boot_info->sample_info) || (HW_SAMPLE_C1 == ms_boot_info->sample_info) || (HW_SAMPLE_C2 == ms_boot_info->sample_info) || (HW_SAMPLE_C3 == ms_boot_info->sample_info))
//	{
//		variant = variant + 16;
//	}

	fptr=fopen("/tmp/media_args","w");
	if(fptr==NULL)
	{
		DBG_MSG("Error creating media args!");
		exit(1);
	}

	/*J5ECO: No video support, so no need to check EOL Parameters*/
	fprintf(fptr,"%s","-c /etc/MediaService.cfg -- --tp=/fs/etfs/media/cfg/Media.hbtc\n");
	fclose(fptr);

	fptr=fopen("/tmp/iomedia_args","w");
	if(fptr==NULL)
	{
		DBG_MSG("Error creating io-media args!");
		exit(1);
	}
	fprintf(fptr,"%s","-c /etc/io-media-generic.cfg\n");
	fclose(fptr);

	fptr=fopen("/tmp/mme_args","w");
	if(fptr==NULL)
	{
		DBG_MSG("Error creating mme args!");
		exit(1);
	}
	fprintf(fptr,"%s","-c /etc/system/config/mme.conf\n");
	fclose(fptr);

	if ( System_USB1 != 0)
	{
		variant = variant + 32;
		fprintf(can_fptr,"%s ","USB1");
		fprintf(mcNet_fptr,"%s ","USB1");

		fptr=fopen("/tmp/usb1_args","w");
		if(fptr==NULL)
		{
			DBG_MSG("Error creating usb args!");
			exit(1);
		}

		if ( oemvariant == 4)
		{
			fprintf(fptr,"%s","-S1 -E -e -r -t -d Apple -c /etc/usblauncher/otg.lua -M /etc/dummy.mnt\n");
		}
		else
		{
			fprintf(fptr,"%s","-S1 -E -e -r -t -d Apple -c /etc/usblauncher/otg-man.lua -M /etc/dummy.mnt\n");
		}

		fclose(fptr);
	}

	if (SD1 != 0)
	{
		fprintf(can_fptr,"%s ","SD1");
		fprintf(mcNet_fptr,"%s ","SD1");
	}

	if (System_Bluetooth == 1)
	{
		if(System_Bluetooth_ph1 != 0)
		{
			fprintf(can_fptr,"%s ","BT1");
			fprintf(mcNet_fptr,"%s ","BT1");
		}
		if(Function_BTRemote == 1)
		{
			variant = variant + 64;	// BTREMOTE enabled
		}
		variant = variant + 128; //Scania:BT Enabled
	}
	else
	{
		DBG_ERROR("BT functionality is disabled");
	}

	AUX1 = (ms_boot_info->system_lineIn & 0x1);
	AUX2 = ((ms_boot_info->system_lineIn >> 1) & 0x3);

	if(AUX1 != 0)
	{
		fprintf(can_fptr,"%s ","AUX1");
		fprintf(mcNet_fptr,"%s ","AUX1");
	}

	if(AUX2 == 1)
	{
		fprintf(can_fptr,"%s ","TV");
		fprintf(mcNet_fptr,"%s ","TV");
	}
	else if(AUX2 != 0)
	{
		fprintf(can_fptr,"%s ","AUX2");
		fprintf(mcNet_fptr,"%s ","AUX2");
	}

	if ( oemvariant == 4)
	{
		fprintf(can_fptr,"%s","--tp=/fs/etfs/CANAdapter/canservice.hbtc\n");
	}
	else
	{
		if(audio_mute == 0){
			fprintf(can_fptr,"%s ","AUDIO_MUTE");
			fprintf(can_fptr,"%s","--tp=/fs/etfs/CANAdapter/canservice.hbtc\n");
		}
		else
		{
			fprintf(can_fptr,"%s","--tp=/fs/etfs/CANAdapter/canservice.hbtc\n");
		}
		fprintf(mcNet_fptr,"%s","--tp=/fs/etfs/McNetAdapter/McNetAdapter.hbtc\n");
	}

	fclose(can_fptr);
	fclose(mcNet_fptr);

    // Enable inetd by default.
    // We will control the configuration.
    variant = variant + 256;

	if (0 == bolo) // you are in app mode
	{
		variant = variant + 512;
	}
	else // you are in bolo mode for software update
	{
		variant = variant + 1024;
	}

	return variant;
}

const char* helperEnumToString(int id)
{
	if(id < PACKAGE_COUNT)
	{
		return PackageStrings[id];
	}
	else
	{
		return "Unknown";
	}
}

void startPackage (int istarthandle, ePackageId packageId)
{
	hbsrvstr_package_cmd_request_t	packagerequest;

	//OutputToConsole("Starting Package[%s] PackageId[%d]", helperEnumToString(packageId), (packageId));
	packagerequest.domainId 		= 0;
	packagerequest.packageId 		= (Int32)packageId;
	packagerequest.requestedState 	= PACKAGE_STATE_RUN;

	// Start the package
	devctl(istarthandle, DCMD_HBSRVSTR_PACKAGE_COMMAND, &packagerequest, sizeof(packagerequest), NULL);

	mPackageLoadStatus[packageId].currentState = PACKAGE_STATE_PRE_STARTING;
}

void *startSWDLMonitor (void *args)
{
	struct stat fileStat       = {0};
	while (stat("/tmp/EarlySWDL", &fileStat))
	{
		delay(100);
	}
	pthread_mutex_lock(&gSWDLMutexCond);
	pthread_cond_signal(&gSWDLCondVar);
	pthread_mutex_unlock(&gSWDLMutexCond);

	pthread_exit (0);
	return NULL;
}

void *startPackages (void* args)
{
	int istarthandle = open("/dev/starter/start", O_RDONLY);

	if(-1 >= istarthandle)
	{
		OutputToConsole("Unable to open /dev/starter/start");
		return;
	}
	else
	{
		sMyBool bAllLoaded 	= FALSE;
		sMyBool bStage1 	= FALSE;
		sMyBool bStage2 	= FALSE;
		sMyBool bStage3 	= FALSE;
		sMyBool bStage4 	= FALSE;
		sMyBool bStage5 	= FALSE;
		sMyBool bStage6 	= FALSE;
		sMyBool bStage7 	= FALSE;
		sMyBool bStage8 	= FALSE;
		sMyBool bStage9		= FALSE;

		int nStage = 0;

		OutputToConsole("start Package thread active, all clear proceed");

		while (FALSE == bAllLoaded)
		{
			// ----------- Protected area starts here -----------------
			pthread_mutex_lock(&gMutexCond);
			while (FALSE == g_bSignalled)
			{	pthread_cond_wait (&gCondVar, &gMutexCond);		}
			g_bSignalled = FALSE;
			pthread_mutex_unlock(&gMutexCond);
			// ----------- Protected area ends here -------------------

			// -------------- STAGE 1 Packages -----------------------//
			if (0 == nStage)
			{
				if (FALSE == bStage1)
				{
					/* If RVC is engaged, than start the RVC packages immediately */
					if (b_rvcengage)
					{
						startPackage (istarthandle, CAN);
						startPackage(istarthandle, IVN);
						/* If Camera is Present, than start the corresponding package */
						if (g_RVCSupported)
						{
							startPackage(istarthandle, RVC);
						}
					}

					bStage1 = TRUE;
				}

				if (b_rvcengage)
				{
					if ((PACKAGE_STATE_RUN == mPackageLoadStatus[IVN].currentState)
							&& (PACKAGE_STATE_RUN == mPackageLoadStatus[CAN].currentState)
							&& ((g_RVCSupported == 1) ? (PACKAGE_STATE_RUN == mPackageLoadStatus[RVC].currentState) : 1))
					{
						nStage = 1;
					}
				}
				else
					nStage = 1;
			}

			// -------------- STAGE 2 Packages -----------------------//
			if (1 == nStage)
			{
				if (FALSE == bStage2)
				{
					startPackage (istarthandle, HMI);
					startPackage (istarthandle, USBLAUNCHER);

					bStage2 = TRUE;
				}

				if ((PACKAGE_STATE_RUN == mPackageLoadStatus[HMI].currentState)
							&& (PACKAGE_STATE_RUN == mPackageLoadStatus[USBLAUNCHER].currentState))

				{
					nStage = 2;
				}

			}

			// -------------- STAGE 3 Packages -----------------------//
			if (2 == nStage)
			{
				if (FALSE == bStage3)
				{
					/* Based on LastUserMode, start the corresponding package */
					if (g_nlastMode == LM_TUNER)
						startPackage (istarthandle, TUNER);
					else if (g_nlastMode == LM_MEDIA)
						startPackage (istarthandle, MEDIA);
					else if (g_nlastMode == LM_AUX || g_nlastMode == LM_BT)
						startPackage (istarthandle, TUNER);

					bStage3 = TRUE;
				}

				if ((PACKAGE_STATE_RUN == mPackageLoadStatus[TUNER].currentState)
						|| (PACKAGE_STATE_RUN == mPackageLoadStatus[MEDIA].currentState))
				{
					nStage = 3;
				}

			}

			// -------------- STAGE 4 Packages -----------------------//
			if (3 == nStage)
			{
				if (FALSE == bStage4)
				{
					if (g_nlastMode == LM_TUNER)
						startPackage (istarthandle, MEDIA);
					else if (g_nlastMode == LM_MEDIA)
						startPackage (istarthandle, TUNER);
					else if (g_nlastMode == LM_AUX || g_nlastMode == LM_BT)
						startPackage (istarthandle, MEDIA);

					bStage4 = TRUE;
				}

				if ((PACKAGE_STATE_RUN == mPackageLoadStatus[TUNER].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[MEDIA].currentState))
				{
					nStage = 4;
				}

			}

			// -------------- STAGE 5 Packages -----------------------//
			if (4 == nStage)
			{
				if (FALSE == bStage5)
				{
					startPackage (istarthandle, DIAGNOSTICS);
					startPackage (istarthandle, CONNECTIVITY);
					startPackage (istarthandle, SMARTAPPLECONNECT);
					startPackage (istarthandle, TRACEMONITOR);

					bStage5 = TRUE;
				}

				if ((PACKAGE_STATE_RUN == mPackageLoadStatus[DIAGNOSTICS].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[CONNECTIVITY].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[SMARTAPPLECONNECT].currentState)
						&&(PACKAGE_STATE_RUN == mPackageLoadStatus[TRACEMONITOR].currentState))
				{
					nStage = 5;
				}

			}

			// -------------- STAGE 6 Packages -----------------------//
			if (5 == nStage)
			{
				if (FALSE == bStage6)
				{
					if (!b_rvcengage)
					{
						startPackage (istarthandle, CAN);
						startPackage (istarthandle, IVN);
						if (g_RVCSupported)
						{
							startPackage(istarthandle, RVC);
						}
					}

					bStage6 = TRUE;
				}

				if (!b_rvcengage)
				{
					if ((PACKAGE_STATE_RUN == mPackageLoadStatus[CAN].currentState)
							&&((PACKAGE_STATE_RUN == mPackageLoadStatus[IVN].currentState))
							&& ((g_RVCSupported == 1) ? (PACKAGE_STATE_RUN == mPackageLoadStatus[RVC].currentState) : 1))

					nStage = 6;
				}
				else
					nStage = 6;

			}

			// -------------- STAGE 7 Packages -----------------------//
			if (6 == nStage)
			{
				if (FALSE == bStage7)
				{
					/* If VR is supported, start the package */
					if (g_VoiceAlertFeature)
					{
						startPackage (istarthandle, VOICEALERT);
					}
					/* If GPS is supported, start the package */
					if (g_GPSAntennaAvailable)
					{
						startPackage (istarthandle, GPS);
					}
					startPackage (istarthandle, ANDROIDAUTO);
					startPackage (istarthandle,	SPEECH);
					startPackage (istarthandle,	ENGGMENU);
					startPackage (istarthandle,	SMARTLINK);

					bStage7 = TRUE;
				}

				if (((g_VoiceAlertFeature == 1) ? (PACKAGE_STATE_RUN == mPackageLoadStatus[VOICEALERT].currentState) : 1)
						&& ((g_GPSAntennaAvailable == 1) ? (PACKAGE_STATE_RUN == mPackageLoadStatus[GPS].currentState) : 1)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[ANDROIDAUTO].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[SPEECH].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[ENGGMENU].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[SMARTLINK].currentState))
				{
					nStage = 7;
				}

			}

			// -------------- STAGE 8 Packages -----------------------//
			if (7 == nStage)
			{
				if (FALSE == bStage8)
				{
					/* If CarPlay is supported, start the package */
					if (g_CarPlayAvailability)
					{
						startPackage (istarthandle,	CARPLAY);
					}
					startPackage (istarthandle,	SWUPDATE);
					startPackage (istarthandle,	TOUCHSIMULATION);
					startPackage (istarthandle,	USBSTATEREADER);

					bStage8 = TRUE;
				}

				if (((g_CarPlayAvailability == 1) ? (PACKAGE_STATE_RUN == mPackageLoadStatus[CARPLAY].currentState) : 1)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[SWUPDATE].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[TOUCHSIMULATION].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[USBSTATEREADER].currentState))
				{
					nStage = 8;
				}

			}

			// -------------- STAGE 9 Packages -----------------------//
			if (8 == nStage)
			{
				if (FALSE == bStage9)
				{
					startPackage (istarthandle,	NANDMANAGER);
					startPackage (istarthandle,	STOPTATALOGO);

					bStage9 = TRUE;
				}

				if ((PACKAGE_STATE_RUN == mPackageLoadStatus[NANDMANAGER].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[STOPTATALOGO].currentState))
				{
					nStage = 9;
					bAllLoaded = TRUE;
				}

			}
		}

		OutputToConsole("package start thread done!!!");
	}

	close(istarthandle);
	// Ready to join
	pthread_exit (0);

	return 0;

}
void* starterMonitor(void* args)
{
	tStarterObjState state[MAX_READ_ENTRIES];
	int istarthandle = open("/dev/starter/start",O_RDONLY);
	uint16_t n = 0u;
	uint16_t i =0u;
	int total_packages = LAST_PKG;
	if(-1 >= istarthandle)
	{
		OutputToConsole("Unable to open /dev/starter/start");
		return NULL;
	}
	OutputToConsole("MonitorThread Started\n");
	if (!g_RVCSupported)
	{
		total_packages -= 1;
	}
	if (!g_VoiceAlertFeature)
	{
		total_packages -= 1;
	}
	if (!g_GPSAntennaAvailable)
	{
		total_packages -= 1;
	}
	if (!g_CarPlayAvailability)
	{
		total_packages -= 1;
	}
	while ((total_packages  > g_nPackagesLoaded))
	{
		if (ENOSYS != (n = read (istarthandle, state, sizeof(state))))
		{
		if (MAX_READ_ENTRIES > n )
		{
			for (i = 0; i < n ; i++)
			{
				switch(state[i].objType)
				{
					case PACKAGE_ENTITY:
					{
						if (PACKAGE_STATE_RUN == state[i].objState)
						{
							//Inform the main thread of Package Availability
							//OutputToConsole("Package Loaded [%s] g_nPackagesLoaded : %d\n", helperEnumToString(state[i].objId), g_nPackagesLoaded);
							pthread_mutex_lock(&gMutexCond);

							mPackageLoadStatus[state[i].objId].currentState = PACKAGE_STATE_RUN;
							g_nPackagesLoaded++;

							pthread_cond_signal(&gCondVar);
							g_bSignalled = TRUE;
							pthread_mutex_unlock(&gMutexCond);
						}

						break;
					}
					default :
					{
						break;
					}
				}
			}
		}
		}
		else
		{
			OutputToConsole("Read failed\n");
			pthread_mutex_lock(&gMutexCond);
			pthread_cond_signal(&gCondVar);
			g_bSignalled = TRUE;
			pthread_mutex_unlock(&gMutexCond);

		}
	}
	OutputToConsole("thread monitoring job done!!!");

	close(istarthandle);
	return 0;
}

void initGlobaldata()
{
	gCondVar.__count		= _NTO_SYNC_COND;
	gCondVar.__owner		= _NTO_SYNC_INITIALIZER;

	gMutexCond.__count 		= _NTO_SYNC_NONRECURSIVE;
	gMutexCond.__owner 		= _NTO_SYNC_INITIALIZER;

	pthread_mutex_init (&gMutexCond, NULL);
	pthread_cond_init (&gCondVar, NULL);


}

void destroyGlobalData()
{
	 pthread_mutex_destroy (&gMutexCond);
	 pthread_cond_destroy (&gCondVar);

}

/* Start the packages */
void startAppPackages ()
{
	pthread_t	starterThread  = 0;
	pthread_t	monitorThread  = 0;
	tStarterObjState state[MAX_READ_ENTRIES];
	int istarthandle = open("/dev/starter/start", O_RDONLY);
	uint16_t n = 0u;
	uint16_t i = 0u;
	bool quit = true;

	if(-1 >= istarthandle)
	{
		OutputToConsole("Unable to open /dev/starter/start");
		goto startthread;
	}

	/* Make sure that the AUDIO_DRIVERS_TWO package gets successfully started
	 * before starting any manual packages
	 */
	while (quit) {
		if (ENOSYS != (n = read (istarthandle, state, sizeof(state))))
		{
			switch(state[i].objType)
			{
			case PACKAGE_ENTITY:
				{
					if (PACKAGE_STATE_RUN == state[i].objState)
					{
						if (!strcmp(helperEnumToString(state[i].objId), "AUDIO_DRIVERS_TWO"))
						{
							quit = false;
							//OutputToConsole("Packages loaded [%s], ObjectType : %d, domainid : %d, objectid : %d Read returned : %d\n",
								//helperEnumToString(state[i].objId), state[i].objType, state[i].domainId, state[i].objId, n);
						}
					}
					break;
				}
			default:
					break;
			}
		}
		else
			continue;
	}
	close(istarthandle);

	initGlobaldata ();

startthread:
	/* Start the Starter and Monitoring thread */
	pthread_create (&starterThread, NULL, startPackages, NULL);
	pthread_create (&monitorThread, NULL, starterMonitor, NULL);

	/* Wait here till thread finishes its job */

	pthread_join (starterThread, NULL);
	pthread_join (monitorThread, NULL);
	destroyGlobalData ();

	OutputToConsole("All Packages loaded !!");
}

void restartToAppMode()
{
#if 0
	struct stat fileStat       = {0};
	FILE *fp = NULL;


	// start the recovery package
	int ihandle = open ("/dev/starter/start", O_RDONLY);
	if(-1 >= ihandle)
	{
		// Add retry if required in future
		DBG_MSG("open /dev/starter/start failed, cannot load packages");
		return;
	}

	//Write params for OnOff
	fp = fopen("/tmp/OnOff", "w");
	if (fp == NULL)
	{
		DBG_MSG("FAILED!!! to open/create /tmp/OnOff");
		exit(1);
	}
	fprintf(fp, "-i 50 -l 1 -t J5 APP\n");

	fclose(fp);

	startPackage (ihandle, RECOVERY);

	while(-1 == stat("/dev/serv-mon/com.harman.service.ScaniaOnOff",&fileStat))
	{
		DBG_MSG("OnOff not yet available, sleep 500ms");
		usleep(500000);
	}
	DBG_MSG("[VD] Calling reset to V850");

	const char * cmd = "/bin/sh /usr/bin/OnOffUtility.sh reset app";
	fp = popen(cmd, "w");
	if(fp == NULL)
	{
		DBG_MSG("popen : Unable to execute command");
	}
	else
	{
		pclose(fp);
	}
	close (ihandle);
#else
	unsigned int i = 0;

	DBG_MSG("ERROR: Corrupted bootinfo read from SYS_PAGE.");
	DBG_MSG("ERROR: Resetting the target");

	// If the system is stuck because the bootinfo is not available/corrupt,
	// toggle the 5th Bit of GPIO[1] for 10 times.
	// This will reset the HU in app mode.
	for (i = 0; i < 20; i++)
	{
		poke_mem(0x4804C13C, 0x00000020, IO_DEPTH_32BIT);
		usleep(1000);
		poke_mem(0x4804C13C, 0x00000000, IO_DEPTH_32BIT);
		usleep(1000);
	}
	DBG_MSG("ERROR: We should not see this message. Target did not reset.");
#endif
}

int readEOLparameters(void)
{
	int eol_fd;
	char eol_pps_path[] = "/ppsqdb/eol?wait,delta,nopersist";
	char eol_buf[1024];
	char *pData = NULL;
	size_t nread;
	pps_status_t status;
	pps_attrib_t info;

	/* Read EOL Parameter Configuration */
	eol_fd = open(eol_pps_path, O_RDONLY, 0666);
	if (eol_fd < 0)
	{
		OutputToConsole("Fail to open %s, error : %d\n", eol_pps_path, eol_fd);
		return -1;
	}

	memset(eol_buf, 0, sizeof(eol_buf));
	/* Read the EOL Parameters */
	nread = read(eol_fd, (void *)eol_buf, sizeof(eol_buf) - 1);
	if (nread < 0)
	{
		OutputToConsole("EOL Parameter read failed, error : %d\n", eol_fd);
		close(eol_fd);
		return -1;
	}
	//OutputToConsole("EOL Parameters : %s", eol_buf);
	/* End with null char */
	eol_buf[nread] = '\0';
	pData = eol_buf;
	/* Go through all the EOL Parameters */
	while ((status = ppsparse(&pData, NULL, NULL, &info, 0)) != PPS_END)
	{
		if (status == PPS_ATTRIBUTE)
		{
			if (!strcmp(info.attr_name, "CarPlayAvailability"))
			{
				/* Save CarPlayAvailability availability */
				g_CarPlayAvailability = atoi(info.value);
				OutputToConsole("CarPlayAvailability : %d\n", g_CarPlayAvailability);
				OutputToConsole("CarPlayAvailability is %s\n",
						(g_CarPlayAvailability == 1) ? "ENABLE" :"DISABLE");
			}
			else if (!strcmp(info.attr_name, "GPSAntennaAvailable"))
			{
				/* Save GPSAntennaAvailable availability */
				g_GPSAntennaAvailable = atoi(info.value);
				OutputToConsole("GPSAntennaAvailable : %d\n", g_GPSAntennaAvailable);
				OutputToConsole("GPSAntennaAvailable is %s\n",
						(g_GPSAntennaAvailable == 1) ? "AVAILABLE" :"UNAVAILABLE");
			}
			else if (!strcmp(info.attr_name, "RVCSupported"))
			{
				/* Save RVCSupported availability */
				g_RVCSupported = atoi(info.value);
				OutputToConsole("RVCSupported : %d\n", g_RVCSupported);
				OutputToConsole("RVCSupported is %s\n",
						(g_RVCSupported == 1) ? "AVAILABLE" :"UNAVAILABLE");
			}
			else if (!strcmp(info.attr_name, "VoiceAlertFeature"))
			{
				/* Save VoiceAlertFeature availability */
				g_VoiceAlertFeature = atoi(info.value);
				OutputToConsole("VoiceAlertFeature : %d\n", g_VoiceAlertFeature);
				OutputToConsole("VoiceAlertFeature is %s\n",
						(g_VoiceAlertFeature == 1) ? "ENABLE" :"DISABLE");
			}
		}

	}
	close(eol_fd);
	return 0;
}
int main (int argc, char *argv[])
{
	FILE *fptr = NULL;
	FILE *fp;
	int size = 0, i;
	char *str;
	char *ret;
	char foundstring[10];
	const char *comparestring = "sourceType";

	OutputToConsole("StarterManager Started");
	/* Read the LastAudioSource state */
	fp = fopen("/fs/etfs/ModeManager/PersistencyStore.txt", "r");
	if (fp == NULL){
		OutputToConsole("File not present\n");
		g_nlastMode = LM_TUNER;
		goto startpackage;
	}

	/* Find the total size of the file */
	fseek(fp, 0, 2);
	size = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	/* Allocate the string as per size calculated */
	str = malloc(size);
	if(!str){
		OutputToConsole("Malloc failed to allocate\n");
		g_nlastMode = LM_TUNER;
		goto startpackage;
	}

	/* Copy the string into buffer allocated */
	fgets(str, size, fp);
	if(str == NULL){
		OutputToConsole("String read failed\n");
		free(str);
		g_nlastMode = LM_TUNER;
		goto startpackage;
	}

	/* Check for "SourceType" substring in the buffer */
	ret = strstr(str, comparestring);
	if (ret == NULL){
		OutputToConsole("SourceType not found in the file\n");
		free(str);
		g_nlastMode = LM_TUNER;
		goto startpackage;
	}

	/* Move the pointer till comparison with the audio source types */
	for (i = 0; i < strlen(comparestring); i++){
		ret++;
	}
	ret = ret + 3;
	memset(foundstring, '\0', sizeof(foundstring));
	/* Copy the audio source string into buffer */
	strncpy(foundstring, ret, 8);

	/* Find the last audio source */
	if(strncmp(foundstring, "tuner_AM", 8) == 0)
		g_nlastMode = LM_TUNER;
	else if (strncmp(foundstring, "tuner_FM", 8) == 0)
		g_nlastMode = LM_TUNER;
	else if (strncmp(foundstring, "usb", 3) == 0)
		g_nlastMode = LM_MEDIA;
	else if (strncmp(foundstring, "btsa", 4) == 0)
		g_nlastMode = LM_BT;
	else
		g_nlastMode = LM_AUX;

	free(str);
	fclose(fp);

	if (readEOLparameters() != 0)
	{
		OutputToConsole("Reading EOL params failed, assuming default values\n");
	}

	/* Check whether RVC is engaged or not */
#if 0
	rvc = system("mkdir -p /pps/can");
	if( rvc == -1 )
		{
			OutputToConsole( "************************** mkdir failed : %d\n ****************************", rvc );
			goto startpackage;
		}

		else
		{
			OutputToConsole( "result of running command is %d\n",
			WEXITSTATUS( rvc ) );
		}
	pa = open("/pps/can/parkassist?wait,delta,nopersist", O_RDWR| O_CREAT, 0666 );
	if (pa < 0)
	{
		OutputToConsole("************************ File failed to open\n ************************");
		goto startpackage;
	}

	rvc = system( "echo RearPDCPowerandActiveState::1 >> /pps/can/parkassist" );
	//rvc = write(pa, buf, sizeof(buf));
	if( rvc == -1 )
	{
		OutputToConsole( "************************** system called failed : %d\n ****************************", rvc );
		goto startpackage;
	}

	else
	{
		printf( "result of running command is %d\n",
		WEXITSTATUS( rvc ) );
	}
	close(pa);
#endif

	/* Copy the lastusermode value to temp file for debug purpose */
	fptr = fopen("/fs/etfs/lum.txt", "w+");
	if (fptr == NULL)
	{
		OutputToConsole("Failed to create lum.txt file");
		goto startpackage;
	}
	fprintf(fptr, "%s %d\n","Last User Mode is", g_nlastMode);
	fclose(fptr);
startpackage:
	/* Start the Application Packages */
	startAppPackages ();

	OutputToConsole("Done bye!");

	return EXIT_SUCCESS;
}
