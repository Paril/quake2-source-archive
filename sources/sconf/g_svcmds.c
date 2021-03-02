
#include "g_local.h"
#include "s_readconfig.h"

// IP BANNING, 3.20 SOURCE

typedef struct
{
	unsigned	mask;
	unsigned	compare;
} ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;

// END IP BANNING, 3.20 SOURCE

void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

// ***********************************************************
// Start of Assorted
// ***********************************************************
void	Svcmd_timehud_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		timehud=atoi(cmdvalue);
		if (timehud == 0)
			gi.bprintf (PRINT_MEDIUM, "Time removed from HUD\n");
			
		else 
			gi.bprintf (PRINT_MEDIUM, "Time added to HUD");
	}
	// If they are just performing an inquiry give them the current value
	else if (timehud == 0)
		gi.dprintf ("Time is not on the HUD\n");
	else
		gi.dprintf ("Time is on the HUD\n");
}	


void Svcmd_chasekeepscore_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		chasekeepscore=atoi(cmdvalue);
	if (chasekeepscore == 0)
	  gi.bprintf (PRINT_MEDIUM, "Entering Chase Cam will result in the reset of your score.\n");
	else
	  gi.bprintf (PRINT_MEDIUM, "Entering Chase Cam will allow you to keep your score.\n");
	}
	else if (chasekeepscore == 0)
		gi.dprintf ("Players will NOT keep score when they enter chase cam. (0)\n");
	else
		gi.dprintf ("Players WILL keep score when they enter chase cam. (1)\n");
}

void	Svcmd_playershud_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
	playershud=atoi(cmdvalue);
	if (playershud == 0)
	  gi.bprintf (PRINT_MEDIUM, "Players removed from HUD\n");
	else
	  gi.bprintf (PRINT_MEDIUM, "Players added to HUD\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (playershud == 0)
		gi.dprintf ("Players is not on the HUD\n");
	else
		gi.dprintf ("Players is on the HUD\n");
}	

void	Svcmd_rankhud_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
	rankhud=atoi(cmdvalue);
	  if (rankhud == 0)
	    gi.bprintf (PRINT_MEDIUM, "Rank removed from HUD\n");
	  else 
		gi.bprintf (PRINT_MEDIUM, "Rank added to HUD\n");	
	}
	// If they are just performing an inquiry give them the current value
	else if (rankhud == 0)
		gi.dprintf ("Rank is not on the HUD\n");
	else
		gi.dprintf ("Rank is on the HUD\n");
}	


void	Svcmd_starthealth_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		CF_StartHealth=atoi(cmdvalue);
		  gi.bprintf (PRINT_MEDIUM, "Start Health set to %i\n", CF_StartHealth);
	}
	// If they are just performing an inquiry give them the current value
	else 
		gi.dprintf ("Start Health set to %i\n", CF_StartHealth);
}	
void	Svcmd_maxhealth_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		CF_MaxHealth=atoi(cmdvalue);
		  gi.bprintf (PRINT_MEDIUM, "Max Health set to %i\n", CF_MaxHealth);
	}
	// If they are just performing an inquiry give them the current value
	else 
		gi.dprintf ("Max Health set to %i\n", CF_MaxHealth);
}	

/*void	Svcmd_vwep_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		VWPATCH=atoi(cmdvalue);
		if (VWPATCH == 1)
		  gi.bprintf (PRINT_MEDIUM, "Visual Weapons Patch is always on.\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Visual Weapons Patch is always on.\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (VWPATCH == 1)
		gi.dprintf ("Visual Weapons Patch is always on.\n");   
	else
		gi.dprintf ("Visual Weapons Patch is always on.\n");   
}	*/

void	Svcmd_scoreboard_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		scoreboard=atoi(cmdvalue);
		if (scoreboard == 0)
		  gi.bprintf (PRINT_MEDIUM, "Using standard scoreboard\n");
		else if (scoreboard == 1)
		  gi.bprintf (PRINT_MEDIUM, "Using enchanced scoreboard\n");
		else if (scoreboard == 2)
		  gi.bprintf (PRINT_MEDIUM, "Using split/spectator enhanced scoreboard\n");
	}
	// If they are just performing an inquiry give them the current value
	else 
		gi.dprintf ("Scoreboard mode: %i\n", scoreboard);   
}
void	Svcmd_globalfraglimit_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		strcpy(GlobalFragLimit,cmdvalue);
		  gi.bprintf (PRINT_MEDIUM, "Global Fraglimit set to %s\n", GlobalFragLimit);
	}
	else
		gi.dprintf("Global Fraglimit: %s\n", GlobalFragLimit);
}
void	Svcmd_globaltimelimit_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		strcpy(GlobalTimeLimit,cmdvalue);
		  gi.bprintf (PRINT_MEDIUM, "Global Timelimit set to %s\n", GlobalTimeLimit);
	}
	else
		gi.dprintf("Global Timelimit: %s\n", GlobalTimeLimit);
}
void	Svcmd_globalgravitylimit_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		strcpy(GlobalGravity,cmdvalue);
		  gi.bprintf (PRINT_MEDIUM, "Global Gravity set to %s\n", GlobalGravity);
	}
	else
		gi.dprintf("Global Gravity: %s\n", GlobalGravity);
}
// ***********************************************************
// End of Assorted
// ***********************************************************

// ***********************************************************
// Start of Start Weapons
// ***********************************************************
void	Svcmd_startshotgun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_ShotGun=atoi(cmdvalue);
		if (SW_ShotGun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Shotgun\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Shotgun\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_ShotGun == 1)
		gi.dprintf ("Start with Shotgun (1)\n");   
	else
		gi.dprintf ("Do not start with Shotgun (0)\n");   
}
void	Svcmd_startsupershotgun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_SuperShotGun=atoi(cmdvalue);
		if (SW_SuperShotGun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Super Shotgun\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Super Shotgun\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_SuperShotGun == 1)
		gi.dprintf ("Start with Super Shotgun (1)\n");   
	else
		gi.dprintf ("Do not start with Super Shotgun (0)\n");   
}
void	Svcmd_startmachinegun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_MachineGun=atoi(cmdvalue);
		if (SW_MachineGun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Machinegun\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Machinegun\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_MachineGun == 1)
		gi.dprintf ("Start with Machinegun (1)\n");   
	else
		gi.dprintf ("Do not start with Machinegun (0)\n");   
}
void	Svcmd_startchaingun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_ChainGun=atoi(cmdvalue);
		if (SW_ChainGun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Chaingun\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Chaingun\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_ChainGun == 1)
		gi.dprintf ("Start with Chaingun (1)\n");   
	else
		gi.dprintf ("Do not start with Chaingun (0)\n");   
}
void	Svcmd_startgrenadelaunch_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_GrenadeLauncher=atoi(cmdvalue);
		if (SW_GrenadeLauncher == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Grenade Launcher\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Grenade Launcher\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_GrenadeLauncher == 1)
		gi.dprintf ("Start with Grenade Launcher (1)\n");   
	else
		gi.dprintf ("Do not start with Grenade Launcher (0)\n");   
}
void	Svcmd_startrocketlaunch_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_RocketLauncher=atoi(cmdvalue);
		if (SW_RocketLauncher == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Rocket Launcher\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Rocket Launcher\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_RocketLauncher == 1)
		gi.dprintf ("Start with Rocket Launcher (1)\n");   
	else
		gi.dprintf ("Do not start with Rocket Launcher (0)\n");   
}
void	Svcmd_starthyperblaster_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_HyperBlaster=atoi(cmdvalue);
		if (SW_HyperBlaster == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Hyperblaster\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Hyperblaster\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_HyperBlaster == 1)
		gi.dprintf ("Start with Hyperblaster (1)\n");   
	else
		gi.dprintf ("Do not start with Hyperblaster (0)\n");   
}
void	Svcmd_startrailgun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_RailGun=atoi(cmdvalue);
		if (SW_RailGun == 1)
			gi.bprintf (PRINT_MEDIUM, "Start with Railgun\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Railgun\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_RailGun == 1)
		gi.dprintf ("Start with Railgun (1)\n");   
	else
		gi.dprintf ("Do not start with Railgun (0)\n");   
}
void	Svcmd_startbfg_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SW_BFG10K=atoi(cmdvalue);
		if (SW_BFG10K == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with BFG10K\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with BFG10K\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SW_BFG10K == 1)
		gi.dprintf ("Start with BFG10K (1)\n");   
	else
		gi.dprintf ("Do not start with BFG10K (0)\n");   
}
// ***********************************************************
// End of Start Weapons
// ***********************************************************

