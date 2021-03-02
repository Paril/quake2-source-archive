/* s_readconfig.c */

#include "s_readconfig.h"
#include "g_local.h"
#include <stdio.h>


/* Function will grab all the KEYS, and variable/value
   pairs, each line is to be no longer than 120 characters */
void getKeys(NodePtr top)
{
	char pair[120];
	char Key[120];
	char k,b;
	int count=0;
	int strcount=0;
	char str[25];
	int found=0;

	printf("Reading 'config.txt' file...\n");
	while (top != NULL)
	{
	void readPair(char str[], char pair[]);

		count=0;
		strcount=0;

		for(b=0; b<120; b++)
			pair[b]='\0';

        strcpy(Key,top->value);
		k=Key[count];

		if (k != '[' && k != ';')
		{
			found=0;
			if (k != '\n') {
				strcpy(pair,Key);
		    if (strcmp(str,"")!=0) { // Make sure it's not blank
		  		  readPair(str, pair);
		   }
		}
		top = top->next;
		if (top != NULL) {  // Don't print if NULL
		  strcpy(Key,top->value);
		  k=Key[count];
		}

		} else {

		for(b=0; b<25; b++)
			str[b]='\0';

		// Find first bracket, get keyword
		if (k == '[') {
			count++;
			k=Key[count];
			while (k != ']') {
				 str[strcount]=k;
				 strcount++;
				 count++;
				 k=Key[count];
			}
			str[strcount] = '\0';
			found=1; //set flag
		}

	 top = top->next;

	}

} // end else
printf("Complete.\n");
}


void readPair(char str[], char pair[])
{

	if (strcmp("MOTD", str)== 0 )
		parseMOTD(pair);
	else if (strcmp("Misc", str)==0)
		parseMisc(pair);
	else if (strcmp("Grapple", str)==0)
		parseGrapple(pair); // Sconfig 2.6 (grapple)
	else if (strcmp("ServObits", str)==0)
		parseServObits(pair);
	else if (strcmp("BanWeapons",str)==0 )
		parseBan(pair);
	else if (strcmp("Rocket", str)==0)
		parseRocket(pair);
	else if (strcmp("GrenadeLauncher", str)==0)
		parseGLauncher(pair);
	else if (strcmp("Grenade", str)==0)
		parseGrenade(pair);
	else if (strcmp("Blaster", str)==0)
		parseBlaster(pair);
	else if (strcmp("Guns", str)==0)
		parseGuns(pair);
	else if (strcmp("Quad", str)==0)
		parseQuad(pair);
	else if (strcmp("LostQuad", str)==0)
		parseLostQuad(pair);
	else if (strcmp("LogOptions", str)==0)
		parseLogging(pair);
	else if (strcmp("StartWeapons", str)==0)
		parseStartWeapons(pair);
	else if (strcmp("StartItems", str)==0)
		parseStartItems(pair);
	else if (strcmp("StartAmmo", str)==0)
		parseStartAmmo(pair);
	else if (strcmp("MaxAmmo", str)==0)
		parseMaxAmmo(pair);
	else if (strcmp("Health", str)==0)
		parseHealth(pair);
	else if (strcmp("LevelChange", str)==0) // Only grav settings and such.
		parseLevelChange(pair);
}


void parseServObits(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start-1] = '\0'; /* Now we have value */
						   /* get rid of that \n */

/* Assignment code */

	if (strcmp(variable,"ModelGenDir")==0)
		strcpy(ModelGenDir,value);
	else if (strcmp(variable,"obitsDir")==0)
		strcpy(obitsDir,value); // both obits.txt and SrvObLog.txt
}


// Sconfig 2.6 (grapple)
void parseGrapple(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

	/* Assignment code */
	if (strcmp(variable,"EnableHook")==0) {
		allowgrapple=atoi(value);
		if (allowgrapple > 1) // some error checking
			allowgrapple=1;
	}
	else if (strcmp(variable,"HookTime")==0)
		HOOK_TIME=atoi(value);
	else if (strcmp(variable,"HookSpeed")==0)
		HOOK_SPEED=atoi(value);
    else if (strcmp(variable,"HookDamage")==0)
		HOOK_DAMAGE=atoi(value);
	else if (strcmp(variable,"PullSpeed")==0)
		PULL_SPEED=atoi(value);
	else if (strcmp(variable,"SkySolid")==0)
		EXPERT_SKY_SOLID=atoi(value);
	else if (strcmp(variable,"CloakGrapple")==0) {
		cloakgrapple=atoi(value);
		if (cloakgrapple > 1) // some error checking
			cloakgrapple=1;
	}
	else if (strcmp(variable,"HookPathColor")==0) {
		hookcolor=atoi(value);
		if (hookcolor > 6) // some error checking
			hookcolor=4;
	}
}


	
void parseMisc(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];
    int tempcloakvar=1;
	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value which includes return */

