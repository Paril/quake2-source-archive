
#include "g_local.h"
#include "b_local.h"

void Bot_PutClientInServer (edict_t *bot, qboolean respawn, int team)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	
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
	bot->jumping = false;
	
//	if(ctf->value)
//	{
//		client->resp.ctf_team = team;
//		client->resp.ctf_state = CTF_STATE_START;
//		s = Info_ValueForKey (client->pers.userinfo, "skin");
//		CTFAssignSkin(bot, s);
//	}

	VectorCopy (mins, bot->mins);
	VectorCopy (maxs, bot->maxs);
	VectorClear (bot->velocity);

	// clear playerstate values
	memset (&bot->client->ps, 0, sizeof(client->ps));
	
	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

//RipVTide
//	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
//	{
//		client->ps.fov = 90;
//	}
//	else
//	{
//		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
//		if (client->ps.fov < 1)
//			client->ps.fov = 90;
//		else if (client->ps.fov > 160)
//			client->ps.fov = 160;
//	}

//	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	bot->s.effects = 0;
	bot->s.skinnum = bot - g_edicts - 1;
	bot->s.modelindex = 255;		// will use the skin specified model
	bot->s.modelindex2 = 255;		// custom gun model
	bot->s.frame = 0;
	VectorCopy (spawn_origin, bot->s.origin);
	bot->s.origin[2] += 1;	// make sure off ground

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
//	bot->state = STATE_MOVE;

	// Set the current node
//	bot->current_node = ACEND_FindClosestReachableNode(bot,NODE_DENSITY, NODE_ALL);
//	bot->goal_node = bot->current_node;
//	bot->next_node = bot->current_node;
//	bot->next_move_time = level.time;		
//	bot->suicide_timeout = level.time + 15.0;

	// If we are not respawning hold off for up to three seconds before releasing into game
	if (!KillBox (bot))
	{	// could't spawn in?
	}

	gi.linkentity (bot);

	bot->think = Bot_AI_Think;
	bot->nextthink = level.time + FRAMETIME;

		// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (bot-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (bot->s.origin, MULTICAST_PVS);
}

edict_t *Bot_Find_Free_Client (void)
{
	edict_t *client;
	int	i;
	int max_count;
	
	max_count = 0;

	for (i=maxclients->value; i>0; i--)
	{
		client = g_edicts + i + 1;
		
		if(client->count > max_count)
			max_count = client->count;
	}

	for (i=maxclients->value; i>0; i--)
	{
		client = g_edicts + i + 1;

		if (!client->inuse)
			break;
	}

	client->count = max_count + 1;

	if (client->inuse)
		client = NULL;
	
	return client;
}

//void Bot_Set_Data(edict_t *bot, char *name, char *skin, char *team)
//{
//	float rnd;
//	char userinfo[MAX_INFO_STRING];
//	char bot_skin[MAX_INFO_STRING];
//	char bot_name[MAX_INFO_STRING];
//
	// Set the name for the bot.
	// name
//	if(strlen(name) == 0)
//		sprintf(bot_name,"Bot_%d",bot->count);
//	else
//		strcpy(bot_name,name);

	// skin
//	if(strlen(skin) == 0)
//	{
		// randomly choose skin 
//		rnd = random();
//		if(rnd  < 0.05)
//			sprintf(bot_skin,"female/athena");
//		else if(rnd < 0.1)
//			sprintf(bot_skin,"female/brianna");
//		else if(rnd < 0.15)
//			sprintf(bot_skin,"female/cobalt");
//		else if(rnd < 0.2)
//			sprintf(bot_skin,"female/ensign");
//		else if(rnd < 0.25)
//			sprintf(bot_skin,"female/jezebel");
//		else if(rnd < 0.3)
//			sprintf(bot_skin,"female/jungle");
//		else if(rnd < 0.35)
//			sprintf(bot_skin,"female/lotus");
//		else if(rnd < 0.4)
//			sprintf(bot_skin,"female/stiletto");
//		else if(rnd < 0.45)
//			sprintf(bot_skin,"female/venus");
//		else if(rnd < 0.5)
//			sprintf(bot_skin,"female/voodoo");
//		else if(rnd < 0.55)
//			sprintf(bot_skin,"male/cipher");
//		else if(rnd < 0.6)
//			sprintf(bot_skin,"male/flak");
//		else if(rnd < 0.65)
//			sprintf(bot_skin,"male/grunt");
//		else if(rnd < 0.7)
//			sprintf(bot_skin,"male/howitzer");
//		else if(rnd < 0.75)
//			sprintf(bot_skin,"male/major");
//		else if(rnd < 0.8)
//			sprintf(bot_skin,"male/nightops");
//		else if(rnd < 0.85)
//			sprintf(bot_skin,"male/pointman");
//		else if(rnd < 0.9)
//			sprintf(bot_skin,"male/psycho");
//		else if(rnd < 0.95)
//			sprintf(bot_skin,"male/razor");
//		else 
//			sprintf(bot_skin,"male/sniper");
//	}
//	else
//		strcpy(bot_skin,skin);

	// initialise userinfo
//	memset (userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
//	Info_SetValueForKey (userinfo, "name", bot_name);
//	Info_SetValueForKey (userinfo, "skin", bot_skin);
//	Info_SetValueForKey (userinfo, "hand", "2"); // bot is center handed for now!

//	ClientConnect (bot, userinfo);
//}

void Setup_Bot (edict_t *bot)
{
	players[num_players++] = bot;
	bot->inuse = 1;
	bot->is_bot = 1;
	bot->yaw_speed = 90;
	bot->path_node = NO_NODES;
	bot->last_path_node = NO_NODES;
	bot->next_path_node = NO_NODES;
	bot->check_path_time = NO_NODES;
	bot->moveflags |= MOVE_ROAM;

	gi.dprintf("players on server = %i\n", num_players);
}

void Spawn_Bot (char *name, char *skin, char *userinfo)
{
	edict_t *bot;

	bot = Bot_Find_Free_Client();

	if(!bot)
	{
		gi.dprintf("cannot add any more bots\n");
	}

	Setup_Bot (bot);

	if(userinfo == NULL)
	{
		Bot_Set_Data(bot, name, skin, 0);
	}
	else
	{
		ClientConnect(bot, userinfo);
	}

	G_InitEdict (bot);
	InitClientResp (bot->client);

	Bot_PutClientInServer (bot, 0, 0);

	ClientEndServerFrame (bot);
}