// ***********************************************************
// Start of Start Ammo and Max Ammo Settings
// ***********************************************************
void	Svcmd_startbullets_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SA_Bullets=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Start with %i bullets\n", SA_Bullets);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Start Bullets: %i\n", SA_Bullets);   
}
void	Svcmd_startshells_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SA_Shells=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Start with %i shells\n", SA_Shells);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Start Shells: %i\n", SA_Shells);   
}
void	Svcmd_startgrenades_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SA_Grenades=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Start with %i grenades\n", SA_Grenades);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Start Grenades: %i\n", SA_Grenades);   
}
void	Svcmd_startrockets_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SA_Rockets=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Start with %i rockets\n", SA_Rockets);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Start Rockets: %i\n", SA_Rockets);   
}
void	Svcmd_startslugs_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SA_Slugs=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Start with %i slugs\n", SA_Slugs);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Start Slugs: %i\n", SA_Slugs);   
}
void	Svcmd_startcells_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SA_Cells=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Start with %i cells\n", SA_Cells);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Start Cells: %i\n", SA_Cells);   
}
void	Svcmd_maxbullets_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		MA_Bullets=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Max bullets set to %i\n", MA_Bullets);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Max Bullets: %i\n", MA_Bullets);   
}
void	Svcmd_maxshells_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		MA_Shells=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Max shells set to %i\n", MA_Shells);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Max Shells: %i\n", MA_Shells);
}
void	Svcmd_maxgrenades_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		MA_Grenades=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Max grenades set to %i\n", MA_Grenades);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Max Grenades: %i\n", MA_Grenades);
}
void	Svcmd_maxcells_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		MA_Cells=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Max cells set to %i\n", MA_Cells);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Max Cells: %i\n", MA_Cells);
}
void	Svcmd_maxrockets_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		MA_Rockets=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Max rockets set to %i\n", MA_Rockets);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Max Rockets: %i\n", MA_Rockets);
}
void	Svcmd_maxslugs_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		MA_Slugs=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Max slugs set to %i\n", MA_Slugs);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Max Slugs: %i\n", MA_Slugs);
}
// ***********************************************************
// End of Start Ammo and Max Ammo Settings
// ***********************************************************

// ***********************************************************
// Start of Start Item Settings
// ***********************************************************
void	Svcmd_start4x_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SI_QuadDamage=atoi(cmdvalue);
		if (SI_QuadDamage == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Quad Damage\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Quad Damage\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SI_QuadDamage == 1)
		gi.dprintf ("Start with Quad Damage (1)\n");   
	else
		gi.dprintf ("Do not start Quad Damage (0)\n");   
}
void	Svcmd_start4xtime_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		QuadDamageTime=(atoi(cmdvalue))*10;
		// If out of bounds
		if (QuadDamageTime < 100 || QuadDamageTime > 1200) {
			 QuadDamageTime=300; // Default is 30 seconds
			 gi.bprintf (PRINT_MEDIUM, "Quad Damage time now 30 seconds\n");
			 gi.dprintf ("Valid vales are 10 - 120, set default to 30 seconds\n");
		}
		else
		  gi.bprintf (PRINT_MEDIUM, "Quad Damage time now %i seconds\n", (QuadDamageTime/10));
	}
	else
		gi.dprintf ("Quad Damage time: %i seconds\n", (QuadDamageTime/10));
}
void	Svcmd_startautouse4x_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		 AutoUseQuad=atoi(cmdvalue);
		// If out of bounds
		if (AutoUseQuad < 0 || AutoUseQuad > 1) {
			 AutoUseQuad=0; // Default is 0 (off)
			 gi.bprintf (PRINT_MEDIUM, "Auto use Quad is off\n");
			 gi.dprintf ("Valid vales are 0 or 1, set default to off (0)\n");
		}
		else if (AutoUseQuad == 1)
		    gi.bprintf (PRINT_MEDIUM, "Auto use Quad is on\n");
		else
			gi.bprintf (PRINT_MEDIUM, "Auto use Quad is off\n");
	}
	else
		gi.dprintf ("Auto use Quad: %i", AutoUseQuad);
}

void	Svcmd_start666_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SI_Invulnerability=atoi(cmdvalue);
		if (SI_Invulnerability == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Invulnerability\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Invulnerability\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SI_Invulnerability == 1)
		gi.dprintf ("Start with Invulnerability (1)\n");   
	else
		gi.dprintf ("Do not start Invulnerability (0)\n");   
}
void	Svcmd_start666time_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		InvulnerabilityTime=(atoi(cmdvalue))*10;
		// If out of bounds
		if (InvulnerabilityTime < 100 || InvulnerabilityTime > 1200) {
			 InvulnerabilityTime=300; // Default is 30 seconds
			 gi.bprintf (PRINT_MEDIUM, "Invulnerability time now 30 seconds\n");
			 gi.dprintf ("Valid vales are 10 - 120, set default to 30 seconds\n");
		}
		else
		  gi.bprintf (PRINT_MEDIUM, "Invulnerability time now %i seconds\n", (InvulnerabilityTime/10));
	}
	else
		gi.cprintf (NULL, PRINT_MEDIUM, "Invulnerability time: %i seconds\n", (InvulnerabilityTime/10));
}
void	Svcmd_startautouse666_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		AutoUseInvulnerability=atoi(cmdvalue);
		// If out of bounds
		if (AutoUseInvulnerability < 0 || AutoUseInvulnerability > 1) {
			 AutoUseInvulnerability=0; // Default is 0 (off)
			 gi.bprintf (PRINT_MEDIUM, "Auto use Invulnerability is off\n");
			 gi.dprintf ("Valid vales are 0 or 1, set default to off (0)\n");
		}
		else if (AutoUseInvulnerability == 1)
		    gi.bprintf (PRINT_MEDIUM, "Auto use Invulnerability is on\n");
		else
			gi.bprintf (PRINT_MEDIUM, "Auto use Invulnerability is off\n");
	}
	else
		gi.dprintf ("Auto use Invulnerability: %i", AutoUseInvulnerability);
}
void	Svcmd_startsilencer_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SI_Silencer=atoi(cmdvalue);
		if (SI_Silencer == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Silencer\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Silencer\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SI_Silencer == 1)
		gi.dprintf ("Start with Silencer (1)\n");   
	else
		gi.dprintf ("Do not start with Silencer (0)\n");   
}
void	Svcmd_startsilencershots_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SilencerShots=atoi(cmdvalue);
		// If out of bounds
		if (SilencerShots < 10 || SilencerShots > 120) {
			 SilencerShots=30; // Default is 30 seconds
			 gi.bprintf (PRINT_MEDIUM, "Silencer shots total 30\n");
			 gi.dprintf ("Valid vales are 10 - 120, set default to 30 shots\n");
		}
		else
		  gi.bprintf (PRINT_MEDIUM, "Silencer shots total: %i shots\n", SilencerShots);
	}
	else
		gi.dprintf ("Silencer shots total: %i shots\n", SilencerShots);
}
void	Svcmd_startrebreath_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SI_Rebreather=atoi(cmdvalue);
		if (SI_Rebreather == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Rebreather\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Rebreather\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SI_Rebreather == 1)
		gi.dprintf ("Start with Rebreather (1)\n");   
	else
		gi.dprintf ("Do not start with Rebreather (0)\n");   
}
void	Svcmd_startrebreathtime_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		RebreatherTime=(atoi(cmdvalue))*10;
		// If out of bounds
		if (RebreatherTime < 100 || RebreatherTime > 1200) {
			 RebreatherTime=300; // Default is 30 seconds
			 gi.bprintf (PRINT_MEDIUM, "Rebreather time now 30 seconds\n");
			 gi.dprintf ("Valid vales are 10 - 120, set default to 30 seconds\n");
		}
		else
		  gi.bprintf (PRINT_MEDIUM, "Rebreather time now %i seconds\n", (RebreatherTime/10));
	}
	else
		gi.dprintf ("Rebreather time: %i seconds\n", (RebreatherTime/10));
}
void	Svcmd_startenvsuit_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SI_EnvironmentSuit=atoi(cmdvalue);
		if (SI_EnvironmentSuit == 1)
		  gi.bprintf (PRINT_MEDIUM, "Start with Environment Suit\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Do not start with Environment Suit\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (SI_EnvironmentSuit == 1)
		gi.dprintf ("Start with Environment Suit (1)\n");   
	else
		gi.dprintf ("Do not start with Environment Suit (0)\n");   
}
void	Svcmd_startenvsuittime_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		EnvironmentSuitTime=(atoi(cmdvalue))*10;
		// If out of bounds
		if (EnvironmentSuitTime < 100 || EnvironmentSuitTime > 1200) {
			 EnvironmentSuitTime=300; // Default is 30 seconds
			 gi.bprintf (PRINT_MEDIUM, "Environment Suit time now 30 seconds\n");
			 gi.dprintf ("Valid vales are 10 - 120, set default to 30 seconds\n");
		}
		else
		  gi.bprintf (PRINT_MEDIUM, "Environment Suit time now %i seconds\n", (EnvironmentSuitTime/10));
	}
	else
		gi.dprintf ("Environment Suit time: %i seconds\n", (EnvironmentSuitTime/10));
}
void	Svcmd_startregen666_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		RegenInvulnerability=atoi(cmdvalue);
		if (RegenInvulnerability == 1)
		  gi.bprintf (PRINT_MEDIUM, "Regen Invulnerability Active\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Regen Invulnerability Deactivated\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (RegenInvulnerability == 1)
		gi.dprintf ("Regen Invulnerability Active (1)\n");   
	else
		gi.dprintf ("Regen Invulnerability Deactivated (0)\n");   
}
void	Svcmd_startregen666time_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		RegenInvulnerabilityTime=(atoi(cmdvalue))*10;
		// If out of bounds
		if (RegenInvulnerabilityTime > 100 || RegenInvulnerabilityTime < 30) {
			 RegenInvulnerabilityTime=50;
			 gi.bprintf (PRINT_MEDIUM, "Regen Invulnerability time now 5 seconds\n");
			 gi.dprintf ("Valid vales are 3 - 10, set default to 5 seconds\n");
		}
		else
		  gi.bprintf (PRINT_MEDIUM, "Regen Invulnerability time now %i seconds\n", (RegenInvulnerabilityTime/10));
	}
	else 
		gi.dprintf ("Regen Invulnerability time: %i seconds\n", (RegenInvulnerabilityTime/10));
}
// ***********************************************************
// End of Start Item Settings
// ***********************************************************

