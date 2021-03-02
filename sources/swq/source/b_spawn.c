
#include "g_local.h"

void Bot_respawn (edict_t *self)
{
	if (deathmatch->value || coop->value || ctf->value)
	{
		// spectators don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		if(ctf->value)
			Bot_PutClientInServer (self,true, self->client->resp.ctf_team);
		else
			Bot_PutClientInServer (self, true, 0);

		// add a teleportation effect
//		self->s.event = EV_PLAYER_TELEPORT;
		RespawnExplosion(self);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_NUKEBLAST);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PHS);
		gi.sound (self, CHAN_VOICE, gi.soundindex("misc/spawn.wav"), 1, ATTN_NORM, 0);

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;

		return;
	}
}

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
	char *s;
	
	SelectSpawnPoint (bot, spawn_origin, spawn_angles);
	
	index = bot-g_edicts-1;
	client = bot->client;

	if (deathmatch->value || ctf->value)
	{
		char userinfo[MAX_INFO_STRING];

		resp = bot->client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (bot, userinfo);
	}
	else
//		memset (&resp, 0, sizeof(resp));
	{
		char		userinfo[MAX_INFO_STRING];

		memset (&resp, 0, sizeof(resp));
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		ClientUserinfoChanged (bot, userinfo);
	}

	// clear everything but the persistant data
