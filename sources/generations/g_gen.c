#include "g_local.h"
#include "g_genutil.h"

// model paths
#define WOLFGUY_MODEL	"wolfguy/"		
#define DOOMGUY_MODEL	"doomguy/"
#define QUAKEGUY_MODEL	"quakeguy/"
#define WOLF_SKIN		"base"		
#define DOOM_SKIN		"green"
#define QUAKE_SKIN		"base"

generations_game	gengame;

cvar_t *gen;
cvar_t *gen_forcejoin;


void stuffcmd(edict_t *ent, char *s) 	
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}

//================================================================================
//================================================================================

void GenInit(void)
{
	gen = gi.cvar("gen", "1", CVAR_SERVERINFO);

	if(deathmatch->value)
	{
		gi.cvar_forceset("gen_sp","0");
		if(gen_ctf->value)
		{
			gi.cvar_forceset("gen_team", "0");
			gi.dprintf("==== Init GenCTF ====\n");
		}
		else if(gen_team->value)
		{
			gi.cvar_forceset("gen_ctf", "0");
			gi.dprintf("==== Init Gen TeamPlay ====\n");
			if((int)genflags->value & GEN_REDBLUE_TEAMS)
				gi.dprintf("=== Red vs Blue Teams ===\n");
			else
				gi.dprintf("=== Class Based Teams ===\n");
		}
		GenTryCTF();
	}
	else
	{
		gi.cvar_forceset("gen_sp","1");
	}

	//max speed
	gi.cvar_forceset("cl_forwardspeed", "200");
	gi.cvar_forceset("cl_sidespeed", "200");
	
	gen_forcejoin = gi.cvar("gen_forcejoin", "", 0);
	memset(&gengame, 0, sizeof(gengame));
}

//================================================================================

char *GenClassName(int player_class)
{
	switch (player_class) 
	{
		case CLASS_WOLF:
			return "Wolfguy";
		case CLASS_DOOM:
			return "Doomguy";
		case CLASS_Q1:
			return "Quakeguy";
		case CLASS_Q2:
			return "Quake2 gumby";
	}
	return "UNKNOWN";
}

//================================================================================
//================================================================================

qboolean ValidSkin(int pclass,const char *s)
{
	FILE *fp;
	char skinname[MAX_QPATH];

	if(!s || !(*s))
		return false;

	strcpy(skinname, "./baseq2/players/");
	switch(pclass)
	{
		case CLASS_WOLF:
			strcat(skinname,WOLFGUY_MODEL);
			break;
		case CLASS_DOOM:
			strcat(skinname,DOOMGUY_MODEL);
			break;
		case CLASS_Q1:
			strcat(skinname,QUAKEGUY_MODEL);
			break;
	}
	strcat(skinname,s);
	strcat(skinname,".pcx");

	fp = fopen(skinname, "r" );
	if(fp != NULL)
	{
		fclose(fp);
		return true;
	}
	return false;
}


// Local Function to check if the model for your class is valid

qboolean ValidModel(int player_class, const char *t)
{
	if(!t || !(*t))
		return false;

	switch(player_class)
	{
		case CLASS_Q1:
			{
				if(memcmp(t,QUAKEGUY_MODEL,9)==0)
					return true;
				break;
			}
		case CLASS_DOOM:
			{
				if(memcmp(t,DOOMGUY_MODEL,8)==0)
					return true;
				break;
			}
		case CLASS_WOLF:
			{
				if(memcmp(t,WOLFGUY_MODEL,8)==0)
					return true;
				break;
			}
		case CLASS_Q2:
		default:
			{
			  if((memcmp(t,QUAKEGUY_MODEL,9)==0) ||      
			     (memcmp(t,DOOMGUY_MODEL,8)==0)	 ||
				 (memcmp(t,WOLFGUY_MODEL,8)==0))
					return false;
				return true;
			}
	}
	return false;
}
					
//================================================================================

qboolean GenAssignSkin(edict_t *ent, const char *qs)
{
	int playernum = ent-g_edicts-1;
	int pclass = ent->client->resp.player_class;
	char p[64];

	//New Q1 Skin Var		
	if(!(gen_ctf->value || 
		(gen_team->value && ((int)genflags->value & GEN_REDBLUE_TEAMS)))
		&& (strlen(qs) > 0) 
		&& ValidSkin(pclass,qs))
	{
		
		switch(pclass)
		{
		case CLASS_WOLF:
			strcpy(p,WOLFGUY_MODEL);
			break;
		case CLASS_DOOM:
			strcpy(p,DOOMGUY_MODEL);
			break;
		case CLASS_Q1:
			strcpy(p,QUAKEGUY_MODEL);
			break;
		}

		strcat(p,qs);
		ent->flags |= FL_VALID_MODEL;
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, p));
//
		Info_SetValueForKey(ent->client->pers.userinfo, "skin", va("%s", p));
		return true;
	}
	return false;
}


