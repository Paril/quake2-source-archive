
#include "g_local.h"

#define Function(f) {#f, f}

mmove_t mmove_reloc;

field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"model", FOFS(model), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"accel", FOFS(accel), F_FLOAT},
	{"decel", FOFS(decel), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"pathtarget", FOFS(pathtarget), F_LSTRING},
	{"deathtarget", FOFS(deathtarget), F_LSTRING},
	{"killtarget", FOFS(killtarget), F_LSTRING},
	{"combattarget", FOFS(combattarget), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"move_origin", FOFS(move_origin), F_VECTOR},
	{"move_angles", FOFS(move_angles), F_VECTOR},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},

	{"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
	{"movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN},
	{"enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN},
	{"oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN},
	{"activator", FOFS(activator), F_EDICT, FFL_NOSPAWN},
	{"groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN},
	{"teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN},
	{"teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN},
	{"owner", FOFS(owner), F_EDICT, FFL_NOSPAWN},
	{"mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN},
	{"mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN},
	{"target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN},
	{"chain", FOFS(chain), F_EDICT, FFL_NOSPAWN},

	{"prethink", FOFS(prethink), F_FUNCTION, FFL_NOSPAWN},
	{"think", FOFS(think), F_FUNCTION, FFL_NOSPAWN},
	{"blocked", FOFS(blocked), F_FUNCTION, FFL_NOSPAWN},
	{"touch", FOFS(touch), F_FUNCTION, FFL_NOSPAWN},
	{"use", FOFS(use), F_FUNCTION, FFL_NOSPAWN},
	{"pain", FOFS(pain), F_FUNCTION, FFL_NOSPAWN},
	{"die", FOFS(die), F_FUNCTION, FFL_NOSPAWN},

	{"stand", FOFS(monsterinfo.stand), F_FUNCTION, FFL_NOSPAWN},
	{"idle", FOFS(monsterinfo.idle), F_FUNCTION, FFL_NOSPAWN},
	{"search", FOFS(monsterinfo.search), F_FUNCTION, FFL_NOSPAWN},
	{"walk", FOFS(monsterinfo.walk), F_FUNCTION, FFL_NOSPAWN},
	{"run", FOFS(monsterinfo.run), F_FUNCTION, FFL_NOSPAWN},
	{"dodge", FOFS(monsterinfo.dodge), F_FUNCTION, FFL_NOSPAWN},
	{"attack", FOFS(monsterinfo.attack), F_FUNCTION, FFL_NOSPAWN},
	{"melee", FOFS(monsterinfo.melee), F_FUNCTION, FFL_NOSPAWN},
	{"sight", FOFS(monsterinfo.sight), F_FUNCTION, FFL_NOSPAWN},
	{"checkattack", FOFS(monsterinfo.checkattack), F_FUNCTION, FFL_NOSPAWN},
	{"currentmove", FOFS(monsterinfo.currentmove), F_MMOVE, FFL_NOSPAWN},

	{"endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN},

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},

//need for item field in edict struct, FFL_SPAWNTEMP item will be skipped on saves
	{"item", FOFS(item), F_ITEM},

	{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},

	{0, 0, 0, 0}

};

field_t		levelfields[] =
{
	{"changemap", LLOFS(changemap), F_LSTRING},
                   
	{"sight_client", LLOFS(sight_client), F_EDICT},
	{"sight_entity", LLOFS(sight_entity), F_EDICT},
	{"sound_entity", LLOFS(sound_entity), F_EDICT},
	{"sound2_entity", LLOFS(sound2_entity), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		clientfields[] =
{
	{"pers.weapon", CLOFS(pers.weapon), F_ITEM},
	{"pers.lastweapon", CLOFS(pers.lastweapon), F_ITEM},
	{"newweapon", CLOFS(newweapon), F_ITEM},

	{NULL, 0, F_INT}
};

cvar_t *zk_logonly;		// Z-bot detection

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
void InitGame (void)
{
	char	tempstr[50];
	int		i;

//	debugmsg("InitGame\n");
	zk_logonly = gi.cvar("zk_logonly", "0", CVAR_ARCHIVE);	// Z-bot detection

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar ("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	gamename = gi.cvar ("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	maxspectators = gi.cvar ("maxspectators", "4", CVAR_SERVERINFO);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	skill = gi.cvar ("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", "1024", CVAR_LATCH);

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar ("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar ("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar ("needpass", "0", CVAR_SERVERINFO);
	filterban = gi.cvar ("filterban", "1", 0);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", 0);

	// Gunslinger Quake2 cvars -- Stone
	playmode = gi.cvar ("playmode", "1", CVAR_SERVERINFO | CVAR_LATCH);
	teamplay = gi.cvar ("teamplay", "0", CVAR_SERVERINFO | CVAR_LATCH);
	team_damage = gi.cvar ("team_damage", "0", 0);
	clear_teams = gi.cvar ("clear_teams", "0", 0);
	number_of_teams = gi.cvar ("number_of_teams", "2", CVAR_SERVERINFO | CVAR_LATCH);
	teamdata[0].name = gi.cvar ("team1_name", "Team 1", 0);
	teamdata[1].name = gi.cvar ("team2_name", "Team 2", 0);
	teamdata[2].name = gi.cvar ("team3_name", "Team 3", 0);
	teamdata[3].name = gi.cvar ("team4_name", "Team 4", 0);
	force_join = gi.cvar ("force_join", "0", CVAR_LATCH);
	use_ctf_skins = gi.cvar ("use_ctf_skins", "0", CVAR_LATCH);
	drop_badge = gi.cvar ("drop_badge", "0", 0);
	drop_bag = gi.cvar ("drop_bag", "1", 0);
	ready_all = gi.cvar ("ready_all", "0", CVAR_LATCH);
	use_classes = gi.cvar ("use_classes", "1", CVAR_SERVERINFO | CVAR_LATCH);
	announce_deaths = gi.cvar ("announce_deaths", "0", 0);
	respawn_invuln = gi.cvar ("respawn_invuln", "1", 0);
	weapons_unique = gi.cvar ("weapons_unique", "0", CVAR_LATCH);
	drop_all_on_death = gi.cvar ("drop_all_on_death", "", 0 | CVAR_LATCH);
	bullet_ricochet = gi.cvar ("bullet_ricochet", "1", 0);
	item_weight = gi.cvar ("item_weight", "0", CVAR_SERVERINFO);
	hit_locations = gi.cvar ("hit_locations", "1", 0);
	starting_cash = gi.cvar ("starting_cash", "190", CVAR_SERVERINFO);
	artifact_min_timeout = gi.cvar ("artifact_min_timeout", "0", 0);
	artifact_max_timeout = gi.cvar ("artifact_max_timeout", "0", 0);
	artifact_glow = gi.cvar ("artifact_glow", "0", 0 | CVAR_LATCH);

	allow_hands_of_lightning = gi.cvar ("allow_hands_of_lightning", "1", CVAR_LATCH);
	allow_strength_of_the_bear = gi.cvar ("allow_strength_of_the_bear", "1", CVAR_LATCH);
	allow_flesh_of_the_salamander = gi.cvar ("allow_flesh_of_the_salamander", "1", CVAR_LATCH);
	allow_aura_of_pain = gi.cvar ("allow_aura_of_pain", "1", CVAR_LATCH);
	allow_tongue_of_the_leech = gi.cvar ("allow_tongue_of_the_leech", "1", CVAR_LATCH);
	allow_tail_of_the_scorpion = gi.cvar ("allow_tail_of_the_scorpion", "1", CVAR_LATCH);
	allow_shroud_of_darkness = gi.cvar ("allow_shroud_of_darkness", "1", CVAR_LATCH);

	allow_cannon = gi.cvar ("allow_cannon", "1", CVAR_LATCH);
	allow_sheriff_badge = gi.cvar ("allow_sheriff_badge", "1", CVAR_LATCH);
	allow_marshal_badge = gi.cvar ("allow_marshal_badge", "1", CVAR_LATCH);
	allow_bandolier = gi.cvar ("allow_bandolier", "1", CVAR_LATCH);
	allow_pack = gi.cvar ("allow_pack", "1", CVAR_LATCH);
	allow_lantern = gi.cvar ("allow_lantern", "1", CVAR_LATCH);
	allow_bandages = gi.cvar ("allow_bandages", "1", CVAR_LATCH);
	allow_laudanum = gi.cvar ("allow_laudanum", "1", CVAR_LATCH);
	number_of_money_bags = gi.cvar ("number_of_money_bags", "0", CVAR_LATCH);
	max_holes = gi.cvar ("max_holes", "20", 0);

	// Perform any changes needed and 
	if (!Q_strcasecmp(playmode->string, "Badge Wars") || !Q_strcasecmp(playmode->string, "BW") || ((int)playmode->value == PM_BADGE_WARS)) {
		sprintf(tempstr, "%i", PM_BADGE_WARS);
		gi.cvar_set("playmode", tempstr);
		gi.dprintf("  Playmode: Badge Wars\n");
	} else if (!Q_strcasecmp(playmode->string, "Big Heist") || !Q_strcasecmp(playmode->string, "The Big Heist") || !Q_strcasecmp(playmode->string, "Heist") || ((int)playmode->value == PM_BIG_HEIST)) {
		sprintf(tempstr, "%i", PM_BIG_HEIST);
		gi.cvar_set("playmode", tempstr);
		gi.dprintf("  Playmode: The Big Heist\n");
		if ((int)number_of_money_bags->value <1)
			gi.cvar_set("number_of_money_bags", "1");
	} else if (!Q_strcasecmp(playmode->string, "Last Man Standing") || ((int)playmode->value == PM_LAST_MAN_STANDING)) {
		sprintf(tempstr, "%i", PM_LAST_MAN_STANDING);
		gi.cvar_set("playmode", tempstr);
		gi.dprintf("  Playmode: Last Man Standing\n");
	} else if (!Q_strcasecmp(playmode->string, "Capture The Flag") || !Q_strcasecmp(playmode->string, "CTF") || ((int)playmode->value == PM_CAPTURE_THE_FLAG)) {
		sprintf(tempstr, "%i", PM_CAPTURE_THE_FLAG);
		gi.cvar_set("playmode", tempstr);
		gi.dprintf("  Playmode: Capture The Flag\n");
	} else if (!Q_strcasecmp(playmode->string, "King Of The Hill") || !Q_strcasecmp(playmode->string, "KOTH") || ((int)playmode->value == PM_KING_OF_THE_HILL)) {
		sprintf(tempstr, "%i", PM_KING_OF_THE_HILL);
		gi.cvar_set("playmode", tempstr);
		gi.dprintf("  Playmode: King Of The Hill\n");
	} else {
		sprintf(tempstr, "%i", PM_DEATHMATCH);
		gi.cvar_set("playmode", tempstr);
		gi.dprintf("  Playmode: Deathmatch\n");
	}

	if (teamplay->value) {
		if ((int)playmode->value == PM_BADGE_WARS) {
			gi.dprintf("  Teamplay is not available in this playmode\n");
			gi.cvar_set("teamplay", "0");
		} else {
			gi.dprintf("  Teamplay on\n");
		}
	} else {
		if (((int)playmode->value == PM_BIG_HEIST) || ((int)playmode->value == PM_CAPTURE_THE_FLAG)) {
			gi.dprintf("  Teamplay is required for this playmode\n");
			gi.cvar_set("teamplay", "1");
		} else {
			gi.dprintf("  Teamplay off\n");
		}
	}
	if (team_damage->value) {
		if (teamplay->value) {
			gi.dprintf("  Team damage enabled\n");
		} else {
			gi.dprintf("  Team damage enabled (ignored)\n");
		}
	} else {
		if (teamplay->value) {
			gi.dprintf("  Team damage disabled\n");
		}
	}	
	if ((int)number_of_teams->value<2) {
		gi.cvar_set("number_of_teams", "2");
	}
	if (teamplay->value) {
		gi.dprintf("  Number of teams: %i\n",(int)number_of_teams->value);
		for (i=0;i<(int)number_of_teams->value;i++) {
			teamdata[i].players=0;
			teamdata[i].score=0;
			gi.dprintf("  Team %i: %s\n",i+1, teamdata[i].name->string);
		}
	} else {
		gi.dprintf("  %i teams (ignored)\n",(int)number_of_teams->value);
	}
	if (force_join->value) {
		gi.dprintf("  Players are forced to join a team\n");
	} else {
		gi.dprintf("  Players may choose a team\n");
	}
	if (use_ctf_skins->value) {
		gi.dprintf("  Using CTF skins\n");
	} else {
		gi.dprintf("  Using triangle team identifiers\n");
	}
	if (drop_badge->value && ((int)playmode->value == PM_BADGE_WARS)) {
		gi.dprintf("  Badge can be dropped\n");
	} else {
		gi.dprintf("  Badge CANNOT be dropped\n");
	}
	if (drop_bag->value && ((int)playmode->value == PM_BIG_HEIST)) {
		gi.dprintf("  Bags can be dropped\n");
	} else {
		gi.dprintf("  Bags CANNOT be dropped\n");
	}
	if ((int)playmode->value == PM_BIG_HEIST) {
		if (ready_all->value) {
			gi.dprintf("  Game will wait until players are ready\n");
		} else {
			gi.dprintf("  Prep-time is 30 seconds\n");
		}
		if (use_classes->value) {
			gi.dprintf("  Using player classes\n");
		} else {
			gi.dprintf("  Using inventory menus\n");
		}
		if (announce_deaths->value) {
			gi.dprintf("  Deaths are announced to all\n");
		} else {
			gi.dprintf("  Only announcing deaths to spectators\n");
		}
	}
	if (respawn_invuln->value) {
		gi.dprintf("  Temporary respawn invulnerability\n");
	} else {
		gi.dprintf("  Players are vulnerable when they respawn\n");
	}
	if (weapons_unique->value) {
		gi.dprintf("  Weapons are unique\n");
		gi.cvar_set("drop_all_on_death", "1");
	} else {
		gi.dprintf("  Weapons are NOT unique\n");
	}
	if (drop_all_on_death->value) {
		gi.dprintf("  Drop all on death\n");
	} else {
		gi.dprintf("  Do NOT drop all on death\n");
	}
	if (bullet_ricochet->value) {
		gi.dprintf("  Bullet ricochet on\n");
	} else {
		gi.dprintf("  NO bullet ricochet\n");
	}
	if (item_weight->value) {
		gi.dprintf("  Item weights enabled\n");
	} else {
		gi.dprintf("  Item weights NOT enabled\n");
	}
	if (hit_locations->value) {
		gi.dprintf("  Hit locations on\n");
	} else {
		gi.dprintf("  Hit locations off\n");
	}
	if ((int)max_holes->value>100)
		gi.cvar_set("max_holes", "100");
	gi.dprintf("  max_holes is %i\n", (int)max_holes->value);

	if (allow_hands_of_lightning->value) {
		gi.dprintf("  Artifact: Hands of Lightning will spawn\n");
	} else {
		gi.dprintf("  Artifact: Hands of Lightning will NOT spawn\n");
	}
	if (allow_strength_of_the_bear->value) {
		gi.dprintf("  Artifact: Strength of the Bear will spawn\n");
	} else {
		gi.dprintf("  Artifact: Strength of the Bear will NOT spawn\n");
	}
	if (allow_flesh_of_the_salamander->value) {
		gi.dprintf("  Artifact: Flesh of the Salamander will spawn\n");
	} else {
		gi.dprintf("  Artifact: Flesh of the Salamander will NOT spawn\n");
	}
	if (allow_aura_of_pain->value) {
		gi.dprintf("  Artifact: Aura of Pain will spawn\n");
	} else {
		gi.dprintf("  Artifact: Aura of Pain will NOT spawn\n");
	}
	if (allow_tongue_of_the_leech->value) {
		gi.dprintf("  Artifact: Tongue of the Leech will spawn\n");
	} else {
		gi.dprintf("  Artifact: Tongue of the Leech will NOT spawn\n");
	}
	if (allow_tail_of_the_scorpion->value) {
		gi.dprintf("  Artifact: Tail of the Scorpion will spawn\n");
	} else {
		gi.dprintf("  Artifact: Tail of the Scorpion will NOT spawn\n");
	}
	if (allow_shroud_of_darkness->value) {
		gi.dprintf("  Artifact: Shroud of Darkness will spawn\n");
	} else {
		gi.dprintf("  Artifact: Shroud of Darkness will NOT spawn\n");
	}

	if (!allow_sheriff_badge->value) {
		gi.dprintf("  No sheriff badges allowed\n");
	}
	if (!allow_marshal_badge->value) {
		gi.dprintf("  No marshal badges allowed\n");
	}
	if (!allow_bandolier->value) {
		gi.dprintf("  No bandoliers allowed\n");
	}
	if (!allow_pack->value) {
		gi.dprintf("  No ammo packs allowed\n");
	}
	if (!allow_lantern->value) {
		gi.dprintf("  No lanterns allowed\n");
	}
	if (!allow_bandages->value) {
		gi.dprintf("  No bandages allowed\n");
	}
	if (!allow_laudanum->value) {
		gi.dprintf("  No laudanum allowed\n");
	}
	if ((int)number_of_money_bags->value>20)
		gi.cvar_set("number_of_money_bags", "20");
	gi.dprintf("  %i money bags will spawn\n", (int)number_of_money_bags->value);
// End of Gunslinger cvars -- Stone


	// items
	InitItems ();

	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "");

	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	if (deathmatch->value)
		game.maxclients = maxclients->value*2;	// Doubled for antibot
	else
		game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients+1;

	// AntiBot variables
	sv_botdetection = gi.cvar("sv_botdetection", "31", CVAR_SERVERINFO);
	bot_talk = gi.cvar("bot_talk", "0", 0);
	
	//downloads 
	allow_download = gi.cvar("allow_download", "1", 0); 
	allow_download_models = gi.cvar("allow_download_models", "1", 0);
	allow_download_players = gi.cvar("allow_download_players", "1", 0);
	gi.cvar_set("allow_download", "1");
	gi.cvar_set("allow_download_models", "1");
	gi.cvar_set("allow_download_players", "1");

	allowed_models = gi.cvar("allowed_models", "male,female,cyborg,crackhor,messiah", 0);

	if (deathmatch->value)
		srand(time(NULL));		// Let's get some real random numbers!!!

	playerlist = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	playerlist[0]=-1;
//	gi.dprintf("Maxclients: %i\n",(int)maxclients->value);

	gi.dprintf("Done InitGame\n");
}


//=========================================================

void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:
		if ( *(char **)p )
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = len;
		break;
	case F_EDICT:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = index;
		break;
	case F_CLIENT:
		if ( *(gclient_t **)p == NULL)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = index;
		break;
	case F_ITEM:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(gitem_t **)p - itemlist;
		*(int *)p = index;
		break;

	//relative to code segment
	case F_FUNCTION:
		if (*(byte **)p == NULL)
			index = 0;
		else
			index = *(byte **)p - ((byte *)InitGame);
		*(int *)p = index;
		break;

	//relative to data segment
	case F_MMOVE:
		if (*(byte **)p == NULL)
			index = 0;
		else
			index = *(byte **)p - (byte *)&mmove_reloc;
		*(int *)p = index;
		break;

	default:
		gi.error ("WriteEdict: unknown field type");
	}
}


void WriteField2 (FILE *f, field_t *field, byte *base)
{
	int			len;
	void		*p;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
		if ( *(char **)p )
		{
			len = strlen(*(char **)p) + 1;
			fwrite (*(char **)p, len, 1, f);
		}
		break;
	}
}

void ReadField (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
		len = *(int *)p;
		if (!len)
			*(char **)p = NULL;
		else
		{
			*(char **)p = gi.TagMalloc (len, TAG_LEVEL);
			fread (*(char **)p, len, 1, f);
		}
		break;
	case F_EDICT:
		index = *(int *)p;
		if ( index == -1 )
			*(edict_t **)p = NULL;
		else
			*(edict_t **)p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *(int *)p;
		if ( index == -1 )
			*(gclient_t **)p = NULL;
		else
			*(gclient_t **)p = &game.clients[index];
		break;
	case F_ITEM:
		index = *(int *)p;
		if ( index == -1 )
			*(gitem_t **)p = NULL;
		else
			*(gitem_t **)p = &itemlist[index];
		break;

	//relative to code segment
	case F_FUNCTION:
		index = *(int *)p;
		if ( index == 0 )
			*(byte **)p = NULL;
		else
			*(byte **)p = ((byte *)InitGame) + index;
		break;

	//relative to data segment
	case F_MMOVE:
		index = *(int *)p;
		if (index == 0)
			*(byte **)p = NULL;
		else
			*(byte **)p = (byte *)&mmove_reloc + index;
		break;

	default:
		gi.error ("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field=clientfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=clientfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient (FILE *f, gclient_t *client)
{
	field_t		*field;

	fread (client, sizeof(*client), 1, f);

	for (field=clientfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)client);
	}
}

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void WriteGame (char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if (!autosave)
		SaveClientData ();

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	memset (str, 0, sizeof(str));
	strcpy (str, __DATE__);
	fwrite (str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite (&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i=0 ; i<game.maxclients ; i++)
		WriteClient (f, &game.clients[i]);

	fclose (f);
}

void ReadGame (char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];

	gi.FreeTags (TAG_GAME);

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	fread (str, sizeof(str), 1, f);
	if (strcmp (str, __DATE__))
	{
		fclose (f);
		gi.error ("Savegame from an older version.\n");
	}

	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread (&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i=0 ; i<game.maxclients ; i++)
		ReadClient (f, &game.clients[i]);

	fclose (f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field=fields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=fields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals (FILE *f)
{
	field_t		*field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field=levelfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=levelfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict (FILE *f, edict_t *ent)
{
	field_t		*field;

	fread (ent, sizeof(*ent), 1, f);

	for (field=fields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)ent);
	}
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals (FILE *f)
{
	field_t		*field;

	fread (&level, sizeof(level), 1, f);

	for (field=levelfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)&level);
	}
}

/*
=================
WriteLevel

=================
*/
void WriteLevel (char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
	void	*base;

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite (&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = (void *)InitGame;
	fwrite (&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals (f);

	// write out all the entities
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;
		fwrite (&i, sizeof(i), 1, f);
		WriteEdict (f, ent);
	}
	i = -1;
	fwrite (&i, sizeof(i), 1, f);

	fclose (f);
}


/*
=================
ReadLevel

SpawnEntities will allready have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void ReadLevel (char *filename)
{
	int		entnum;
	FILE	*f;
	int		i;
	void	*base;
	edict_t	*ent;

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.FreeTags (TAG_LEVEL);

	// wipe all the entities
	memset (g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));
	if (deathmatch->value)
		globals.num_edicts = (maxclients->value*2)+1;	// Doubled for antibot
	else
		globals.num_edicts = maxclients->value+1;

	// check edict size
	fread (&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose (f);
		gi.error ("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
	fread (&base, sizeof(base), 1, f);
#ifdef _WIN32
	if (base != (void *)InitGame)
	{
		fclose (f);
		gi.error ("ReadLevel: function pointers have moved\nThis game version does not match the version that saved this game file");
	}
#else
	gi.dprintf("Function offsets %d\n", ((byte *)base) - ((byte *)InitGame));
#endif

	// load the level locals
	ReadLevelLocals (f);

	// load all the entities
	while (1)
	{
		if (fread (&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose (f);
			gi.error ("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum+1;

		ent = &g_edicts[entnum];
		ReadEdict (f, ent);

		// let the server rebuild world links for this ent
		memset (&ent->area, 0, sizeof(ent->area));
		gi.linkentity (ent);
	}

	fclose (f);

	// mark all clients as unconnected
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = &g_edicts[i+1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// fire any cross-level triggers
		if (ent->classname)
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;
	}
}