/* Assignment code */

	//if (strcmp(variable,"VWPatch")==0)
	//	VWPATCH=atoi(value);
	if (strcmp(variable,"NameBanning")==0)
		namebanning=atoi(value);
	
	//char PLAYERS_LOGFILE[50];
	else if (strcmp(variable, "ClientLog")==0){ 
    clientlog=atoi(value);
		if (clientlog > 1)
			clientlog = 1; // some level of error checking...
	}

	else if (strcmp(variable, "HighScoreDir")==0){ 
		value[start-1] = '\0'; /* Now we have value */
		strcpy(HIGHSCORE_DIR,value);
	}
	else if (strcmp(variable, "ClientLogFile")==0){ 
        value[start-1] = '\0'; /* Now we have value */
		strcpy(PLAYERS_LOGFILE,value);
	}
	else if (strcmp(variable, "BanDirectory")==0){
		value[start-1] = '\0'; /* Now we have value */
		strcpy(bandirectory,value);
	}
	else if (strcmp(variable, "MatchFullName")==0){
		matchfullnamevalue=atoi(value);
		if (matchfullnamevalue > 1)
			matchfullnamevalue = 1; // some level of error checking...
	}
	
	// default it 25000, some server ops make at 8000.. T1 is at full load
	// with 12 clients at 25000 rate.. Adjust accordingly
	else if (strcmp(variable,"MaxClientRate")==0){
		MAX_CLIENT_RATE=atoi(value);
		value[start-1] = '\0';
	    strcpy(MAX_CLIENT_RATE_STRING, value); // set to string and integer	
	}

	else if (strcmp(variable,"Cloaking")==0)
		cloaking=atoi(value);
	// sconfig 2.8
	else if (strcmp(variable, "ChaseKeepScore")==0)
		chasekeepscore=atoi(value);
	else if (strcmp(variable, "FastChangeWeapons")==0)
		fastchange=atoi(value);
	
	// end sconfig 2.8
	else if (strcmp(variable, "CloakingDrain")==0) {
		tempcloakvar=atoi(value);
		
		if (tempcloakvar == 0 )
			CLOAK_DRAIN = 0;
		else if (tempcloakvar == 1 )
			CLOAK_DRAIN = 20;
        else if (tempcloakvar == 2 )
	        CLOAK_DRAIN = 14;
		else if (tempcloakvar == 3 )
	        CLOAK_DRAIN = 8;
		else if (tempcloakvar == 4 )
	        CLOAK_DRAIN = 7;
		else if (tempcloakvar == 5 )
	        CLOAK_DRAIN = 6;
		else if (tempcloakvar == 6 )
	        CLOAK_DRAIN = 5;
		else if (tempcloakvar == 7 )
	        CLOAK_DRAIN = 4;
		else if (tempcloakvar == 8 )
	        CLOAK_DRAIN = 3;
		else if (tempcloakvar == 9 )
	        CLOAK_DRAIN = 2;
		else if (tempcloakvar == 10 )
	        CLOAK_DRAIN = 1;
	}

	else if (strcmp(variable,"Scoreboard")==0)
		scoreboard=atoi(value);
	else if (strcmp(variable,"timehud")==0)
		timehud=atoi(value);
	else if (strcmp(variable,"rankhud")==0)
		rankhud=atoi(value);
	else if (strcmp(variable,"playershud")==0)
		playershud=atoi(value);
}



