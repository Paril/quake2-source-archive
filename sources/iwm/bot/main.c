#include "../g_local.h"

#define BOT_MOVESPEED 800

void SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
qboolean ClientConnect (edict_t *ent, char *userinfo);

edict_t *CopyToBodyQue(edict_t *ent);
void ClientThink(edict_t *ent, usercmd_t *cmd);

void Cmd_Bot_f(edict_t *ent);
void Bot_Create(float delay);
qboolean Bot_Spawn(edict_t *ent);
void Bot_Respawn(edict_t *ent);
void Bot_Pain(edict_t *ent, edict_t *other, float kickback, int damage);
void Bot_Move(edict_t *ent, edict_t *goal, usercmd_t *cmd, vec3_t angles);
edict_t *Bot_FindEnemy(edict_t *ent);
void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles);
void Bot_Aim(edict_t *ent, edict_t *target, vec3_t angles);

void Bot_Think (edict_t *ent);


void Svcmd_Bot_f(void)
{
	char *arg = gi.argv(2);
	int num = atoi(gi.argv(3));
	float del = atof(gi.argv(4));
	int i;
	float delay = 0;

	if (Q_stricmp(arg, "spawn") == 0)
	{
		if (!num)
		{
			if (!del)
				del = 0;

			Bot_Create(delay);
		}
		else
		{
			for (i = 0; i < num; i++)
			{
				if (!del)
					del = 0.6;

				Bot_Create(delay);
				delay += del;
			}
		}
	}
	else if (Q_stricmp(arg, "kill") == 0)
	{
		int i;
		char *name = gi.argv(3);
		edict_t *bot;
		int found = 0;
		char *say[6] = {"Cya.", "Bye", "Have fun without me", "I need to return to bytes, cya.", "Cya inferior humans and fellow bots!", "Bye-bye!"};

		if (name == NULL || !name || Q_stricmp(name, "") == 0)
		{
			for (i = maxclients->value; i > 0; i--)
			{
				bot = g_edicts + i + 1;
				
				if (bot && bot->client && bot->classname && Q_stricmp("bot", bot->classname) == 0)
				{
					safe_bprintf (PRINT_CHAT, "%s: %s\n", bot->client->pers.netname, say[rand()%6]);
					ClientDisconnect(bot);
					found = 1;
					break;
				}
			}
			if (!found)
				gi.dprintf ("No bots are in the game!\n");

			return;
		}

		if (Q_stricmp(name, "all") == 0)
		{
			for (i = maxclients->value; i > 0; i--)
			{
				bot = g_edicts + i + 1;
				
				if (bot && bot->client && bot->classname &&  Q_stricmp("bot", bot->classname) == 0)
				{
					safe_bprintf (PRINT_CHAT, "%s: %s\n", bot->client->pers.netname, say[rand()%6]);
					ClientDisconnect(bot);
					found = 1;
				}
			}
			if (!found)
				gi.dprintf ("No bots are in the game!\n");

			return;
		}
		
		for (i = maxclients->value; i > 0; i--)
		{
			bot = g_edicts + i + 1;

			if (!name)
				name = "NULL";
			
			if (bot && bot->inuse && bot->client &&
				(Q_stricmp("bot", bot->classname) == 0) && bot->classname && 
				(Q_stricmp(name, bot->client->pers.netname) == 0))
			{
				safe_bprintf (PRINT_CHAT, "%s: %s\n", bot->client->pers.netname, say[rand()%6]);
				ClientDisconnect(bot);
				found = 1;
				break;
			}
		}

		if (!found)
			gi.dprintf ("Bot %s not found!\nYou must include the [Bot] tag when killing the bot.\n", name);
	}
}


// Skins
// Male
#define SKIN_CIPHER				"cipher"
#define SKIN_CLAYMORE			"claymore"
#define SKIN_DISGUISE			"male/disguise"
#define SKIN_FLAK				"flak"
#define SKIN_GRUNT				"grunt"
#define SKIN_HOWITZER			"howitzer"
#define SKIN_MAJOR				"major"
#define SKIN_NIGHTOPS			"nightops"
#define SKIN_POINTMAN			"pointman"
#define SKIN_PHYCHO				"phycho"
#define SKIN_RAMPAGE			"rampage"
#define SKIN_RAZOR				"razor"
#define SKIN_RECON				"recon"
#define SKIN_ROGUEB				"rogue_b"
#define SKIN_ROGUER				"rogue_r"
#define SKIN_SCOUT				"scout"
#define SKIN_SNIPER				"sniper"
#define SKIN_VIPER				"viper"
// Female
#define SKIN_ATHENA				"athena"
#define SKIN_BRIANNA			"brianna"
#define SKIN_COBALT				"cobalt"
#define SKIN_DISGUISEF			"disguise"
#define SKIN_ENSIGN				"ensign"
#define SKIN_JEZEBEL			"jezebel"
#define SKIN_JUNGLE				"jungle"
#define SKIN_LOTUS				"lotus"
#define SKIN_ROGUEBF			"rogue_b"
#define SKIN_ROGUERF			"rogue_r"
#define SKIN_STILETTO			"stiletto"
#define SKIN_VENUS				"venus"
#define SKIN_VOODOO				"voodoo"

// Cyborg
// Skins
#define SKIN_ONI911				"oni911"
#define SKIN_PS9000				"ps9000"
#define SKIN_TYR574				"tyr574"

const char *c_szNames[] = { "Parilbot", "Skiesbot", "Wolf", "Crossfire", "Mr. Crazy", "Chuck",
							"Will", "Haxxo", "SnIpEr", "Hajaz", "Chris", "Santa", 
							"Asploder", "Whoa", "Willium", "Sephy", "TheOwner", "l337", "Mike",
							"Radud", "EViL" };


const char *c_szClans[] = { "[OPFD]", "(GGFG)", "[PDP]", "{WRTC}", "[ELO]", 
							"_MNN_", "~TILL~", "_MUL_", "-KLD-", "__CL__", 
							"(-SIX-)", "[HL]", "" };


const char *gender[] = { "male", "female", "cyborg" };

