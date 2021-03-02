/*****************************************************************

	Eraser Bot source code - by Ryan Feltrin, Added to by Acrid-

	..............................................................

	This file is Copyright(c) 1998, Ryan Feltrin, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................
	
	Should you decide to release a modified version of Eraser, you MUST
	include the following text (minus the BEGIN and END lines) in the 
	documentation for your modification.

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for 
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

	..............................................................

	You will find p_trail.c has not been included with the Eraser
	source code release. This is NOT an error. I am unable to 
	distribute this file because it contains code that is bound by
	legal documents, and signed by myself, never to be released
	to the public. Sorry guys, but law is law.

	I have therefore include the compiled version of these files
	in .obj form in the src\Release and src\Debug directories.
	So while you cannot edit and debug code within these files,
	you can still compile this source as-is. Although these will only
	work in MSVC v5.0, linux versions can be made available upon
	request.

	NOTE: When compiling this source, you will get a warning
	message from the compiler, regarding the missing p_trail.c
	file. Just ignore it, it will still compile fine.

	..............................................................

	I, Ryan Feltrin/Acrid-, hold no responsibility for any harm caused by the
	use of this source code. I also am NOT willing to provide any form
	of help or support for this source code. It is provided as-is,
	as a service by me, with no documentation, other then the comments
	contained within the code. If you have any queries, I suggest you
	visit the "official" Eraser source web-board, at
	http://www.telefragged.com/epidemic/. I will stop by there from
	time to time, to answer questions and help with any problems that
	may arise.

	Otherwise, have fun, and I look forward to seeing what can be done
	with this.

	-Ryan Feltrin
	-Acrid-

 *****************************************************************/
/* bot_spawn.c */

#include "g_local.h"
#include "m_player.h"
#include "bot_procs.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
#include "stdlog.h"

qboolean ClientConnect (edict_t *ent, char *userinfo, qboolean loadgame);

// BEGIN: SABIN code
edict_t *bot_GetLastFreeClient (void)
{
	edict_t *bot;
	int	i;

	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (!bot->inuse)
			break;
	}

	if (bot->inuse)
		bot = NULL;

	return bot;
}
// END: SABIN code

void respawn_bot (edict_t *self)
{
	if (level.intermissiontime)
		return;

	self->s.event = EV_PLAYER_TELEPORT;

	PutClientInServer(self);

	self->last_goal = NULL;
	self->enemy = self->goalentity = self->movetarget = NULL;
	self->viewheight = 22;

	// reset weapon to blaster
	self->last_fire = level.time + 0.2;
	self->fire_interval = FIRE_INTERVAL_BLASTER;
	self->bot_fire = botBlaster;

	self->bored_suicide_time = -1;
	self->checkstuck_time = level.time;
	self->last_reached_trail = level.time + 1;

	self->client->killer_yaw = 0;	// chaingun wind-up
	self->avoid_ent = NULL;
	self->flagpath_goal = NULL;
	self->last_move_nocloser = level.time;
   
	// go for it
	walkmonster_start(self);
}

// Find an available edict, and initialize some default values
edict_t *G_SpawnBot ()
{
	edict_t		*bot;
			
	if (!deathmatch->value)
	{
		return NULL;
	}

	if ((!bot_calc_nodes->value) && !loaded_trail_flag)
	{
		my_bprintf(PRINT_HIGH, "Route-table not found!\n");
		return NULL;
	}

	last_bot_spawn = level.time;

//	bot = G_Spawn();
	bot = bot_GetLastFreeClient();

	if (!bot)
	{
		gi.dprintf("No client spots available!\n");
		return NULL;
	}

//	bot->bot_client = gi.TagMalloc (sizeof(struct gclient_s), TAG_GAME);
	bot->bot_stats =  gi.TagMalloc (sizeof(bot_stats_t), TAG_GAME);

	if (!bot->bot_stats)
	{
		gi.dprintf("Could not allocate Bot Stats!\n");
		return NULL;
	}


	bot->classname = "player";

	bot->movetype = MOVETYPE_WALK;
	bot->solid = SOLID_BBOX;

	//K2:Begin//fixme remove
	bot->takedamage = DAMAGE_YES;
	//K2:End

	VectorSet (bot->mins, -16, -16, -24);
	VectorSet (bot->maxs, 16, 16, 32);

	bot->health = bot->max_health = 100;
	bot->mass = 200;
	bot->gravity = 1;

	bot->last_goal = NULL;

	bot->pain = bot_pain;
	bot->die = bot_die;

	bot->monsterinfo.stand = bot_run; //bot_stand;
	bot->monsterinfo.walk = bot_run; //bot_walk;
	bot->monsterinfo.run = bot_run;
	bot->monsterinfo.attack = bot_run;
	bot->monsterinfo.melee = NULL;
	bot->monsterinfo.sight = NULL;

	bot->monsterinfo.scale = MODEL_SCALE;

	bot->enemy = bot->goalentity = bot->movetarget = NULL;

	players[num_players++] = bot;

	bot->last_fire = level.time + 0.2;
	bot->fire_interval = FIRE_INTERVAL_BLASTER;
	bot->bot_fire = botBlaster;
	
	bot->bored_suicide_time = -1;
	bot->checkstuck_time = level.time;

	bot->viewheight = 22;
	bot->yaw_speed = 50;	// turn at yaw_speed degrees per FRAME

	bot->last_reached_trail = level.time + 1;

	bot->avoid_ent = NULL;
	bot->last_move_nocloser = level.time;

	return bot;
}

