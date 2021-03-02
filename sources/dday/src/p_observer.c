/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_observer.c,v $
 *   $Revision: 1.20 $
 *   $Date: 2002/07/25 08:28:43 $
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
#include "p_menus.h"
void M_ChooseMOS(edict_t *ent);
void Killed(edict_t * targ , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point ); 
//this file is for all the stuff that relates to observer mode, particularly during the begingg
//of the game when players are joining teams and setting mos.


//faf:  returns a count for # of players on team.  using this to fix
//		bugs with auto-join team and to fix reporting team counts incorrectly
int PlayerCountForTeam (int team_number)
{
	int i;
	int playercount = 0;
    edict_t *check_ent;

        
	for (i = 1; i <= maxclients->value; i++)
    {
         check_ent = g_edicts + i;
         if (!check_ent->inuse)
			 continue;
		 if (!check_ent->client ||
			 !check_ent->client->resp.team_on)
			 continue;

		 if (check_ent->client->resp.team_on->index == team_number)
			 playercount++;

	}
	return playercount;
                              
}



// clients will use these

void SwitchToObserver(edict_t *ent)
{

// start as observer 
	ent->movetype = MOVETYPE_NOCLIP; 
	ent->solid = SOLID_NOT; 
	ent->svflags |= SVF_NOCLIENT; 
	ent->client->ps.gunindex = 0; 
	//ent->client->pers.weapon=NULL;
	gi.linkentity (ent); 
	ent->client->limbo_mode=true;
//	ent->client->deathfade = 0;

	if (!ent->client->display_info)
	{
		if(team_list[0])
			MainMenu(ent);
	}
}

//this function exits observer mode, presumably after they have chosen mos. They must have
//joined a team, if one is avaiable...

void Find_Mission_Start_Point(edict_t *ent, vec3_t origin, vec3_t angles);

void EndObserverMode(edict_t* ent) 
{ 
	vec3_t	spawn_origin, spawn_angles;

	if (!ent->client->limbo_mode) 
		return;

	if (ent->leave_limbo_time > level.time)
		return;


	if( !team_list[0] || !ent->client->resp.team_on)
	{
		gi.cprintf(ent,PRINT_HIGH,"You must join a team first!\n");
		return;
	}

	if (!(ent->svflags & SVF_NOCLIENT)) 
		return; // not in observer mode
/*
	//if they are the first on team, make em captain
	if( ent->client->resp.team_on->units[0]==ent && deathmatch->value) 
	{
		if (ent->client->resp.mos != OFFICER)
			gi.centerprintf(ent, "You have been promoted to Officer!\n");

		ent->client->resp.team_on->officer_mos=ent->client->resp.mos;
		//store the new officer's old backup mos
		ent->client->resp.bkupmos=ent->client->resp.mos;
		ent->client->resp.mos=OFFICER;
		DoEndOM(ent);
	}
	*/

	DoEndOM(ent);

	//ok put the player where he's supposed to be
	Find_Mission_Start_Point(ent, spawn_origin, spawn_angles);

	// unlink to make sure it can't possibly interfere with KillBox	
	gi.unlinkentity (ent);
	
	ent->client->ps.pmove.origin[0] = spawn_origin[0]*8;
	ent->client->ps.pmove.origin[1] = spawn_origin[1]*8;
	ent->client->ps.pmove.origin[2] = spawn_origin[2]*8;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// clear the velocity and hold them in place briefly
	VectorClear (ent->velocity);

	ent->client->ps.pmove.pm_time = 160>>3;		// hold time
	ent->client->ps.pmove.pm_flags |= PMF_TIME_LAND; // pbowens: changed from PMF_TIME_TELEPORT, no particles 
	ent->client->limbo_mode=false;

//	VectorClear (ent->s.angles);
//	VectorClear (ent->client->ps.viewangles);
//	VectorClear (ent->client->v_angle);

	gi.linkentity (ent);

	ent->client->resp.AlreadySpawned=true;
	//gi.bprintf (PRINT_HIGH, "%s has entered the battle.\n", ent->client->pers.netname); 
	WeighPlayer(ent);
	ent->client->spawntime = level.time;

	if (ent->client->resp.mos == SPECIAL)
		ent->client->landed = false;
	else
		ent->client->landed = true;
}