void parseMOTD(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start-1] = '\0'; /* Now we have value */

	/* Assignment code */

	if (strcmp(variable,"line1")==0)
		strcpy(somevar0,value);
	else if (strcmp(variable,"line2")==0)
		strcpy(somevar1,value);
	else if (strcmp(variable,"line3")==0)
		strcpy(somevar2,value);
	else if (strcmp(variable,"line4")==0)
		strcpy(somevar3,value);
	else if (strcmp(variable,"line5")==0)
		strcpy(somevar4,value);
	else if (strcmp(variable,"line6")==0)
		strcpy(somevar5,value);
	else if (strcmp(variable,"line7")==0)
		strcpy(somevar6,value);
	else if (strcmp(variable,"line8")==0)
		strcpy(somevar7,value);
	else if (strcmp(variable,"line9")==0)
		strcpy(somevar8,value);
	else if (strcmp(variable,"line10")==0)
		strcpy(somevar9,value);
	else if (strcmp(variable,"line11")==0)
		strcpy(somevar10,value);
	else if (strcmp(variable,"line12")==0)
		strcpy(somevar11,value);
	else if (strcmp(variable,"line13")==0)
		strcpy(somevar12,value);
	else if (strcmp(variable,"line14")==0)
		strcpy(somevar13,value);
	else if (strcmp(variable,"line15")==0)
		strcpy(somevar14,value);
}

void parseBan(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

	/* Assignment code */

	if (strcmp(variable,"banShotgun")==0)
		ban_shotgun=atoi(value);
	else if (strcmp(variable,"banSupershotgun")==0)
		ban_supershotgun=atoi(value);
	else if (strcmp(variable,"banMachinegun")==0)
		ban_machinegun=atoi(value);
	else if (strcmp(variable,"banChaingun")==0)
		ban_chaingun=atoi(value);
	else if (strcmp(variable,"banGrenadelauncher")==0)
		ban_grenadelauncher=atoi(value);
	else if (strcmp(variable,"banRocketlauncher")==0)
		ban_rocketlauncher=atoi(value);
	else if (strcmp(variable,"banHyperblaster")==0)
		ban_hyperblaster=atoi(value);
	else if (strcmp(variable,"banRailgun")==0)
		ban_railgun=atoi(value);
	else if (strcmp(variable,"banBFG")==0)
		ban_BFG=atoi(value);

}


void parseLostQuad(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"LoseQuad")==0)
		LoseQ=atoi(value);
	else if (strcmp(variable,"Fragee")==0)
		LoseQ_Fragee=atoi(value);

}


void parseHealth(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"StartHealth")==0)
		CF_StartHealth=atoi(value);
	else if (strcmp(variable,"MaxHealth")==0)
		CF_MaxHealth=atoi(value);

}

void parseMaxAmmo(char pair[])
{

	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"Bullets")==0)
		MA_Bullets=atoi(value);
	else if (strcmp(variable,"Shells")==0)
		MA_Shells=atoi(value);
	else if (strcmp(variable,"Cells")==0)
		MA_Cells=atoi(value);
	else if (strcmp(variable,"Grenades")==0)
		MA_Grenades=atoi(value);
	else if (strcmp(variable,"Rockets")==0)
		MA_Rockets=atoi(value);
	else if (strcmp(variable,"Slugs")==0)
		MA_Slugs=atoi(value);

}

void parseStartAmmo(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"Bullets")==0)
		SA_Bullets=atoi(value);
	else if (strcmp(variable,"Shells")==0)
		SA_Shells=atoi(value);
	else if (strcmp(variable,"Cells")==0)
		SA_Cells=atoi(value);
	else if (strcmp(variable,"Grenades")==0)
		SA_Grenades=atoi(value);
	else if (strcmp(variable,"Rockets")==0)
		SA_Rockets=atoi(value);
	else if (strcmp(variable,"Slugs")==0)
		SA_Slugs=atoi(value);

}


