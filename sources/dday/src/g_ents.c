/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_ents.c,v $
 *   $Revision: 1.8 $
 *   $Date: 2002/07/23 21:11:37 $
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
#include "g_cmds.h"
#include "g_dll.h"

// g_ents.c
// D-Day: Normandy Old / Unsorted Entities



#define NO_MISSION 99

void multi_trigger (edict_t *ent);
void InitTrigger (edict_t *self);
int Last_Team_Winner=99;
//char map_tree[MAX_TEAMS][MAX_QPATH];

/*********************************************
** This is for all of the custom entity and 
** trigger functions for DDay				
*********************************************/


/////////////////////////////////////////////////////
//the following functions keep count of how many troops have triggered it


//count is total number required...
//heath is the mission number (0 defence, 1 offence, 2 patrol)
//dmg is the ammount of pointes added to the team
//delay is the time to win before the game is won


/////////////////////
void trigger_enough_troops_use (edict_t *self, edict_t *other, edict_t *activator)
{
	int i;

	gi.dprintf("trigger_enough_troops_use called\n");

	if( (self->obj_owner < MAX_TEAMS) && (activator->client->resp.team_on->index != self->obj_owner) )
	{
		gi.dprintf("TRIGGER return code 1\n");
		return;
	}

	if (self->count == 0) {
		gi.dprintf("TRIGGER return code 2 \n");
		return;
	}
	

	if( (activator->client->resp.team_on->mission != self->health) && (self->health != NO_MISSION) )
	{
		team_list[self->obj_owner]->time_to_win=0;
		self->obj_owner=activator->client->resp.team_on->index;
		gi.dprintf("TRIGGER return code 3\n");
		return;				// if not for this mission get out of here...
	}

	for(i=0; i < MAX_TEAM_MATES; i++)
	{
		if(self->who_touched[i])
		{	//if they are already registered...
			if(self->who_touched[i]==activator)
				i=MAX_TEAM_MATES;
			continue;
		}
		//otherwise go on through and add the to the list
		self->count--;
		self->who_touched[i]=activator;
		self->obj_owner=activator->client->resp.team_on->index;

		if (self->count)
		{
			if (! (self->spawnflags & 1))
			{ 
				gi.centerprintf(activator, "%i more to go...", self->count);
			} 
			gi.dprintf("TRIGGER return code 4\n");
			return;
		} 
	
		else
		{

			if (! (self->spawnflags & 1))
			{ 
				gi.centerprintf(activator, "Ok, we got 'em all here!");
			}
			activator->client->resp.team_on->score=self->dmg;
			team_list[self->obj_owner]->time_to_win+=self->delay;
		} 
				
		
		gi.dprintf("TRIGGER return code 5\n");
		self->activator = activator;
		multi_trigger (self);
		i = MAX_TEAM_MATES;
	}
	gi.dprintf("TRIGGER return code 6\n");
}


void SP_trigger_enough_troops (edict_t *self)
{
	self->wait = -1;
	if (!self->count)
		self->count = 2;

	self->who_touched = gi.TagMalloc((sizeof(edict_t)*MAX_TEAM_MATES),TAG_LEVEL);
	self->use = trigger_enough_troops_use;
}

void SP_info_Mission_Results(edict_t *ent)
{
//put next map info here.
/*	strcpy(map_tree[ent->dmg], ent->map);

	if(!ent->classname)
		strcpy(map_tree[ent->mass],level.mapname);
	else 
		strcpy(map_tree[ent->mass], ent->classname);

	G_FreeEdict(ent);*/
}

/////////////////////////////////////////////////////////////////////////////////
//The following functions keep track of who owns the entity
/////////////////////////////////////////////////////////////////////////////////
//dmg is points for owning objtive
void target_objective_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if(self->obj_owner==activator->client->resp.team_on->index)
		return; // if team already in possesion, continue

	if(team_list[self->obj_owner]!=NULL)
		team_list[self->obj_owner]->score-=self->dmg;

	gi.bprintf(PRINT_HIGH, "%s has captured an objective point for team %s!\n",
		activator->client->pers.netname,
		team_list[self->obj_owner]->teamname);

	self->obj_owner=activator->client->resp.team_on->index;
	team_list[self->obj_owner]->score+=self->dmg;
}