qboolean OpenSpot (edict_t *ent, mos_t class)
{
	int index, spots, taken;
	TeamS_t *team;

	team=ent->client->resp.team_on;

	if (class_limits->value == 0) // class limits turned off
	{
		team->mos[class]->available = 99;
		return true;
	}

	for (taken = 0, index = 0; index < MAX_TEAM_MATES; index++)
	{
		if (!team->units[index]) 
			continue;
		
		if (team->units[index]->client->resp.mos == class) 
		{
			// if you're already that class, leave a spot for yourself
			//if (team->units[index] == ent)
			//	continue;

			 taken++;
		}
	}

	// Not-so-good way of doing things, but it gets the job done
	switch (class)
	{
	case INFANTRY:
		spots							= MAX_INFANTRY;
		team->mos[INFANTRY]->available	= MAX_INFANTRY	- taken;
		break;
	case OFFICER:
		spots							= MAX_OFFICERS;
		team->mos[OFFICER]->available	= MAX_OFFICERS	- taken;
		break;
	case L_GUNNER:
		spots							= MAX_L_GUNNER;
		team->mos[L_GUNNER]->available	= MAX_L_GUNNER	- taken;
		break;
	case H_GUNNER:
		spots							= MAX_H_GUNNER;				// Fix:  Used to say MAX_L_GUNNER
		team->mos[H_GUNNER]->available	= MAX_H_GUNNER	- taken;
		break;
	case SNIPER:
		spots							= MAX_SNIPER;
		team->mos[SNIPER]->available	= MAX_SNIPER	- taken;
		break;
	case SPECIAL:
		spots							= MAX_SPECIAL;
		team->mos[SPECIAL]->available	= MAX_SPECIAL	- taken;
		break;
	case ENGINEER:
		spots							= MAX_ENGINEER;
		team->mos[ENGINEER]->available	= MAX_ENGINEER	- taken;
		break;
	case MEDIC:
		spots							= MAX_MEDIC;
		team->mos[MEDIC]->available		= MAX_MEDIC		- taken;
		break;
	case FLAMER:
		spots							= MAX_FLAMER;
		team->mos[FLAMER]->available	= MAX_FLAMER	- taken;
		break;
	default:
		spots							= 0;
		team->mos[class]->available		= 0;
		break;
	}

/*	gi.bprintf(PRINT_HIGH, "class_stat %s: %s -- %i/%i (%i)\n",
		ent->client->pers.netname,
		team->mos[class]->name,
		taken, spots, 
		team->mos[class]->available);*/

	if (team->mos[class]->available > 0)
		return true;
	else
		return false;
}

void DoEndOM(edict_t *ent /*,qboolean notOfficer*/)
{

	/*if (!ent->client->resp.mos) {
		gi.cprintf(ent, PRINT_HIGH, "You aren't assigned to a class!\n");
		return;
	}*/

	if (!ent->client->resp.team_on) {
		gi.cprintf(ent, PRINT_HIGH, "You aren't assigned to any team!\n");
		return;
	}

	//	assign bkupmos to mos
	//	ent->client->resp.bkupmos=ent->client->resp.mos;

	// if they changed class

	if (ent->client->resp.newmos)
	{
		if (ent->client->resp.mos == NONE || ent->client->resp.mos != ent->client->resp.newmos)
		{
			if (OpenSpot(ent, ent->client->resp.newmos)) {
				ent->client->resp.mos =  ent->client->resp.newmos;
				ent->client->resp.team_on->mos[ent->client->resp.mos]->available--;
			}
			else {
				if (ent->client->resp.mos == NONE) {
					gi.centerprintf(ent, "Request for class denied: Infantry\n");
					ent->client->resp.mos = INFANTRY;						
				} else
					gi.centerprintf(ent, "Your new selected class already\nhas enough players. Retain your\nassignment.\n");

			}
			ent->client->resp.newmos = NONE;
		}
	}


	// reset playermodel with team's
	SyncUserInfo(ent, true); 
	
	ent->takedamage = DAMAGE_YES;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 21;//faf 22;
	ent->inuse = true;
	//ent->classname = "private";
	ent->mass = 200;
	ent->solid = SOLID_TRIGGER; //don't set this until seconds after respawn
	//ent->client->OBTime=level.time+OBDELAY;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
    ent->svflags &= ~SVF_NOCLIENT; 
	ent->wound_location=0;
	Give_Class_Weapon(ent);
	Give_Class_Ammo(ent);

	//if (ent->client->resp.mos == AIRBORNE)
	//	ent->flags |= FL_BOOTS;

	gi.cprintf(ent,PRINT_HIGH, "Your class is %s.\n", 
		ent->client->resp.team_on->mos[ent->client->resp.mos]->name);
	ent->client->limbo_mode = false;
	ent->client->resp.changeteam = false;

} 