qboolean GenValidateModel(int pclass, const char *s)
{
	char *p;
	char t[64];

	//Validate Regular Skin Var
	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)	//did we find a '/' character?
		p[1] = 0;						//yes
	else
		return false;

	if(ValidModel(pclass,t))
		return true;
	return false;
}


void GenAssignModel(edict_t *ent, const char *s) 
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];

	//Validate Regular Skin Var
	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)	//did we find a '/' character?
		p[1] = 0;						//yes
	else
	{
		// Set model
		switch (ent->client->resp.player_class)
		{
			case CLASS_WOLF:
				strcpy(t,WOLFGUY_MODEL);	
				break;
			case CLASS_DOOM:
				strcpy(t,DOOMGUY_MODEL);
				break;
			case CLASS_Q1:
				strcpy(t,QUAKEGUY_MODEL);
				break;
			case CLASS_Q2:
			default:
				strcpy(t,"male/");			//default to male when they select an invalid model
				break;
		}
	}

	
	// check if current "skin" is valid for your class
	if(ValidModel(ent->client->resp.player_class,t))
	{
		if(gen_ctf->value || 
		  (gen_team->value && ((int)genflags->value & GEN_REDBLUE_TEAMS)))
		{
			switch (ent->client->resp.team) 
			{
				case CTF_TEAM1:
					strcat(t, CTF_TEAM1_SKIN);
					break;
				case CTF_TEAM2:
					strcat(t, CTF_TEAM2_SKIN);
					break;
			}
			
			// add ctf skin to the model and send the config string
			ent->flags |= FL_VALID_MODEL;
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, t) );
//
			Info_SetValueForKey(ent->client->pers.userinfo, "skin", va("%s", t));
			return;
		}
		else
		{
			// as the model is valid for the class, use whatever skin
			ent->flags |= FL_VALID_MODEL;
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s));

//
			Info_SetValueForKey(ent->client->pers.userinfo, "skin", va("%s", s));
			return;
		}
	}
	else	// player selected an invalid model
	{
			// Set model

		switch (ent->client->resp.player_class)
		{
			case CLASS_WOLF:
				strcpy(t,WOLFGUY_MODEL);	
				break;
			case CLASS_DOOM:
				strcpy(t,DOOMGUY_MODEL);
				break;
			case CLASS_Q1:
				strcpy(t,QUAKEGUY_MODEL);
				break;
			case CLASS_Q2:
			default:
				strcpy(t, "male/");			//default to male when they select an invalid model
				break;
		}

		// Set skin name
		if(gen_ctf->value  || (gen_team->value && (int)genflags->value & GEN_REDBLUE_TEAMS))
		{
			switch (ent->client->resp.team) 
			{
				case CTF_TEAM1:
					strcat(t, CTF_TEAM1_SKIN);
					break;
				case CTF_TEAM2:
					strcat(t, CTF_TEAM2_SKIN);
					break;
			}
		}
		else
		{
			switch(ent->client->resp.player_class)
			{
				case CLASS_WOLF:
					strcat(t,WOLF_SKIN);
					break;
				case CLASS_DOOM:
					strcat(t,DOOM_SKIN);
					break;
				case CLASS_Q1:
					strcat(t,QUAKE_SKIN);
					break;
				case CLASS_Q2:
					strcat(t, "grunt");
					break;
			}
		}
		// send config string
		ent->flags &= ~FL_VALID_MODEL;
		gi.configstring (CS_PLAYERSKINS+playernum,va("%s\\%s", ent->client->pers.netname, t));
//
		Info_SetValueForKey(ent->client->pers.userinfo, "skin", va("%s", t));
	}
}

//================================================================================

/*===========================================
 Assigns Class if FORCE_JOIN is on
 Support Q1 and DOOM only modes
===========================================*/