// ***********************************************************
// Start Banning Weapon Settings
// ***********************************************************
void	Svcmd_banshotgun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_shotgun=atoi(cmdvalue);
		if (ban_shotgun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Shotgun is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Shotgun is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_shotgun == 1)
		gi.dprintf ("Shotgun is banned (1)\n");   
	else
		gi.dprintf ("Shotgun is live! (0)\n");   
}
void	Svcmd_bansupershotgun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_supershotgun=atoi(cmdvalue);
		if (ban_supershotgun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Super Shotgun is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Super Shotgun is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_supershotgun == 1)
		gi.dprintf ("Super Shotgun is banned (1)\n");   
	else
		gi.dprintf ("Super Shotgun is live! (0)\n");   
}
void	Svcmd_banmachinegun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_machinegun=atoi(cmdvalue);
		if (ban_machinegun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Machinegun is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Machinegun is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_machinegun == 1)
		gi.dprintf ("Machinegun is banned (1)\n");   
	else
		gi.dprintf ("Machinegun is live! (0)\n");   
}
void	Svcmd_banchaingun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_chaingun=atoi(cmdvalue);
		if (ban_chaingun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Chaingun is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Chaingun is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_chaingun == 1)
		gi.dprintf ("Chaingun is banned (1)\n");   
	else
		gi.dprintf ("Chaingun is live! (0)\n");   
}
void	Svcmd_bangrenadelauncher_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_grenadelauncher=atoi(cmdvalue);
		if (ban_grenadelauncher == 1)
		  gi.bprintf (PRINT_MEDIUM, "Grenade Launcher is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Grenade Launcher is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_grenadelauncher == 1)
		gi.dprintf ("Grenade Launcher is banned (1)\n");   
	else
		gi.dprintf ("Grenade Launcher is live! (0)\n");   
}
void	Svcmd_banrocketlauncher_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_rocketlauncher=atoi(cmdvalue);
		if (ban_rocketlauncher == 1)
		  gi.bprintf (PRINT_MEDIUM, "Rocket Launcher is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Rocket Launcher is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_rocketlauncher == 1)
		gi.dprintf ("Rocket Launcher is banned (1)\n");   
	else
		gi.dprintf ("Rocket Launcher is live! (0)\n");   
}
void	Svcmd_banhyperblaster_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_hyperblaster=atoi(cmdvalue);
		if (ban_hyperblaster == 1)
		  gi.bprintf (PRINT_MEDIUM, "Hyperblaster is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Hyperblaster is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_hyperblaster == 1)
		gi.dprintf ("Hyperblaster is banned (1)\n");   
	else
		gi.dprintf ("Hyperblaster is live! (0)\n");   
}
void	Svcmd_banrailgun_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_railgun=atoi(cmdvalue);
		if (ban_railgun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Railgun is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Railgun is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_railgun == 1)
		gi.dprintf ("Railgun is banned (1)\n");   
	else
		gi.dprintf ("Railgun is live! (0)\n");   
}
void	Svcmd_banbfg_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ban_BFG=atoi(cmdvalue);
		if (ban_BFG == 1)
		  gi.bprintf (PRINT_MEDIUM, "BFG is banned!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "BFG is live!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (ban_BFG == 1)
		gi.dprintf ("BFG is banned (1)\n");   
	else
		gi.dprintf ("BFG is live! (0)\n");   
}
// ***********************************************************
// End Banning Weapon Settings
// ***********************************************************
	
// ***********************************************************
// Start Rocket Launcher Damage Settings
// ***********************************************************
void	Svcmd_rocketdamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ConfigRD=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Rocket Damage set to %i\n", ConfigRD);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Rocket Damage: %i\n", ConfigRD);   
}
void	Svcmd_rocketvelocity_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		rocketspeed=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Rocket Velocity set to %i\n", rocketspeed);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Rocket Velocity: %i\n", rocketspeed);   
}
void	Svcmd_rocketradiusdamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		RadiusDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Rocket Radius Damage set to %i\n", RadiusDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Rocket Radius Damage: %i\n", RadiusDamage);   
}
void	Svcmd_rocketdamageradius_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		DamageRadius=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Rocket Damage Radius set to %i\n", DamageRadius);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Rocket Damage Radius: %i\n", DamageRadius);   
}
// ***********************************************************
// End Rocket Launcher Damage Settings
// ***********************************************************

// ***********************************************************
// Start Grenade Launcher Damage Settings
// ***********************************************************
void	Svcmd_glaunchertimer_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GLauncherTimer=atof(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Launcher Timer set to %f\n", GLauncherTimer);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Launcher Timer: %i\n", GLauncherTimer);   
}
void	Svcmd_glauncherfiredis_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GLauncherFireDistance=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Launcher Fire Distance set to %i\n", GLauncherFireDistance);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Launcher Fire Distance: %i\n", GLauncherFireDistance);   
}
void	Svcmd_glauncherdamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GLauncherDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Launcher Damage set to %i\n", GLauncherDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Launcher Damage: %i\n", GLauncherDamage);   
}
void	Svcmd_glauncherradius_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GLauncherRadius=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Launcher Radius set to %i\n", GLauncherRadius);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Launcher Radius: %i\n", GLauncherRadius);   
}
// ***********************************************************
// End Grenade Launcher Damage Settings
// ***********************************************************