void M_MOS_Join(edict_t *ent, pmenu_t *p, int choice)
{
	choice -= 4;

	PMenu_Close(ent);

	ent->client->resp.newmos = choice;

	if (ent->client->resp.AlreadySpawned && ent->client->resp.changeteam == false)
	{
		if (choice == ent->client->resp.mos) { // Already playing that class!
			gi.cprintf(ent, PRINT_HIGH, "You've already been assigned the %s class!\n",
				ent->client->resp.team_on->mos[choice]->name);
		
		} else {
			gi.cprintf(ent, PRINT_HIGH, "Requesting %s class assignment your next operation.\n",
				ent->client->resp.team_on->mos[choice]->name);
		}

		return;
	}

	if (ent->client->resp.changeteam) {
		ent->client->resp.mos = INFANTRY;
//faf		respawn(ent);
	}
//	} else
//		EndObserverMode(ent);//faf: handle this in begin client frame
//	else if (level.framenum >   ((int)(level_wait->value * 10) +  (ent->client->spawn_delay * 10))   )
//		EndObserverMode(ent);

}

/*
void SMOS_Join(edict_t *ent,int choice)
{
	//pbowens: just in case
	gi.cprintf(ent, PRINT_HIGH, "Secondary MOS/CLASS has been disabled!\n");
	return;

	if(choice!=0) choice--;
	ent->client->resp.smos=choice;
	ent->client->usr_menu_sel=NULL;
	EndObserverMode(ent);
}
*/


// There are many ways to do this.. but this way was easier on the eyes
void client_menu(edict_t *ent, int entry, char *text, int align, void *arg, void (*SelectFunc)(edict_t *ent, struct pmenu_s *entry, int choice)) {
	ent->client->menu_cur[entry].text		= text;
	ent->client->menu_cur[entry].align		= align;
	ent->client->menu_cur[entry].arg		= arg;
	ent->client->menu_cur[entry].SelectFunc = SelectFunc;
}