void parseStartItems(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"QuadDamage")==0)
		SI_QuadDamage=atoi(value);
	else if (strcmp(variable,"Invulnerability")==0)
		SI_Invulnerability=atoi(value);
	else if (strcmp(variable,"Silencer")==0)
		SI_Silencer=atoi(value);
	else if (strcmp(variable,"Rebreather")==0)
		SI_Rebreather=atoi(value);
	else if (strcmp(variable,"EnvironmentSuit")==0)
		SI_EnvironmentSuit=atoi(value);
	else if (strcmp(variable,"PowerScreen")==0)
		SI_PowerScreen=atoi(value);
	else if (strcmp(variable,"PowerShield")==0)
		SI_PowerShield=atoi(value);
	// new items in v1.5, all check bounds
	else if (strcmp(variable,"QuadDamageTime")==0) {
		QuadDamageTime=(atoi(value))*10;
		if (QuadDamageTime < 100 || QuadDamageTime > 1200)
			QuadDamageTime = 300;
	}
	
	else if (strcmp(variable,"RebreatherTime")==0) {
		RebreatherTime=(atoi(value))*10;
 		if (RebreatherTime < 100 || RebreatherTime > 1200)
			RebreatherTime = 300;
	}
    
	else if (strcmp(variable,"EnvironmentSuitTime")==0) {
		EnvironmentSuitTime=(atoi(value))*10;
 		if (EnvironmentSuitTime < 100 || EnvironmentSuitTime > 1200)
			EnvironmentSuitTime = 300;
	}
    
	else if (strcmp(variable,"InvulnerabilityTime")==0) {
		InvulnerabilityTime=(atoi(value))*10;
		if (InvulnerabilityTime < 100 || InvulnerabilityTime > 1200)
			InvulnerabilityTime = 300;
	}

	else if (strcmp(variable,"SilencerShots")==0) {
		SilencerShots=atoi(value);
		if (SilencerShots < 10 || SilencerShots > 120)
			SilencerShots = 30;
	}
	
	else if (strcmp(variable,"RegenInvulnerability")==0) {
  	    RegenInvulnerability=atoi(value); // 0 or 1
		  if (RegenInvulnerability < 0 || RegenInvulnerability > 1)
			  RegenInvulnerability = 0; // set to false
	}
	else if (strcmp(variable,"RegenInvulnerabilityTime")==0) {
  	    RegenInvulnerabilityTime=(atoi(value))*10; 
		 if (RegenInvulnerabilityTime > 100 || RegenInvulnerabilityTime < 30) // 3-10 seconds
			 RegenInvulnerabilityTime=0; // Set to 0.
	}

	else if (strcmp(variable,"AutoUseQuad")==0) {
  	    AutoUseQuad=atoi(value); 
		 if (AutoUseQuad < 0 || AutoUseQuad > 1)
			 AutoUseQuad=0; // Set to 0.
	}

	else if (strcmp(variable,"AutoUseInvulnerability")==0) {
  	    AutoUseInvulnerability=atoi(value); 
		 if (AutoUseInvulnerability < 0 || AutoUseInvulnerability > 1)
			 AutoUseInvulnerability=0; // Set to 0.
	}

}


void parseStartWeapons(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"Blaster")==0)
		SW_Blaster=atoi(value);
	else if (strcmp(variable,"ShotGun")==0)
		SW_ShotGun=atoi(value);
	else if (strcmp(variable,"SuperShotGun")==0)
		SW_SuperShotGun=atoi(value);
	else if (strcmp(variable,"MachineGun")==0)
		SW_MachineGun=atoi(value);
	else if (strcmp(variable,"ChainGun")==0)
		SW_ChainGun=atoi(value);
	else if (strcmp(variable,"GrenadeLauncher")==0)
		SW_GrenadeLauncher=atoi(value);
	else if (strcmp(variable,"RocketLauncher")==0)
		SW_RocketLauncher=atoi(value);
	else if (strcmp(variable,"HyperBlaster")==0)
		SW_HyperBlaster=atoi(value);
	else if (strcmp(variable,"RailGun")==0)
		SW_RailGun=atoi(value);
	else if (strcmp(variable,"BFG10K")==0)
		SW_BFG10K=atoi(value);
}

void parseQuad(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"Killer")==0)
		Q_Killer=atoi(value);
	else if (strcmp(variable,"Killee")==0)
		Q_Killee=atoi(value);
}


void parseRocket(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"Damage")==0)
		ConfigRD=atoi(value);
    //if (strcmp(variable,"Speed")==0) {
	//  	rocketSpeed=atoi(value);
	//	 if (rocketSpeed <1 || rocketSpeed >2)
	//		 rocketSpeed=1;
	//} // simple debug message, assure default of 1 is assigned
	
	else if (strcmp(variable, "FastRocketFire")==0)
		fastrocketfire=atoi(value);
	else if (strcmp(variable, "RocketVelocity")==0)
		rocketspeed=atoi(value);
	else if (strcmp(variable, "RadiusDamage")==0)
		RadiusDamage=atoi(value);
	else if (strcmp(variable, "DamageRadius")==0)
		DamageRadius=atoi(value);

}

