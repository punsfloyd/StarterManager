/*
 * utils.h
 *
 *  Created on: Nov 25, 2015
 *      Author: PGaddikeri
 */

#ifndef CONFIG_DEFINES_H
#define CONFIG_DEFINES_H

#define PACKAGE_COUNT	32

const int BOOT_INFO_LENGTH = 16;

const uint16_t MAX_READ_ENTRIES = 512;		// Ensure that this value must be greater then PackageCount+InterfaceCount+ProcessCount as in j5_starter.cfg

/*====================================================================================================================
 Strictly do not change the order, keep the order in sync with Package Definition in j5_starter.cfg
====================================================================================================================*/
typedef enum
{
	BASE_DRIVERS_ONE,				// 0
	BASE_DRIVERS_TWO,				// 1
	AUDIO_DRIVERS_ONE,				// 2
	AUDIO_DRIVERS_TWO,				// 3
	PPSQDB,							// 4
	QDB,							// 5
	BACKEND_SERVICES,				// 6
	PERSISTENCY_PPS,				// 7
	STARTUPMANAGER,					// 8
	HMI,							// 9
	USBLAUNCHER,					// 10
	MEDIA,							// 11
	IVN,							// 12
	RVC,							// 13
	TUNER,							// 14
	DIAGNOSTICS,					// 15
	CONNECTIVITY,					// 16
	SMARTAPPLECONNECT,				// 17
	TRACEMONITOR,					// 18
	CAN,							// 19
	VOICEALERT,						// 20
	GPS,					// 21
	ANDROIDAUTO,							// 22
	SPEECH,							// 23
	ENGGMENU,						// 24
	SMARTLINK,						// 25
	CARPLAY,						// 26
	SWUPDATE,						// 27
	TOUCHSIMULATION,				// 28
	USBSTATEREADER,					// 29
	NANDMANAGER,					// 30
	STOPTATALOGO,					// 31
	LAST_PKG						// 32
} ePackageId;

static const char * PackageStrings[] = {
		"BASE_DRIVERS_ONE",			// 0
		"BASE_DRIVERS_TWO",			// 1
		"AUDIO_DRIVERS_ONE",		// 2
		"AUDIO_DRIVERS_TWO",		// 3
		"PPSQDB",					// 4
		"QDB",						// 5
		"BACKEND_SERVICES",			// 6
		"PERSISTENCY_PPS",			// 7
		"STARTUPMANAGER",			// 8
		"HMI",						// 9
		"USBLAUNCHER",				// 10
		"MEDIA",					// 11
		"IVN",						// 12
		"RVC",						// 13
		"TUNER",					// 14
		"DIAGNOSTICS",				// 15
		"CONNECTIVITY",				// 16
		"SMARTAPPLECONNECT",		// 17
		"TRACEMONITOR",				// 18
		"CAN",						// 19
		"VOICEALERT",				// 20
		"GPS",						// 21
		"ANDROIDAUTO",				// 22
		"SPEECH",					// 23
		"ENGGMENU",					// 24
		"SMARTLINK",				// 25
		"CARPLAY",					// 26
		"SWUPDATE",					// 27
		"TOUCHSIMULATION",			// 28
		"USBSTATEREADER",			// 29
		"NANDMANAGER",				// 30
		"STOPTATALOGO",				// 31
		"UNKNOWN"					// 32
	};