void SP_target_objective(edict_t *ent)
{
	ent->wait=-1;
	ent->use=target_objective_use;

	if (ent->delay && ent->obj_owner != 99) 
		team_list[ent->obj_owner]->time_to_win = (level.time + ent->delay);
}




////////////////////////////////////////////////////////////////////////////////
//The following is for creating teams on spawn
////////////////////////////////////////////////////////////////////////////////
int InitializeUserDLLs(userdll_list_node_t *unode,int teamindex);
userdll_list_node_t *LoadUserDLLs(edict_t *ent, int team);



void SP_info_reinforcement_start(edict_t *ent)
{
	ent->think= reinforcement_think;
	ent->nextthink = level.time+((ent->delay)?ent->delay : RI->value);
	ent->nextspawn = ent->nextthink;
	ent->spawnpoint = true;
} 



void SP_info_team_start(edict_t *ent)
{
	int i;
	
	i=ent->obj_owner;
	
	team_list[i]=gi.TagMalloc(sizeof(TeamS_t),TAG_LEVEL);
	team_list[i]->teamname=gi.TagMalloc(sizeof(ent->message + 2),TAG_LEVEL);
	//strcpy(team_list[i]->teamname,ent->message);
	team_list[i]->teamname = ent->message;
//	team_list[i]->playermodel = gi.TagMalloc( 64, TAG_LEVEL );
//	team_list[i]->teamid = gi.TagMalloc( 64, TAG_LEVEL );
	team_list[i]->nextmap = gi.TagMalloc( 64, TAG_LEVEL );

	team_list[i]->kills=0;
	team_list[i]->losses=0;
	team_list[i]->index=i;
	team_list[i]->score=0;

	if (ent->map)
		team_list[i]->nextmap = ent->map;
	else
		team_list[i]->nextmap = level.mapname;


	//make it so if allies win dday5 it goes to dday1
	if (!stricmp(level.mapname, "dday5"))  
	{
		team_list[0]->nextmap = "dday1";
	}


	//faf:  servers can turn off campaign mode
	if (campaign_mode->value != 1)
	{
		//sets it so the rotation always continues like the allied team won
		if (team_list[0] &&
			team_list[0]->nextmap &&
			team_list[1] &&
			team_list[1]->nextmap)
		{
			strcpy (team_list[1]->nextmap, team_list[0]->nextmap);
		}

		//so we dont get stuck
		if (!stricmp(level.mapname, "mp1dday3"))  
		{
			ent->map = "mp1dday1";        
			team_list[i]->nextmap = ent->map;   
		}

	}





	
	if(ent->count!=99 || Last_Team_Winner==99 )
	{
		if(ent->count==TEAM_OFFENCE) team_list[i]->mission=TEAM_OFFENCE;
		else if(ent->count==TEAM_DEFENCE) team_list[i]->mission=TEAM_DEFENCE;
		else team_list[i]->mission=TEAM_PATROL;
	}
	else
	{
		if(Last_Team_Winner==i) team_list[i]->mission=TEAM_OFFENCE;
		else team_list[i]->mission=TEAM_DEFENCE;
	}

//	if (ent->delay)
		team_list[i]->time_to_win = 0; //ent->delay;

	if (ent->health)
		team_list[i]->need_points = ent->health;
	else
		team_list[i]->need_points = 0;

	if (ent->dmg)
		team_list[i]->need_kills = ent->dmg;


// now is the time to hook up the mos .dll files...
	   
    InitializeUserDLLs(LoadUserDLLs(ent, i),i);

//	InitMOS_List(team_list[i]);

	G_FreeEdict (ent);	//clean up entity now that it's not needed.

}
////////////////////////////////////////////////////////////////
//  The following entity changes the default value for the max_mos types 
//  Per team
/////////////////////////////////////////////////////////////////


