/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_arty.c,v $
 *   $Revision: 1.11 $
 *   $Date: 2002/06/04 19:49:45 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"
// g_arty.c
// D-Day: Normandy Artillery and Airstrikes

//////////////////////////////////////////////////////////////////////
//   This file is for handeling artillery and airstrike functions   //
//////////////////////////////////////////////////////////////////////

/* user definable variables 

	arty_delay  -- seconds for artillary to position           default: 10 
	arty_time   -- seconds between each volley                 default: 60
	arty_max    -- number of shots to be fired in each volley  default: 1
*/

/*
=================
Cmd_Arty_f
CCH: new function to call in airstrikes
=================
*/
void Cmd_Arty_f (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	end;
	vec3_t world_up;

	trace_t	tr;
	//int randnum;		

	if (!IsValidPlayer)
		return;

	if (ent->client->resp.mos != OFFICER) {
		gi.cprintf(ent, PRINT_HIGH, "You're not an officer, soldier!\n");
		return;
	}

	if (ent->deadflag || ent->client->limbo_mode ) {
		gi.cprintf(ent, PRINT_HIGH, "You are dead!\n");
		return;
	}




	//faf:  moving this up so you dont have to look through binocs to cancel arty
	// make sure artillary hasn't already been called
	if ( ent->client->arty_called )
	{
		if (ent->client->arty_fired)
		{
			gi.cprintf(ent, PRINT_HIGH, "Artillery has already been fired, sir!\n");
			return;
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "Holding fire sir!\n");
			ent->client->arty_called = 0;
			ent->client->arty_num--;
			return;
		}
	}






	/* Wheaty: Officers can only call airstrikes with binoculars! */
	if (strcmp(ent->client->pers.weapon->classname, "weapon_binoculars"))
	{
		gi.cprintf(ent, PRINT_HIGH, "What the hell are you aiming at? Use your binoculars!\n");
		return;
	}
	else // pbowens: they have to be aiming, too
	{
		if (!(ent->client->buttons & BUTTON_ATTACK)) {
			gi.cprintf(ent, PRINT_HIGH, "Aim at the location, sir.\n");
			return;
		}
	}



	if ( ent->client->arty_time_restrict > level.time && ent->client->arty_num >= (int)arty_max->value)
	{
		float delay;

		delay = ent->client->arty_time_restrict - level.time;

		if (delay > 60)
			gi.cprintf(ent, PRINT_HIGH, "Can not fire for another %i minutes, sir!\n", (int)delay/60);
		else
			gi.cprintf(ent, PRINT_HIGH, "Can not fire for another %i seconds, sir!\n", (int)delay);

		return;
	}

	// reset the fired counter if past restrict time
	if ( ent->client->arty_time_restrict <= level.time && ent->client->arty_num >= (int)arty_max->value )
		ent->client->arty_num = 0;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	if ( tr.surface && !(tr.surface->flags & SURF_SKY) )
	{ // We hit something but it wasn't sky, so let's see if there is sky above it

		VectorCopy(tr.endpos,ent->client->arty_target); //assign target to Arty
		VectorSet(world_up, 0, 0, 1);
		VectorMA(start, 8192, world_up, end);

		tr = gi.trace(ent->client->arty_target, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

		if ( tr.surface && !(tr.surface->flags & SURF_SKY))  // No sky above it either
		{
		gi.cprintf(ent, PRINT_HIGH, "Sir! Artillery must be fired in open areas at stationary grounds.\n");
			return;
		}
	}
	else  //faf:  this stops horizontal airstrikes
	{
		gi.cprintf(ent, PRINT_HIGH, "Sir! Artillery must be fired in open areas at stationary grounds.\n");
			return;
	}


	// set up for the arty strike
	VectorCopy(tr.endpos, ent->client->arty_entry);
	ent->client->arty_called = true;
	

	//srand( (unsigned)time( NULL ) );
	
	ent->client->arty_fired = false;
			
	//randnum = ((rand() % ARTILLARY_WAIT) + 5);  //generate random number for eta
		if (ent)
			gi.cprintf(ent, PRINT_HIGH, "Ok, give us %d seconds to position the guns!\n", (int)arty_delay->value);

	ent->client->arty_num++;
	ent->client->arty_time_fire = level.time + arty_delay->value;

	ent->client->arty_location = 1;//(rand() % 4) + 1;
	gi.sound(ent, CHAN_ITEM, gi.soundindex(va("%s/arty/target%i.wav", 
		ent->client->resp.team_on->teamid, ent->client->arty_location)), 1, ATTN_NORM, 0);

}