const char *male_skins[] = { SKIN_CIPHER, SKIN_CLAYMORE, SKIN_DISGUISE, 
							 SKIN_FLAK, SKIN_GRUNT, SKIN_HOWITZER,
							 SKIN_MAJOR, SKIN_NIGHTOPS, SKIN_POINTMAN,
							 SKIN_PHYCHO, SKIN_RAMPAGE, SKIN_RAZOR, SKIN_RECON,
							 SKIN_ROGUEB, SKIN_ROGUER, SKIN_SCOUT, SKIN_SNIPER, 
							 SKIN_VIPER };

const char *female_skins[] = { SKIN_ATHENA, SKIN_BRIANNA, SKIN_COBALT, 
							 SKIN_DISGUISEF, SKIN_ENSIGN, SKIN_JEZEBEL,
							 SKIN_JUNGLE, SKIN_LOTUS, SKIN_ROGUEBF,
							 SKIN_ROGUERF, SKIN_STILETTO, SKIN_VENUS, 
							 SKIN_VOODOO };

const char *cyborg_skins[] = { SKIN_ONI911, SKIN_PS9000, SKIN_TYR574 };

unsigned int nNameCount = sizeof( c_szNames ) / sizeof( c_szNames[0] );
unsigned int nClanCount = sizeof( c_szClans ) / sizeof( c_szClans[0] );

unsigned int gendercount = sizeof( gender ) / sizeof( gender[0] );

unsigned int male_skinscount = sizeof( male_skins ) / sizeof( male_skins[0] );
unsigned int female_skinscount = sizeof( female_skins ) / sizeof( female_skins[0] );
unsigned int cyborg_skinscount = sizeof( cyborg_skins ) / sizeof( cyborg_skins[0] );

typedef enum
{
	SHOTGUN,
	SUPERSHOTGUN,
	MACHINEGUN,
	CHAINGUN,
	GRENADELAUNCHER,
	ROCKETLAUNCHER,
	HYPERBLASTER,
	RAILGUN,
	BFG
} bot_favweapon;

void SpawnAfterDelay (edict_t *bot)
{
	Bot_Create(0);

	G_FreeEdict (bot);
}

int FindBotPing (edict_t *bot)
{
	// The bot should like.. no blaster naaaaa.
	int pings[9] = {40+rand()%34, 35+rand()%40, 35+rand()%35, 45+rand()%35, 40+rand()%40, 30+rand()%50, 45+rand()%45, 25+rand()%30, 50+rand()%15};

	return pings[bot->bot_favwep];
}

void FindBotPersonality (edict_t *bot)
{
//	int favwep;
	int weapons[9] = {SHOTGUN, SUPERSHOTGUN, MACHINEGUN, CHAINGUN, GRENADELAUNCHER, ROCKETLAUNCHER, HYPERBLASTER, RAILGUN, BFG};
	// Okay, first, let's find his favorite weapon.
	// This isn't complicated and is the first step.
	// It'll also determine what ping he has.

	bot->bot_favwep = weapons[rand()%9];

//	gi.dprintf ("%s is fav wep\n", debug_weapnames[bot->bot_favwep]);

	// Okay, so he has a weapon.. now what?
}

void Bot_Create (float delay)
{
	int i;
	char userinfo[MAX_INFO_STRING];
	edict_t *bot = NULL; 
	float r;
	char *name;
	char *gender_;
	char *skin_ = NULL;
	int nName = ( ( float ) rand( ) ) / RAND_MAX * nNameCount;
	int gender_n = ( ( float ) rand( ) ) / RAND_MAX * gendercount;
	int malesk_n = ( ( float ) rand( ) ) / RAND_MAX *  male_skinscount;
	int femalesk_n = ( ( float ) rand( ) ) / RAND_MAX *  female_skinscount;
	int cyborgsk_n = ( ( float ) rand( ) ) / RAND_MAX *  cyborg_skinscount;
	int skin = 0;
	edict_t *timer;
	char *say[8] = {"Hey guys.", "Hello.", "Hey", "Howdy", "Hi.", "G'day humans.", "My friend here?", "Lets go robots, lets go!"};

	if (delay)
	{
		timer = G_Spawn();
		timer->think3 = SpawnAfterDelay;
		timer->nextthink3 = level.time + delay;
		return;
	}

	switch( gender_n )
	{
		case 0:
			skin = ( ( float ) rand( ) ) / RAND_MAX *  male_skinscount;

			gender_ = "male";

			skin_ = malloc(33);
			sprintf(skin_, "%s/%s", gender_, male_skins [malesk_n]);

			break;

		case 1:
			skin = ( ( float ) rand( ) ) / RAND_MAX *  female_skinscount;

			gender_ = "female";

			skin_ = malloc(33);
			sprintf(skin_, "%s/%s", gender_, female_skins [femalesk_n]);

			break;

		case 2:
			skin = ( ( float ) rand( ) ) / RAND_MAX *  cyborg_skinscount;

			gender_ = "cyborg";

			skin_ = malloc(33);
			sprintf(skin_, "%s/%s", gender_, cyborg_skins [cyborgsk_n]);

			break;
	}

	r = random();
	
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;
		if (!bot->inuse)
			break;
	}
	
	if (bot->inuse)
		bot = NULL;
	
	if (bot)
	{

		memset(userinfo, 0, MAX_INFO_STRING);
		
		name = malloc(33);
		sprintf(name, "[Bot]%s", c_szNames [nName]);
		Info_SetValueForKey(userinfo, "name", name); // Random names
		Info_SetValueForKey (userinfo, "password", password->string);
		free(name);


		Info_SetValueForKey(userinfo, "skin", skin_);
		free(skin_);

		// Randomize hand.. full client emulation, you know!
		if (r <= 0.3)
			Info_SetValueForKey(userinfo, "hand", "2");
		else if (r <= 0.5)
			Info_SetValueForKey(userinfo, "hand", "0");
		else
			Info_SetValueForKey(userinfo, "hand", "1");
		
		ClientConnect(bot, userinfo);
		G_InitEdict(bot);
		InitClientResp(bot->client);
		
		if (Bot_Spawn(bot))
		{
		
			gi.WriteByte(svc_muzzleflash);
			gi.WriteShort(bot - g_edicts);
			gi.WriteByte(MZ_LOGIN);
			gi.multicast(bot->s.origin, MULTICAST_PVS);
			
			safe_bprintf(PRINT_HIGH, "%s entered the game\n", bot->client->pers.netname);
			ClientEndServerFrame(bot);

			safe_bprintf (PRINT_CHAT, "%s: %s\n", bot->client->pers.netname, say[rand()%8]);

			// Paril: Let's find the bot's personality.
			FindBotPersonality (bot);
		}
	}
	else
		gi.dprintf("Bot cannot connect - server is full!\n");
}

