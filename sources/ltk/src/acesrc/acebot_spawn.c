///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  Original file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
////////////////////////////////////////////////////////////////////////
/*
 * $Header: /LicenseToKill/src/acesrc/acebot_spawn.c 29    16/10/99 18:59 Riever $
 *
 * $Log: /LicenseToKill/src/acesrc/acebot_spawn.c $
 * 
 * 29    16/10/99 18:59 Riever
 * Fixed bug in WeaponChoice code.
 * 
 * 28    16/10/99 14:03 Riever
 * Added favourite weapon and equipment to config file.
 * 
 * 27    16/10/99 10:37 Riever
 * Fixed a mis-type in a skin name.
 * 
 * 26    16/10/99 9:45 Riever
 * New config files now working...
 * 
 * 25    16/10/99 9:15 Riever
 * Changed to use new bot config data.
 * Removed bot saving code.
 * Enabled configs per level.
 * 
 * 24    16/10/99 8:59 Riever
 * New bot config file code added.
 * ltk_chat CVAR enabled to turn off botchat.
 * 
 * 23    10/10/99 14:46 Riever
 * Changed removebot call to use ClientDisconnect.
 * 
 * 22    6/10/99 20:19 Riever
 * Fixed bugs in Teamplay spawning so teams always balance out.
 * 
 * 21    6/10/99 18:33 Riever
 * Random weapon choice for Teamplay now working.
 * 
 * 20    6/10/99 17:40 Riever
 * Added TeamPlay state STATE_POSITION to enable bots to seperate and
 * avoid centipede formations.
 * 
 * 19    5/10/99 20:09 Riever
 * Changes to let bots load the route file and load the bots in a
 * dedicated server after the level has changed.
 * 
 * 18    3/10/99 21:43 Riever
 * Re-enabled bot naming...
 * 
 * 17    29/09/99 17:42 Riever
 * Random switch chooses between standard or Sindarin names.
 * 
 * 16    29/09/99 16:52 Riever
 * Set the name switch to show that a particular name is used.
 * Added extra name pairs from Sindarin.
 * 
 * 15    29/09/99 8:45 Riever
 * lastkilledby is now used to insult the person who killed the bot.
 * 
 * 14    29/09/99 8:08 Riever
 * Modified killed message code to find last enemy
 * 
 * 13    29/09/99 7:53 Riever
 * Changed respawn insults to use last attacker.
 * 
 * 12    29/09/99 7:47 Riever
 * Comment added
 * 
 * 11    29/09/99 7:37 Riever
 * Increased max available names to 100
 * 
 * 10    29/09/99 7:21 Riever
 * Added first version of LTKsetBotName to get random bot names that look
 * different to eachother.
 * 
 * 9     28/09/99 7:38 Riever
 * Reduced frequency of respawn chats.
 * 
 * 8     28/09/99 7:37 Riever
 * LTK_Chat method added and utilised by bots on spawn and respawn.
 * Need to get insults in for killing opponents and then work on graphical
 * taunts.
 * 
 * 7     27/09/99 13:22 Riever
 * Initialise pathList on spawn
 * 
 * 6     21/09/99 21:15 Riever
 * Random angle facing added for  TeamPlay mode starts.
 * 
 * 5     21/09/99 17:20 Riever
 * Temporary centipede fix achieved by having teamPauseTime set forward
 * and not allowing goal following until this time is reached.
 * 
 * 4     21/09/99 7:56 Riever
 * Fixed Team spawning code so that it automatically selects the best team
 * for a spawned bot to join. This works with the bots.tmp file too!
 * 
 * 3     16/09/99 7:40 Riever
 * Changed a bprint I missed to a safeprint
 * 
 * 2     13/09/99 19:52 Riever
 * Added headers
 *
 */

///////////////////////////////////////////////////////////////////////
//
//  acebot_spawn.c - This file contains all of the 
//                   spawing support routines for the ACE bot.
//
///////////////////////////////////////////////////////////////////////

#include "g_local.h"
#include "m_player.h"
#include "acebot.h"
#include "botchat.h"
#include "botscan.h"