// ***********************************************************
// Start Grenade Damage Settings
// ***********************************************************
void	Svcmd_grenadetimer_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GrenadeTimer=atof(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Timer set to %f\n", GrenadeTimer);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Timer: %i\n", GrenadeTimer);   
}
void	Svcmd_grenademinspeed_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GrenadeMinSpeed=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Min Speed set to %i\n", GrenadeMinSpeed);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Min Speed: %i\n", GrenadeMinSpeed);   
}
	void	Svcmd_grenademaxspeed_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GrenadeMaxSpeed=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Max Speed set to %i\n", GrenadeMaxSpeed);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Max Speed: %i\n", GrenadeMaxSpeed);   
}
void	Svcmd_grenadedamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GrenadeDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Damage set to %i\n", GrenadeDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Damage: %i\n", GrenadeDamage);   
}
void	Svcmd_grenaderadius_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		GrenadeRadius=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Grenade Damage Radius set to %i\n", GrenadeRadius);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Grenade Damage Radius: %i\n", GrenadeRadius);   
}

// ***********************************************************
// End Grenade Damage Settings
// ***********************************************************

// ***********************************************************
// Start Hyperblaster/Blaster Damage Settings
// ***********************************************************
void	Svcmd_hyperblasterdamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		HyperBlasterDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Hyperblaster Damage set to %i\n", HyperBlasterDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Hyperblaster Damage: %i\n", HyperBlasterDamage);   
}
void	Svcmd_blasterprojspeed_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		BlasterProjectileSpeed=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Blaster Projectile Speed set to %i\n", BlasterProjectileSpeed);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Blaster Projectile Speed: %i\n", BlasterProjectileSpeed);   
}
void	Svcmd_blasterdamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		BlasterDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Blaster Damage set to %i\n", BlasterDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Blaster Damage: %i\n", BlasterDamage);   
}
// ***********************************************************
// End Hyperblaster/Blaster Damage Settings
// ***********************************************************

// ***********************************************************
// Start Machinegun/Chaingun Damage Settings
// ***********************************************************
void	Svcmd_machinegundamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		MachinegunDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Machinegun Damage set to %i\n", MachinegunDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Machinegun Damage: %i\n", MachinegunDamage);   
}
void	Svcmd_machinegunkick_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		MachinegunKick=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Machinegun Kick set to %i\n", MachinegunKick);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Machinegun Kick: %i\n", MachinegunKick);   
}
void	Svcmd_chaingundamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ChaingunDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Chaingun Damage set to %i\n", ChaingunDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Chaingun Damage: %i\n", ChaingunDamage);   
}
void	Svcmd_chaingunkick_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ChaingunKick=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Chaingun Kick set to %i\n", ChaingunKick);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Chaingun Kick: %i\n", ChaingunKick);   
}
// ***********************************************************
// End Machinegun/Chaingun Damage Settings
// ***********************************************************

// ***********************************************************
// Start Shotgun/Super Shotgun Damage Settings
// ***********************************************************
void	Svcmd_shotgundamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ShotgunDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Shotgun Damage set to %i\n", ShotgunDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Shotgun Damage: %i\n", ShotgunDamage);   
}
void	Svcmd_shotgunkick_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		ShotgunKick=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Shotgun Kick set to %i\n", ShotgunKick);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Shotgun Kick: %i\n", ShotgunKick);   
}
void	Svcmd_supershotgundamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SuperShotgunDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Super Shotgun Damage set to %i\n", SuperShotgunDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Super Shotgun Damage: %i\n", SuperShotgunDamage);   
}
void	Svcmd_supershotgunkick_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		SuperShotgunKick=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Super Shotgun Kick set to %i\n", SuperShotgunKick);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Super Shotgun Kick: %i\n", SuperShotgunKick);   
}
// ***********************************************************
// End Shotgun/Super Shotgun Damage Settings
// ***********************************************************

// ***********************************************************
// Start Railgun Damage Settings
// ***********************************************************
void	Svcmd_railgundamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		RailgunDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Railgun Damage set to %i\n", RailgunDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Railgun Damage: %i\n", RailgunDamage);   
}
void	Svcmd_railgunkick_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		RailgunKick=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Railgun Kick set to %i\n", RailgunKick);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Railgun Kick: %i\n", RailgunKick);   
}
// ***********************************************************
// End Railgun Damage Settings
// ***********************************************************

// ***********************************************************
// Start BFG Damage Settings
// ***********************************************************
void	Svcmd_bfgdamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		BFGDamage=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "BFG10K Damage set to %i\n", BFGDamage);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("BFG10K Damage: %i\n", BFGDamage);   
}
void	Svcmd_bfgdamageradius_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		BFGDamageRadius=atof(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "BFG10K Damage Radius set to %f\n", BFGDamageRadius);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("BFG10K Damage Radius: %f\n", BFGDamageRadius);   
}

// Version 2.3
void Svcmd_namebanning_f (char *cmdvalue)
{

if (strlen(cmdvalue) > 0) {
		namebanning=atoi(cmdvalue);
		if (namebanning == 1)
		  gi.bprintf (PRINT_MEDIUM, "Name banning activated on server.\n");
		else if (namebanning == 0)
		  gi.bprintf (PRINT_MEDIUM, "Name banning de-activated on server.\n");
		else {
			namebanning=0;		 
			gi.bprintf (PRINT_MEDIUM, "Name banning de-activated on server.\n");
		}
	}
	// If they are just performing an inquiry give them the current value
	else if (namebanning == 1)
		gi.dprintf ("Name banning currently actived\n");   
	else
		gi.dprintf ("Name banning currently not activated\n");   
}

void Svcmd_matchfullname_f (char *cmdvalue)
{

// Keep this private

if (strlen(cmdvalue) > 0) {
		matchfullnamevalue=atoi(cmdvalue);

		if (matchfullnamevalue > 1)
		  	matchfullnamevalue=1;		 
	}
	// If they are just performing an inquiry give them the current value
	else if (matchfullnamevalue == 1)
		gi.dprintf ("Now banning users which name 'matches' items in banlist.txt\n");   
	else
		gi.dprintf ("Now banning users which name 'contains an item' found banlist.txt file.\n");   

}


// Version 2.3
void Svcmd_fastrailgun_f (char *cmdvalue)
{
	
if (strlen(cmdvalue) > 0) {
		fastrailgun=atoi(cmdvalue);
		if (fastrailgun == 1)
		  gi.bprintf (PRINT_MEDIUM, "Railgun Speed Increased, Factor 1!\n");
		else if (fastrailgun == 0)
		  gi.bprintf (PRINT_MEDIUM, "Railgun Speed Normalized, Factor 0!\n");
		else
		  gi.bprintf (PRINT_MEDIUM, "Railgun Speed Maximized, Factor 2!\n");
	}
	// If they are just performing an inquiry give them the current value
	else if (fastrailgun == 1)
		gi.dprintf ("Railgun Speed is se to Increased (1)\n");   
	else if (fastrailgun == 0)
		gi.dprintf ("Railgun Speed is set to Normal (0)\n");   
	else
		gi.dprintf ("Railgun Speed is set to Maximum (2)\n");   

}

// Version 2.3

//CLOAK_DRAIN	
void Svcmd_cloakingdrain_f (char *cmdvalue)
{
// If value is 1 then translate to 20
// if value is 2 then translate to 16
// if value is 3 then trasnlate to 14
// if value is 4 then translate to 12
// if value is 5 then translate to 8
// if value is 6 then translate to 4
// if value is 7 then translate to 3
// if value is 8 then translate to 2
// if value is 9 then translate to 1
// if value is 10 then translate to 0 (MAX), too damn fast there, but oh well, up to server op!

	int tempcloakvar;

    	if (strlen(cmdvalue) > 0) {
		tempcloakvar=atoi(cmdvalue);
		
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
		
		gi.bprintf (PRINT_MEDIUM, "Cloak will now drain approx %i cells per second.\n", tempcloakvar);
	}
	// If they are just performing an inquiry give them the current value
		else {
		if (CLOAK_DRAIN == 0 )
			tempcloakvar = 0;
		else if (CLOAK_DRAIN == 20 )
			tempcloakvar = 1;
        else if (CLOAK_DRAIN == 14 )
	        tempcloakvar = 2;
		else if (CLOAK_DRAIN == 8 )
	        tempcloakvar = 3;
		else if (CLOAK_DRAIN == 7 )
	        tempcloakvar = 4;
		else if (CLOAK_DRAIN == 6 )
	        tempcloakvar = 5;
		else if (CLOAK_DRAIN == 5 )
	        tempcloakvar = 6;
		else if (CLOAK_DRAIN == 4 )
	        tempcloakvar = 7;
		else if (CLOAK_DRAIN == 3 )
	        tempcloakvar = 8;
		else if (CLOAK_DRAIN == 2 )
	        tempcloakvar = 9;
		else if (CLOAK_DRAIN == 1 )
	        tempcloakvar = 10;
			
		gi.dprintf ("Cloak set to drain approx %i cells per second\n", tempcloakvar);   

		}
		
}