qboolean Bot_Spawn(edict_t *ent)
{
	vec3_t origin, angles;
	vec3_t mins = {-16, -16, -24};
	vec3_t maxs = {16, 16, 32};
	int i, index;
	client_persistant_t pers; // Note: wrong spelling for "persistent"!
	client_respawn_t resp;
	edict_t *ye;
	gclient_t *client = ent->client;
	int y;
	
	/*if (!deathmatch->value)
	{
		gi.dprintf("Must be in Deathmatch to spawn a bot!\n");
		return false;
	}*/
	
	SelectSpawnPoint(ent, origin, angles);
	
	index = ent - g_edicts - 1;
	
	if (deathmatch->value)
	{
		char userinfo[MAX_INFO_STRING];
		
		resp = ent->client->resp;
		memcpy(userinfo, ent->client->pers.userinfo, MAX_INFO_STRING);
		InitClientPersistant(ent->client);
		ClientUserinfoChanged(ent, userinfo);
	}
	else
		memset(&resp, 0, sizeof(client_respawn_t));
	
	pers = ent->client->pers;
	memset(ent->client, 0, sizeof(gclient_t));
	ent->client->pers = pers;
	ent->client->resp = resp;
	
	FetchClientEntData(ent);
	
	ent->svflags&= ~SVF_NOCLIENT;
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "bot";
	ent->mass = 200;
	ent->gravity = 800;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->think = Bot_Think;
	ent->touch = NULL;
	ent->pain = Bot_Pain;
	ent->die = player_die; 
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->enemy = NULL;
	ent->movetarget = NULL;
	ent->is_bot = true;
	ent->bot_goal = NULL;
	ent->bot_hasgoal = false;
	client->blindBase = 0;
	client->blindTime = 0;
	ent->frozen = 0;
	ent->DrunkTime = 0;
	ent->frozentime = 0;
	ent->client->pers.grenadetype = 1;
	ent->client->resp.awards.awards_base[AWARD_ACCURACY] = 0;
	ent->client->resp.awards.award_needed[AWARD_ACCURACY] = 0;
	ent->client->clones = 0;
	ent->s.renderfx |= RF_IR_VISIBLE;
	ent->burning_framenum = 0;
	ent->bot_playergoal = NULL;

	ye = &g_edicts[(int)maxclients->value+1];
	for ( y=maxclients->value+1 ; y<globals.num_edicts ; y++, ye++)
	{
		if (ye->movetype != MOVETYPE_STUCK)
			continue;

		if (ye->stuckentity == ent)
			G_FreeEdict (ye);
	}

	RemoveInfests(ent);

	VectorCopy(mins, ent->mins);
	VectorCopy(maxs, ent->maxs);
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 32);
	VectorClear(ent->velocity);
	
	memset(&ent->client->ps, 0, sizeof(player_state_t));
	
	for (i = 0; i < 3; i++)
	{
		ent->client->ps.pmove.origin[i] = origin[i] * 8;
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(angles[i] - ent->client->resp.cmd_angles[i]);
	}
	
	ent->client->ps.fov = 90;
	if (!ent->client->pers.weapon)
		ent->client->pers.weapon = FindItem("Blaster");
	ent->client->ps.gunindex = ModelIndex(ent->client->pers.weapon->view_model);
	
	ent->s.effects = 0;
	ent->s.skinnum = index;
	ent->s.modelindex = 255;
	ent->s.modelindex2 = 255;
	ent->s.frame = 0;
	
	VectorCopy(origin, ent->s.origin);
	ent->s.origin[2]++;
	
	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = angles[YAW];
	ent->s.angles[ROLL] = 0;
	
	VectorCopy(ent->s.angles, ent->client->ps.viewangles);
	VectorCopy(ent->s.angles, ent->client->v_angle);
	
	gi.unlinkentity(ent);
	KillBox(ent); // Telefrag!
	gi.linkentity(ent);
	
	ent->client->newweapon = ent->client->pers.weapon;
	ChangeWeapon(ent);
	
	ent->nextthink = level.time + FRAMETIME;

	ent->client->invincible_framenum = level.framenum + 30;

	return true;
}

void Bot_Respawn(edict_t *ent)
{
	CopyToBodyQue(ent);
	
	if (Bot_Spawn(ent))
	{
		ent->s.event = EV_PLAYER_TELEPORT;
		
//		ent->client->ps.pmove.pm_time = 160>>3;		// hold time
//		ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
	}
}

void Bot_AvoidWalls (edict_t *ent)
{
	trace_t tr;
	vec3_t  start;
	vec3_t  forward;
	vec3_t  end;
	
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 64, forward, end);

	tr = gi.trace (start, NULL, NULL, end, ent, MASK_ALL);

	// Onoz, wall!
	if (tr.fraction < 1.0)
	{
		ent->bot_wandertime = -1;
		//gi.dprintf ("Bot ran into wall, turning..\n");
	}

	//gi.WriteByte (svc_temp_entity);
	//gi.WriteByte (TE_DEBUGTRAIL);
	//gi.WritePosition (start);
	//gi.WritePosition (end);
	//gi.multicast (start, MULTICAST_PVS);
}

qboolean Bot_CanMove(edict_t *self, int direction, usercmd_t *cmd);