//	saved = client->pers;
//	memset (client, 0, sizeof(*client));
//	client->pers = saved;
//	client->resp = resp;

	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	FetchClientEntData (bot);

	bot->groundentity = NULL;
	bot->client = &game.clients[index];
	bot->takedamage = DAMAGE_AIM;
	bot->movetype = MOVETYPE_WALK;
	bot->viewheight = 24;
	bot->inuse = true;
	bot->classname = "player";
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

	bot->fog_fog = 0;
	bot->saber_colour = CRYSTAL_YELLOW;

	if(ctf->value)
	{
		client->resp.ctf_team = team;
		client->resp.ctf_state = CTF_STATE_START;
		s = Info_ValueForKey (client->pers.userinfo, "skin");
		CTFAssignSkin(bot, s);
	}

	VectorCopy (mins, bot->mins);
	VectorCopy (maxs, bot->maxs);
	VectorClear (bot->velocity);

	// clear playerstate values
	memset (&bot->client->ps, 0, sizeof(client->ps));
	
	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

	client->ps.fov = 90;

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	bot->s.effects = 0;
	bot->s.skinnum = bot - g_edicts - 1;
	bot->s.modelindex = 255;		// will use the skin specified model
	bot->s.modelindex2 = 255;		// custom gun model
	bot->s.frame = 0;
	VectorCopy (spawn_origin, bot->s.origin);
	bot->s.origin[2] += 1;	// make sure off ground
	VectorCopy (bot->s.origin, bot->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	bot->s.angles[PITCH] = 0;
	bot->s.angles[YAW] = spawn_angles[YAW];
	bot->s.angles[ROLL] = 0;
	VectorCopy (bot->s.angles, client->ps.viewangles);
	VectorCopy (bot->s.angles, client->v_angle);
	
	bot->enemy = NULL;
	bot->movetarget = NULL; 

	// If we are not respawning hold off for up to three seconds before releasing into game
	if (!KillBox (bot))
	{	// could't spawn in?
	}

	gi.linkentity (bot);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (bot);

	bot->think = Bot_AI_Think;
	bot->nextthink = level.time + FRAMETIME;

		// send effect
	if(!respawn)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (bot-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (bot->s.origin, MULTICAST_PVS);
	}
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

void Bot_Set_Data(edict_t *bot, char *name, char *skin, char *team)
{
	float r;
	char userinfo[MAX_INFO_STRING];
	char bot_skin[16];
	char bot_name[16];

	r = random();

	if(name == NULL)
	{
		if(num_players == 1 || num_players == 6)
		{
			if(r < 0.2)
			{
				sprintf(bot_name,"CaRRaC");
				sprintf(bot_skin,"male/jedi1");
			}
			else if(r < 0.4)
			{
				sprintf(bot_name,"RipVTide");
				sprintf(bot_skin,"male/maul");
			}
			else if(r < 0.6)
			{
				sprintf(bot_name,"Legion");
				sprintf(bot_skin,"male/jedi2");
			}
			else if(r < 0.8)
			{
				sprintf(bot_name,"Darth Maul");
				sprintf(bot_skin,"male/maul");
			}
			else if(r < 1.0)
			{
				sprintf(bot_name,"Broken Fixed");
				sprintf(bot_skin,"male/ob1");
			}
			else
			{
				sprintf(bot_name,"-2+");
				sprintf(bot_skin,"male/playerskin");
			}
		}
		else if (num_players == 2 || num_players == 7)
		{
			if(r < 0.2)
			{
				sprintf(bot_name,"Privateer");
				sprintf(bot_skin,"male/hansolo");
			}
			else if(r < 0.4)
			{
				sprintf(bot_name,"Falkon2");
				sprintf(bot_skin,"male/obiwan");
			}
			else if(r < 0.6)
			{
				sprintf(bot_name,"ViolentBlue");
				sprintf(bot_skin,"male/combat");
			}
			else if(r < 0.8)
			{
				sprintf(bot_name,"Darth Vader");
				sprintf(bot_skin,"male/jedi1");
			}
			else if(r < 1.0)
			{
				sprintf(bot_name,"Anakin");
				sprintf(bot_skin,"male/jedi2");
			}
			else
			{
				sprintf(bot_name,"Armage");
				sprintf(bot_skin,"male/hansolo");
			}
		}
		else if (num_players == 2 || num_players == 8)
		{
			if(r < 0.2)
			{
				sprintf(bot_name,"Red Knight");
				sprintf(bot_skin,"male/maul");
			}
			else if(r < 0.4)
			{
				sprintf(bot_name,"Grey Knight");
				sprintf(bot_skin,"male/combat");
			}
			else if(r < 0.6)
			{
				sprintf(bot_name,"Tim Elek");
				sprintf(bot_skin,"male/player1");
			}
			else if(r < 0.8)
			{
				sprintf(bot_name,"Obiwan Kenobi");
				sprintf(bot_skin,"male/obiwan");
			}
			else if(r < 1.0)
			{
				sprintf(bot_name,"Quigon Jinn");
				sprintf(bot_skin,"male/jedi1");
			}
			else
			{
				sprintf(bot_name,"McClane3");
				sprintf(bot_skin,"male/lando");
			}
		}
		else if (num_players == 3 || num_players == 9)
		{
			if(r < 0.2)
			{
				sprintf(bot_name,"ApocX");
				sprintf(bot_skin,"male/player1");
			}
			else if(r < 0.4)
			{
				sprintf(bot_name,"Electric");
				sprintf(bot_skin,"male/player2");
			}
			else if(r < 0.6)
			{
				sprintf(bot_name,"Han Solo");
				sprintf(bot_skin,"male/hansolo");
			}
			else if(r < 0.8)
			{
				sprintf(bot_name,"Boba Fett");
				sprintf(bot_skin,"male/combat");
			}
			else if(r < 1.0)
			{
				sprintf(bot_name,"War|ocK");
				sprintf(bot_skin,"male/ctfred");
			}
			else
			{
				sprintf(bot_name,"deadguy");
				sprintf(bot_skin,"male/maul");
			}
		}
		else if (num_players == 4 || num_players == 10)
		{
			if(r < 0.2)
			{
				sprintf(bot_name,"Maxer");
				sprintf(bot_skin,"male/jedi2");
			}
			else if(r < 0.4)
			{
				sprintf(bot_name,"Bitterman");
				sprintf(bot_skin,"male/ctfblue");
			}
			else if(r < 0.6)
			{
				sprintf(bot_name,"Luke Skywalker");
				sprintf(bot_skin,"male/jedi1");
			}
			else if(r < 0.8)
			{
				sprintf(bot_name,"Damaramu");
				sprintf(bot_skin,"male/player1");
			}	
			else if(r < 1.0)
			{
				sprintf(bot_name,"moresmart");
				sprintf(bot_skin,"male/obiwan");
			}
			else
			{
				sprintf(bot_name,"AGWAR", num_players);
				sprintf(bot_skin,"male/maul");
			}
		}
		else if (num_players == 5 || num_players >= 11)
		{
			if(r < 0.2)
			{
				sprintf(bot_name,"Lando");
				sprintf(bot_skin,"male/lando");
			}
			else if(r < 0.4)
			{
				sprintf(bot_name,"Yoda");
				sprintf(bot_skin,"male/jedi2");
			}
			else if(r < 0.6)
			{
				sprintf(bot_name,"Palpatine");
				sprintf(bot_skin,"male/obiwan");
			}
			else if(r < 0.8)
			{
				sprintf(bot_name,"Bib Fortuna");
				sprintf(bot_skin,"male/player2");
			}
			else if(r < 1.0)
			{
				sprintf(bot_name,"Wicket");
				sprintf(bot_skin,"male/jedi2");
			}
			else
			{
				sprintf(bot_name,"Drakis");
				sprintf(bot_skin,"male/player1");
			}
		}
	}
	else
		strcpy(bot_name,name);

//	if(skin == NULL)
//	{
//		if(r < 0.1)
//			sprintf(bot_skin,"male/baseskin");
//		else if(r < 0.2)
//			sprintf(bot_skin,"male/ep1mace");
//		else if(r < 0.3)
//			sprintf(bot_skin,"male/ep1maul");
//		else if(r < 0.4)
//			sprintf(bot_skin,"male/ep1ob1");
//		else if(r < 0.5)
//			sprintf(bot_skin,"male/swqmp1");
//		else if(r < 0.6)
//			sprintf(bot_skin,"male/swqmp2");
//		else if(r < 0.7)
//			sprintf(bot_skin,"male/ep1maul");
//		else if(r < 0.8)
//			sprintf(bot_skin,"male/ep1ob1");
//		else if(r < 0.85)
//			sprintf(bot_skin,"male/swqmp1");
//		else if(r < 0.9)
//			sprintf(bot_skin,"male/swqmp1");
//		else if(r < 0.95)
//			sprintf(bot_skin,"male/swqmp2");
//		else if(r < 1.0)
//			sprintf(bot_skin,"male/swqmp3");
//		else
//			sprintf(bot_skin,"male/baseskin");
//	}
//	else
//		strcpy(bot_skin,skin);

	memset (userinfo, 0, sizeof(userinfo));

	Info_SetValueForKey (userinfo, "hand", "2");
	Info_SetValueForKey (userinfo, "name", bot_name);
	Info_SetValueForKey (userinfo, "skin", bot_skin);

	ClientConnect (bot, userinfo);
}

void Setup_Bot (edict_t *bot)
{
	bot->flags = num_players;
	players[num_players++] = bot;
	bot->inuse = 1;
	bot->is_bot = 1;
	bot->yaw_speed = 90;
	bot->path_node = NO_NODES;
	bot->last_path_node = NO_NODES;
	bot->next_path_node = NO_NODES;
	bot->check_path_time = NO_NODES;
	bot->moveflags |= MOVE_ROAM;
}

int ValidateSkin (char *skin)
{
	counter_t		i = 0, j = 0;
	FILE			*input;
	char			file_name[256];
	cvar_t			*game_dir, *basedir;

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar ("basedir", ".", 0);

	if (!Q_stricmp (game_dir->string, ""))
		sprintf (game_dir->string, "baseq2");

	sprintf (file_name, "%s\\%s\\players\\%s.pcx", basedir->string, game_dir->string, skin);

	input = fopen (file_name, "rb");

	if (!input)
	{
		return 0;
	}
	return 1;
}

void Spawn_New_Bot (char *name, char *skin)
{
	if(!strcmp("", name))
	{
		name = NULL;
	}
	if(!strcmp("", skin))
	{
		skin = NULL;
	}
	else
	{
		if(!(ValidateSkin(skin)))
			skin = NULL;
	}
	Spawn_Bot (name, skin);
}

void Spawn_Bot (char *name, char *skin)
{
	edict_t *bot;

	if(!sv_cheats->value && ctf->value)
	{
		safe_bprintf(PRINT_MEDIUM, "Cannot add bots in CTF mode. (set cheats 1)\n");
		return;
	}

	if(!deathmatch->value)
	{
		safe_bprintf(PRINT_MEDIUM, "Can only add bots in multiplayer\n");
		return;
	}

	if(node_count == 0)
	{
		safe_bprintf(PRINT_MEDIUM, "No node file\n");
		return;
	}

	bot = Bot_Find_Free_Client();

	if(!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "cannot add any more bots\n");
		return;
	}

	Setup_Bot (bot);

	Bot_Set_Data(bot, name, skin, 0);

	G_InitEdict (bot);
	InitClientResp (bot->client);

    if(ctf->value)
	{
		if(random() > 0.5)
			Bot_PutClientInServer (bot,false, CTF_TEAM1);
		else
			Bot_PutClientInServer (bot,false, CTF_TEAM2);
	}
	else
		Bot_PutClientInServer (bot, false, 0);

	ClientEndServerFrame (bot);
}