void Svcmd_cloaking_f (char *cmdvalue)
{
	
    if (strlen(cmdvalue) > 0) {
		cloaking=atoi(cmdvalue);
		if (cloaking == 1)
		  gi.bprintf (PRINT_MEDIUM, "Cloaking Enabled, 'cloak' toggles client activation!\n");
		else {
		    cloaking = 0;
			gi.bprintf (PRINT_MEDIUM, "Cloaking Disabled, no more hiding!\n");
		}
	}
	// If they are just performing an inquiry give them the current value
	else if (cloaking == 1)
		gi.dprintf ("Cloaking Enabled.\n");   
	else
		gi.dprintf ("Cloaking Disabled.\n");
}


void Svcmd_fastrocketfire_f (char *cmdvalue)
{
	
if (strlen(cmdvalue) > 0) {
		fastrocketfire=atoi(cmdvalue);
		if (fastrocketfire == 1)
		  gi.bprintf (PRINT_MEDIUM, "Rocket Launcher Fire Rate Maximized!\n");
		else 
		  gi.bprintf (PRINT_MEDIUM, "Rocket Launcher Fire Rate Normalized!\n");
		}
	// If they are just performing an inquiry give them the current value
	else if (fastrocketfire == 1)
		gi.dprintf ("Rocket Launcher Fire Rate is Maximum (1)\n");   
	else
		gi.dprintf ("Rocket Launcher Fire Rate is Normal (0)\n");

}

void	Svcmd_bfgprojspeed_f (char *cmdvalue)
{
	// If they are assigning it. do so and announce.
	if (strlen(cmdvalue) > 0) {
		BFGProjectileSpeed=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "BFG10K Projectile Speed set to %i\n", BFGProjectileSpeed);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("BFG10K Projectile Speed: %i\n", BFGProjectileSpeed);   
}


void	Svcmd_enablehook_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		allowgrapple=atoi(cmdvalue);
	
		if (allowgrapple == 1)
		  gi.bprintf (PRINT_MEDIUM, "Hook enabled, players use 'bind <key> +hook' to use.\n");
		else 
		  gi.bprintf (PRINT_MEDIUM, "Hook disabled.\n");
	}
	else {
		if (allowgrapple == 1)
			gi.dprintf ("Hook currently enabled on server (1).\n");   
		else
			gi.dprintf ("Hook currently is not enabled (0).\n");   
	}
}

void	Svcmd_hooktime_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		HOOK_TIME=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Hook Time set to %i\n", HOOK_TIME);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Hook time currently set to: %i\n", HOOK_TIME);   
}


void	Svcmd_hookspeed_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		HOOK_SPEED=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Hook Speed set to %i\n", HOOK_SPEED);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Hook speed currently set to: %i\n", HOOK_SPEED);   

}

void	Svcmd_hookdamage_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		HOOK_DAMAGE=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Hook Damage set to %i\n", HOOK_DAMAGE);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Hook damage currently set to: %i\n", HOOK_DAMAGE);   

}
void Svcmd_hookpathcolor_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		hookcolor=atoi(cmdvalue);
		if (hookcolor > 6)
			hookcolor=4;

		switch (hookcolor) {
		case 0: gi.bprintf (PRINT_MEDIUM, "Hook trail set to 'Red Blood Trail'\n");
			    break;
	    case 1: gi.bprintf (PRINT_MEDIUM, "Hook trail set to 'Red Particles'\n");
	            break;
		case 2: gi.bprintf (PRINT_MEDIUM, "Hook trail set to 'Wide Red Particles'\n");
			    break;
		case 3: gi.bprintf (PRINT_MEDIUM, "Hook trail set to 'Wide Blue Particles'\n");
				break;
		case 4: gi.bprintf (PRINT_MEDIUM, "Hook trail set to 'Gold Particles'\n");
				break;
		case 5: gi.bprintf (PRINT_MEDIUM, "Hook trail set to 'Yellow Teleporter'\n");
				break;
		case 6: gi.bprintf (PRINT_MEDIUM, "Hook trail set to 'Black Flies'\n");
				break;
		default: gi.bprintf (PRINT_MEDIUM, "Hook trail set to 'Gold Particles'\n");
				 break;

		} // end switch
	} else {

		switch (hookcolor) {
		case 0: gi.dprintf ("Hook trail set to 'Red Blood Trail (0)'\n");
			    break;
	    case 1: gi.dprintf ("Hook trail set to 'Red Particles (1)'\n");
	            break;
		case 2: gi.dprintf ("Hook trail set to 'Wide Red Particles (2)'\n");
			    break;
		case 3: gi.dprintf ("Hook trail set to 'Wide Blue Particles (3)'\n");
				break;
		case 4: gi.dprintf ("Hook trail set to 'Gold Particles' (4)\n");
				break;
		case 5: gi.dprintf ("Hook trail set to 'Yellow Teleporter' (5)\n");
				break;
		case 6: gi.dprintf ("Hook trail set to 'Black Flies' (6)\n");
				break;
		default: gi.dprintf ("Hook trail set to 'Gold Particles' (4)\n");
				 break;
		} //end inquiry switch

	}

}

void	Svcmd_pullspeed_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		PULL_SPEED=atoi(cmdvalue);
		gi.bprintf (PRINT_MEDIUM, "Hook Pull Speed set to %i\n", PULL_SPEED);
	}
	// If they are just performing an inquiry give them the current value
	else
		gi.dprintf ("Hook Pull Speed currently set to: %i\n", PULL_SPEED);   

}

void Svcmd_serverrate_f (char *cmdvalue){
		
	gi.dprintf ("Server max rate is set at: %i\n",MAX_CLIENT_RATE);   
	
}

void	Svcmd_skysolid_f (char *cmdvalue)
{
	
	if (strlen(cmdvalue) > 0) {
		EXPERT_SKY_SOLID=atoi(cmdvalue);
	
		if (EXPERT_SKY_SOLID > 1)
			EXPERT_SKY_SOLID=1;

		if (EXPERT_SKY_SOLID == 1)
		  gi.bprintf (PRINT_MEDIUM, "Sky is considered a solid, hook will attach.\n");
		else 
		  gi.bprintf (PRINT_MEDIUM, "Sky is not a solid, hook won't attach.\n");
	}
	else {
		if (EXPERT_SKY_SOLID == 1)
			gi.dprintf ("Sky is solid for hook (1).\n");   
		else
			gi.dprintf ("Sky is not solid for hook (0).\n");   
	}
}

void	Svcmd_fastchangeweapons_f (char *cmdvalue)
{
	if (strlen(cmdvalue) > 0) {
		fastchange=atoi(cmdvalue);
		if (fastchange > 1)
			fastchange=1;
	if (fastchange == 1)
		  gi.bprintf (PRINT_MEDIUM, "Fast weapon changing activated!\n");
		else 
		  gi.bprintf (PRINT_MEDIUM, "Normal weapon changing speed now active!\n");
	} else {
		if (fastchange == 1)
			gi.dprintf ("Fast weapon changing active. (1).\n");   
		else
			gi.dprintf ("Normal weapon changing active (0).\n");   
	}
}