void Bot_Wander (edict_t *ent, usercmd_t *cmd, vec3_t angles)
{
	if (cmd->upmove)
	{
		ent->bot_wandertime = level.time + 2;
		return;
	}

	if (ent->bot_wandertime <= level.time)
	{
		int r = rndnum(0, 360);
		int best_angle;

		best_angle = r;

		if (r > 270)
			best_angle = 90;
		else if (r > 180)
			best_angle = 270;
		else if (r > 90)
			best_angle = 90;
		else
			best_angle = 180;

		// [End Bad Way To Calculate Angles]
		ent->s.angles[1] += best_angle;
		VectorCopy(ent->s.angles, ent->client->ps.viewangles);
		VectorCopy(ent->s.angles, ent->client->v_angle);  // Angles
		VectorCopy (ent->s.angles, angles);
		
		//gi.dprintf ("Called ent->client->resp.nextturn thing, best angle is %i\n", best_angle);
		ent->bot_wandertime = level.time + 4;

	}
	if (Bot_CanMove(ent, 3, cmd))
	{
		cmd->forwardmove = BOT_MOVESPEED;
		// Randomly jump
		if (ent->waterlevel < 1)
		{
			if (random() < 0.3)
				cmd->upmove = 400;
		}
		else
		//if (ent->waterlevel)
			cmd->upmove = 400;
	}
	else
		ent->bot_wandertime = 0;

	Bot_AvoidWalls (ent);
}

// Paril, this code is used to find a bot's favorite weapon.
// The bot will keep going until he gets his favorite weapon.
qboolean Pickup_Health (edict_t *ent, edict_t *other);
void Bot_FindFavoriteWeapon (edict_t *ent)
{
	edict_t *blip = NULL;
	qboolean goal = ent->bot_hasgoal;
	int already = 0;
	int range = 3500;
	gitem_t *items[9] = {FindItem("Shotgun"), FindItem("Super Shotgun"), FindItem("Machinegun"), FindItem("Chaingun"), FindItem("Grenade Launcher"), FindItem("Rocket Launcher"), FindItem("Hyperblaster"), FindItem("Railgun"), FindItem("BFG10k")};

	if (goal == false)
	{
		while (blip = findradius(blip, ent->s.origin, range))
		{
			if (!blip->item)
				continue;
			if (!visible(ent, blip))
				continue;
			if (!infront(ent, blip))
				continue;
			// Check to see if you're on the same height
			// FIXME: Maybe we can make the bots jump to a 12 unit object..
			/*if (ent->s.origin[2] != blip->s.origin[2])
			{
				float dif = (blip->s.origin[2] - ent->s.origin[2]);

				if (dif < 50 && dif > 19)
				{
					//gi.dprintf ("Up! 2\n");
				}
				else
					continue;
			}*/
			if (blip->item != items[ent->bot_favwep])
				continue;
			if ((int)dmflags->value & DF_WEAPONS_STAY)
			{
				int n;
				gitem_t *it = itemlist;
//				gitem_t *i;
//				int old;

				for (n = 0; n < game.num_items; n++, it++)
				{
					// Only weapons can stay.
					if (!blip->item->flags & IT_WEAPON)
						continue;
					if (ent->client->pers.inventory[ITEM_INDEX(FindItem(blip->item->pickup_name))] > 0)
					{
						already = 1;
						goto sh;
					}
				}				
			}
sh:
			if (already)
				continue;

			ent->bot_goal = blip;
			ent->bot_hasgoal = true;
//			gi.dprintf ("favorite weapon found: %s\n", blip->classname);
		}
	}
}

// Paril, this code is used to find a short-range object such as ammo or items.
// This will change his ent->bot_goal to the right object.
// Note: Needs to be balanced for max rounds, etc.
qboolean Pickup_Health (edict_t *ent, edict_t *other);
void Bot_FindShortRangeObject (edict_t *ent)
{
	edict_t *blip = NULL;
	qboolean goal = ent->bot_hasgoal;
	int already = 0;
	int range = 1200;
	int infested = 0;

	if (goal == false)
	{
		while (blip = findradius(blip, ent->s.origin, range))
		{
			if (!blip->item)
				continue;
			if (!visible(ent, blip))
				continue;
			if (!infront(ent, blip))
				continue;
			// Check to see if you're on the same height
			// FIXME: Maybe we can make the bots jump to a 12 unit object..
			if (ent->s.origin[2] != blip->s.origin[2])
			{
				float dif = (ent->s.origin[2] - blip->s.origin[2]);

				if (dif < 50 && dif > 19)
				{
					//gi.dprintf ("Up! 2\n");
				}
				else
					continue;
			}
			// Desperately find health to fix infestation
			if (Q_stricmp(blip->item->pickup_name, "item_health_large") == 0 && ent->health > 99)
			{
					edict_t *yuck;
					for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) 
					{
						if (!yuck->inuse)
							continue;
						if (yuck->infester && yuck->target_ent == ent) 
						{
							infested = 1;
						}
					}

					if (infested)
						goto sh;
					else
						continue;
			}
			if (blip->item->pickup != Pickup_Health)
			{
				edict_t *yuck;
				for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) 
				{
					if (!yuck->inuse)
						continue;
					if (yuck->infester && yuck->target_ent == ent) 
					{
						infested = 1;
					}
				}
				
				if (infested)
					continue;
			}
			if ((int)dmflags->value & DF_WEAPONS_STAY)
			{
				int n;
				gitem_t *it = itemlist;
//				gitem_t *i;
//				int old;

				for (n = 0; n < game.num_items; n++, it++)
				{
					// Only weapons can stay.
					if (!blip->item->flags & IT_WEAPON)
						continue;
					if (ent->client->pers.inventory[ITEM_INDEX(FindItem(blip->item->pickup_name))] > 0)
					{
						already = 1;
						goto sh;
					}
				}				
			}
sh:
			if (already)
				continue;

			ent->bot_goal = blip;
			ent->bot_hasgoal = true;
			//gi.dprintf ("bot_goal found: %s at %s\n", blip->classname, vtos(blip->s.origin));
		}
	}
}

// Priority List:
/*Hyperblaster
Railgun
Rocket Launcher
Grenade Launcher
Grenades
Super Shotgun
Shotgun
Machinegun
BFG10k
Chaingun
Blaster*/
float GetDistanceBetweenTwoEntities (edict_t *one, edict_t *two)
{
	vec3_t temp;

	VectorSubtract (one->s.origin, two->s.origin, temp);
	return VectorLength (temp);
}

