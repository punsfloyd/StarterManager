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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/* Macro for Enable/Disable Console Prints */
#undef PRINT_CONSOLE
#ifdef PRINT_CONSOLE
#define OutputToConsole(args...) fflush(stdout); printf("Line %d : ", __LINE__); printf("STMGR - "); printf(args); printf("\n"); fflush(stdout)
#else
#define OutputToConsole(args...) (void)(0)
#endif

tStarterObjState	gObjState;
pthread_cond_t		gCondVar;
pthread_mutex_t     gMutexCond;

sMyBool				g_bSignalled 		= FALSE;
unsigned int		g_nPackagesLoaded 	= 0;

enum LAST_MODE
{
	LM_TUNER,
	LM_MEDIA,
	LM_AUX,
	LM_BT,
};
/* Last User Mode Variable */
unsigned int g_nlastMode = LM_TUNER;
/* Boolean for RVC Engage */
sMyBool b_rvcengage = FALSE;
/* EOL Parameters Default Value */
int g_CarPlayAvailability = 1;		/* Default */
int g_GPSAntennaAvailable = 1;		/* Default */
int g_VoiceAlertFeature = 1;		/* Default */
int g_AndroidAutoFeature = 1;		/* Default */
int g_CarPlayFileExist = 1;
int g_DbustraceFileExist = 1;

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