void	Svcmd_cloakgrapple_f (char *cmdvalue)
{
	
	if (strlen(cmdvalue) > 0) {
		cloakgrapple=atoi(cmdvalue);
	
		if (cloakgrapple > 1)
			cloakgrapple=1;

		if (cloakgrapple == 1)
		  gi.bprintf (PRINT_MEDIUM, "Players can be cloaked while using hook.\n");
		else 
		  gi.bprintf (PRINT_MEDIUM, "Players cannot be cloaked while using hook.\n");
	}
	else {
		if (cloakgrapple == 1)
			gi.dprintf ("Players can be cloaked while using hook (1).\n");   
		else
			gi.dprintf ("Players cannot be cloaked while using hook (0).\n");   
	}
}


// ***********************************************************
// End BFG Damage Settings
// ***********************************************************

void	Svcmd_Help_f (void)
{
	gi.dprintf ("Using the sv <command> can alter game settings on the fly:\n");
	gi.dprintf ("----------------------------------------------------------\n");
	gi.dprintf ("Available Commands: 'sv <command> <value>'\n\n");
	gi.dprintf ("ASSORTED:\n");
	gi.dprintf ("  timehud            <0> or <1>\n");
	gi.dprintf ("  rankhud            <0> or <1>\n");
	gi.dprintf ("  playershud         <0> or <1>\n");
	gi.dprintf ("  starthealth        <value>\n");
	gi.dprintf ("  maxhealth          <value>\n");
	gi.dprintf ("  scoreboard         <0> or <1> or <2>\n");
	gi.dprintf ("  namebanning        <0> or <1>\n");
	gi.dprintf ("  matchfullname      <0> or <1>\n");
    gi.dprintf ("  cloaking           <0> or <1>\n");
	gi.dprintf ("  cloakingdrain      <1> -> <10>\n");
	gi.dprintf ("  globalfraglimit    <value>\n");
	gi.dprintf ("  globaltimelimit    <value>\n");
	gi.dprintf ("  globalgravitylimit <value>\n\n");
	gi.dprintf ("START WEAPONS:\n");
	gi.dprintf ("  startshotgun       <0> or <1>\n");
	gi.dprintf ("  startsupershotgun  <0> or <1>\n");
	gi.dprintf ("  startmachinegun    <0> or <1>\n");
	gi.dprintf ("  startchaingun      <0> or <1>\n");
	gi.dprintf ("  startgrenadelaunch <0> or <1>\n");
	gi.dprintf ("  startrocketlaunch  <0> or <1>\n");
	gi.dprintf ("  starthyperblaster  <0> or <1>\n");
	gi.dprintf ("  startrailgun       <0> or <1>\n");
	gi.dprintf ("  startbfg           <0> or <1>\n\n");
	gi.dprintf ("sv help2 for next page of help...\n");
}

void	Svcmd_Help2_f (void)
{
	gi.dprintf ("START AMMO:\n");
	gi.dprintf ("  startbullets         <value>\n");
	gi.dprintf ("  startshells          <value>\n");
	gi.dprintf ("  startgrenades        <value>\n");
	gi.dprintf ("  startrockets         <value>\n");
	gi.dprintf ("  startslugs           <value>\n");
	gi.dprintf ("  startcells           <value>\n\n");
	gi.dprintf ("MAX AMMO:\n");
	gi.dprintf ("  maxbullets         <value>\n");
	gi.dprintf ("  maxshells          <value>\n");
	gi.dprintf ("  maxgrenades        <value>\n");
	gi.dprintf ("  maxrockets         <value>\n");
	gi.dprintf ("  maxslugs           <value>\n");
	gi.dprintf ("  maxcells           <value>\n\n");
	gi.dprintf ("sv help3 for next page of help...\n");
}

void	Svcmd_Help3_f (void)
{
	gi.dprintf ("START ITEMS:\n");
	gi.dprintf ("  start4x            <1>\n");
	gi.dprintf ("  start4xtime        <30>\n");
	gi.dprintf ("  startautouseq4x    <0>\n");
	gi.dprintf ("  start666           <1>\n");
	gi.dprintf ("  start666time       <30>\n");
	gi.dprintf ("  startautouse666    <0>\n");
	gi.dprintf ("  startsilencer      <0>\n");
	gi.dprintf ("  startsilencershots <30>\n");
	gi.dprintf ("  startrebreath      <0>\n");
	gi.dprintf ("  startrebreathtime  <30>\n");
	gi.dprintf ("  startenvsuit       <0>\n");
	gi.dprintf ("  startenvsuittime   <30>\n");
	gi.dprintf ("  startregen666      <1>\n");
	gi.dprintf ("  startregen666time  <5>\n\n");
	gi.dprintf ("sv help4 for next page of help...\n");
}

void	Svcmd_Help4_f (void)
{
	gi.dprintf ("BAN WEAPONS:\n");
	gi.dprintf ("  banshotgun         <0> or <1>\n");
	gi.dprintf ("  bansupershotgun    <0> or <1>\n");
	gi.dprintf ("  banmachinegun      <0> or <1>\n");
	gi.dprintf ("  banchaingun        <0> or <1>\n");
	gi.dprintf ("  bangrenadelauncher <0> or <1>\n");
	gi.dprintf ("  banrocketlauncher  <0> or <1>\n");
	gi.dprintf ("  banhyperblaster    <0> or <1>\n");
	gi.dprintf ("  banrailgun         <0> or <1>\n");
	gi.dprintf ("  banbfg             <0> or <1>\n\n");
	gi.dprintf ("WEAPON CONFIGURATION:\n");
    gi.dprintf ("  fastrocketfire     <0> or <1>\n");  // Version 2.3
	gi.dprintf ("  rocketdamage       <100>\n");
	gi.dprintf ("  rocketvelocity     <650>\n");
	gi.dprintf ("  rocketradiusdamage <120>\n");
	gi.dprintf ("  rocketdamageradius <120>\n");
	gi.dprintf ("  glaunchertimer     <2.5>\n");
	gi.dprintf ("  glauncherfiredis   <600>\n");
	gi.dprintf ("  glauncherdamage    <120>\n");
	gi.dprintf ("  glauncherradius    <120>\n");
	gi.dprintf ("  grenadetimer       <3.0>\n\n");
	gi.dprintf ("sv help5 for next page of help...\n");
}

void	Svcmd_Help5_f (void)
{
	gi.dprintf ("WEAPON CONFIGURATION (continued):\n");
	gi.dprintf ("  grenademinspeed    <400>\n");
	gi.dprintf ("  grenademaxspeed    <800>\n");
	gi.dprintf ("  grenadedamage      <125>\n");
	gi.dprintf ("  grenaderadius      <40>\n");
	gi.dprintf ("  hyperblasterdamage <15>\n");
	gi.dprintf ("  blasterprojspeed   <1000>\n");
	gi.dprintf ("  blasterdamage      <15>\n");
	gi.dprintf ("  machinegundamage   <8>\n");
	gi.dprintf ("  machinegunkick     <2>\n");
	gi.dprintf ("  chaingundamage     <6>\n");
	gi.dprintf ("  chaingunkick       <2>\n");
	gi.dprintf ("  shotgundamage      <4>\n");
	gi.dprintf ("  shotgunkick        <8>\n");
	gi.dprintf ("  supershotgundamage <6>\n");
	gi.dprintf ("  supershotgunkick   <12>\n");
    gi.dprintf ("  fastrailgun        <0>,<1> or <2>\n"); // Version 2.3
	gi.dprintf ("  railgundamage      <100>\n");
	gi.dprintf ("  railgunkick        <200>\n");
	gi.dprintf ("  bfgdamage          <200>\n");
	gi.dprintf ("  bfgdamageradius    <1000>\n");
	gi.dprintf ("  bfgprojspeed       <400>\n\n");
	gi.dprintf ("sv help5 for next page...\n");
}

void	Svcmd_Help6_f (void)
{
	gi.dprintf ("GRAPPLE CONFIGURATION:\n");
	gi.dprintf ("  enablehook         <0> or <1>\n");
	gi.dprintf ("  hooktime           <4>\n");
	gi.dprintf ("  hookspeed          <1900>\n");
	gi.dprintf ("  hookdamage         <10>\n");
	gi.dprintf ("  pullspeed          <420>\n");
	gi.dprintf ("  skysolid           <0> or <1>\n");
	gi.dprintf ("  cloakgrapple       <0> or <1>\n");
	gi.dprintf ("  hookpathcolor      <0> -> <6>\n");
	gi.dprintf ("-------------------------------\n");
	gi.dprintf ("OTHER OPTIONS:\n");
	gi.dprintf ("  chasekeepscore     <0> or <1>\n");
	gi.dprintf ("  fastchangeweapons  <0> or <1>\n");
	
	gi.dprintf ("End of help...\n");
}