// This function finds the correct weapon
// for the bot to use while fighting. This calls
// for desperate calls. See "Bot_UseWeapon" for
// when he is wandering/goaling
qboolean Bot_UsingWeapon (edict_t *ent, char *weapname)
{
	if (ent->client->pers.weapon == FindItem(weapname))
		return true;
	else
		return false;
}

void Bot_FindAndUseCorrectWeapon (edict_t *ent)
{
	char *currentweap = ent->client->pers.weapon->pickup_name;
	float len;

	// Run!
	if (!ent->enemy)
	{
		gi.dprintf ("Bot tried to use a weapon with no enemy (Bot_FindAndUseCorrectWeapon)\n");
		return;
	}

	len = GetDistanceBetweenTwoEntities (ent, ent->enemy);

//	gi.dprintf ("%f\n", len);

	// Very close range
	if (len <= 80)
	{
		// Nothing else suitable for meleeing, use the chainfist.
		if (Q_stricmp(currentweap, "Chainfist") == 1)
			ent->client->newweapon = FindItem ("Chainfist");
		return;
	}
	else if (len <= 500)
	{
		// Check for SSG and if we're not already using it
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Super Shotgun"))] > 0 && !Bot_UsingWeapon(ent, "Super Shotgun"))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Super Shotgun");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Super Shotgun"))] > 0 && Bot_UsingWeapon(ent, "Super Shotgun"))
			return;

		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Shotgun"))] > 0 && (!Bot_UsingWeapon(ent, "Shotgun") && !Bot_UsingWeapon(ent, "Super Shotgun")))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Shotgun");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Shotgun"))] > 0 && ent->client->pers.weapon == FindItem("Shotgun"))
			return;
	}
	// HB, Chaingun
	else if (len <= 1000)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Hyperblaster"))] > 0 && !Bot_UsingWeapon(ent, "Hyperblaster"))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Hyperblaster");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Hyperblaster"))] > 0 && Bot_UsingWeapon(ent, "Hyperblaster"))
			return;

		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Chaingun"))] > 0 && (!Bot_UsingWeapon(ent, "Chaingun") && !Bot_UsingWeapon(ent, "Hyperblaster")))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Chaingun");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Chaingun"))] > 0 && Bot_UsingWeapon(ent, "Chaingun"))
			return;
	}
	else if (len <= 1500)
	{
		// Mid-range. This is good for grenades, etc.
		// Fixme: Need to fix up their grenading.
		// Right now they kill themselves with grenades.
		
		// Check for BFG and if we're not already using it
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("BFG10k"))] > 0 && !Bot_UsingWeapon(ent, "Bfg10k"))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("BFG10k");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("BFG10k"))] > 0 && Bot_UsingWeapon(ent, "Bfg10k"))
			return;
		
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] > 0 && (!Bot_UsingWeapon(ent, "Bfg10k") && !Bot_UsingWeapon(ent, "Rocket Launcher")))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Rocket Launcher");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] > 0 && Bot_UsingWeapon(ent, "Rocket Launcher"))
			return;

		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenade Launcher"))] > 0 && (!Bot_UsingWeapon(ent, "Bfg10k") && !Bot_UsingWeapon(ent, "Rocket Launcher") && !Bot_UsingWeapon(ent, "Grenade Launcher")))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Grenade Launcher");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenade Launcher"))] > 0 && Bot_UsingWeapon(ent, "Grenade Launcher"))
			return;

		/*
		Paril: FIXME UP
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] > 0 && ent->client->pers.weapon != FindItem("Grenades"))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Grenades");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] > 0 && Bot_UsingWeapon(ent, "Grenades"))
			return;
		*/

		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Machinegun"))] > 0 && (!Bot_UsingWeapon(ent, "Bfg10k") && !Bot_UsingWeapon(ent, "Rocket Launcher") && !Bot_UsingWeapon(ent, "Grenade Launcher") && !Bot_UsingWeapon(ent, "Machinegun")))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Machinegun");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Machinegun"))] > 0 && Bot_UsingWeapon(ent, "Machinegun"))
			return;

	}
	// Farthest range, use Rail.
	else
	{
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Railgun"))] > 0 && !Bot_UsingWeapon(ent, "Railgun"))
		{
			// We have one and we're not using it, let's use it.
			ent->client->newweapon = FindItem ("Railgun");
			return;
		}

		// Already using
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Railgun"))] > 0 && Bot_UsingWeapon(ent, "Railgun"))
			return;
	}
}

void Bot_EvadeGrenades (edict_t *ent, usercmd_t *cmd)
{
	edict_t *blip = NULL;
	int range = 250;
	float r = random();

	while (blip = findradius(blip, ent->s.origin, range))
	{
		// Only evade grenades he can see and is in front of.
		// If he's turned around, we assume he's running in the opposite direction.
		// Note: If the bot is looking for a long range goal, he MIGHT be turned in the opposite
		// direction, or is running towards target backwards. This can cause a few problems...
		// if (ent->bot_longgoal)
		//	continue;
		if (!visible(ent, blip))
			continue;
		if (!infront(ent, blip))
			continue;
		if (Q_stricmp(blip->classname, "grenade") && Q_stricmp(blip->classname, "hgrenade") && Q_stricmp(blip->classname, "trap") && Q_stricmp(blip->classname, "tesla"))
			continue;
		if (blip->owner == ent)
			continue;

		// So, a grenade is in our area.
		// Screw this, let's run.

		//gi.dprintf ("Bot Grenade Evade!\n");
		// Let's try to walk around it
		// Confuse players by randomizing directions for each grenade.
		if (r < 0.5)
			cmd->sidemove = BOT_MOVESPEED;
		else
			cmd->sidemove = -BOT_MOVESPEED;

		cmd->forwardmove = -BOT_MOVESPEED;
	}
}