void parseGLauncher(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */


	if (strcmp(variable,"Timer")==0)
		GLauncherTimer=atof(value);
	else if (strcmp(variable, "FireDistance")==0)
		GLauncherFireDistance=atoi(value);
	else if (strcmp(variable, "Damage")==0)
		GLauncherDamage=atoi(value);
	else if (strcmp(variable, "Radius")==0)
		GLauncherRadius=atoi(value);

}

void parseGrenade(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"Timer")==0)
		GrenadeTimer=atof(value);
	else if (strcmp(variable, "Radius")==0)
		GrenadeRadius=atoi(value);
	else if (strcmp(variable, "Damage")==0)
		GrenadeDamage=atoi(value);
	else if (strcmp(variable, "MinSpeed")==0)
		GrenadeMinSpeed=atoi(value);
	else if (strcmp(variable, "MaxSpeed")==0)
		GrenadeMaxSpeed=atoi(value);

}

void parseBlaster(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"HyperBlasterDamage")==0)
		HyperBlasterDamage=atoi(value);
	else if (strcmp(variable, "ProjectileSpeed")==0)
		BlasterProjectileSpeed=atoi(value);
	else if (strcmp(variable, "BlasterDamage")==0)
		BlasterDamage=atoi(value);
}

void parseGuns(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start] = '\0'; /* Now we have value */

/* Assignment code */

	if (strcmp(variable,"MachinegunDamage")==0)
		MachinegunDamage=atoi(value);
	else if (strcmp(variable, "MachinegunKick")==0)
		MachinegunKick=atoi(value);
	else if (strcmp(variable, "ChaingunDamage")==0)
		ChaingunDamage=atoi(value);
	else if (strcmp(variable, "ChaingunKick")==0)
		ChaingunKick=atoi(value);
	
	
	else if (strcmp(variable, "ShotgunDamage")==0)
		ShotgunDamage=atoi(value);
	else if (strcmp(variable, "ShotgunKick")==0)
		ShotgunKick=atoi(value);
	else if (strcmp(variable, "SuperShotgunDamage")==0)
		SuperShotgunDamage=atoi(value);
	else if (strcmp(variable, "SuperShotgunKick")==0)
		SuperShotgunKick=atoi(value);
	else if (strcmp(variable, "FastRailgun")==0)
	    fastrailgun=atoi(value);
	else if (strcmp(variable, "RailgunDamage")==0)
		RailgunDamage=atoi(value);
	else if (strcmp(variable, "RailgunKick")==0)
		RailgunKick=atoi(value);
	else if (strcmp(variable, "BFGDamage")==0)
		BFGDamage=atoi(value);
	else if (strcmp(variable, "BFGDamageRadius")==0)
		BFGDamageRadius=atof(value);
	else if (strcmp(variable, "BFGProjectileSpeed")==0)
		BFGProjectileSpeed=atoi(value);

}

void parseLevelChange(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];
	
	length = strlen(pair);
	count = 0;
	start=0;


	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start-1] = '\0'; /* Now we have value */

/* Assignment code */
	
	/* Handle Default Fraglimit and Default Time limit */
	if (strcmp(variable,"GlobalFragLimit")==0)
		strcpy(GlobalFragLimit,value);
	else if (strcmp(variable,"GlobalTimeLimit")==0)
		strcpy(GlobalTimeLimit,value);
	else if (strcmp(variable,"GlobalGravity")==0)
		strcpy(GlobalGravity,value);


}


void parseLogging(char pair[])
{
	int d, length,count,start;
	char variable[25];
	char value[120];

	length = strlen(pair);
	count = 0;
	start=0;

	/* Get count (will be one less than what we need) */
	while (pair[count] != '=')
	{
		variable[count] = pair[count];
		count++;
	}

	variable[count] = '\0'; /* Now we have variable */

	for(d=count+1; d<strlen(pair); d++)
	{
		value[start] = pair[d];
		start++;
	}

	value[start-1] = '\0'; /* Now we have value */
						   /* get rid of that \n */

/* Assignment code */

	if (strcmp(variable,"QWStyleLogging")==0) {
		QWLOG=atoi(value);
		if (QWLOG != 0) 
		  strcpy(recordLOG, value);
		recordLOG[1]='\0';
	}
	else if (strcmp(variable,"Dir")==0)
		strcpy(directory,value);
	
}
