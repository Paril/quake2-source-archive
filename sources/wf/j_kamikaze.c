#include        "g_local.h"
/*Function: Start_Kamikaze_Mode
  Places the edict passed to it into Kamikaze Mode
  (probably best to pass a player to it)
  Warns everyone that so and so is a kamikaze...*/
void stuffcmd(edict_t *e, char *s);

void Start_Kamikaze_Mode(edict_t *the_doomed_one)
{	
	//jR The great bug fix
	if (the_doomed_one->health<0)
	{
		//gi.bprintf (PRINT_HIGH, "%s tried to cheat or crash the server!\n But good old Cryect stopped him\n Cheaters never!!!\nByeBye!", the_doomed_one->client->pers.netname);
		//stuffcmd(the_doomed_one, "alias kickme say I cheat;say I try to cheat and crash servers thank you all and tell everyone how all about me!;disconnect;echo Wow what a great server crasher!");
		//stuffcmd(the_doomed_one, "kickme");
		return;
	}


	/* see if we are already in  kamikaze mode*/
	if (the_doomed_one->client->kamikaze_mode & 1)  
	{
		safe_cprintf(the_doomed_one, PRINT_MEDIUM, "Already in Kamikaze Mode! Cancel to stop!");
		return;		    
	}	
	/* dont run if in god mode  */
	if (the_doomed_one->flags & FL_GODMODE)
	{
		safe_cprintf(the_doomed_one, PRINT_MEDIUM, "Can't Kamikaze in God Mode!");
		return;	
	}	
	/* not in kamikaze mode yet */
	the_doomed_one->client->kamikaze_mode = 1;
	/*  Give us only so long */
	the_doomed_one->client->kamikaze_timeleft = KAMIKAZE_BLOW_TIME;
	the_doomed_one->client->kamikaze_framenum = level.framenum + the_doomed_one->client->kamikaze_timeleft;
	/* Warn the World */
    //gi.sound( the_doomed_one, CHAN_WEAPON, gi.soundindex("makron/rail_up.wav"), 1, ATTN_NONE, 0 );
//GREGG - use 10 second countdown instead
	//JR - I like it Gregg
    gi.sound( the_doomed_one, CHAN_WEAPON, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0 );

	return;
	}
	
/* Function: Kamikaze_Active
 Are we in Kamikaze Mode? 
  a helper function to see if we are running in Kamikaze Mode*/
qboolean Kamikaze_Active(edict_t *the_doomed_one)  
{
	return (the_doomed_one->client->kamikaze_mode);
}

/* Function: Kamikaze_Cancel
  Canceled for Some Reason  Call if Player is killed before time is up*/
void Kamikaze_Cancel(edict_t *the_spared_one)
{
	/* not in kamikaze mode yet */
	the_spared_one->client->kamikaze_mode = 0;
	/* Give us only so long */
	the_spared_one->client->kamikaze_timeleft = 0;
	the_spared_one->client->kamikaze_framenum = 0;	
	return;
}

void Kamikaze_Explode(edict_t *the_doomed_one)
{    
     /* BANG ! and show the clients */    
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition(the_doomed_one -> s.origin);
	gi.multicast (the_doomed_one->s.origin, MULTICAST_PVS);

	/* A whole Lotta Damage */
    T_RadiusDamage (the_doomed_one, the_doomed_one, KAMIKAZE_DAMAGE, NULL, KAMIKAZE_DAMAGE_RADUIS, MOD_KAMIKAZE);
}