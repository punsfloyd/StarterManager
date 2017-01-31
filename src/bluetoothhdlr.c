/*
 * bluetoothhdlr.c
 *
 *  Created on: May 4, 2016
 *      Author: PGaddikeri
 */

#include "rw_utils.h"

extern int System_Bluetooth;
extern int Function_BTRemote;
extern int System_Bluetooth_ph1;
extern int System_Bluetooth_ph2;
extern int BT_ID[];
//extern int System_Bluetooth_Functionality[3];

int EXEC_CMD(const char* cmd)
{
    FILE* fp = popen(cmd, "r");

    if (fp == NULL)
    {
        return -1;
    }

    return pclose(fp);
}

void System_BT_ID ( void )
{
	FILE *fptr;
	struct timespec start;
	long randVal = 0;
	static char state[32];
	struct stat sBuff;

	if(!(BT_ID[0] || BT_ID[1] || BT_ID[2] || BT_ID[3] || BT_ID[4] || BT_ID[5]))
	{
		if((stat("/usr/var/volatile/wicome/BT_ID", &sBuff)) || (sBuff.st_size != 12))
		{
			fptr = fopen("/usr/var/volatile/wicome/BT_ID","w");
			if (fptr != NULL)
			{
				if( clock_gettime( CLOCK_REALTIME, &start) == -1 )
				{
					DBG_ERROR("Unable to get clock time");
				}

				initstate(time(NULL), state, sizeof(state));
				setstate( state);
				randVal = random();
				fprintf(fptr,"%.4lX", (randVal & 0xFFFF));
				fprintf(fptr,"%.4X", ((start.tv_sec) & 0xFFFF));
				fprintf(fptr,"%.4lX", ((randVal >> 4) & 0xFFFF));
				fclose(fptr);
				// copy file
				EXEC_CMD("cp -f /usr/var/volatile/wicome/BT_ID /usr/var/wicome/BT_ID");
				DBG_MSG("Random BT_ID generated [%.4lX%.4X%.4lX]", (randVal & 0xFFFF),((start.tv_sec) & 0xFFFF), ((randVal >> 4) & 0xFFFF));
			}
			else
			{
				DBG_ERROR("Unable to write /usr/var/volatile/wicome/BT_ID");
			}
		}
		else
		{
			DBG_MSG("/usr/var/volatile/wicome/BT_ID exists");
		}
	}
	else
	{
		if(!stat("/usr/var/volatile/wicome/BT_ID", &sBuff))
		{
			remove("/usr/var/volatile/wicome/BT_ID");
		}
		fptr = fopen("/usr/var/volatile/wicome/BT_ID","w");
		if(fptr != NULL)
		{
			fprintf(fptr,"%.2X%.2X%.2X%.2X%.2X%.2X", BT_ID[0], BT_ID[1], BT_ID[2], BT_ID[3], BT_ID[4], BT_ID[5]);
			fclose(fptr);
			EXEC_CMD("cp -rf /usr/var/volatile/wicome/BT_ID /usr/var/wicome/BT_ID");
			DBG_MSG("BT Address reset to [%.2X%.2X%.2X%.2X%.2X%.2X]",BT_ID[0], BT_ID[1], BT_ID[2], BT_ID[3], BT_ID[4], BT_ID[5]);
			EXEC_CMD("cat /usr/var/volatile/wicome/BT_ID");
			EXEC_CMD("cat /usr/var/wicome/BT_ID");
		}
		else
		{
			DBG_ERROR("Unable to write /usr/var/volatile/wicome/BT_ID when BT_ID EOL is non zero");
		}
	}
}


int MAN_Bluetooth_Arguments( void )
{
	FILE *fptr_wicome;

	fptr_wicome=fopen("/tmp/wicome_args","w");
	if(fptr_wicome==NULL)
	{
		printf("Error!");
		exit(1);
	}

	DBG_MSG( "MAN:System_Bluetooth:%d, Function_BTRemote:%d, System_Bluetooth_ph1:%d, System_Bluetooth_ph2:%d\n", System_Bluetooth, Function_BTRemote, System_Bluetooth_ph1, System_Bluetooth_ph2 );

	if( System_Bluetooth != 0)
	{
		if( Function_BTRemote != 0)
		{
			if	( 	(System_Bluetooth_ph1 == 3 && System_Bluetooth_ph2 == 3) ||
					(System_Bluetooth_ph1 == 2 && System_Bluetooth_ph2 == 3) ||
					(System_Bluetooth_ph1 == 3 && System_Bluetooth_ph2 == 2) )
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_scania_J5.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_hfp_a2dp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth Two Phone Full");
			 }
			else if(( System_Bluetooth_ph1 == 3 ) || (System_Bluetooth_ph2 == 3))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp_a2dp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_a2dp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One Phone Full");
			 }
			else if(( System_Bluetooth_ph1 == 2 ) && (System_Bluetooth_ph2 == 2 ))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp_hfp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_hfp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth Two phone only telephone");
			 }
			else if(( System_Bluetooth_ph1 == 0 && System_Bluetooth_ph2 == 1 ) ||
					( System_Bluetooth_ph1 == 1 && System_Bluetooth_ph2 == 0 ) )
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_a2dp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_a2dp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One phone only media");
			 }
			else if(( System_Bluetooth_ph1 == 0 && System_Bluetooth_ph2 == 2 ) ||
				    ( System_Bluetooth_ph1 == 2 && System_Bluetooth_ph2 == 0 ) )
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One phone only telephone");
			 }
			else
			{
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_remote.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_remote.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				DBG_MSG( "No Bluetooth Configured");
        	 }
			DBG_MSG( "Remote Functionality configured\n");
		}
		else
		{
			if( ( System_Bluetooth_ph1 == 3 && System_Bluetooth_ph2 == 3 ) ||
				( System_Bluetooth_ph1 == 2 && System_Bluetooth_ph2 == 3 ) ||
				( System_Bluetooth_ph1 == 3 && System_Bluetooth_ph2 == 2 )
			  )
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_scania_J5.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_hfp_a2dp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth Two Phone Full");
			 }
			else if(( System_Bluetooth_ph1 == 3 ) || (System_Bluetooth_ph2 == 3))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp_a2dp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_a2dp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One Phone Full");
			 }
			else if(( System_Bluetooth_ph1 == 2 ) && (System_Bluetooth_ph2 == 2 ))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp_hfp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_hfp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth Two phone only telephone");
			 }
			else if(( System_Bluetooth_ph1 == 0 && System_Bluetooth_ph2 == 1 ) ||
					( System_Bluetooth_ph1 == 1 && System_Bluetooth_ph2 == 0 ) )
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_a2dp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_a2dp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One phone only media");
			 }
			else if(( System_Bluetooth_ph1 == 0 && System_Bluetooth_ph2 == 2 ) ||
					( System_Bluetooth_ph1 == 2 && System_Bluetooth_ph2 == 0 ) )
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One phone only telephone");
			 }
			DBG_MSG( "Remote Functionality  not configured");
		}
	}
	else
	{
		DBG_ERROR("No MAN EOL Bluetooth parameter is set properly and hence NO BT functionality");
	}

	fclose(fptr_wicome);

	return 0;
}