void GenAssignClass(gclient_t *who)
{
	edict_t		*player;
	int i;
	int team1count = 0, 
		team2count = 0, 
		team3count = 0, 
		team4count = 0;
//	float team;

	who->resp.state = START_GEN;
		
	if((int)genflags->value & GEN_RANDOM_CLASS)
	{
		who->resp.player_next_class = GetRandomClass();
/*		team = random();
		if(team > 0.25)
		{
			who->resp.player_next_class = CLASS_Q1;
		}
		else if(team > 0.5)
		{
			who->resp.player_next_class = CLASS_DOOM;
		}
		else if(team > 0.75)
		{
			who->resp.player_next_class = CLASS_WOLF;
		}
		else
		{
			who->resp.player_next_class = CLASS_Q2;
		}
*/
		return;
	}
	else if (!((int)dmflags->value & DF_FORCEJOIN)) 
	{
		if((int)genflags->value & GEN_Q1_ONLY)
		{
			who->resp.player_next_class = CLASS_Q1;
		}
		else if((int)genflags->value & GEN_DOOM_ONLY)
		{
			who->resp.player_next_class = CLASS_DOOM;
		}
		else 
		{
			who->resp.player_class = NO_CLASS;
		}
		return;
	}

	for (i = 1; i <= maxclients->value; i++) 
	{
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.player_class) 
		{
			case CLASS_WOLF:
				team1count++;
				break;
			case CLASS_DOOM:
				team2count++;
				break;
			case CLASS_Q1:
				team3count++;
				break;
			case CLASS_Q2:
				team4count++;
				break;
		}
	}

	if ((team2count < team3count) && (team2count <team4count ) &&(team2count < team1count))
		who->resp.player_next_class = CLASS_DOOM;
	else if ((team1count < team2count) && (team1count <team3count ) && (team1count < team4count))
		who->resp.player_next_class = CLASS_WOLF;
	else if ((team3count < team2count) && (team3count < team4count) && (team3count < team1count))
		who->resp.player_next_class = CLASS_Q1;
	else if ((team4count < team2count) && (team4count <team3count ) && (team4count < team1count))
		who->resp.player_next_class = CLASS_Q2;
	else
	{

		who->resp.player_next_class = GetRandomClass();
/*		team= random();
		if (team < 0.3)
			who->resp.player_next_class = CLASS_Q2;
		else if (team < 0.6)
			who->resp.player_next_class = CLASS_Q1;
		else if (team < 0.8)
			who->resp.player_next_class = CLASS_DOOM;
		else
			who->resp.player_next_class = CLASS_WOLF;
*/
	}
}

//================================================================================

/*===================================
  Functions for Changing Class
===================================*/

void GenJoinClass(edict_t *ent, int desired_class);

void GenClassPicked(edict_t *ent, int desired_class)
{
	if (desired_class < CLASS_WOLF || desired_class > CLASS_Q2) 
		desired_class = 3;
	
	if (desired_class == ent->client->resp.player_class)
	{
		gi.cprintf(ent,PRINT_HIGH, "Already on the %s class\n", 
					GenClassName(ent->client->resp.player_class));
		return;
	}

	ent->client->resp.player_next_class = desired_class;
	
	if((gen_ctf->value || (gen_team->value && (int)genflags->value & GEN_REDBLUE_TEAMS))
		&& ent->client->resp.team == CTF_NOTEAM)
	{
		ent->client->resp.player_class = ent->client->resp.player_next_class;
		CTFOpenJoinMenu(ent);
	}
	else
	{
		if(ent->client->resp.player_class <= NO_CLASS)
		{
			GenJoinClass(ent, desired_class);
			return;
		}
		gi.cprintf (ent, PRINT_HIGH, "Class will be changed on Respawn\n");
	}
}

/*===================================
 Client Command, process message
===================================*/

void Gen_Class_f (edict_t *ent)
{
	char *t;
	int desired_class;

	t = gi.args();
	
	if (!*t) 
	{
		gi.cprintf(ent, PRINT_HIGH, "You are on the %s class\n",
			GenClassName(ent->client->resp.player_class));
		return;
	}

	if((int)genflags->value & GEN_Q1_ONLY || 
	   (int)genflags->value & GEN_DOOM_ONLY ||
	   (int)genflags->value & GEN_RANDOM_CLASS)
	{
		gi.cprintf(ent,PRINT_HIGH, "This Server doesnt allow class changing\n");
		return;
	}
	
	if ((Q_stricmp(t, "q1")==0) || (Q_stricmp(t,"quake")==0))
	{
		desired_class = CLASS_Q1;
		ent->client->random_class = false;
	}
	else if ((Q_stricmp(t, "q2")==0) || (Q_stricmp(t,"quake2")==0))
	{
		desired_class = CLASS_Q2;
		ent->client->random_class = false;
	}
	else if (Q_stricmp(t, "doom")==0)  
	{
		desired_class = CLASS_DOOM;
		ent->client->random_class = false;
	}
	else if ((Q_stricmp(t, "wolf3d")==0) || (Q_stricmp(t,"wolf")==0))
	{
		desired_class = CLASS_WOLF;
		ent->client->random_class = false;
	}
	else if((Q_stricmp(t, "random")==0))
	{
		ent->client->random_class = true;
		desired_class = GetRandomClass();
	}
	else 
	{
		gi.cprintf(ent, PRINT_HIGH, "Unknown class %s\n", t);
		return;
	}
	
	GenClassPicked(ent,desired_class);
}
    
