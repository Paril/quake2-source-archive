#include "g_local.h"
#include "g_sogutil.h"

generations_game	gengame;

// model paths
#define WOLFGUY_MODEL	"idg1/"		
#define DOOMGUY_MODEL	"idg2/"
#define QUAKEGUY_MODEL	"idg3/"
#define WOLF_SKIN		"base"		
#define DOOM_SKIN		"green"
#define QUAKE_SKIN		"base"

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
	if(deathmatch->value)
	{
		if(sog_ctf->value)
		{
			gi.cvar_forceset("sog_team", "0");
			gi.dprintf("==== Init SOGCTF ====\n");
		}
		else if(sog_team->value)
		{
			gi.cvar_forceset("sog_ctf", "0");
			gi.dprintf("==== Init SOG TeamPlay ====\n");
			if((int)sogflags->value & SOG_REDBLUE_TEAMS)
				gi.dprintf("=== Red vs Blue Teams ===\n");
			else
				gi.dprintf("=== Class Based Teams ===\n");
		}
		GenTryCTF();
	}

	//max speed
	gi.cvar_forceset("cl_forwardspeed", "200");
	gi.cvar_forceset("cl_sidespeed", "200");
	
	memset(&gengame, 0, sizeof(gengame));

	music_init();

}

//================================================================================

char *GenClassName(int player_class)
{
	switch (player_class) 
	{
		case CLASS_WOLF:
			return "Blaze";
		case CLASS_DOOM:
			return "Flynn";
		case CLASS_Q1:
			return "Axe";
		case CLASS_Q2:
			return "Sgt. Payne";
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
				if(memcmp(t,QUAKEGUY_MODEL,5)==0)
					return true;
				break;
			}
		case CLASS_DOOM:
			{
				if(memcmp(t,DOOMGUY_MODEL,5)==0)
					return true;
				break;
			}
		case CLASS_WOLF:
			{
				if(memcmp(t,WOLFGUY_MODEL,5)==0)
					return true;
				break;
			}
		case CLASS_Q2:
		default:
			{
			  if((memcmp(t,QUAKEGUY_MODEL,5)==0) ||      
			     (memcmp(t,DOOMGUY_MODEL,5)==0)	 ||
				 (memcmp(t,WOLFGUY_MODEL,5)==0))
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
	if(!(sog_ctf->value || 
		(sog_team->value && ((int)sogflags->value & SOG_REDBLUE_TEAMS)))
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
		if(sog_ctf->value || 
		  (sog_team->value && ((int)sogflags->value & SOG_REDBLUE_TEAMS)))
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
		if(sog_ctf->value  || (sog_team->value && (int)sogflags->value & SOG_REDBLUE_TEAMS))
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

	if (!((int)dmflags->value & DF_FORCEJOIN)) 
	{
		if((int)sogflags->value & SOG_IDG3_ONLY)
		{
			who->resp.player_next_class = CLASS_Q1;
		}
		else if((int)sogflags->value & SOG_IDG2_ONLY)
		{
			who->resp.player_next_class = CLASS_DOOM;
		}
		else if((int)sogflags->value & SOG_IDG1_ONLY)
		{
			who->resp.player_next_class = CLASS_WOLF;
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
		safe_cprintf(ent,PRINT_HIGH, "Already on the %s class\n", 
					GenClassName(ent->client->resp.player_class));
		return;
	}

	ent->client->resp.player_next_class = desired_class;
	
	if((sog_ctf->value || (sog_team->value && (int)sogflags->value & SOG_REDBLUE_TEAMS))
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
		safe_cprintf (ent, PRINT_HIGH, "Class will be changed on Respawn\n");
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
		safe_cprintf(ent, PRINT_HIGH, "You are on the %s class\n",
			GenClassName(ent->client->resp.player_class));
		return;
	}

	if((int)sogflags->value & SOG_IDG3_ONLY || 
	   (int)sogflags->value & SOG_IDG2_ONLY ||
	   (int)sogflags->value & SOG_IDG1_ONLY)
	{
		safe_cprintf(ent,PRINT_HIGH, "This Server doesnt allow class changing\n");
		return;
	}
	
	if (Q_stricmp(t, "axe")==0)
	{
		desired_class = CLASS_Q1;
		ent->client->random_class = false;
	}
	else if (Q_stricmp(t, "payne")==0)
	{
		desired_class = CLASS_Q2;
		ent->client->random_class = false;
	}
	else if (Q_stricmp(t, "flynn")==0)
	{
		desired_class = CLASS_DOOM;
		ent->client->random_class = false;
	}
	else if (Q_stricmp(t, "blaze")==0)
	{
		desired_class = CLASS_WOLF;
		ent->client->random_class = false;
	}
	else if (Q_stricmp(t, "random")==0)
	{
		ent->client->random_class = true;
		desired_class = GetRandomClass();
	}
	else 
	{
		safe_cprintf(ent, PRINT_HIGH, "Unknown class %s\n", t);
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
	if(sog_ctf->value || (sog_team->value && (int)sogflags->value & SOG_REDBLUE_TEAMS)) 
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
			
			if(sog_ctf->value || (sog_team->value && (int)sogflags->value & SOG_REDBLUE_TEAMS))
				GenOpenJoinClassMenu(ent);
			else			
				GenOpenJoinMenu(ent);
			return true;
		}
		return false;
	}
//  add SP SUPPORT - Skid		
	else if (!deathmatch->value)
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

// Arno added

/*
===================================
 Calculate Team Scores

 Since updating the teamscores at every
 event isn't sufficient (players going
 to spectator-mode are ignored), I've chosen
 to recalculate the teamscores at every frame.
===================================
*/

void SOGCalculateTeamScores(void)
{
	int i;
	edict_t		*cl_ent;
	
	if (!sog_ctf->value && !sog_team->value)
		return;
		   	
	gengame.team1 = gengame.team2 = gengame.team3 = gengame.team4 = 0;

	if (((int)sogflags->value & SOG_REDBLUE_TEAMS) || sog_ctf->value)
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
		
			if (!cl_ent->inuse)
				continue;

			if (cl_ent->client->resp.spectator)
				continue;
		
			if (game.clients[i].resp.team == CTF_TEAM1)
				gengame.team1 += game.clients[i].resp.score;
			else if (game.clients[i].resp.team == CTF_TEAM2)
				gengame.team2 += game.clients[i].resp.score;
			else
				continue; // unknown team?

		}
	}
	else   // Class based Teamplay
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
		
			if (!cl_ent->inuse)
				continue;
			
			if (cl_ent->client->resp.spectator)
				continue;

			if (game.clients[i].resp.player_class == CLASS_WOLF)
		  		gengame.team1 += game.clients[i].resp.score;
			else if (game.clients[i].resp.player_class == CLASS_DOOM)
		  		gengame.team2 += game.clients[i].resp.score;
			else if (game.clients[i].resp.player_class == CLASS_Q1)
		  		gengame.team3 += game.clients[i].resp.score;
			else if (game.clients[i].resp.player_class == CLASS_Q2)
	  			gengame.team4 += game.clients[i].resp.score;
	  		else
	  			continue;  // unknown team		  	
	  	}
	}

}