void M_ChooseMOS(edict_t *ent)
{
	int i;
	char* theText = NULL;
	int taken;
	int maxSlots;
	int index;

	//pmenu = (ent->client->resp.team_on->index) ? menu_classes_grm : menu_classes_usa;
	//memcpy(ent->client->menu_cur, menu_classes, sizeof(pmenu_t));

	PMenu_Close(ent);

	client_menu( ent, 0, "*D-DAY: NORMANDY "  DEVVERSION,	PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 2,  NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 3, "Choose A Class",					PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 4, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu( ent, 15, NULL,								PMENU_ALIGN_RIGHT,	NULL, NULL );
	client_menu( ent, 16, "Main Menu",						PMENU_ALIGN_RIGHT,	NULL, M_Main_Menu );

	if (ent->client->limbo_mode && 
	   !ent->client->resp.AlreadySpawned &&
	  (!ent->client->resp.team_on || !ent->client->resp.team_on->teamname) )
		return;

	for(i=1; i < MAX_MOS;i++) 
	{
		//char theText[24];
		
/*		Com_sprintf(ent->client->menu_cur[i+4].text, sizeof(ent->client->menu_cur[i+4].text), "%s [%i]",
			ent->client->resp.team_on->mos[i]->name,
			ent->client->resp.team_on->mos[i]->available);
*/

		// Tally available for this MOS, AKA Roll up on this bitch and count, sucka
		for (taken = 0, maxSlots = 0, index = 0; index < MAX_TEAM_MATES; index++)
		{
			if (!ent->client->resp.team_on->units[index])
				continue;

			if (ent->client->resp.team_on->units[index]->client->resp.mos == ent->client->resp.team_on->mos[i]->mos && ent->client->resp.team_on->units[index]->client->ping < 999)
				taken++;
		}

		// Now set the available for this class
		switch (ent->client->resp.team_on->mos[i]->mos)
		{
		case INFANTRY:
			maxSlots = MAX_INFANTRY;
			ent->client->resp.team_on->mos[i]->available = MAX_INFANTRY - taken;
			break;
		case OFFICER:
			maxSlots = MAX_OFFICERS;
			ent->client->resp.team_on->mos[i]->available = MAX_OFFICERS - taken;
			break;
		case L_GUNNER:
			maxSlots = MAX_L_GUNNER;
			ent->client->resp.team_on->mos[i]->available = MAX_L_GUNNER - taken;
			break;
		case H_GUNNER:
			maxSlots = MAX_H_GUNNER;
			ent->client->resp.team_on->mos[i]->available = MAX_H_GUNNER - taken;
			break;
		case SNIPER:
			maxSlots = MAX_SNIPER;
			ent->client->resp.team_on->mos[i]->available = MAX_SNIPER - taken;
			break;
		case SPECIAL:
			maxSlots = MAX_SPECIAL;
			ent->client->resp.team_on->mos[i]->available = MAX_SPECIAL - taken;
			break;
		case ENGINEER:
			maxSlots = MAX_ENGINEER;
			ent->client->resp.team_on->mos[i]->available = MAX_ENGINEER - taken;
			break;
		case MEDIC:
			maxSlots = MAX_MEDIC;
			ent->client->resp.team_on->mos[i]->available = MAX_MEDIC - taken;
			break;
		case FLAMER:
			maxSlots = MAX_FLAMER;
			ent->client->resp.team_on->mos[i]->available = MAX_FLAMER - taken;
			break;
		default:
			maxSlots = 0;
			ent->client->resp.team_on->mos[i]->available = 0;
			break;
		}

		// Setup text variable
		theText = gi.TagMalloc(sizeof("123456789012 [00/00]"), TAG_GAME);
		strcpy(theText, va("%12s [%i/%i]", ent->client->resp.team_on->mos[i]->name, taken, maxSlots));

		ent->client->menu_cur[i+4].text  = (class_limits->value)?(char *)theText:ent->client->resp.team_on->mos[i]->name;
		ent->client->menu_cur[i+4].align = PMENU_ALIGN_LEFT;
		ent->client->menu_cur[i+4].arg   = NULL;
		ent->client->menu_cur[i+4].SelectFunc = M_MOS_Join;

	}

	// You can't go back and change stuff before you've spawned
	if (ent->client->resp.AlreadySpawned || !ent->client->resp.changeteam)
		client_menu(ent, 16, "Main Menu",		PMENU_ALIGN_RIGHT,	NULL, M_Main_Menu );

	client_menu(ent, 25, "*Use [ and ] to select", PMENU_ALIGN_CENTER,	NULL, NULL );

	PMenu_Open(ent, ent->client->menu_cur, 5, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
	//gi.TagFree(theText);
}


void M_Team_Join(edict_t *ent, pmenu_t *p, int choice)
{

	qboolean foundspot=false;
	int i,j,k;

	PMenu_Close(ent);

	choice -= 5;

	if (choice == 3) { // auto team
		i = j = k = 0;

		/*faf
		for (k = 0; k <= MAX_TEAM_MATES; k++)
		{
			if (team_list[0]->units[k])
				i++;
			if (team_list[1]->units[k])
				j++;
		}*/
		//faf
		i = PlayerCountForTeam(0);
		j = PlayerCountForTeam(1);

		//faf:  if theyre already on a team subtract them from the total
		if (ent->client->resp.team_on &&
			ent->client->resp.team_on->index == 0)
			i--;

		if (ent->client->resp.team_on &&
			ent->client->resp.team_on->index == 1)
			j--;


		if (i > j) 
			choice = 1;
		else if (i < j) 
			choice = 0;
		else if (team_list[0]->kills > team_list[1]->kills)//faf
			choice = 1;
		else if (team_list[1]->kills > team_list[0]->kills)//faf
			choice = 0;
		else
		{
			if (ent->client->resp.team_on)
			{
				PMenu_Close(ent);
				return;
			}

			if (crandom() * 3 > 1)
				choice = 0;
			else
				choice = 1;
		}
	}

	if (ent->client->resp.AlreadySpawned)
	{
		if (ent->client->resp.team_on->index == team_list[choice]->index)
		{
			gi.cprintf(ent, PRINT_HIGH, "Already on team %s!\n", team_list[choice]->teamname);
			PMenu_Close(ent);
			return;
		}
	}

	for(i=0;i<MAX_TEAM_MATES;i++)
	{
		if (!team_list[choice])
			continue;

		if (!team_list[choice]->units[i])
		{
			if (ent->client->resp.team_on)
			{
//faf: "total" not used	now			team_list[ent->client->resp.team_on->index]->total--;
				team_list[ent->client->resp.team_on->index]->units[ent->client->resp.unit_index] = NULL;
				ent->client->resp.unit_index = i;
			}
			ent->client->resp.team_on=team_list[choice];
//faf: not used			team_list[choice]->total++;
			team_list[choice]->units[i]=ent;
			foundspot=true;

			ent->client->resp.mos = NONE; // reset MOS
			break;
		}
	}
	
	if(!foundspot)
	{
		gi.cprintf(ent,PRINT_HIGH,"No room left on the team. ");
		ChooseTeam(ent);
		return;
	} 
	else 
	{
		if (ent->client->resp.AlreadySpawned) // used choose_team cmd
		{
			/*
			ent->flags &= ~FL_GODMODE;
			ent->health = 0;
			meansOfDeath = MOD_SUICIDE;
			ent->deadflag = 0; // PLAY the animations
			player_die (ent, ent, ent, 100000, vec3_origin);
			*/
			T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 999, 0, 0, MOD_CHANGETEAM);
			ent->client->resp.mos = INFANTRY;
			ent->client->resp.changeteam = true;
			ent->client->forcespawn = level.time + .5;//faf: fixes standing corpse bug
					
			gi.bprintf(PRINT_HIGH, "%s has switched to team %s.\n", ent->client->pers.netname, ent->client->resp.team_on->teamname);
		}
		else
			gi.bprintf(PRINT_HIGH, "%s has joined team %s.\n", ent->client->pers.netname, ent->client->resp.team_on->teamname);

		M_ChooseMOS(ent);
		//EndObserverMode(ent);	// *RSH Copied this from SMOS_Join code to try and start the game. 

		return;
	}

	gi.dprintf("warning: %s got to end of M_Team_Join().\n", ent->client->pers.netname);


}



void ChooseTeam(edict_t *ent) {
	int i,j;
	char* theText = NULL;
	int max_clients;
	PMenu_Close(ent);

	if (ent->client->resp.changeteam == true) {
		gi.centerprintf(ent, "You have already changed teams once!\nYou must wait for your next assignment\n");
		return;
	}

	// rezmoth - must wait until end of lobby time //faf:  not
//faf	if (level.framenum < ((int)level_wait->value * 10))
//faf		return;

	// Eliminates ghost-bug
	if ((ent->client->limbo_mode || ent->deadflag) && ent->client->resp.team_on)
	{
		gi.centerprintf(ent, "You must wait for your next assignment\nto change teams!");
		return;
	}

	client_menu(ent, 0, "*D-DAY: NORMANDY "  DEVVERSION,PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 1, "*by Vipersoft",			PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 2, NULL,						PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 3, "*Choose Team",				PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 4, NULL,						PMENU_ALIGN_CENTER, NULL, NULL );


	if (!force_auto_select->value)//faf
	{

		for(i=0;i<MAX_TEAMS;i++)
		{
			if (!team_list[i]) 
				continue;

			//faf:  password off teams
			if (i == 0)
			{
				if (Q_stricmp(allied_password->string, "") != 0)
				{
					if (Q_stricmp(allied_password->string, Info_ValueForKey (ent->client->pers.userinfo, "password")) != 0) 
					{
						if (Q_stricmp(allied_password->string, "none") != 0)
						continue;
					}
				}
			}				

			if (i == 1)
			{
				if (Q_stricmp(axis_password->string, "") != 0)
				{
					if (Q_stricmp(axis_password->string, Info_ValueForKey (ent->client->pers.userinfo, "password")) != 0) 
					{
						if (Q_stricmp(axis_password->string, "none") != 0)
						continue;
					}
				}
			}				



			for (j=0; team_list[i]->units[j]; j++);

			max_clients = maxclients->value;
			// Make the text look good
			theText = gi.TagMalloc(sizeof("123456789012 [00/00]"), TAG_GAME);
			strcat(theText, va("%12s [%i/%i]", team_list[i]->teamname, PlayerCountForTeam(i), max_clients));//faf: removed "team_list[i]->total,"

			// Put it on the menu
			client_menu(ent, (i + 5), theText, PMENU_ALIGN_LEFT, NULL, M_Team_Join );
		}
	}

//	client_menu(ent, 7, NULL,				PMENU_ALIGN_CENTER, NULL, NULL );
	if (((Q_stricmp(allied_password->string, "") == 0) ||
		(Q_stricmp(allied_password->string, "none") == 0)) && 
		((Q_stricmp(axis_password->string, "") == 0) ||
		(Q_stricmp(axis_password->string, "none") == 0)))
		client_menu(ent, 8, "Auto Select",		PMENU_ALIGN_LEFT,	NULL, M_Team_Join );		

//	client_menu(ent, 9, NULL,				PMENU_ALIGN_RIGHT,	NULL, NULL );
	client_menu(ent, 10, "Main Menu",		PMENU_ALIGN_RIGHT,	NULL, M_Main_Menu );
//	client_menu(ent, 11, NULL,				PMENU_ALIGN_RIGHT,	NULL, NULL );
	client_menu(ent, 25, "*Use [ and ] to select", PMENU_ALIGN_CENTER,	NULL, NULL );


	PMenu_Open(ent, ent->client->menu_cur , 5, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
	//gi.TagFree(theText);


} 


void MainMenu(edict_t *ent)
{
	PMenu_Close(ent);

	client_menu(ent, 0, "*D-DAY: NORMANDY " DEVVERSION, PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 1, "*by Vipersoft",	PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 2,  NULL,				PMENU_ALIGN_CENTER, NULL, NULL ),
	client_menu(ent, 3,  "*Main Menu",		PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 4,  NULL,				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 5,  "Choose a Team",	PMENU_ALIGN_LEFT,	NULL, M_Team_Choose );
	client_menu(ent, 6,  "View Credits",	PMENU_ALIGN_LEFT,	NULL, M_View_Credits );
//	client_menu(ent, 7,  NULL,				PMENU_ALIGN_RIGHT,	NULL, NULL );
//	client_menu(ent, 8,  "*Spectate",		PMENU_ALIGN_LEFT,	NULL, NULL );
	client_menu(ent, 25, "*Use [ and ] to select",			PMENU_ALIGN_CENTER,	NULL, NULL );


	PMenu_Open(ent, ent->client->menu_cur, 5, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}

void M_Main_Menu (edict_t *ent, pmenu_t *p, int choice) 
{ MainMenu(ent); }

void M_Team_Choose (edict_t *ent, pmenu_t *p, int choice) 
{
	PMenu_Close(ent);
	ChooseTeam(ent);
}

void M_View_Credits (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY " DEVVERSION,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 2, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 3, "*Development Credits",				PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 4, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 5, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 6, "D-Day Was Created By",				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 7,  "*Vipersoft",		PMENU_ALIGN_CENTER,	NULL, M_View_Credits_Vipersoft);
//	client_menu(ent, 8,  NULL,				PMENU_ALIGN_CENTER, NULL, NULL );
//	client_menu(ent, 9,  NULL,				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 10, "Continued",				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 11, "Development By",				PMENU_ALIGN_CENTER, NULL, NULL );
	client_menu(ent, 12,  "*SHAEF",		PMENU_ALIGN_CENTER,	NULL, M_View_Credits_Shaef);
//	client_menu(ent, 13,  NULL,				PMENU_ALIGN_RIGHT,	NULL, NULL );
//	client_menu(ent, 14,  "*Spectate",		PMENU_ALIGN_LEFT,	NULL, NULL );
	client_menu(ent, 15, "Main Menu",						PMENU_ALIGN_CENTER,	NULL, M_Main_Menu );
	client_menu(ent, 16, "*Use [ and ] to select",			PMENU_ALIGN_CENTER,	NULL, NULL );


	PMenu_Open(ent, ent->client->menu_cur, 7, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}



void M_View_Credits_Vipersoft (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY " DEVVERSION,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 2, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 3, "*Development Credits",				PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 4, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );

		client_menu(ent, 5, "Project Leader",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 6, "*Jason 'Abaris' Mohr",				PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 7, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 8, "Programming",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 9, "*Phil Bowens",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 10, "*Species",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 11, "*Adam 'RezMoth' Sherburne",		PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 12, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 13, "Level Design",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 14, "*Peter 'Castrator' Lipman",		PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 15, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 16, "Visual Artist",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 17, "*Darwin Allen",					PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 18, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
//		client_menu(ent, XX, "Sound Engineer",					PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, XX, "*Oliver 'JumperDude' Snavely",	PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 19, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 20, "Webmistress",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 21, "*Wheaty",							PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 22, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );

	
		client_menu(ent, 25, "Main Menu",						PMENU_ALIGN_CENTER,	NULL, M_Main_Menu );

	PMenu_Open(ent, ent->client->menu_cur, -1, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}


void M_View_Credits_Shaef (edict_t *ent, pmenu_t *p, int choice) 
{
		PMenu_Close(ent);

		client_menu(ent, 0, "*D-DAY: NORMANDY " DEVVERSION,		PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 1, "*by Vipersoft",					PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 2, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 3, "*Supreme Headquarters,",				PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 4, "*Allied Expeditionary",				PMENU_ALIGN_CENTER, NULL, NULL );
		client_menu(ent, 5, "*Forces",				PMENU_ALIGN_CENTER, NULL, NULL );
//		client_menu(ent, 6, NULL,								PMENU_ALIGN_CENTER, NULL, NULL );

		client_menu(ent, 7, "Project Coordinator",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 8, "*Col Piron",				PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 9, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 10, "Programming",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 11, "*Fafner",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 12, "*Cpt. Bill Stokes",						PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 13, "*Kermit",						PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 14, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
		client_menu(ent, 15, "Modelling",					PMENU_ALIGN_CENTER,	NULL, NULL );
		client_menu(ent, 16, "*Mjr. Parts",		PMENU_ALIGN_CENTER,	NULL, NULL );
//		client_menu(ent, 17, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );
//		client_menu(ent, 18, NULL,								PMENU_ALIGN_LEFT,	NULL, NULL );

	
		client_menu(ent, 19, "Main Menu",						PMENU_ALIGN_CENTER,	NULL, M_Main_Menu );

	PMenu_Open(ent, ent->client->menu_cur, -1, sizeof(ent->client->menu_cur) / sizeof(pmenu_t));
}