//AQ2 ADD
#define	CONFIG_FILE_VERSION 1

void	AllItems( edict_t *ent );
void	AllWeapons( edict_t *ent );
void	EquipClient( edict_t *ent );
char	*TeamName(int team);
void	LTKsetBotName( char	*bot_name );
void	ACEAI_Cmd_Choose( edict_t *ent, char *s);

//==========================================
// Joining a team (hacked from AQ2 code)
//==========================================
/*	defined in a_team.h
#define NOTEAM          0
#define TEAM1           1
#define TEAM2           2
*/

//==============================
// Get the number of the next team a bot should join
//==============================
int GetNextTeamNumber()
{
        int i, onteam1 = 0, onteam2 = 0;
        edict_t *e;

        // only use this function during [2]team games...
        if (!teamplay->value )
                return 0;

//		gi.bprintf(PRINT_HIGH, "Checking team balance..\n");
        for (i = 1; i < maxclients->value+1; i++)
        {
                e = g_edicts + i;
                if (e->inuse)
                {
                    if (e->client->resp.team == TEAM1)
                        onteam1++;
                    else if (e->client->resp.team == TEAM2)
						onteam2++;
			    }  
        }
		// Return the team number that needs the next bot
        if (onteam1 > onteam2)
			return (2);
        else if (onteam2 >= onteam1)
			return (1);
		//default
		return (1);
}

//==========================
// Join a Team
//==========================
void ACESP_JoinTeam(edict_t *ent, int desired_team)
{
        char *s, *a;
  

        if (ent->client->resp.team == desired_team)
                return;

        a = (ent->client->resp.team == NOTEAM) ? "joined" : "changed to";

        ent->client->resp.team = desired_team;
        s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
        AssignSkin(ent, s);

        if (ent->solid != SOLID_NOT)  // alive, in game
        {
                ent->health = 0;
                player_die (ent, ent, ent, 100000, vec3_origin);
                ent->deadflag = DEAD_DEAD;
        }

        safe_bprintf(PRINT_HIGH, "%s %s %s.\n",
                        ent->client->pers.netname, a, TeamName(desired_team));

        ent->client->resp.joined_team = level.framenum;

        CheckForUnevenTeams();
}

//======================================
// ACESP_LoadBotConfig()
//======================================
// Using RiEvEr's new config file
//
void ACESP_LoadBotConfig()
{
    FILE	*pIn;
	cvar_t	*game_dir;
	int		i;
	char	filename[60];
	// Scanner stuff
	int		fileVersion = 0;
	char	inString[81];
	char	tokenString[81];
	char	*sp, *tp;
	int		ttype;

	game_dir = gi.cvar ("game", "", 0);

	// Try to load the file for THIS level
#ifdef	_WIN32
	i =  sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\bots\\");
	i += sprintf(filename + i, level.mapname);
	i += sprintf(filename + i, ".cfg");
#else
#ifndef AMIGA
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
#else
	strcpy(filename, game_dir->string);
#endif
	strcat(filename, "/bots/");
	strcat(filename, level.mapname);
	strcat(filename,".cfg");
#endif

	// If there's no specific file for this level then get the normal one
	if((pIn = fopen(filename, "rb" )) == NULL)
	{
#ifdef	_WIN32
		i =  sprintf(filename, ".\\");
		i += sprintf(filename + i, game_dir->string);
		i += sprintf(filename + i, "\\bots\\botdata.cfg");
#else
#ifndef AMIGA
		strcpy(filename, "./");
		strcat(filename, game_dir->string);
#else
		strcpy(filename, game_dir->string);
#endif
		strcat(filename,"/bots/botdata.cfg");
#endif

		// No bot file available, get out of here!
		if((pIn = fopen(filename, "rb" )) == NULL)
			return; // bail
	}

	// Now scan each line for information
	// First line should be the file version number
	fgets( inString, 80, pIn );
	sp = inString;
	tp = tokenString;
	ttype = UNDEF;

	// Scan it for the version number
	scanner( &sp, tp, &ttype );
	if(ttype == BANG)
	{
		scanner( &sp, tp, &ttype );
		if(ttype == INTLIT)
		{
			fileVersion = atoi( tokenString );
		}
		if( fileVersion != CONFIG_FILE_VERSION )
		{
			// ERROR!
			safe_bprintf(PRINT_HIGH, "Bot Config file is out of date!\n");
			fclose(pIn);
			return;
		}
	}

	// Now process each line of the config file
	while( fgets(inString, 80, pIn) )
	{
		ACESP_SpawnBotFromConfig( inString );
	}

/*	fread(&count,sizeof (int),1,pIn); 

	for(i=0;i<count;i++)
	{
		fread(userinfo,sizeof(char) * MAX_INFO_STRING,1,pIn); 
		ACESP_SpawnBot (NULL, NULL, NULL, userinfo);
	}*/
		
    fclose(pIn);
}