// end Arno


/*===================================
 Check for Map changing conditions
===================================*/

qboolean GenCheckRBCTFRules(void)
{
	if (sog_team->value && ((int)sogflags->value & SOG_REDBLUE_TEAMS))
	{
		if(limit->value)
		{
			if (gengame.team1 >= limit->value)
			{
				safe_bprintf (PRINT_HIGH, "Red Team has won the game.\n");
				return true;
			}
			else if(gengame.team2 >= limit->value)
			{
				safe_bprintf (PRINT_HIGH, "Blue Team has won the game.\n");
				return true;
			}
		}
		return false;
	}
	else  // CTF
	{
		if(limit->value)
		{
			if (gengame.total1 >= limit->value)
			{
				safe_bprintf (PRINT_HIGH, "Red Team has won the game.\n");
				return true;
			}
			else if(gengame.total2 >= limit->value)
			{
				safe_bprintf (PRINT_HIGH, "Blue Team has won the game.\n");
				return true;
			}
		}
		return false;	
	}
}

qboolean GenCheckTeamRules(void)
{
	if(limit->value)
	{
		if (gengame.team1 >= limit->value)
		{
			safe_bprintf (PRINT_HIGH, "Team Blaze has flamed the game.\n");
			return true;
		}
		else if(gengame.team2 >= limit->value)
		{
			safe_bprintf (PRINT_HIGH, "Team Flynn reloads its Super Shotguns.\n");
			return true;
		}
		else if(gengame.team3 >= limit->value)
		{
			safe_bprintf(PRINT_HIGH, "Team Axe rules with its Rocket Launchers.\n");
			return true;
		}
		else if(gengame.team4 >= limit->value)
		{
			safe_bprintf(PRINT_HIGH, "Team Sgt. Payne rails the opposition.\n");
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
		!((int)sogflags->value & SOG_CTF_BASE_RESPAWN))
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