// Raj : This dependency is not used now, for LUM, we may have to override this
/*int pkgDependencyMatrix[PACKAGE_COUNT][PACKAGE_COUNT] = {
----------------------------DEPENDS ON---------------------------------------------------------------------------------------
 * ------------------------MODE_IDENTIFIER---EDMA_CH------SYS_REGISTER------SoftwareUpdate---DISPLAY_LAYER------BASE_DRIVERS------MEMnSTORAGE------TUNER_BASE------DEBUGnHELPERS------MnS_HMI------MEDIA------CANSERVICE------BLUETOOTHnUTILITY---SWUPDATEnUTILITY
	MODE_IDENTIFIER   	{	0,				0,				0,				0,				0,					0,				0,					0,				0,				0,			0,				0,				0,					0 },
	EDMA_CH           	{	0,				0,				0,				0,				0,					0,				0,					0,				0,				0,			0,				0,				0,					0 },
	SYS_REGISTER      	{	1,				0,				0,				0,				0,					0,				0,					0,				0,				0,			0,				0,				0,					0 },
	SoftwareUpdate    	{	1,				0,				0,				0,				0,					0,				0,					0,				0,				0,			0,				0,				0,					0 },
	DISPLAY_LAYER     	{	0,				0,				1,				0,				0,					0,				0,					0,				0,				0,			0,				0,				0,					0 },
	BASE_DRIVERS      	{	0,				0,				0,				0,				1,					0,				0,					0,				0,				0,			0,				0,				0,					0 },
	MEMnSTORAGE       	{	0,				0,				0,				0,				0,					1,				0,					0,				0,				0,			0,				0,				0,					0 },
	TUNER_BASE        	{	0,				0,				0,				0,				0,					0,				1,					0,				0,				0,			0,				0,				0,					0 },
	DEBUGnHELPERS     	{	0,				0,				0,				0,				0,					0,				0,					1,				0,				0,			0,				0,				0,					0 },
	MnS_HMI           	{	0,				0,				0,				0,				0,					0,				0,					1,				0,				0,			0,				0,				0,					0 },
	MEDIA             	{	0,				0,				0,				0,				0,					0,				0,					1,				0,				0,			0,				0,				0,					0 },
	CANSERVICE        	{	0,				0,				0,				0,				0,					0,				0,					0,				0,				0,			1,				0,				0,					0 },
	BLUETOOTHnUTILITY 	{	0,				0,				0,				0,				0,					0,				0,					0,				0,				0,			0,				1,				0,					0 },
	SWUPDATEnUTILITY  	{	0,				0,				0,				0,				0,					0,				0,					0,				0,				0,			0,				0,				0,					0 }
};*/


typedef struct
{
	ePackageId		packageid;
	ePackageState	currentState;
}	sPackageStatus;

sPackageStatus		mPackageLoadStatus[PACKAGE_COUNT] = {
		{BASE_DRIVERS_ONE, PACKAGE_STATE_STOP},
		{BASE_DRIVERS_TWO, PACKAGE_STATE_STOP},
		{AUDIO_DRIVERS_ONE, PACKAGE_STATE_STOP},
		{AUDIO_DRIVERS_TWO, PACKAGE_STATE_STOP},
		{PPSQDB, PACKAGE_STATE_STOP},
		{QDB, PACKAGE_STATE_STOP},
		{BACKEND_SERVICES, PACKAGE_STATE_STOP},
		{PERSISTENCY_PPS, PACKAGE_STATE_STOP},
		{STARTUPMANAGER, PACKAGE_STATE_STOP},
		{HMI, PACKAGE_STATE_STOP},
		{USBLAUNCHER, PACKAGE_STATE_STOP},
		{MEDIA, PACKAGE_STATE_STOP},
		{IVN, PACKAGE_STATE_STOP},
		{RVC, PACKAGE_STATE_STOP},
		{TUNER, PACKAGE_STATE_STOP},
		{DIAGNOSTICS, PACKAGE_STATE_STOP},
		{CONNECTIVITY,PACKAGE_STATE_STOP},
		{SMARTAPPLECONNECT, PACKAGE_STATE_STOP},
		{TRACEMONITOR, PACKAGE_STATE_STOP},
		{CAN, PACKAGE_STATE_STOP},
		{VOICEALERT, PACKAGE_STATE_STOP},
		{GPS, PACKAGE_STATE_STOP},
		{ANDROIDAUTO, PACKAGE_STATE_STOP},
		{SPEECH, PACKAGE_STATE_STOP},
		{ENGGMENU, PACKAGE_STATE_STOP},
		{SMARTLINK, PACKAGE_STATE_STOP},
		{CARPLAY, PACKAGE_STATE_STOP},
		{SWUPDATE, PACKAGE_STATE_STOP},
		{TOUCHSIMULATION, PACKAGE_STATE_STOP},
		{USBSTATEREADER, PACKAGE_STATE_STOP},
		{NANDMANAGER, PACKAGE_STATE_STOP},
		{STOPTATALOGO, PACKAGE_STATE_STOP}
};

typedef enum
{
	FALSE,
	TRUE
} sMyBool;


#endif /* CONFIG_DEFINES_H */