//===========================
// ACESP_SpawnBotFromConfig
//===========================
// Input string is from the config file and should have
// all the data we need to spawn a bot
//
void	ACESP_SpawnBotFromConfig( char *inString )
{
	edict_t	*bot;
	char	userinfo[MAX_INFO_STRING];
	int		count=1;
	char	name[32];
	char	modelskin[80];
	int		team=0, weaponchoice=0, equipchoice=0;

	// Scanner stuff
	char	tokenString[81];
	char	*sp, *tp;
	int		ttype;

	sp = inString;
	ttype = UNDEF;

	while( ttype != EOL )
	{
		tp = tokenString;
		scanner(&sp, tp, &ttype);

		// Check for comments
		if( ttype == HASH || ttype == LEXERR)
			return;

		// Check for semicolon (end of input marker)
		if( ttype == SEMIC || ttype == EOL)
			continue;

		// Keep track of which parameter we are reading
		if( ttype == COMMA )
		{
			count++;
			continue;
		}

		// NAME (parameter 1)
		if(count == 1 && ttype == STRLIT)
		{
//			strncpy( name, tokenString, 32 );
			strcpy( name, tokenString);
			continue;
		}

		// MODELSKIN (parameter 2)
		if(count == 2 && ttype == STRLIT)
		{
//			strncpy( modelskin, tokenString, 32 );
			strcpy( modelskin, tokenString);
			continue;
		}

		if(count == 3 && ttype == INTLIT )
		{
			team = atoi(tokenString);
			continue;
		}

		if(count == 4 && ttype == INTLIT )
		{
			weaponchoice = atoi(tokenString);
			continue;
		}

		if(count == 5 && ttype == INTLIT )
		{
			equipchoice = atoi(tokenString);
			continue;
		}

	}// End while
	
	bot = ACESP_FindFreeClient ();
	
	if (!bot)
	{
		safe_bprintf (PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	// To allow bots to respawn
	// initialise userinfo
	memset (userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey (userinfo, "name", name);
	Info_SetValueForKey (userinfo, "skin", modelskin);
	Info_SetValueForKey (userinfo, "hand", "2"); // bot is center handed for now!
	Info_SetValueForKey (userinfo, "spectator", "0"); // NOT a spectator

	ClientConnect (bot, userinfo);
	
	G_InitEdict (bot);

	// locate ent at a spawn point
    if(teamplay->value)
	{
		// Make sure we have a team
		if(!team)
			team = GetNextTeamNumber();
	}

	// Set up the preferred weapon & equipment
	bot->weaponchoice = weaponchoice;
	bot->equipchoice = equipchoice;

	InitClientResp (bot->client);
	
    if(teamplay->value)
	{
		ACESP_PutClientInServer (bot,true, team);
	}
	else
 		ACESP_PutClientInServer (bot,true,0);


	// make sure all view stuff is valid
	ClientEndServerFrame (bot);
	
	ACEIT_PlayerAdded (bot); // let the world know we added another

	ACEAI_PickLongRangeGoal(bot); // pick a new goal

	// LTK chat stuff
	if( random() < 0.33)
	{
		// Store current enemies available
		int		i, counter = 0;
		edict_t *myplayer[MAX_BOTS];

		for(i=0;i<=num_players;i++)
		{
			// Find all available enemies to insult
			if(players[i] == NULL || players[i] == bot || 
			   players[i]->solid == SOLID_NOT)
			   continue;
		
			if(teamplay->value && OnSameTeam( bot, players[i]) )
			   continue;
			myplayer[counter++] = players[i];
		}
		if(counter > 0)
		{
			// Say something insulting to them!
			if(ltk_chat->value)	// Some people don't want this *sigh*
				LTK_Chat( bot, myplayer[rand()%counter], DBC_WELCOME);
		}
	}	
}
//AQ2 END


///////////////////////////////////////////////////////////////////////
// Called by PutClient in Server to actually release the bot into the game
// Keep from killin' each other when all spawned at once
///////////////////////////////////////////////////////////////////////
void ACESP_HoldSpawn(edict_t *self)
{
	if (!KillBox (self))
	{	// could't spawn in?
	}

	gi.linkentity (self);

	self->think = ACEAI_Think;
	self->nextthink = level.time + FRAMETIME;

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

/*	if(ctf->value)
	safe_bprintf(PRINT_MEDIUM, "%s joined the %s team.\n",
		self->client->pers.netname, CTFTeamName(self->client->resp.ctf_team));
	else*/
		safe_bprintf (PRINT_MEDIUM, "%s entered the game\n", self->client->pers.netname);

}

///////////////////////////////////////////////////////////////////////
// Modified version of id's code
///////////////////////////////////////////////////////////////////////
void ACESP_PutClientInServer (edict_t *bot, qboolean respawn, int team)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i, counter;
	client_persistant_t	saved;
	client_respawn_t	resp;
	char *s;
    int     going_observer;//AQ2
	
	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (bot, spawn_origin, spawn_angles);
	
	index = bot-g_edicts-1;
	client = bot->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char userinfo[MAX_INFO_STRING];

		resp = bot->client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (bot, userinfo);
	}
	else
		memset (&resp, 0, sizeof(resp));
	
	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	client->resp = resp;
	
	// copy some data from the client to the entity
	FetchClientEntData (bot);
	
	// clear entity values
	bot->groundentity = NULL;
	bot->client = &game.clients[index];
	bot->takedamage = DAMAGE_AIM;
	bot->movetype = MOVETYPE_WALK;
	bot->viewheight = 24;
	bot->classname = "bot";
	bot->mass = 200;
	bot->solid = SOLID_BBOX;
	bot->deadflag = DEAD_NO;
	bot->air_finished = level.time + 12;
	bot->clipmask = MASK_PLAYERSOLID;
	bot->model = "players/male/tris.md2";
	bot->pain = player_pain;
	bot->die = player_die;
	bot->waterlevel = 0;
	bot->watertype = 0;
	bot->flags &= ~FL_NO_KNOCKBACK;
	bot->svflags &= ~SVF_DEADMONSTER;
	bot->is_jumping = false;
	
//AQ2 ADD
        if (!teamplay->value || bot->client->resp.team != NOTEAM)
        {
                bot->flags &= ~FL_GODMODE;
                bot->svflags &= ~SVF_NOCLIENT;
        }

//AQ2 END

	VectorCopy (mins, bot->mins);
	VectorCopy (maxs, bot->maxs);
	VectorClear (bot->velocity);

	// clear playerstate values
	memset (&bot->client->ps, 0, sizeof(client->ps));
	
	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

//ZOID
//AQ2	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	bot->s.effects = 0;
	bot->s.skinnum = bot - g_edicts - 1;
	bot->s.modelindex = 255;		// will use the skin specified model
//AQ2	bot->s.modelindex2 = 255;		// custom gun model
//AQ2 ADD
        ShowGun(bot);
//AQ2 END
	bot->s.frame = 0;
	VectorCopy (spawn_origin, bot->s.origin);
	bot->s.origin[2] += 1;	// make sure off ground
    VectorCopy (bot->s.origin, bot->s.old_origin);//AQ2

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	bot->s.angles[PITCH] = 0;
	bot->s.angles[YAW] = spawn_angles[YAW];
	bot->s.angles[ROLL] = 0;
	VectorCopy (bot->s.angles, client->ps.viewangles);
	VectorCopy (bot->s.angles, client->v_angle);
	
	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (bot);

	bot->enemy = NULL;
	bot->movetarget = NULL; 
	if( !teamplay->value)
		bot->state = STATE_MOVE;
	else
		bot->state = STATE_POSITION;

	// Set the current node
	bot->current_node = ACEND_FindClosestReachableNode(bot,NODE_DENSITY, NODE_ALL);
	bot->goal_node = bot->current_node;
	bot->next_node = bot->current_node;
	bot->next_move_time = level.time;		
	bot->suicide_timeout = level.time + 15.0;

	// If we are not respawning hold off for up to three seconds before releasing into game
    if(!respawn)
	{
		bot->think = ACESP_HoldSpawn;
		bot->nextthink = level.time + 0.1;
		bot->nextthink = level.time + random()*3.0; // up to three seconds
	}
	else
	{
//AQ2 ADD
		//@@ check this out!
		// Sets to an observer unless joined a team! (ent->client->resp.team != NOTEAM)
        if (teamplay->value)
        {
			going_observer = StartClient(bot);
        }
		else
        {
                going_observer = bot->client->pers.spectator;
                if (going_observer)
                {
                        bot->movetype = MOVETYPE_NOCLIP;
                        bot->solid = SOLID_NOT;
                        bot->svflags |= SVF_NOCLIENT;
                        bot->client->resp.team = NOTEAM;
                        bot->client->ps.gunindex = 0; 
                }
        }
//FIREBLADE
        if (!going_observer && !teamplay->value)
        { // this handles telefrags...
                KillBox(bot);
        }
//FIREBLADE

		gi.linkentity (bot);

		bot->think = ACEAI_Think;
		bot->nextthink = level.time + FRAMETIME;

//AQ2 ADD
        client->mk23_max = 12;
        client->mp5_max = 30;
        client->m4_max = 24;
        client->shot_max = 7;
        client->sniper_max = 6;
        client->cannon_max = 2;
        client->dual_max = 24;
        client->mk23_rds = client->mk23_max;
        client->dual_rds = client->mk23_max;
        client->knife_max = 10;
        client->grenade_max = 2;        

        bot->lasersight = NULL;

        //other
        client->bandaging = 0;
        client->leg_damage = 0;
        client->leg_noise = 0;
        client->leg_dam_count = 0;
        client->desired_fov = 90;
        client->ps.fov = 90;
        client->idle_weapon = 0;
        client->drop_knife = 0;
        client->no_sniper_display = 0;
        client->knife_sound     = 0;
        client->doortoggle = 0;
        client->have_laser = 0; 

		// Choose Teamplay weapon
		if( bot->weaponchoice == 0)
			counter = rand() % 5;
		else
			counter = bot->weaponchoice -1; // Range is 1..5

		switch(counter)
		{
		case 0:
			ACEAI_Cmd_Choose( bot, MP5_NAME);
			break;
		case 1:
			ACEAI_Cmd_Choose( bot, M4_NAME);
			break;
		case 2:
			ACEAI_Cmd_Choose( bot, M3_NAME);
			break;
		case 3:
			ACEAI_Cmd_Choose( bot, HC_NAME);
			break;
		case 4:
			ACEAI_Cmd_Choose( bot, SNIPER_NAME);
			break;
		default:
			ACEAI_Cmd_Choose( bot, M3_NAME);
			break;
		}

		// Choose Teamplay equipment
		if(bot->equipchoice == 0)
			counter = rand() % 5;
		else
			counter = bot->equipchoice - 1; // Range is 1..5

		switch(counter)
		{
		case 0:
			ACEAI_Cmd_Choose( bot, SIL_NAME);
			break;
		case 1:
			ACEAI_Cmd_Choose( bot, SLIP_NAME);
			break;
		case 2:
			ACEAI_Cmd_Choose( bot, BAND_NAME);
			break;
		case 3:
			ACEAI_Cmd_Choose( bot, KEV_NAME);
			break;
		case 4:
			ACEAI_Cmd_Choose( bot, LASER_NAME);
			break;
		default:
			ACEAI_Cmd_Choose( bot, KEV_NAME);
			break;
		}

//FIREBLADE
        if (!going_observer)    
        {
                        // items up here so that the bandolier will change equipclient below
                        if ( allitem->value )
                        {
                                AllItems( bot );
                        }
                        
                        
                        if (teamplay->value)
                                EquipClient(bot);
                        
                        if (bot->client->menu)
                        {
                                PMenu_Close(bot);
                                return;
                        }
//FIREBLADE
                        if ( allweapon->value )
                        {
                                AllWeapons( bot );
                        }
                        
                        // force the current weapon up
                        client->newweapon = client->pers.weapon;
                ChangeWeapon (bot);

//FIREBLADE
                if (teamplay->value)
                {
                        bot->solid = SOLID_TRIGGER;
                        gi.linkentity(bot);
                }
//FIREBLADE
        }
	if(teamplay->value)
	{
		int randomnode;
		bot->client->resp.team = team;
		s = Info_ValueForKey (bot->client->pers.userinfo, "skin");
		AssignSkin(bot, s);
		// Anti centipede timer
		bot->teamPauseTime = level.time + 3.0 + (rand() % 7);
		// Change facing angle for each bot
		randomnode = (int)(num_players * random() );
		VectorSubtract (nodes[randomnode].origin, bot->s.origin, bot->move_vector);
		bot->move_vector[2] = 0;
	}
	else
		bot->teamPauseTime = level.time;

//AQ2 END

	//RiEvEr - new node pathing system
	memset(&bot->pathList, 0, sizeof(bot->pathList) );
	bot->pathList.head = bot->pathList.tail = NULL;
	//R
		/*
//AQ2 REMOVED
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (bot-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (bot->s.origin, MULTICAST_PVS);
//AQ2 END
		*/
	}
	
}

///////////////////////////////////////////////////////////////////////
// Respawn the bot
///////////////////////////////////////////////////////////////////////
void ACESP_Respawn (edict_t *self)
{
// AQ2 CHANGED
	if (self->solid != SOLID_NOT || self->deadflag == DEAD_DEAD)
	{
		CopyToBodyQue (self);
	}

	if(teamplay->value)
		ACESP_PutClientInServer (self,true, self->client->resp.team);
	else
		ACESP_PutClientInServer (self,true,0);

	self->svflags &= ~SVF_NOCLIENT;
//AQ2 END

	// add a teleportation effect
//AQ2	self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
//AQ2	self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
//AQ2	self->client->ps.pmove.pm_time = 14;

	self->client->respawn_time = level.time;

	if( random() < 0.15)
	{
		// Store current enemies available
		int		i, counter = 0;
		edict_t *myplayer[MAX_BOTS];

		if( self->lastkilledby)
		{
			// Have a comeback line!
			if(ltk_chat->value)	// Some people don't want this *sigh*
				LTK_Chat( self, self->lastkilledby, DBC_KILLED);
			self->lastkilledby = NULL;
		}
		else
		{
			// Pick someone at random to insult
			for(i=0;i<=num_players;i++)
			{
				// Find all available enemies to insult
				if(players[i] == NULL || players[i] == self || 
				   players[i]->solid == SOLID_NOT)
				   continue;
			
				if(teamplay->value && OnSameTeam( self, players[i]) )
				   continue;
				myplayer[counter++] = players[i];
			}
			if(counter > 0)
			{
				// Say something insulting to them!
				if(ltk_chat->value)	// Some people don't want this *sigh*
					LTK_Chat( self, myplayer[rand()%counter], DBC_INSULT);
			}
		}
	}	
}

///////////////////////////////////////////////////////////////////////
// Find a free client spot
///////////////////////////////////////////////////////////////////////
edict_t *ACESP_FindFreeClient (void)
{
	edict_t *bot;
	int	i;
	int max_count=0;
	
	// This is for the naming of the bots
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;
		
		if(bot->count > max_count)
			max_count = bot->count;
	}

	// Check for free spot
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (!bot->inuse)
			break;
	}

	bot->count = max_count + 1; // Will become bot name...

	if (bot->inuse)
		bot = NULL;
	
	return bot;
}