void Bot_EvadeProjectiles (edict_t *ent, usercmd_t *cmd)
{
	edict_t *blip = NULL;
	int range = 400;
	float ra = random();
	float r = random();

	while (blip = findradius(blip, ent->s.origin, range))
	{
		// Only evade grenades he can see and is in front of.
		// If he's turned around, we assume he's running in the opposite direction.
		// Note: If the bot is looking for a long range goal, he MIGHT be turned in the opposite
		// direction, or is running towards target backwards. This can cause a few problems...
		// if (ent->bot_longgoal)
		//	continue;
		if (!visible(ent, blip))
			continue;
		if (!infront(ent, blip))
			continue;
		if (blip->movetype != MOVETYPE_FLYMISSILE && !blip->can_teleport_through_destinations)
			continue;
		if (!Q_stricmp(blip->classname, "hook") || !Q_stricmp(blip->classname, "class"))
			continue;
		if (blip->owner == ent)
			continue;

		// So, a grenade is in our area.
		// Screw this, let's run.

		//gi.dprintf ("Bot Grenade Evade!\n");
		// Let's try to walk around it
		// Confuse players by randomizing directions for each grenade.
		if (r < 0.5)
			cmd->sidemove = 400;
		else
			cmd->sidemove = -400;

		if (ra < 0.7)
			cmd->upmove = -400;
		else
			cmd->upmove = 400;
	}
}

void Bot_EvadeStuff (edict_t *ent, usercmd_t *cmd)
{
	Bot_EvadeGrenades (ent, cmd);
	Bot_EvadeProjectiles (ent, cmd);
}

///////////////////////////////////////////////////////////////////////
// Checks if bot can move (really just checking the ground)
// Also, this is not a real accurate check, but does a
// pretty good job and looks for lava/slime. 
///////////////////////////////////////////////////////////////////////
// Paril, from Ace Source
qboolean Bot_CheckJumpLocation (edict_t *self)
{
	vec3_t forward, right;
	vec3_t offset,start,end;
	vec3_t end2;
	vec3_t angles;
	trace_t tr;
	trace_t tr2;
	
	// Now check to see if move will move us off an edge
	VectorCopy(self->s.angles,angles);

	// Set up the vectors
	AngleVectors (angles, forward, right, NULL);
	
	VectorCopy (self->s.origin, start);
		
	VectorSet(offset, 200, 0, 0);
	G_ProjectSource (self->s.origin, offset, forward, right, end);

	//end[2] -= 10;
	tr = gi.trace(self->s.origin, NULL, NULL, end, self, MASK_ALL);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (end);
	gi.multicast (start, MULTICAST_PVS);

	if(tr.contents & CONTENTS_SOLID || gi.pointcontents(end) == CONTENTS_SOLID)
		return false;

	VectorSet (offset, 0, 0, -50); // 50 units should be fine, otherwise they can just walk over it.
	G_ProjectSource (end, offset, forward, right, end2);

	tr2 = gi.trace(end, NULL, NULL, end, self, MASK_ALL);

	//end2[2] -= 10;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (end);
	gi.WritePosition (end2);
	gi.multicast (start, MULTICAST_PVS);

	if(tr2.contents & CONTENTS_SOLID || gi.pointcontents(end2) == CONTENTS_SOLID)
		return true;
	else
		return false;
}

void Bot_JumpOverLocation (edict_t *self, usercmd_t *cmd)
{
	cmd->upmove = 400;
}

qboolean Bot_CanMove(edict_t *self, int direction, usercmd_t *cmd)
{
	vec3_t forward, right;
	vec3_t offset,start,end;
	vec3_t angles;
	trace_t tr;

	// Now check to see if move will move us off an edge
	VectorCopy(self->s.angles,angles);
	
	if(direction == 0)
		angles[1] += 90;
	else if(direction == 1)
		angles[1] -= 90;
	else if(direction == 2)
		angles[1] -=180;

	// Set up the vectors
	AngleVectors (angles, forward, right, NULL);
	
	VectorSet(offset, 36, 0, 24);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
		
	VectorSet(offset, 36, 0, -100); // RiEvEr reduced drop distance
	G_ProjectSource (self->s.origin, offset, forward, right, end);
	
	//AQ2 ADDED MASK_SOLID
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID|MASK_OPAQUE);
	
	if(tr.contents & (CONTENTS_LAVA|CONTENTS_SLIME))
	{
		if (Bot_CheckJumpLocation (self))
		{
			//gi.dprintf ("Move blocked, but he made it over!\n");
			Bot_JumpOverLocation(self, cmd);
		}
		else
		{
			//gi.dprintf("%s: move blocked\n",self->client->pers.netname);
			return false;
		}
	}
	
	return true; // yup, can move
}
edict_t *Bot_FindFriendly (edict_t *ent);