void SP_info_Max_MOS(edict_t *ent)
{
/*	int i = ent->obj_owner;

	if(!team_list[i]) 
		return;

	if(ent->spawnflags) 
		team_list[i]->mos[1]->available=ent->spawnflags;
	if(ent->style) 
		team_list[i]->mos[2]->available=ent->style;
	if(ent->count) 
		team_list[i]->mos[3]->available=ent->count;
	if(ent->health)
		team_list[i]->mos[4]->available=ent->health;
	if(ent->sounds)
		team_list[i]->mos[5]->available=ent->sounds;
	if(ent->dmg) 
		team_list[i]->mos[6]->available=ent->dmg;
	if(ent->takedamage)
		team_list[i]->mos[7]->available=ent->takedamage;
	if(ent->mass) 
		team_list[i]->mos[8]->available=ent->mass;

	G_FreeEdict (ent);	//clean up entity now that it's not needed.
*/
}

///////////////////////////////////////////////////////////////////////////
//  The Following entity sets the skin type for the mission
//////////////////////////////////////////////////////////////////////////

/*
void SP_info_Skin(edict_t *ent)
{
	int i=ent->obj_owner;

	if(!team_list[i]) 
		return;

	team_list[i]->skindir=gi.TagMalloc(sizeof(ent->pathtarget)+1,TAG_LEVEL);
	
	strcpy(team_list[i]->skindir,ent->pathtarget);
	G_FreeEdict(ent);
}

*/
///////////////////////////////////////////////////////////////////////////
// The Following entity prepares artillery strikes
///////////////////////////////////////////////////////////////////////////
/*
void SP_info_Arty_Battery(edict_t *ent)
{
	int j,i=ent->obj_owner;
	size_t size = sizeof(ent)*(ent->count);


	if(!team_list[i]) return;
	if(!team_list[i]->art_bat)
	{
		team_list[i]->art_bat=gi.TagMalloc(size ,TAG_LEVEL);
		for(j=0;j<ent->count;j++) team_list[i]->art_bat[j]=NULL;
	}
	
	for(j=0;j<=ent->count;j++)
	{
		if(!team_list[i]->art_bat[j])
		{
			team_list[i]->art_bat[j]=ent;
			ent->arty_delay=((rand()%5)+5);
			ent->think=Think_Arty;
			break;
		}
	}
	ent->arty_called=ent->arty_fired=0;
}

void SP_info_Air_Battery(edict_t *ent)
{
	int j,i=ent->obj_owner;
	size_t size = sizeof(ent)*(ent->count);

	if(!team_list[i]) return;
	if(!team_list[i]->air_bat)
	{
		team_list[i]->air_bat=gi.TagMalloc(size ,TAG_LEVEL);
		for(j=0;j<=ent->count;j++)
			team_list[i]->air_bat[j]=NULL;
	}
	
	for(j=0;j<=ent->count;j++)
	{
		if(!team_list[i]->air_bat[j])
		{
			team_list[i]->air_bat[j]=ent;
			ent->arty_delay=0;
			ent->think=Think_Airstrike;
			break;
		}
	}
	ent->arty_called=ent->arty_fired=0;

}

void SP_event_Arty_Strike(edict_t *ent)
{
	edict_t *battary;
		
	if(ent->mass)
	{
		if(!team_list[ent->obj_owner]->air_bat[ent->count])
			return;
		battary=team_list[ent->obj_owner]->air_bat[ent->count];

	}
	else
	{
		if(!team_list[ent->obj_owner]->art_bat[ent->count])
			return;
		battary=team_list[ent->obj_owner]->art_bat[ent->count];
		battary->num_on_target=(ent->dmg>0)?ent->dmg:1;
	}
		
	VectorCopy(ent->s.origin,battary->arty_target);
	battary->arty_time=level.time + ent->health;
	battary->arty_called=true;
	G_FreeEdict(ent);
}
*/
