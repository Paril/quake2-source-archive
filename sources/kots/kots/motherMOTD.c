#include "g_local.h"

void KOTS_MOTD (edict_t *ent)		//added by Mother
{	
	
	//Norb changing the number below will cause it to display longer
	//1 means it will display 1 time (1.7 seconds) if you want to add more
	//to the MOTD you would want to move this up to 3 or 4 to give people time
	//to read it
	if(ent->MOTD_count < 2)//change to 2 or 3 if you want it to display longer
	{ 
		ent->MOTD_time = level.time + 1.7; //slows down MOTD to update every 1.7 secs
		ent->MOTD_count = ent->MOTD_count + 1;//add 1 to count

		if ( kots_teamplay->value )
			gi.centerprintf(ent, "Welcome to King of the Server Teamplay");
		else
			gi.centerprintf(ent, "Welcome to King of the Server");

	}//end if count < 1
	else //else stop showing MOTD
		ent->MOTD_time = 0; //dont show anymore
	
}//end the MOTD