///////////////////////////////////////////////////////////////////////
// Set the name of the bot and update the userinfo
///////////////////////////////////////////////////////////////////////
void ACESP_SetName(edict_t *bot, char *name, char *skin, char *team)
{
	float rnd;
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];

	// Set the name for the bot.
	// name
	if(strlen(name) == 0)
	{
		// RiEvEr - new code to get random bot names
		LTKsetBotName(bot_name);
	}
	else
		strcpy(bot_name,name);

	// skin
	if(strlen(skin) == 0)
	{
		// randomly choose skin 
		rnd = random();
		if(rnd  < 0.05)
			sprintf(bot_skin,"male/bluebeard");
		else if(rnd < 0.1)
			sprintf(bot_skin,"female/brianna");
		else if(rnd < 0.15)
			sprintf(bot_skin,"male/blues");
		else if(rnd < 0.2)
			sprintf(bot_skin,"female/ensign");
		else if(rnd < 0.25)
			sprintf(bot_skin,"female/jezebel");
		else if(rnd < 0.3)
			sprintf(bot_skin,"female/jungle");
		else if(rnd < 0.35)
			sprintf(bot_skin,"sas/sasurban");
		else if(rnd < 0.4)
			sprintf(bot_skin,"terror/urbanterr");
		else if(rnd < 0.45)
			sprintf(bot_skin,"female/venus");
		else if(rnd < 0.5)
			sprintf(bot_skin,"sydney/sydney");
		else if(rnd < 0.55)
			sprintf(bot_skin,"male/cajin");
		else if(rnd < 0.6)
			sprintf(bot_skin,"male/commando");
		else if(rnd < 0.65)
			sprintf(bot_skin,"male/grunt");
		else if(rnd < 0.7)
			sprintf(bot_skin,"male/mclaine");
		else if(rnd < 0.75)
			sprintf(bot_skin,"male/robber");
		else if(rnd < 0.8)
			sprintf(bot_skin,"male/snowcamo");
		else if(rnd < 0.85)
			sprintf(bot_skin,"terror/swat");
		else if(rnd < 0.9)
			sprintf(bot_skin,"terror/jungleterr");
		else if(rnd < 0.95)
			sprintf(bot_skin,"sas/saspolice");
		else 
			sprintf(bot_skin,"sas/sasuc");
	}
	else
		strcpy(bot_skin,skin);

	// initialise userinfo
	memset (userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey (userinfo, "name", bot_name);
	Info_SetValueForKey (userinfo, "skin", bot_skin);
	Info_SetValueForKey (userinfo, "hand", "2"); // bot is center handed for now!
//AQ2 ADD
	Info_SetValueForKey (userinfo, "spectator", "0"); // NOT a spectator
//AQ2 END

	ClientConnect (bot, userinfo);

//	ACESP_SaveBots(); // make sure to save the bots
}
//RiEvEr - new global to enable bot self-loading of routes
extern char current_map[55];
//