// Perform the ClientBegin() and ClientBeginDeathmatch() functions in one routine, that is bot-specific
edict_t *spawn_bot (char *botname)
{
	edict_t		*bot, *chat;
	bot_info_t	*botdata=NULL;
//fixme debugger	char	skin[256];
	char	userinfo[MAX_INFO_STRING];
	vec3_t		spawn_origin, spawn_angles;
    char    WFclass[256];

	if (!(botdata = GetBotData(botname)))
	{
		gi.dprintf("Unable to find bot, or no bots left\n");
		return NULL;
	}


	bot = G_SpawnBot();
 
	if (!bot)
	{
		gi.dprintf("Unable to spawn bot: cannot create entity\n");
		return NULL;
	}

	bot->bot_client = true;
	bot->client = &game.clients[bot-g_edicts-1];

	memset(bot->client, 0, sizeof(*(bot->client)));

	// copy the stats across from the bot config
	botdata->ingame_count++;
	bot->botdata = botdata;

	strcpy(WFclass, botdata->wfclass);

// BEGIN: SABIN code
	// initialise userinfo
	memset (userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey (userinfo, "name", botdata->name);
	Info_SetValueForKey (userinfo, "WFclass", botdata->wfclass);
	Info_SetValueForKey (userinfo, "hand", "2"); // bot is center handed for now
// END: SABIN code
    
    ///Q2 Camera Begin
    EntityListAdd(bot);
    ///Q2 Camera End
//WF START ACRID INITIAL CLASS SPAWNING

	//set respawn protection time
    bot->client->protecttime = level.time + RESPAWN_PROTECT_TIME;	
//      bot->client->pers.player_class = bot->client->pers.next_player_class;
if (strcmp("1", WFclass) == 0)
	{	bot->client->pers.player_class = 1;
	bot->client->pers.next_player_class = 1;}
else if (strcmp("2", WFclass) == 0)
	{	bot->client->pers.player_class = 2;
	bot->client->pers.next_player_class = 2;}
else if (strcmp("3", WFclass) == 0)
	{	bot->client->pers.player_class = 3;
	bot->client->pers.next_player_class = 3;}
else if (strcmp("4", WFclass) == 0)
	{bot->client->pers.player_class = 4;
	bot->client->pers.next_player_class = 4;}
else if (strcmp("5", WFclass) == 0)
	{	bot->client->pers.player_class = 5;
	bot->client->pers.next_player_class = 5;}
else if (strcmp("6", WFclass) == 0)
	{	bot->client->pers.player_class = 6;
	bot->client->pers.next_player_class = 6;}
else if (strcmp("7", WFclass) == 0)
	{	bot->client->pers.player_class = 7;
	bot->client->pers.next_player_class = 7;}
else if (strcmp("8", WFclass) == 0)
	{	bot->client->pers.player_class = 8;
	bot->client->pers.next_player_class = 8;}
else if (strcmp("9", WFclass) == 0)
	{	bot->client->pers.player_class = 9;
	bot->client->pers.next_player_class = 9;}
else if (strcmp("10", WFclass) == 0)
	{	bot->client->pers.player_class = 10;
	bot->client->pers.next_player_class = 10;}
//WF END ACRID FIXES SKIN BUGS,CLASS BUGS,WEAPONS BUG clean this up 
	ClientConnect (bot, userinfo, false);

	if (ctf->value)
	{
		my_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			bot->client->pers.netname, CTFTeamName(bot->client->resp.ctf_team));
    	sl_LogPlayerTeamChange( &gi,
            		            bot->client->pers.netname,
                    		    CTFTeamName(bot->client->resp.ctf_team));
	}

	SelectSpawnPoint (bot, spawn_origin, spawn_angles);
//INSERT TESTS HERE
//    botDebugPrint("SPAWN_BOT %s (ACRID)\n",userinfo);

	VectorCopy(spawn_origin, bot->s.origin);
//	bot->s.origin[2] += 1;	// make sure off ground

	VectorCopy(spawn_angles, bot->s.angles);

	bot->client->killer_yaw = 0;	// chaingun wind-up

	// clear playerstate values
	memset (&bot->client->ps, 0, sizeof(bot->client->ps));

	bot->client->ps.pmove.origin[0] = bot->s.origin[0]*8;
	bot->client->ps.pmove.origin[1] = bot->s.origin[1]*8;
	bot->client->ps.pmove.origin[2] = bot->s.origin[2]*8;

	bot->client->ps.fov = 90;

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (bot-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (bot->s.origin, MULTICAST_PVS);

	// copy the bot stats
	memcpy(bot->bot_stats, &(botdata->bot_stats), sizeof(bot_stats_t));

	// set starting skill level
	bot->skill_level = skill->value;
	AdjustRatingsToSkill(bot);

//	bot->s.modelindex = gi.modelindex(model);
    bot->s.skinnum = bot-g_edicts - 1;
	bot->s.modelindex = 255;
	bot->s.modelindex2 = 255;
//    botDebugPrint("SPAWN_BOT (ACRID)\n");
/*
	bot->client->buttons = bot->s.modelindex;
	bot->client->oldbuttons = bot->s.skinnum;
*/
	bot->map = G_CopyString(botdata->name);
	strcpy(bot->client->pers.netname, botdata->name);
	my_bprintf(PRINT_HIGH, "%s entered the game", bot->client->pers.netname);

	// set visible model vwep with 3.20 
	ShowGun(bot); 

/*
	if (view_weapons->value && (bot->s.modelindex2 == 255) && bot_show_connect_info->value)
	{
		my_bprintf(PRINT_HIGH, " (no view weapon)");
	}
*/
	my_bprintf(PRINT_HIGH, "\n");

	bot_count++;

	// generic bot stuff
	if (!KillBox (bot))
	{	// could't spawn in?
	}

	gi.linkentity (bot);

	bot->viewheight = 22;

	bot->inuse = true;

	// go for it
	walkmonster_start(bot);

	if (random() < 0.3)
	{
		// spawn the greetings thinker
		chat = G_Spawn();
		chat->owner = bot;
		chat->think = BotGreeting;
		chat->nextthink = level.time + 1.5 + random();
	}
    bot->wf_team = bot->client->resp.ctf_team;
    wf_InitPlayerClass(bot->client);//ACRID

	//K2 botcam acrid
	bot->client->resp.inServer=true;

	return bot;
}

extern int num_clients;
//K2:Added for camera..must add player back to players array
void botAddPlayer(edict_t *self)
{
	players[num_players] = self;
	num_players++;
	num_clients++;
	return;

}
void botRemovePlayer(edict_t *self)
{
	int i;

	self->health = 0;	// so other bots stop looking for us

	// remove the client from the players array
	for (i=0; i<num_players; i++)
		if (players[i] == self)
			break;

	if (i == num_players)	// didn't find them
	{
		gi.dprintf("WARNING: Unable to remove player from player[] array, problems will arise.\n");
		return;
	}

	i++;
	for (; i<num_players; i++)
		players[i-1] = players[i];

	players[i] = NULL;


	// remove from team list
	if (self->client->team)
	{
		self->client->team->num_players--;
		self->client->team->num_bots--;
		self->client->team = NULL;
	}

	self->client->resp.ctf_team = CTF_NOTEAM;

	num_players--;

	if (self->bot_client)
	{
		bot_count--;
		self->botdata->ingame_count--;
	}
	else
	{
		num_clients--;
	}

	// tell all other bots not to look for this bot anymore
	for (i=0; i<num_players; i++)
		if (players[i]->enemy == self)
			players[i]->enemy = players[i]->goalentity = NULL;
}

void	botDisconnect(edict_t	*self)
{	// disconnects a bot from the game

	ClientDisconnect(self);

	self->bot_client = false;
	self->bot_stats = NULL;
}