//================================================================================
//================================================================================

/*===================================
 Called from PutClientInServer 
 to check if player has a class/team
 Shows Menu if not
===================================*/

//showloading !!!!!!!!

qboolean GenStartClient(edict_t *ent)
{
	if(gen_ctf->value || (gen_team->value && (int)genflags->value & GEN_REDBLUE_TEAMS)) 
	{
		if((ent->client->resp.team > CTF_NOTEAM) && 
		   (ent->client->resp.player_next_class > NO_CLASS))
			return false;
	}
	else if (ent->client->resp.player_next_class > NO_CLASS)
		return false;
	
	if (deathmatch->value)
	{
		if (!((int)dmflags->value & DF_FORCEJOIN)) 
		{
			// start as 'observer'
			ent->movetype = MOVETYPE_NOCLIP;
			ent->solid = SOLID_NOT;
			ent->svflags |= SVF_NOCLIENT;
			ent->client->resp.player_class = NO_CLASS;
			ent->client->resp.team = CTF_NOTEAM;
			ent->client->ps.gunindex = 0;
			gi.linkentity (ent);
			
			if(gen_ctf->value || (gen_team->value && (int)genflags->value & GEN_REDBLUE_TEAMS))
				GenOpenJoinClassMenu(ent);
			else			
				GenOpenJoinMenu(ent);
			return true;
		}
		return false;
	}
//  add SP SUPPORT - Skid		
	else if (gen_sp->value || coop->value)
	{
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.player_class = NO_CLASS;
		ent->client->resp.team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		ent->flags |= FL_NOTARGET;
		gi.linkentity (ent);
		GenOpenJoinMenu(ent);
		return true;
	}
	return false;
}

//================================================================================

/*===================================
 Check for Map changing conditions
===================================*/

qboolean GenCheckRBCTFRules(void)
{
	if(limit->value)
	{
		if (gengame.team1 >= limit->value)
		{
			gi.bprintf (PRINT_HIGH, "Red Team has won the game.\n");
			return true;
		}
		else if(gengame.team2 >= limit->value)
		{
			gi.bprintf (PRINT_HIGH, "Blue Team has won the game.\n");
			return true;
		}
	}
	return false;
}

qboolean GenCheckTeamRules(void)
{
	if(limit->value)
	{
		if (gengame.team1 >= limit->value)
		{
			gi.bprintf (PRINT_HIGH, "Wolfguys have flamed the game.\n");
			return true;
		}
		else if(gengame.team2 >= limit->value)
		{
			gi.bprintf (PRINT_HIGH, "Doomguys reload their Super Shotguns.\n");
			return true;
		}
		else if(gengame.team3 >= limit->value)
		{
			gi.bprintf(PRINT_HIGH, "Quakeguys rule with their Rocket Launchers.\n");
			return true;
		}
		else if(gengame.team4 >= limit->value)
		{
			gi.bprintf(PRINT_HIGH, "Quake2 rails the opposition.\n");
			return true;
		}
	}
	return false;
}



//================================================================================
//================================================================================

edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);

/*
================
SelectGenSpawnPoint

go to a Gen point, but NOT the two points closest
to other players
================
*/

edict_t *SelectGenSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	if ((ent->client->resp.state == PLAYING_GEN) && 
		!((int)genflags->value & GEN_CTF_BASE_RESPAWN))
	{		
		if ((int)dmflags->value & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();
	}
	
	ent->client->resp.state = PLAYING_GEN;
	switch (ent->client->resp.team) 
	{
		case CTF_TEAM1:
			cname = "info_player_team1";
			break;
		case CTF_TEAM2:
			cname = "info_player_team2";
			break;
		default:
			return SelectRandomDeathmatchSpawnPoint();
	}
	
	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return SelectRandomDeathmatchSpawnPoint();

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}




/*-----------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