char	*LocalTeamNames[3] = {"spectator", "1", "2" };

///////////////////////////////////////////////////////////////////////
// Spawn the bot
///////////////////////////////////////////////////////////////////////
void ACESP_SpawnBot (char *team, char *name, char *skin, char *userinfo)
{
	edict_t	*bot;
	
	bot = ACESP_FindFreeClient ();
	
	if (!bot)
	{
		safe_bprintf (PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	// To allow bots to respawn
	if(userinfo == NULL)
		ACESP_SetName(bot, name, skin, team);
	else
		ClientConnect (bot, userinfo);
	
	G_InitEdict (bot);

	// Balance the teams!
	if(teamplay->value)
	{
		if( (team == NULL) || (strlen(team) < 1) ) 
		{
			if( GetNextTeamNumber() == 1 )
			{
				gi.bprintf(PRINT_HIGH, "Assigned to team 1\n");
				team = LocalTeamNames[1];
			}
			else
			{
				team = LocalTeamNames[2];
				gi.bprintf(PRINT_HIGH, "Assigned to team 2\n");
			}
		}
	}

	InitClientResp (bot->client);
	

//AQ2 CHANGE
	// Set up the preferred weapon & equipment
	bot->weaponchoice = 0;
	bot->equipchoice = 0;
// 		ACESP_PutClientInServer (bot,false,0);
	// locate ent at a spawn point
    if(teamplay->value)
	{
		if ((team != NULL) && (strcmp(team,"1")==0) )
			ACESP_PutClientInServer (bot,true, TEAM1);
		else
			ACESP_PutClientInServer (bot,true, TEAM2);
	}
	else
 		ACESP_PutClientInServer (bot,true,0); 
//AQ2 END

	// make sure all view stuff is valid
	ClientEndServerFrame (bot);
	
	ACEIT_PlayerAdded (bot); // let the world know we added another

	ACEAI_PickLongRangeGoal(bot); // pick a new goal

	// LTK chat stuff
	if( random() < 0.33)
	{
		// Store current enemies available
		int		i, counter = 0;
		edict_t *myplayer[MAX_BOTS];

		for(i=0;i<=num_players;i++)
		{
			// Find all available enemies to insult
			if(players[i] == NULL || players[i] == bot || 
			   players[i]->solid == SOLID_NOT)
			   continue;
		
			if(teamplay->value && OnSameTeam( bot, players[i]) )
			   continue;
			myplayer[counter++] = players[i];
		}
		if(counter > 0)
		{
			// Say something insulting to them!
			if(ltk_chat->value)	// Some people don't want this *sigh*
				LTK_Chat( bot, myplayer[rand()%counter], DBC_WELCOME);
		}
	}	

}

void	ClientDisconnect( edict_t *ent );

///////////////////////////////////////////////////////////////////////
// Remove a bot by name or all bots
///////////////////////////////////////////////////////////////////////
void ACESP_RemoveBot(char *name)
{
	int i;
	qboolean freed=false;
	edict_t *bot;

	for(i=0;i<maxclients->value;i++)
	{
		bot = g_edicts + i + 1;
		if(bot->inuse)
		{
			if(bot->is_bot && (strcmp(bot->client->pers.netname,name)==0 || strcmp(name,"all")==0))
			{
				bot->health = 0;
				player_die (bot, bot, bot, 100000, vec3_origin);
				// don't even bother waiting for death frames
//				bot->deadflag = DEAD_DEAD;
//				bot->inuse = false;
				freed = true;
				ClientDisconnect( bot );
//				ACEIT_PlayerRemoved (bot);
//				safe_bprintf (PRINT_MEDIUM, "%s removed\n", bot->client->pers.netname);
			}
		}
	}

	if(!freed)	
		safe_bprintf (PRINT_MEDIUM, "%s not found\n", name);
	
//	ACESP_SaveBots(); // Save them again
}

//====================================
// Stuff to generate pseudo-random names
//====================================
#define NUMNAMES	10
char	*names1[NUMNAMES] = {
	"Bad", "d3th", "L33t", "Fasst", "mAx", "l3thal", "kw1k", "Hard", "Angel", "Red"};

char	*names2[NUMNAMES] = {
	"Moon", "eevil", "wakko", "d00d", "killa", "dog", "sodja", "joos", "frags", "akimbo" };

char	*names3[NUMNAMES] = {
	"An", "Bal", "Calen", "Cor", "Fan", "Gil", "Hal", "Lin", "Mal", "Per"};

char	*names4[NUMNAMES] = {
	"adan", "rog", "born", "dor", "fing", "galad", "iel", "loss", "orch", "riel" };

qboolean	nameused[NUMNAMES][NUMNAMES];

//====================================
// New random bot naming routine
//====================================
void	LTKsetBotName( char	*bot_name )
{
	int	part1,part2;

	part1 = part2 = 0;

	do
	{
		part1 = rand()% NUMNAMES;
		part2 = rand()% NUMNAMES;
	}while( nameused[part1][part2]);

	// Mark that name as used
	nameused[part1][part2] = true;
	// Now put the name together
	if( random() < 0.5 )
	{
		strcpy( bot_name, names1[part1]);
		strcat( bot_name, names2[part2]);
	}
	else
	{
		strcpy( bot_name, names3[part1]);
		strcat( bot_name, names4[part2]);
	}
}