// 3.20 SOURCE
// IP BANNING

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/


/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipfilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			gi.cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
			return false;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return true;
}

/*
=================
SV_FilterPacket
=================
*/
qboolean SV_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numipfilters ; i++)
		if ( (in & ipfilters[i].mask) == ipfilters[i].compare)
			return (int)filterban->value;

	return (int)!filterban->value;
}


/*
=================
SV_AddIP_f
=================
*/
void SVCmd_AddIP_f (void)
{
	int		i;
	
	if (gi.argc() < 3) {
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
		return;
	}

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			gi.cprintf (NULL, PRINT_HIGH, "IP filter list is full\n");
			return;
		}
		numipfilters++;
	}
	
	if (!StringToFilter (gi.argv(2), &ipfilters[i]))
		ipfilters[i].compare = 0xffffffff;
}

/*
=================
SV_RemoveIP_f
=================
*/
void SVCmd_RemoveIP_f (void)
{
	ipfilter_t	f;
	int			i, j;

	if (gi.argc() < 3) {
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
		return;
	}

	if (!StringToFilter (gi.argv(2), &f))
		return;

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].mask == f.mask
		&& ipfilters[i].compare == f.compare)
		{
			for (j=i+1 ; j<numipfilters ; j++)
				ipfilters[j-1] = ipfilters[j];
			numipfilters--;
			gi.cprintf (NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	gi.cprintf (NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
}

/*
=================
SV_ListIP_f
=================
*/
void SVCmd_ListIP_f (void)
{
	int		i;
	byte	b[4];

	gi.cprintf (NULL, PRINT_HIGH, "Filter list:\n");
	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		gi.cprintf (NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

/*
=================
SV_WriteIP_f
=================
*/
void SVCmd_WriteIP_f (void)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	byte	b[4];
	int		i;
	cvar_t	*game;

	game = gi.cvar("game", "", 0);

	if (!*game->string)
		sprintf (name, "%s/listip.cfg", GAMEVERSION);
	else
		sprintf (name, "%s/listip.cfg", game->string);

	gi.cprintf (NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f)
	{
		gi.cprintf (NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}
	
	fprintf(f, "set filterban %d\n", (int)filterban->value);

	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf (f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}
	
	fclose (f);
}
// END 3.20 SOURCE, IP BANNING


/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd;
	char *cmdvalue;

	cmd = gi.argv(1);
	cmdvalue = gi.argv(2);

	if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else if (Q_stricmp (cmd, "help") == 0)
		Svcmd_Help_f ();
	else if (Q_stricmp (cmd, "help2") == 0)
		Svcmd_Help2_f ();
	else if (Q_stricmp (cmd, "help3") == 0)
		Svcmd_Help3_f ();
	else if (Q_stricmp (cmd, "help4") == 0)
		Svcmd_Help4_f ();
	else if (Q_stricmp (cmd, "help5") == 0)
		Svcmd_Help5_f ();
	else if (Q_stricmp (cmd, "help6") == 0)
		Svcmd_Help6_f ();
	// Template
	//else if (Q_stricmp (cmd, "") == 0)
	//	Svcmd__f (cmdvalue);
	else if (Q_stricmp (cmd, "timehud") == 0)
		Svcmd_timehud_f (cmdvalue);
	else if (Q_stricmp (cmd, "rankhud") == 0)
		Svcmd_rankhud_f (cmdvalue);
	else if (Q_stricmp (cmd, "playershud") == 0)
		Svcmd_playershud_f (cmdvalue);

	else if (Q_stricmp (cmd, "starthealth") == 0)
		Svcmd_starthealth_f (cmdvalue);
	else if (Q_stricmp (cmd, "maxhealth") == 0)
		Svcmd_maxhealth_f (cmdvalue);

	else if (Q_stricmp (cmd, "cloaking") == 0)  // Version 2.3
		Svcmd_cloaking_f (cmdvalue);

	else if (Q_stricmp (cmd, "cloakingdrain") == 0)  // Version 2.3
		Svcmd_cloakingdrain_f (cmdvalue);

	else if (Q_stricmp (cmd, "namebanning") == 0)  // Version 2.3
		Svcmd_namebanning_f (cmdvalue);
	else if (Q_stricmp (cmd, "matchfullname") == 0)  // Version 2.3
		Svcmd_matchfullname_f (cmdvalue);

    else if (Q_stricmp (cmd, "fastrocketfire") == 0)  // Version 2.3
		Svcmd_fastrocketfire_f (cmdvalue);
	else if (Q_stricmp (cmd, "fastrailgun") == 0)  // Version 2.3
		Svcmd_fastrailgun_f (cmdvalue);
	else if (Q_stricmp (cmd, "scoreboard") == 0)
		Svcmd_scoreboard_f (cmdvalue);
	else if (Q_stricmp (cmd, "globalfraglimit") == 0)
		Svcmd_globalfraglimit_f (cmdvalue);
	else if (Q_stricmp (cmd, "globaltimelimit") == 0)
		Svcmd_globaltimelimit_f (cmdvalue);
	else if (Q_stricmp (cmd, "globalgravitylimit") == 0)
		Svcmd_globalgravitylimit_f (cmdvalue);
	else if (Q_stricmp (cmd, "startshotgun") == 0)
		Svcmd_startshotgun_f (cmdvalue);
	else if (Q_stricmp (cmd, "startsupershotgun") == 0)
		Svcmd_startsupershotgun_f (cmdvalue);
	else if (Q_stricmp (cmd, "startmachinegun") == 0)
		Svcmd_startmachinegun_f (cmdvalue);
	else if (Q_stricmp (cmd, "startchaingun") == 0)
		Svcmd_startchaingun_f (cmdvalue);
	else if (Q_stricmp (cmd, "startgrenadelaunch") == 0)
		Svcmd_startgrenadelaunch_f (cmdvalue);
	else if (Q_stricmp (cmd, "startrocketlaunch") == 0)
		Svcmd_startrocketlaunch_f (cmdvalue);
	else if (Q_stricmp (cmd, "starthyperblaster") == 0)
		Svcmd_starthyperblaster_f (cmdvalue);
	else if (Q_stricmp (cmd, "startrailgun") == 0)
		Svcmd_startrailgun_f (cmdvalue);
	else if (Q_stricmp (cmd, "startbfg") == 0)
		Svcmd_startbfg_f (cmdvalue);

	else if (Q_stricmp (cmd, "startbullets") == 0)
		Svcmd_startbullets_f (cmdvalue);
	else if (Q_stricmp (cmd, "startshells") == 0)
		Svcmd_startshells_f (cmdvalue);
	else if (Q_stricmp (cmd, "startgrenades") == 0)
		Svcmd_startgrenades_f (cmdvalue);
	else if (Q_stricmp (cmd, "startrockets") == 0)
		Svcmd_startrockets_f (cmdvalue);
	else if (Q_stricmp (cmd, "startslugs") == 0)
		Svcmd_startslugs_f (cmdvalue);
	else if (Q_stricmp (cmd, "startcells") == 0)
		Svcmd_startcells_f (cmdvalue);
    else if (Q_stricmp (cmd, "maxbullets") == 0)
		Svcmd_maxbullets_f (cmdvalue);
	else if (Q_stricmp (cmd, "maxshells") == 0)
		Svcmd_maxshells_f (cmdvalue);
	else if (Q_stricmp (cmd, "maxgrenades") == 0)
		Svcmd_maxgrenades_f (cmdvalue);
	else if (Q_stricmp (cmd, "maxrockets") == 0)
		Svcmd_maxrockets_f (cmdvalue);
	else if (Q_stricmp (cmd, "maxslugs") == 0)
		Svcmd_maxslugs_f (cmdvalue);
	else if (Q_stricmp (cmd, "maxcells") == 0)
		Svcmd_maxcells_f (cmdvalue);


	else if (Q_stricmp (cmd, "start4x") == 0)
		Svcmd_start4x_f (cmdvalue);
	else if (Q_stricmp (cmd, "start4xtime") == 0)
		Svcmd_start4xtime_f (cmdvalue);
	else if (Q_stricmp (cmd, "startautouse4x") == 0)
		Svcmd_startautouse4x_f (cmdvalue);
	else if (Q_stricmp (cmd, "start666") == 0)
		Svcmd_start666_f (cmdvalue);
	else if (Q_stricmp (cmd, "start666time") == 0)
		Svcmd_start666time_f (cmdvalue);
	else if (Q_stricmp (cmd, "startautouse666") == 0)
		Svcmd_startautouse666_f (cmdvalue);
	else if (Q_stricmp (cmd, "startsilencer") == 0)
		Svcmd_startsilencer_f (cmdvalue);
	else if (Q_stricmp (cmd, "startsilencershots") == 0)
		Svcmd_startsilencershots_f (cmdvalue);
	else if (Q_stricmp (cmd, "startrebreath") == 0)
		Svcmd_startrebreath_f (cmdvalue);
	else if (Q_stricmp (cmd, "startrebreathtime") == 0)
		Svcmd_startrebreathtime_f (cmdvalue);
	else if (Q_stricmp (cmd, "startenvsuit") == 0)
		Svcmd_startenvsuit_f (cmdvalue);
	else if (Q_stricmp (cmd, "startenvsuittime") == 0)
		Svcmd_startenvsuittime_f (cmdvalue);
	else if (Q_stricmp (cmd, "startregen666") == 0)
		Svcmd_startregen666_f (cmdvalue);
	else if (Q_stricmp (cmd, "startregen666time") == 0)
		Svcmd_startregen666time_f (cmdvalue);
	else if (Q_stricmp (cmd, "banshotgun") == 0)
		Svcmd_banshotgun_f (cmdvalue);
	else if (Q_stricmp (cmd, "bansupershotgun") == 0)
		Svcmd_bansupershotgun_f (cmdvalue);
	else if (Q_stricmp (cmd, "banmachinegun") == 0)
		Svcmd_banmachinegun_f (cmdvalue);
	else if (Q_stricmp (cmd, "banchaingun") == 0)
		Svcmd_banchaingun_f (cmdvalue);
	else if (Q_stricmp (cmd, "bangrenadelauncher") == 0)
		Svcmd_bangrenadelauncher_f (cmdvalue);
	else if (Q_stricmp (cmd, "banrocketlauncher") == 0)
		Svcmd_banrocketlauncher_f (cmdvalue);
	else if (Q_stricmp (cmd, "banhyperblaster") == 0)
		Svcmd_banhyperblaster_f (cmdvalue);
	else if (Q_stricmp (cmd, "banrailgun") == 0)
		Svcmd_banrailgun_f (cmdvalue);
	else if (Q_stricmp (cmd, "banbfg") == 0)
		Svcmd_banbfg_f (cmdvalue);
	else if (Q_stricmp (cmd, "rocketdamage") == 0)
		Svcmd_rocketdamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "rocketvelocity") == 0)
		Svcmd_rocketvelocity_f (cmdvalue);
	else if (Q_stricmp (cmd, "rocketradiusdamage") == 0)
		Svcmd_rocketradiusdamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "rocketdamageradius") == 0)
		Svcmd_rocketdamageradius_f (cmdvalue);
	else if (Q_stricmp (cmd, "glaunchertimer") == 0)
		Svcmd_glaunchertimer_f (cmdvalue);
	else if (Q_stricmp (cmd, "glauncherfiredis") == 0)
		Svcmd_glauncherfiredis_f (cmdvalue);
	else if (Q_stricmp (cmd, "glauncherdamage") == 0)
		Svcmd_glauncherdamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "glauncherradius") == 0)
		Svcmd_glauncherradius_f (cmdvalue);
	else if (Q_stricmp (cmd, "grenadetimer") == 0)
		Svcmd_grenadetimer_f (cmdvalue);
	else if (Q_stricmp (cmd, "grenademinspeed") == 0)
		Svcmd_grenademinspeed_f (cmdvalue);
	else if (Q_stricmp (cmd, "grenademaxspeed") == 0)
		Svcmd_grenademaxspeed_f (cmdvalue);
	else if (Q_stricmp (cmd, "grenadedamage") == 0)
		Svcmd_grenadedamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "grenaderadius") == 0)
		Svcmd_grenaderadius_f (cmdvalue);
	else if (Q_stricmp (cmd, "hyperblasterdamage") == 0)
		Svcmd_hyperblasterdamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "blasterprojspeed") == 0)
		Svcmd_blasterprojspeed_f (cmdvalue);
	else if (Q_stricmp (cmd, "blasterdamage") == 0)
		Svcmd_blasterdamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "machinegundamage") == 0)
		Svcmd_machinegundamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "machinegunkick") == 0)
		Svcmd_machinegunkick_f (cmdvalue);
	else if (Q_stricmp (cmd, "chaingundamage") == 0)
		Svcmd_chaingundamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "chaingunkick") == 0)
		Svcmd_chaingunkick_f (cmdvalue);
	else if (Q_stricmp (cmd, "shotgundamage") == 0)
		Svcmd_shotgundamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "shotgunkick") == 0)
		Svcmd_shotgunkick_f (cmdvalue);
	else if (Q_stricmp (cmd, "supershotgundamage") == 0)
		Svcmd_supershotgundamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "supershotgunkick") == 0)
		Svcmd_supershotgunkick_f (cmdvalue);
	else if (Q_stricmp (cmd, "railgundamage") == 0)
		Svcmd_railgundamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "railgunkick") == 0)
		Svcmd_railgunkick_f (cmdvalue);
	else if (Q_stricmp (cmd, "bfgdamage") == 0)
		Svcmd_bfgdamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "bfgdamageradius") == 0)
		Svcmd_bfgdamageradius_f (cmdvalue);
	else if (Q_stricmp (cmd, "bfgprojspeed") == 0)
		Svcmd_bfgprojspeed_f (cmdvalue);

	else if (Q_stricmp (cmd, "enablehook") == 0)
		Svcmd_enablehook_f (cmdvalue);
	else if (Q_stricmp (cmd, "hooktime") == 0)
		Svcmd_hooktime_f (cmdvalue);
	else if (Q_stricmp (cmd, "hookspeed") == 0)
		Svcmd_hookspeed_f (cmdvalue);
	else if (Q_stricmp (cmd, "hookdamage") == 0)
		Svcmd_hookdamage_f (cmdvalue);
	else if (Q_stricmp (cmd, "pullspeed") == 0)
		Svcmd_pullspeed_f (cmdvalue);
	else if (Q_stricmp (cmd, "skysolid") == 0)
		Svcmd_skysolid_f (cmdvalue);
	else if (Q_stricmp (cmd, "cloakgrapple") == 0)
		Svcmd_cloakgrapple_f (cmdvalue);
	else if (Q_stricmp (cmd, "serverrate") == 0)  // undocumented
		Svcmd_serverrate_f (cmdvalue);
	else if (Q_stricmp (cmd, "hookpathcolor") == 0)
		Svcmd_hookpathcolor_f (cmdvalue);
	else if (Q_stricmp (cmd, "chasekeepscore") == 0)
		Svcmd_chasekeepscore_f (cmdvalue);
	else if (Q_stricmp (cmd, "fastchangeweapons") == 0)
		Svcmd_fastchangeweapons_f (cmdvalue);
	// IP BANNING CODE, 3.20 SOURCE
	else if (Q_stricmp (cmd, "addip") == 0)
		SVCmd_AddIP_f ();
	else if (Q_stricmp (cmd, "removeip") == 0)
		SVCmd_RemoveIP_f ();
	else if (Q_stricmp (cmd, "listip") == 0)
		SVCmd_ListIP_f ();
	else if (Q_stricmp (cmd, "writeip") == 0)
		SVCmd_WriteIP_f ();


	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\", use sv help.\n", cmd);
}