void *startPackages (void* args)
{
	int istarthandle = open("/dev/starter/start", O_RDONLY);

	if(-1 >= istarthandle)
	{
		OutputToConsole("Unable to open /dev/starter/start");
		return 0;
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
						startPackage(istarthandle, RVC);
					}

					bStage1 = TRUE;
				}

				if (b_rvcengage)
				{
					if ((PACKAGE_STATE_RUN == mPackageLoadStatus[CAN].currentState)
							&& (PACKAGE_STATE_RUN == mPackageLoadStatus[IVN].currentState)
							&& (PACKAGE_STATE_RUN == mPackageLoadStatus[RVC].currentState))
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
					/* As both these packages are now started in AutoStart, removing them */
					//startPackage (istarthandle, USBLAUNCHER);
					//startPackage (istarthandle, HMI);

					bStage2 = TRUE;
				}

				/*if ((PACKAGE_STATE_RUN == mPackageLoadStatus[HMI].currentState)
							&& (PACKAGE_STATE_RUN == mPackageLoadStatus[USBLAUNCHER].currentState))*/

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
					/* As USB Audio needs to be played under 18sec, this delay is introduce so that no other process used CPU */
					if (g_nlastMode == LM_MEDIA)
							sleep(4);
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
					if (g_DbustraceFileExist == 0)
					{
						startPackage (istarthandle, TRACEMONITOR);
					}

					bStage5 = TRUE;
				}

				if ((PACKAGE_STATE_RUN == mPackageLoadStatus[DIAGNOSTICS].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[CONNECTIVITY].currentState)
						&& (PACKAGE_STATE_RUN == mPackageLoadStatus[SMARTAPPLECONNECT].currentState)
						&& ((g_DbustraceFileExist == 0) ? (PACKAGE_STATE_RUN == mPackageLoadStatus[TRACEMONITOR].currentState) : 1))
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
						startPackage(istarthandle, RVC);
					}

					bStage6 = TRUE;
				}

				if (!b_rvcengage)
				{
					if ((PACKAGE_STATE_RUN == mPackageLoadStatus[CAN].currentState)
							&& ((PACKAGE_STATE_RUN == mPackageLoadStatus[IVN].currentState))
							&& (PACKAGE_STATE_RUN == mPackageLoadStatus[RVC].currentState))

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
					if (g_VoiceAlertFeature == 1)
					{
						startPackage (istarthandle, VOICEALERT);
					}
					/* If GPS is supported, start the package */
					if (g_GPSAntennaAvailable == 1)
					{
						startPackage (istarthandle, GPS);
					}
					if ((g_CarPlayAvailability == 1) || (g_AndroidAutoFeature == 1))
					{
						startPackage (istarthandle, MMRENDERER);
					}
					startPackage (istarthandle, ANDROIDAUTO);
					startPackage (istarthandle,	SPEECH);
					startPackage (istarthandle,	ENGGMENU);
					startPackage (istarthandle,	SMARTLINK);

					bStage7 = TRUE;
				}

				if (((g_VoiceAlertFeature == 1) ? (PACKAGE_STATE_RUN == mPackageLoadStatus[VOICEALERT].currentState) : 1)
						&& ((g_GPSAntennaAvailable == 1) ? (PACKAGE_STATE_RUN == mPackageLoadStatus[GPS].currentState) : 1)
						&& (((g_CarPlayAvailability == 1) || (g_AndroidAutoFeature == 1)) ?
								(PACKAGE_STATE_RUN == mPackageLoadStatus[MMRENDERER].currentState) : 1)
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
					/* If CarPlay is supported &
					 * If Marker file is present, start the package */
					if (g_CarPlayAvailability == 1 && g_CarPlayFileExist == 0)
					{
						startPackage (istarthandle,	CARPLAY);
					}
					startPackage (istarthandle,	TOUCHSIMULATION);
					startPackage (istarthandle,	USBSTATEREADER);

					bStage8 = TRUE;
				}

				if ((((g_CarPlayAvailability == 1) && (g_CarPlayFileExist == 0))? (PACKAGE_STATE_RUN == mPackageLoadStatus[CARPLAY].currentState) : 1)
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

					bStage9 = TRUE;
				}

				if ((PACKAGE_STATE_RUN == mPackageLoadStatus[NANDMANAGER].currentState))
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
	//pthread_exit (0);

	return 0;

}
void* starterMonitor(void* args)
{
	tStarterObjState state[MAX_READ_ENTRIES];
	int istarthandle = open("/dev/starter/start",O_RDONLY);
	int16_t n = 0;
	uint16_t i =0u;
	int total_packages = LAST_PKG;
	if(-1 >= istarthandle)
	{
		OutputToConsole("Unable to open /dev/starter/start");
		return NULL;
	}
	OutputToConsole("MonitorThread Started\n");
	if (!g_VoiceAlertFeature)
	{
		total_packages -= 1;
	}
	if (!g_GPSAntennaAvailable)
	{
		total_packages -= 1;
	}
	if (!g_CarPlayAvailability || g_CarPlayFileExist != 0)
	{
		total_packages -= 1;
	}
	/* If both AA and CarPlay DID are disable, decrement the package count
	 * since MM-RENDERER package will not be launched
	 */
	if (!g_CarPlayAvailability && !g_AndroidAutoFeature)
	{
		total_packages -= 1;
	}
	if (g_DbustraceFileExist != 0)
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

	/* Initialize the structure to all zero's */
	memset(state, 0, sizeof(state));

	/* Make sure that the PERSISTENCY package gets successfully started
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
						if (!strcmp(helperEnumToString(state[i].objId), "PERSISTENCY_PPS"))
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

startthread:
	initGlobaldata ();
	/* Start the Starter and Monitoring thread */
	pthread_create (&starterThread, NULL, startPackages, NULL);
	pthread_create (&monitorThread, NULL, starterMonitor, NULL);

	/* Wait here till thread finishes its job */

	pthread_join (starterThread, NULL);
	pthread_join (monitorThread, NULL);
	destroyGlobalData ();

	OutputToConsole("All Packages loaded !!");
}