int Scania_Bluetooth_Arguments( void )
{
	FILE *fptr_wicome;

	fptr_wicome = fopen("/tmp/wicome_args","w");
	if(fptr_wicome==NULL)
	{
		DBG_ERROR("Unable to write wicome_args");
		return -1;
	}


	DBG_MSG( "SCANIA:System_Bluetooth:%d, Function_BTRemote:%d, System_Bluetooth_ph1:%d, System_Bluetooth_ph2:%d", System_Bluetooth, Function_BTRemote, System_Bluetooth_ph1, System_Bluetooth_ph2 );

	if( System_Bluetooth != 0)
	{
		if( Function_BTRemote != 0)
		{
			if(( System_Bluetooth_ph1 == 1 && System_Bluetooth_ph2 == 1 ) ||
			   ( System_Bluetooth_ph1 == 1 && System_Bluetooth_ph2 == 3 ) ||
			   ( System_Bluetooth_ph1 == 3 && System_Bluetooth_ph2 == 1 ))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_scania_J5.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_hfp_a2dp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth Two Phone Full");
			 }
			else if(( System_Bluetooth_ph1 == 1 ) || (System_Bluetooth_ph2 == 1))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp_a2dp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_a2dp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One Phone Full");
			 }
			else if(( System_Bluetooth_ph1 == 3 ) && (System_Bluetooth_ph2 == 3 ))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp_hfp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_hfp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth Two phone only telephone");
			 }
			else if(( System_Bluetooth_ph1 == 0 && System_Bluetooth_ph2 == 2 ) ||
					( System_Bluetooth_ph1 == 2 && System_Bluetooth_ph2 == 0 ) )
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_a2dp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_a2dp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One phone only media");
			 }
			else if(( System_Bluetooth_ph1 == 0 && System_Bluetooth_ph2 == 3 ) ||
					( System_Bluetooth_ph1 == 3 && System_Bluetooth_ph2 == 0 ))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One phone only telephone");
			 }
			else
			{
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_remote.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_remote.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				DBG_MSG( " No Bluetooth Configured");
        	 }
			DBG_MSG( "Remote Functionality configured");
		}
		else
		{
			if( ( System_Bluetooth_ph1 == 1 && System_Bluetooth_ph2 == 1 ) ||
				( System_Bluetooth_ph1 == 1 && System_Bluetooth_ph2 == 3 ) ||
				( System_Bluetooth_ph1 == 3 && System_Bluetooth_ph2 == 1 ))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_scania_J5.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_hfp_a2dp_spp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth Two Phone Full");
			 }
			else if(( System_Bluetooth_ph1 == 1 ) || (System_Bluetooth_ph2 == 1))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp_a2dp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_a2dp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One Phone Full");
			 }
			else if(( System_Bluetooth_ph1 == 3 ) && (System_Bluetooth_ph2 == 3 ))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp_hfp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp_hfp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth Two phone only telephone");
			 }
			else if(( System_Bluetooth_ph1 == 0 && System_Bluetooth_ph2 == 2 ) ||
					( System_Bluetooth_ph1 == 2 && System_Bluetooth_ph2 == 0 ) )
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_a2dp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_a2dp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One phone only media");
			 }
			else if(( System_Bluetooth_ph1 == 0 && System_Bluetooth_ph2 == 3 ) ||
					( System_Bluetooth_ph1 == 3 && System_Bluetooth_ph2 == 0 ))
			 {
				fprintf(fptr_wicome,"%s", "-f /fs/etfs/wicomeConf/wicome_J5_hfp.cfg -r /fs/etfs/wicomeConf -s wicome_config\n");
				EXEC_CMD("cp /fs/etfs/wicomeConf/btservice_hfp.cfg /fs/etfs/wicomeConf/btservice_scania.cfg");
				EXEC_CMD("cp /fs/etfs/wicomeConf/dbservice_hfp.cfg /fs/etfs/wicomeConf/dbservice.cfg");
				DBG_MSG( "Bluetooth One phone only telephone");
			 }
			DBG_MSG( "Remote Functionality  not configured");
		}
	}
	else
	{
		DBG_MSG("No Scania EOL Bluetooth parameter is set properly and hence NO BT functionality");
	}
	fclose(fptr_wicome);

	return 0;
}



// EOF