void Bot_Think(edict_t *ent)
{
	usercmd_t cmd;
	vec3_t angles = { 0, 0, 0 };
	vec3_t mins = {-16, -16, -24};
	vec3_t maxs = {16, 16, 32};

	VectorCopy(ent->client->v_angle, angles);
	VectorSet(ent->client->ps.pmove.delta_angles, 0, 0, 0);
	memset(&cmd, 0, sizeof(usercmd_t));

	//ent->client->pers.weapon = FindItem("Grenades");
	
	if (ent->deadflag == DEAD_DEAD || ent->health < 1)
	{
		ent->client->buttons = 0;
		cmd.buttons = BUTTON_ATTACK;
	}
	else if (ent->enemy)
	{
		if (infront(ent, ent->enemy) && visible(ent, ent->enemy))
		{

			Bot_FindAndUseCorrectWeapon (ent);

			if (visible(ent, ent->enemy))
			{
				//if (range(ent, ent->enemy) == RANGE_MELEE && random() < 0.7)
				//	Bot_Attack(ent, &cmd, angles); // Attack enemy!
				if (Bot_CheckAttack(ent, &cmd))
				{
					if (ent->bot_usingautomatic)
						Bot_Attack(ent, &cmd, angles);
					// Not an automatic weapon, so we need a random.
					else
					{
						if (random() > 0.5)
							Bot_Attack(ent, &cmd, angles); // Attack enemy!
					}
				}
			}
		}
		else
			ent->enemy = NULL;

	}
	// Enemies take priority over goals.
	// Long range objects are an exception: they will
	// go towards the object while aiming at enemy.
	else if (ent->bot_hasgoal)
	{
		// The item went away.
		if (!ent->bot_goal->item || !ent->bot_goal->item->world_model)
		{
			ent->bot_goal = NULL;
			ent->bot_hasgoal = false;
			goto reset;
		}

		// Check to see if you're on the same height
		// FIXME: Maybe we can make the bots jump to a 12 unit object..
		/*if (ent->s.origin[2] != ent->bot_goal->s.origin[2])
		{
			float dif = ent->bot_goal->s.origin[2] - ent->s.origin[2];
				//gi.dprintf ("%f\n", dif);
			if (dif < 50 && dif > 19)
			{
				//gi.dprintf ("Up! 1\n");
				cmd.upmove = 400;
			}
			else
			{
				ent->bot_goal = NULL;
				ent->bot_hasgoal = false;
				goto reset;
			}
		}*/

		// This thing.

		if ((int)dmflags->value & DF_WEAPONS_STAY)
		{
			int n;
			gitem_t *it = itemlist;
			//				gitem_t *i;
			//				int old;
			int already = 0;
			edict_t *blip = ent->bot_goal;
			
			for (n = 0; n < game.num_items; n++, it++)
			{
				// Only weapons can stay.
				if (!blip->item->flags & IT_WEAPON)
					continue;
				if (ent->client->pers.inventory[ITEM_INDEX(FindItem(blip->item->pickup_name))] > 0)
				{
					already = 1;
				}
			}
			
			if (already)
			{
				// Already have. Skip.
				ent->bot_goal = NULL;
				ent->bot_hasgoal = false;
				goto reset;
			}
		}


		// Run towards goal
		Bot_Move (ent, ent->bot_goal, &cmd, angles);
		angles[PITCH] = 0;
		angles[ROLL] = 0;

		// Still find enemies
		ent->enemy = Bot_FindEnemy(ent);
	}
	else
	{
		ent->enemy = Bot_FindEnemy(ent);
	
		if (!ent->bot_playergoal)
			Bot_Wander(ent, &cmd, angles);

		if (!ent->bot_hasgoal)
		{
			Bot_FindShortRangeObject (ent);
			Bot_FindFavoriteWeapon (ent);
		}

		// Friendly code
		// bots follow players.
		// Enemies and items take priority, though.
		if ((coop->value || iwm_gamemode->value == 2))
		{
			if (ent->bot_playergoal != NULL)
			{
				vec3_t d;
				float f;

				if (ent->bot_playergoal->deadflag == DEAD_DEAD)
				{
					ent->bot_playergoal = NULL;
					goto reset;
				}

				// Stay close enough behind the player.
				VectorSubtract (ent->s.origin, ent->bot_playergoal->s.origin, d);
				f = VectorLength (d);

				if (f > 100)
					Bot_Move (ent, ent->bot_playergoal, &cmd, angles);

				if (ent->bot_playergoal->client && ent->bot_playergoal->client->ucmd.upmove > 0)
					cmd.upmove = 400;
			}
			else
				ent->bot_playergoal = Bot_FindFriendly(ent);
		}
	}

reset:

	cmd.msec = 100;
	
	cmd.angles[PITCH] = ANGLE2SHORT(angles[PITCH]);
	cmd.angles[YAW] = ANGLE2SHORT(angles[YAW]);
	cmd.angles[ROLL] = ANGLE2SHORT(angles[ROLL]);

	Bot_EvadeStuff (ent, &cmd);
	
	ClientThink(ent, &cmd);
	
	ent->nextthink = level.time + FRAMETIME;

	//if (gi.pointcontents(ent->s.origin) & CONTENTS_SOLID)
	//{
	//	gi.dprintf ("Bot is in solid, killing...\n");
	//	Cmd_Kill_f (ent);
	//}

	/*if (lightlevel < 'l')
	{
		if (!ent->has_flashlight)
			FL_make(ent);
	}
	else
	{
		if (ent->has_flashlight)
			FL_make(ent);
	}*/
	// Just incase..

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	gi.linkentity(ent);
}

void Bot_Aim(edict_t *ent, edict_t *target, vec3_t angles)
{
	vec3_t dir, start, end;
	
	VectorCopy(target->s.origin, start);
	VectorCopy(ent->s.origin, end);
	if (target && target->client && target->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		start[2] -= 18;
	}
	VectorSubtract(start, end, dir);
	vectoangles(dir, angles);

	if (ent->DrunkTime > level.time || ent->client->blindTime > 0)
	{
		angles[1] += (rand()%45 - rand()%45);
		angles[0] += (rand()%45 - rand()%45);
	}
}

// Checks if the bot can attack.
// Mainly a trace to see if a friendly is in the way.

qboolean Bot_CheckAttack (edict_t *ent, usercmd_t *cmd)
{
	vec3_t		start, end;
	vec3_t		forward, right;
	vec3_t		angles;
	vec3_t		offset;
	trace_t tr;

	// get start / end positions
	VectorCopy (ent->client->v_angle, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorMA (start, 700, forward, end);

	tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);

	if (((coop->value || iwm_gamemode->value == 2 || (ctf->value && (ent->client->resp.ctf_team == tr.ent->client->resp.ctf_team)))) && tr.ent->client)
	{
//		gi.dprintf ("Entity was in the way.\n");
		// Try to move around the object that's blocking him from shooting
		cmd->sidemove = 400;
		return false;
	}
	else
		return true;
}

// Hit them with your best shot! Fire away!
void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles)
{
	if (ent->enemy->deadflag == DEAD_DEAD)
		ent->enemy = NULL;
	else
	{
		if (infront(ent, ent->enemy))
		{
			Bot_Move(ent, ent->enemy, cmd, angles);		

			//if (random() < 0.3) // Don't fire too often!
				cmd->buttons = BUTTON_ATTACK;
		}
		else
			ent->enemy = NULL;
	}
}