int readEOLparameters(void)
{
	int eol_fd;
	char eol_pps_path[] = "/ppsqdb/eol?wait,nopersist";
	char eol_buf[1024];
	char *pData = NULL;
	int nread;
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
				if (g_CarPlayAvailability != 0 && g_CarPlayAvailability != 1)
				{
					/* If the EOL DID is other than zero or one, keep the value as zero(DISABLE) */
					g_CarPlayAvailability = 0;
				}
				OutputToConsole("CarPlayAvailability : %d\n", g_CarPlayAvailability);
				OutputToConsole("CarPlayAvailability is %s\n",
						(g_CarPlayAvailability == 1) ? "ENABLE" :"DISABLE");
			}
			else if (!strcmp(info.attr_name, "GPSAntennaAvailable"))
			{
				/* Save GPSAntennaAvailable availability */
				g_GPSAntennaAvailable = atoi(info.value);
				if (g_GPSAntennaAvailable != 0 && g_GPSAntennaAvailable != 1)
				{
					/* If the EOL DID is other than zero or one, keep the value as zero(DISABLE) */
					g_GPSAntennaAvailable = 0;
				}
				OutputToConsole("GPSAntennaAvailable : %d\n", g_GPSAntennaAvailable);
				OutputToConsole("GPSAntennaAvailable is %s\n",
						(g_GPSAntennaAvailable == 1) ? "AVAILABLE" :"UNAVAILABLE");
			}
			else if (!strcmp(info.attr_name, "VoiceAlertFeature"))
			{
				/* Save VoiceAlertFeature availability */
				g_VoiceAlertFeature = atoi(info.value);
				if (g_VoiceAlertFeature != 0 && g_VoiceAlertFeature != 1)
				{
					/* If the EOL DID is other than zero or one, keep the value as zero(DISABLE) */
					g_VoiceAlertFeature = 0;
				}
				OutputToConsole("VoiceAlertFeature : %d\n", g_VoiceAlertFeature);
				OutputToConsole("VoiceAlertFeature is %s\n",
						(g_VoiceAlertFeature == 1) ? "ENABLE" :"DISABLE");
			}
			else if (!strcmp(info.attr_name, "AndroidAutoFeature"))
			{
				g_AndroidAutoFeature = atoi(info.value);
				if (g_AndroidAutoFeature != 0 && g_AndroidAutoFeature != 1)
				{
					/* If the EOL DID is other than zero or one, keep the value as zero(DISABLE) */
					g_AndroidAutoFeature = 0;
				}
				OutputToConsole("AndroidAutoFeature : %d\n", g_AndroidAutoFeature);
				OutputToConsole("AndroidAutoFeature is %s\n",
						(g_AndroidAutoFeature == 1) ? "ENABLE" :"DISABLE");
			}
		}

	}
	close(eol_fd);
	return 0;
}

int main (int argc, char *argv[])
{
	//FILE *fptr = NULL;
	FILE *fp;
	int size = 0, i;
	char *str;
	char *ret;
	char foundstring[10];
	const char *comparestring = "sourceType";
	const char *filename = "/fs/etfs/carplay_marker_file";
	const char *dbustrace_filename = "/fs/etfs/dbustrace_marker_file";
	struct stat buffer;

	OutputToConsole("StarterManager Started");

	/* Read the LastAudioSource state */
	fp = fopen("/fs/etfs/ModeManager/PersistencyStore.txt", "r");
	if (fp == NULL){
		OutputToConsole("File not present\n");
		g_nlastMode = LM_TUNER;
		goto startpackage;
	}

	/* Find the total size of the file */
	if (fseek(fp, 0, 2) != 0)
	{
		OutputToConsole("fseek failed\n");
		goto startpackage;
	}
	size = ftell(fp);
	if (size < 0)
	{
		size = 500;
	}

	if (fseek(fp, 0, SEEK_SET) != 0)
	{
		OutputToConsole("fseek failed\n");
		goto startpackage;
	}

	/* Allocate the string as per size calculated */
	str = malloc(size);
	if(!str){
		OutputToConsole("Malloc failed to allocate\n");
		g_nlastMode = LM_TUNER;
		goto startpackage;
	}

	/* Copy the string into buffer allocated */
	if (fgets(str, size, fp) == NULL )
	{
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

	/* Check if carplay marker file is present */
	g_CarPlayFileExist = stat(filename, &buffer);
	OutputToConsole("g_CarPlayFileExist : %d", g_CarPlayFileExist);
	if (g_CarPlayFileExist == 0)
	{
		OutputToConsole("Marker file exist");
	}
	g_DbustraceFileExist = stat(dbustrace_filename, &buffer);
	OutputToConsole("g_DbustraceFileExist : %d", g_DbustraceFileExist);
	if (g_DbustraceFileExist == 0)
	{
		OutputToConsole("DbusTrace Marker file exist");
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
#if 0
	fptr = fopen("/fs/etfs/lum.txt", "w+");
	if (fptr == NULL)
	{
		OutputToConsole("Failed to create lum.txt file");
		goto startpackage;
	}
	fprintf(fptr, "%s %d\n","Last User Mode is", g_nlastMode);
	fclose(fptr);
#endif
startpackage:
	/* Start the Application Packages */
	startAppPackages ();

	OutputToConsole("Done bye!");

	return EXIT_SUCCESS;
}