void fire_airstrike (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);//faf

 
/*
=================
Think_Arty
CCH: This will bring the airstrike ordinance into existence in the game
Called by ClientThink
=================
*/
void Think_Arty (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	end;
	vec3_t	targetdir;
	vec3_t  tempvec;
	trace_t	tr;
	trace_t tr_2;

	// find the target point
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	// find the direction from the entry point to the target
    VectorSubtract(ent->client->arty_target, ent->client->arty_entry, targetdir);
    VectorNormalize(targetdir);
    VectorAdd(ent->client->arty_entry, targetdir, start);
	
   // check we have a clear line of fire
    tr_2 = gi.trace(start, NULL, NULL, ent->client->arty_target, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	// check to make sure we're not materializing in a solid
	if ( gi.pointcontents(start) == CONTENTS_SOLID || tr_2.fraction < 1.0 )
	{
		gi.cprintf(ent, PRINT_HIGH, "Artillery fire was unsuccessful, sir!\n");
		return;
	}

	
    gi.sound(ent, CHAN_AUTO, gi.soundindex(va("%s/arty/hit%i.wav", ent->client->resp.team_on->teamid, ent->client->arty_location)), 1, ATTN_NORM, 0);
       

	// fire away!

    /* old arty code
    fire_rocket(ent, start, targetdir, 700, 250, 300, 450);
    fire_rocket(ent, start, targetdir, 600, 450, 200, 430);
    fire_rocket(ent, start, targetdir, 400, 150, 400, 500);
    fire_rocket(ent, start, targetdir, 600, 210, 250, 500);
    fire_rocket(ent, start, targetdir, 300, 430, 200, 450);
    fire_rocket(ent, start, targetdir, 600, 240, 320, 480);
    */

	fire_airstrike(ent, start, targetdir, 700, 250, 300, 450);
	VectorSet(tempvec, 8, 8, 0);
	VectorAdd(tempvec, start, tempvec);
    fire_airstrike(ent, tempvec, targetdir, 600, 450, 200, 430);
	VectorSet(tempvec, 16, 16, 0);
	VectorAdd(tempvec, start, tempvec);
    fire_airstrike(ent, tempvec, targetdir, 400, 150, 400, 500);
	VectorSet(tempvec, 24, 24, 0);
	VectorAdd(tempvec, start, tempvec);
    fire_airstrike(ent, tempvec, targetdir, 600, 210, 250, 500);
	VectorSet(tempvec, 32, 32, 0);
	VectorAdd(tempvec, start, tempvec);
    fire_airstrike(ent, tempvec, targetdir, 300, 430, 200, 450);
	VectorSet(tempvec, 40, 40, 0);
	VectorAdd(tempvec, start, tempvec);
    fire_airstrike(ent, tempvec, targetdir, 600, 240, 320, 480);

	//fire_shell(ent, start, targetdir, 250, ((rand()%500)+900), 300, 75); 	
	
	gi.cprintf(ent, PRINT_HIGH, "Artillery fire confirmed, sir!\n");

	ent->client->arty_time_restrict = level.time + arty_time->value; // delay for user defined minutes

}



#if 0

/*
=================
Call in Artillery 
KMM:    -- 11/06/98: original version
pbowens -- 07/02/99: code revision.. clean up, etc..  
=================
*/
void Pick_Arty(edict_t *ent, int battary)
{

	edict_t	*Battary_ent;
	vec3_t  start;
	vec3_t  forward;
    vec3_t	world_up;
    vec3_t  end;
    trace_t tr;
	int		index = ent->client->resp.team_on->index;

	if (battary == 0) 
		return;

	--battary;

	Battary_ent=team_list[index]->art_bat[battary];

	// pbowens: battary check
	if (!Battary_ent) {
		gi.cprintf(ent, PRINT_HIGH, "Battary doesn't exist, sir!\n");
		return;
	}
	
	if (Battary_ent->sounds == 1) 
		gi.cprintf(ent,PRINT_HIGH,"Last volly from this battary, sir.\n");
	else if (Battary_ent->sounds < 1)
	{
		gi.cprintf(ent,PRINT_HIGH,"Sorry, sir. we have others who need our support, too.\n");
		return;
	}

	if(!Battary_ent->num_on_target) 
		Battary_ent->num_on_target = 1;

             // cancel arty  if it's already been called and not too late
	if (Battary_ent->arty_called )  //they want to call off the fire
	{
		if(!Battary_ent->arty_fired)  //if the guns haven't been fired
		{
			Battary_ent->arty_called = 0;
			gi.cprintf(ent, PRINT_HIGH, "OK, sir, We'll hold fire.\n");
                     
			Battary_ent->num_on_target--;  //don't give spotter credit
			return;
		}
		else // oops, too late
		{
			gi.cprintf(ent, PRINT_HIGH, "Sorry, sir, We've already fired!\n");
			return;
		}
	}

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	if ( tr.surface && !(tr.surface->flags & SURF_SKY) )
	{ // We hit something but it wasn't sky, so let's see if there is sky above it!

		VectorCopy(tr.endpos,Battary_ent->arty_target); //assign target to Arty
		VectorSet(world_up, 0, 0, 1);
		VectorMA(start, 8192, world_up, end);

		tr = gi.trace(Battary_ent->arty_target, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

		if ( tr.surface && !(tr.surface->flags & SURF_SKY))  // No sky above it either!!
		{
			gi.cprintf(ent, PRINT_HIGH, "If we fire, the building you are in will collapse!\n");
			return;
		}
	}

	// set up for the arty strike
	VectorCopy(tr.endpos, Battary_ent->arty_entry);
	Battary_ent->arty_called = 1;


	Fire_Arty(Battary_ent, ent);
}

void Cmd_Artillery_f (edict_t *ent)
{
	int		i, max_battary = 0;
	char	type[6];
	char	tempstr[15];
	int		index = ent->client->resp.team_on->index;

	tempstr[0]='\0';
	
	if (!IsValidPlayer(ent)
 		return;

	if (ent->client->resp.mos != OFFICER)
	{
		gi.cprintf(ent,PRINT_HIGH,"You do not have the authority to call in artillery soldier!\n");
		return;
	}

   // send the layout
	if (team_list[index]->arty_
	
	for (i = 0; i < max_battary; ++i)
	{ 
		if (!team_list[index]->art_bat || 
			!team_list[index]->art_bat[i]) 
			continue;
		
		if (!team_list[index]->art_bat[i]->sounds || 
			!team_list[index]->art_bat[i]->health) 
			continue;
		
		switch(team_list[index]->art_bat[i]->mass)
		{
			case 1:	strcpy(type,"Napalm");
				break;
			case 2: strcpy(type,"Smoke");
				break;
			case 3: strcpy(type,"Gas");
				break;
			case 4: strcpy(type,"CB");
				break;
			default: strcpy(type,"HE");
		}
		
		Com_sprintf(
			tempstr,
			sizeof(tempstr),
			"%c%i %s  %i",
			((team_list[index]->art_bat[i]->arty_fired)?'! ':'  '),
			i+1,
			type,
			team_list[index]->art_bat[i]->sounds);
   }

} 

void Fire_Arty(edict_t *battary, edict_t *ent)
{	
	int randnum;		
// see if we're pointed at the sky
	srand( (unsigned)time( NULL ) );
	
	battary->arty_fired = 0;
			
	if( (abs(battary->arty_target[0]-battary->last_target[0]) <= 60) &&
		(abs(battary->arty_target[1]-battary->last_target[1]) <= 60) )
	{
		battary->num_on_target++;
		randnum = ((rand() % 15) + 5) / battary->num_on_target;  //generate random number for eta
		if (ent) 
			gi.cprintf(ent, PRINT_HIGH, "Ok, refining target...give us %d seconds to recalculate.\n", randnum);
	}
	else 
	{
		randnum = ((rand() % 25) + 5);  //generate random number for eta
		if (ent)
			gi.cprintf(ent, PRINT_HIGH, "Ok, give us %d seconds to position the guns!\n", randnum);
		battary->num_on_target = 1;

	}

	battary->think = Think_Arty;
	battary->nextthink= level.time + randnum;
	//gi.sound(ent, CHAN_ITEM, gi.soundindex("world/pilot3.wav"), 0.8, ATTN_NORM, 0);
}

/*
=================
Cmd_Airstrike_f
CCH: new function to call in airstrikes
JDB: modified 5/4/98
KMM: modified 11/12/98
=================
*/
void Pick_Air(edict_t *ent, int battary)
{

	edict_t *Airstrike_ent;
    vec3_t  start;
	vec3_t  forward;
    vec3_t world_up;
    vec3_t  end;
    trace_t tr;
	int randnum;
	int index = ent->client->resp.team_on->index;

	if(battary == 0) return;
	battary--;

	Airstrike_ent = team_list[index]->air_bat[battary];

	if (Airstrike_ent->sounds == 1) 
		gi.cprintf(ent,PRINT_HIGH,"Last flight from this wing, sir.\n");
	else if (Airstrike_ent->sounds < 1)
	{
		gi.cprintf(ent,PRINT_HIGH,"Sorry, sir. we have others who need our support, too.\n");
		return;
	}

	// cancel airstrike if it's already been called
	if ( Airstrike_ent->arty_called )
	{
		Airstrike_ent->arty_called = 0;
		gi.cprintf(ent, PRINT_HIGH, "Roger, aborting airstrike!\n");
		return;
	}


	// see if we're pointed at the sky
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
    
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
            
	if ( tr.surface && !(tr.surface->flags & SURF_SKY) )
	{ // We hit something but it wasn't sky, so let's see if there is sky above it!
      
		VectorCopy(tr.endpos,Airstrike_ent->arty_target); //assign target to AS
	  //VectorCopy(tr.endpos,start); //should not need this now
		VectorSet(world_up, 0, 0, 1);
		VectorMA(start, 8192, world_up, end);
        
		tr = gi.trace(Airstrike_ent->arty_target, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
        
		if ( tr.surface && !(tr.surface->flags & SURF_SKY))  // No sky above it either!!
        {
			gi.cprintf(ent, PRINT_HIGH, "Why are you waisting our time with airstrikes indoors?\n");
            return;
		}
	}

	// set up for the airstrike

	VectorCopy(tr.endpos, Airstrike_ent->arty_entry);
	Airstrike_ent->arty_called = 1;
	
	srand( (unsigned)time( NULL ) );
	randnum=( (rand() % 30) + 45);  //generate random number for eta

	Airstrike_ent->think=Think_Airstrike;
	Airstrike_ent->nextthink = level.time + randnum;
	
	gi.cprintf(ent, PRINT_HIGH, "Roger, Airstrike is on its way. ETA %d seconds.\n", randnum);
}



void Cmd_Airstrike_f (edict_t *ent)
{
	int		i, max_battary= 0;
	char	type[6];
	char	tempstr[15];
	int		index = ent->client->resp.team_on->index;
	
	tempstr[0]='\0';

	return; //disabled in FULL1
	if (ent->client->resp.mos!=OFFICER)
	{
		gi.cprintf(ent,PRINT_HIGH,"You don't have the authority to call an airstrike!\n");
		return;
	}

	if (!ent->client->resp.team_on)
	{
		gi.cprintf(ent,PRINT_HIGH,"You are not on any team!\n");
		return;
	
	}

   // send the layout
	if (team_list[index]->air_bat) // if none existed, it crashed the game - pbowens (07/12/99)
	{
		if (team_list[index]->air_bat[0])
		{
			max_battary = (team_list[index]->air_bat[0]->count <= MAX_BATTARY)? team_list[index]->air_bat[0]->count:MAX_BATTARY;
		} 
		else 
		{
			gi.cprintf(ent, PRINT_HIGH, "It seems as though there is no primary airstrike battary, sir!\n");
			return;
		}
	} 
	else 
	{
		gi.cprintf(ent, PRINT_HIGH, "It seems as though there are no air strike battaries, sir!\n");
		return;
	}
 
   for (i=0; i<max_battary; i++)
   {
	   if(!team_list[index]->air_bat || !team_list[index]->air_bat[i]) 
		   continue;
	   if(!team_list[index]->air_bat[i]->sounds ||
		  !team_list[index]->air_bat[i]->health) 
			continue;
	
	   	switch(team_list[index]->air_bat[i]->mass)
		{
			case 1:	strcpy(type,"Napalm");
				break;
			case 2: strcpy(type,"Smoke");
				break;
			case 3: strcpy(type,"Guns");
				break;
			case 4: strcpy(type,"CB");
				break;
			default: strcpy(type,"HE");
		}
		Com_sprintf(tempstr,sizeof(tempstr),"%c%i %s  %i",
			((team_list[index]->air_bat[i]->arty_fired)?'! ':' '),i+1,type,
			team_list[index]->air_bat[i]->sounds);

   }

} 


	



/*
=================
Think_Airstrike
CCH: This will bring the airstrike ordinance into existence in the game
Called by ClientThink
=================
*/
void Impact_Airstrike (edict_t *ent)
{
    vec3_t  start;
    vec3_t  targetdir;
	trace_t tr_2;
	edict_t *commander;
	int		i;

	for (i=0; i < MAX_TEAM_MATES; i++)
	   {
		   if (team_list[ent->obj_owner]->units[i]->client->resp.mos == OFFICER)
		   {
			   commander=team_list[ent->obj_owner]->units[i];
			   break;
		   }
	   }			
       // find the direction from the entry point to the target

    VectorSubtract(ent->arty_target, ent->arty_entry, targetdir);
    VectorNormalize(targetdir);
    VectorAdd(ent->arty_entry, targetdir, start);

       // check we have a clear line of fire
    tr_2 = gi.trace(start, NULL, NULL, ent->arty_target, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);


       // check to make sure we're not materializing in a solid
    if ( gi.pointcontents(start) == CONTENTS_SOLID|| tr_2.fraction < 1.0 )
	{
		gi.cprintf(commander, PRINT_HIGH, "Bad news, the airstrike encountered AAA!.\n");
    //    gi.sound(ent, CHAN_ITEM, gi.soundindex("world/pilot1.wav"), 0.8, ATTN_NORM, 0);
		return;
	}

	
    // fire away!
    fire_rocket(ent, start, targetdir, 700, 250, 300, 450);
    fire_rocket(ent, start, targetdir, 600, 450, 200, 430);
    fire_rocket(ent, start, targetdir, 400, 150, 400, 500);
    fire_rocket(ent, start, targetdir, 600, 210, 250, 500);
    fire_rocket(ent, start, targetdir, 300, 430, 200, 450);
    fire_rocket(ent, start, targetdir, 600, 240, 320, 480);
	//gi.cprintf(ent, PRINT_HIGH, "Tally Ho! You ground boys called for a delivery?.\n");
	ent->arty_fired=ent->arty_called=0;
}

void Think_Airstrike (edict_t *ent)
{
	edict_t *commander;
	int i;

	if (!ent->arty_fired)  // then its time to fire the guns
	{
		ent->arty_fired = 1;
		ent->think=Impact_Airstrike;
		ent->nextthink=level.time + ent->arty_delay;

		//there should be gunfire sounds here...
		for (i=0; i <MAX_TEAM_MATES; i++)
		{
			if (team_list[ent->obj_owner]->units[i]->client->resp.mos==OFFICER)
			{
			   commander = team_list[ent->obj_owner]->units[i];
			   break;
			}
		}			
		
		gi.cprintf(commander, PRINT_HIGH, "The planes are on their way!\n");		
	  //gi.sound(ent, CHAN_AUTO, gi.soundindex("world/amb18.wav"), 1, ATTN_NORM, 0);
	} 
	
} 



/*
=================
Think_Artillery
KMM: This will kill lots of things
Called by ClientThink
=================
*/
void Impact_Arty (edict_t *ent, int size)
{
	int		shortround = 0;
	vec3_t  temptarget;
//	vec3_t  tempentry;
	vec3_t  start;
    vec3_t  targetdir;
    trace_t tr_2;
	int		i,volly_size = ent->health;
	edict_t *commander;

	for (i=0; i < MAX_TEAM_MATES; i++)
	{
	   if (team_list[ent->obj_owner]->units[i]->client->resp.mos == OFFICER)
	   {
		   commander=team_list[ent->obj_owner]->units[i];
		   break;
	   }
	}			   
	   	  
	for(i=0; i <= volly_size; i++) //do a calc for each shell
	{
	   //set deviation for shell
		srand( (unsigned)time(NULL) * rand() ); //randomize the random number system	
		VectorCopy(ent->arty_target,temptarget);
		
		VectorSet(
			temptarget,
			( ((rand() % 400) - 200) / ent->num_on_target) + temptarget[0],
			( ((rand() % 400) - 200) / ent->num_on_target) + temptarget[1],
			temptarget[2]);
	   
	  
       // find the direction from the entry point to the target
		VectorSubtract(temptarget, ent->arty_entry, targetdir);
		VectorNormalize(targetdir);
		VectorAdd(ent->arty_entry, targetdir, start);

       // check we have a clear line of fire
		tr_2 = gi.trace(start, NULL, NULL, temptarget, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);


       // check to make sure we're not materializing in a solid
		if ( gi.pointcontents(start) == CONTENTS_SOLID|| tr_2.fraction < 1.0 ) shortround++;
		else
		{
			fire_shell(ent, start, targetdir, 250, ((rand()%500)+900), 300, 75); 	
			gi.sound(ent, CHAN_AUTO, gi.soundindex("world/explod1.wav"), 0.7, ATTN_NORM, 0);
       // fire away!
		}
	   } // end of the for loop      
	   gi.cprintf(commander, PRINT_HIGH, "Volly %d has been fired! %d rounds fired short.\n", ent->num_on_target, shortround);
	   ent->sounds--;	//take away a volly.
	   ent->arty_called=ent->arty_fired=0;
	   VectorCopy(ent->arty_target,ent->last_target);	
}


void Think_Arty (edict_t *ent)
{
	if(!ent->arty_fired)  // then its time to fire the guns
	{
		ent->arty_fired=1;
		ent->think=Impact_Arty;
		ent->nextthink=level.time + ent->arty_delay;
		//there should be gunfire sounds here...
		//gi.cprintf(ent, PRINT_HIGH, "Rounds are on their way!\n");		
		gi.sound(ent, CHAN_AUTO, gi.soundindex("world/amb18.wav"), 1, ATTN_NORM, 0);
	} 
	
} 

#endif //0