void Bot_Pain(edict_t *ent, edict_t *other, float kickback, int damage)
{
	int infested = 0;
	if (other && ent != other && other->takedamage && other->client)
	{
		edict_t *yuck;
		for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) 
		{
			if (!yuck->inuse)
				continue;
			if (yuck->infester && yuck->target_ent == ent) 
			{
				infested = 1;
			}
		}

		if (!infested)
		{
			// Ignore accidental hits.
			if (iwm_gamemode->value == 2 || coop->value)
			{
				if (other->client)
					goto skip;
			}
			ent->oldenemy = ent->enemy;
			ent->enemy = other;
			Bot_Aim (ent, ent->enemy, ent->client->v_angle);
		}
	}
skip:
	player_pain(ent, other, kickback, damage);
}

void Bot_Move(edict_t *ent, edict_t *goal, usercmd_t *cmd, vec3_t angles)
{
	Bot_Aim(ent, goal, angles);
	cmd->forwardmove += BOT_MOVESPEED; // Walk speed
}

// Used in CTF for finding friendly players
// , following captures and COOP.
edict_t *Bot_FindFriendly (edict_t *ent)
{
	int range = 4000;
	char *classname;
	edict_t *enemy = ent->bot_playergoal, *newenemy = NULL;
	
	if (enemy == NULL)
	{
		classname = "player";
		
		while ((newenemy = findradius(newenemy, ent->s.origin, range)) != NULL)
		{
			if (!newenemy->client)
				continue;
			if (newenemy->is_bot)
				continue;
			if (!newenemy->takedamage)
				continue;
			if (newenemy->flags & FL_NOTARGET)
				continue;
			
			// Don't fight self or mess with the dead or invisible
			if ((newenemy != ent) && (newenemy->deadflag != DEAD_DEAD))// && (pxE->light_level > 5))
			{
				if (visible(ent, newenemy))
				{
					if (infront(ent, newenemy))
					{
						enemy = newenemy;
						break;
					}
				}
			}
		}
	}
	
	return enemy;
}

// Might need to change one day to support something
edict_t *Bot_FindMonster(edict_t *ent)
{
	int range = 4000;
	edict_t *enemy = ent->enemy, *newenemy = NULL;
	
	if (enemy == NULL)
	{
		
		while ((newenemy = findradius(newenemy, ent->s.origin, range)) != NULL)
		{
			if (newenemy->client)
				continue;
			if (!newenemy->takedamage)
				continue;
			if (newenemy->flags & FL_NOTARGET)
				continue;
			if (!newenemy->svflags & SVF_MONSTER)
				continue;
			if (Q_strncasecmp(newenemy->classname, "monster_", 8))
				continue;
			
			// Don't fight self or mess with the dead or invisible
			if ((newenemy != ent) && (newenemy->deadflag != DEAD_DEAD))// && (pxE->light_level > 5))
			{
				if (visible(ent, newenemy))
				{
					if (infront(ent, newenemy))
					{
						enemy = newenemy;
						if (ent->bot_hasgoal) // Remove item goal.
						{
							ent->bot_goal = NULL;
							ent->bot_hasgoal = false;
						}
						break;
					}
				}
			}
		}
	}
	
	return enemy;
}

edict_t *Bot_FindEnemy(edict_t *ent)
{
	int range = 4000;
	char *classname;
	edict_t *enemy = ent->enemy, *newenemy = NULL;
	
	if (iwm_gamemode->value == 2 || coop->value)
	{
		return Bot_FindMonster (ent);
	}

	if (enemy == NULL)
	{
		classname = "player";
		
		while ((newenemy = findradius(newenemy, ent->s.origin, range)) != NULL)
		{
			if (!newenemy->client)
				continue;
			if (!newenemy->takedamage)
				continue;
			if (newenemy->flags & FL_NOTARGET)
				continue;
			
			// Don't fight self or mess with the dead or invisible
			if ((newenemy != ent) && (newenemy->deadflag != DEAD_DEAD))// && (pxE->light_level > 5))
			{
				if (visible(ent, newenemy))
				{
					if (infront(ent, newenemy))
					{
						enemy = newenemy;
						if (ent->bot_hasgoal) // Remove item goal.
						{
							ent->bot_goal = NULL;
							ent->bot_hasgoal = false;
						}
						break;
					}
				}
			}
		}
	}
	
	return enemy;
}

///////////////////////////////////////////////////////////////////////
// These routines are bot safe print routines, all id code needs to be 
// changed to these so the bots do not blow up on messages sent to them. 
// Do a find and replace on all code that matches the below criteria. 
//
// (Got the basic idea from Ridah)
//	
//  change: gi.cprintf to safe_cprintf
//  change: gi.bprintf to safe_bprintf
//  change: gi.centerprintf to safe_centerprintf
// 
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Debug print, could add a "logging" feature to print to a file
///////////////////////////////////////////////////////////////////////
void debug_printf(char *fmt, ...)
{
	int     i;
	char	bigbuffer[0x10000];
	int		len;
	va_list	argptr;
	edict_t	*cl_ent;
	
	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_MEDIUM, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->is_bot)
			continue;

		gi.cprintf(cl_ent,  PRINT_MEDIUM, bigbuffer);
	}

}

///////////////////////////////////////////////////////////////////////
// botsafe cprintf
///////////////////////////////////////////////////////////////////////
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...)
{
	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (ent && (!ent->inuse || ent->is_bot))
		return;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.cprintf(ent, printlevel, bigbuffer);
	
}

///////////////////////////////////////////////////////////////////////
// botsafe centerprintf
///////////////////////////////////////////////////////////////////////
void safe_centerprintf (edict_t *ent, char *fmt, ...)
{
	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (!ent->inuse || ent->is_bot)
		return;
	
	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);
	
	gi.centerprintf(ent, bigbuffer);
	
}
///////////////////////////////////////////////////////////////////////
// botsafe bprintf
///////////////////////////////////////////////////////////////////////
void safe_bprintf (int printlevel, char *fmt, ...)
{
	int i;
	char	bigbuffer[0x10000];
	int		len;
	va_list		argptr;
	edict_t	*cl_ent;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (dedicated->value)
		gi.cprintf(NULL, printlevel, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->is_bot)
			continue;

		gi.cprintf(cl_ent, printlevel, bigbuffer);
	}

	Log_Print (bigbuffer);
}
