#include "g_local.h"
#include "sqlite3.h"

// ===  CONSTS ===
#define TOTAL_TABLES 11
#define TOTAL_INSERTONCE 5
#define TOTAL_RESETTABLES 6

// === QUERIES ===
// these are simply too large to put in the function itself
// and are marked SQLITE because this would be terrible in a big hueg rdbms like mysql

const char* SQLITE_CREATEDBQUERY[TOTAL_TABLES] = 
{
	{"CREATE TABLE [abilities] ([index] INTEGER, [level] INTEGER, [max_level] INTEGER, [hard_max] INTEGER, [modifier] INTEGER,   [disable] INTEGER,   [general_skill] INTEGER)"},
	{"CREATE TABLE [ctf_stats] (  [flag_pickups] INTEGER,   [flag_captures] INTEGER,   [flag_returns] INTEGER,   [flag_kills] INTEGER,   [offense_kills] INTEGER,   [defense_kills] INTEGER,   [assists] INTEGER)"},
	{"CREATE TABLE [game_stats] (  [shots] INTEGER,   [shots_hit] INTEGER,   [frags] INTEGER,   [fragged] INTEGER,   [num_sprees] INTEGER,   [max_streak] INTEGER,   [spree_wars] INTEGER,   [broken_sprees] INTEGER,   [broken_spreewars] INTEGER,   [suicides] INT,   [teleports] INTEGER,   [num_2fers] INTEGER)"},
	{"CREATE TABLE [point_data] (  [exp] INTEGER,   [exptnl] INTEGER,   [level] INTEGER,   [classnum] INTEGER,   [skillpoints] INTEGER,   [credits] INTEGER,   [weap_points] INTEGER,   [resp_weapon] INTEGER,   [tpoints] INTEGER)"},
	{"CREATE TABLE [runes_meta] ([index] INTEGER, [itemtype] INTEGER, [itemlevel] INTEGER, [quantity] INTEGER, [untradeable] INTEGER, [id] CHAR(16), [name] CHAR(24), [nummods] INTEGER, [setcode] INTEGER, [classnum] INTEGER)"},
	{"CREATE TABLE [runes_mods] (  [rune_index] INTEGER, [type] INTEGER, [mod_index] INTEGER, [value] INTEGER, [set] INTEGER)"},
	{"CREATE TABLE [talents] ([id] INTEGER, [upgrade_level] INTEGER, [max_level] INTEGER)"},
	{"CREATE TABLE [userdata] ([title] CHAR(24), [playername] CHAR(64), [password] CHAR(24), [email] CHAR(64), [owner] CHAR(24), [member_since] CHAR(30), [last_played] CHAR(30), [playtime_total] INTEGER,[playingtime] INTEGER)"},
	{"CREATE TABLE [weapon_meta] ([index] INTEGER, [disable] INTEGER)"},
	{"CREATE TABLE [weapon_mods] ([weapon_index] INTEGER, [modindex] INTEGER, [level] INTEGER, [soft_max] INTEGER, [hard_max] INTEGER)"},
	{"CREATE TABLE [character_data] (  [respawns] INTEGER,   [health] INTEGER,   [maxhealth] INTEGER,   [armour] INTEGER,   [maxarmour] INTEGER,   [nerfme] INTEGER,   [adminlevel] INTEGER,   [bosslevel] INTEGER)"}
};

// SAVING
const char* SQLITE_INSERTONCE[TOTAL_INSERTONCE] = 
{
	{"INSERT INTO character_data VALUES (0,0,0,0,0,0,0,0)"},
	{"INSERT INTO ctf_stats VALUES (0,0,0,0,0,0,0)"},
	{"INSERT INTO game_stats VALUES (0,0,0,0,0,0,0,0,0,0,0,0)"},
	{"INSERT INTO point_data VALUES (0,0,0,0,0,0,0,0,0)"},
	{"INSERT INTO userdata VALUES (\"\",\"\",\"\",\"\",\"\",\"\",\"\",0,0)"}
};

const char* SQLITE_RESETTABLES[TOTAL_RESETTABLES] =
{
	{"DELETE FROM abilities;"},
	{"DELETE FROM talents;"},
	{"DELETE FROM runes_meta;"},
	{"DELETE FROM runes_mods;"},
	{"DELETE FROM weapon_meta;"},
	{"DELETE FROM weapon_mods;"}
};

// ab/talent

const char* SQLITE_INSERTABILITY = "INSERT INTO abilities VALUES (%d,%d,%d,%d,%d,%d,%d);";

const char* SQLITE_INSERTTALENT = "INSERT INTO talents VALUES (%d,%d,%d);";

// weapons

const char* SQLITE_INSERTWMETA = "INSERT INTO weapon_meta VALUES (%d,%d);";

const char* SQLITE_INSERTWMOD = "INSERT INTO weapon_mods VALUES (%d,%d,%d,%d,%d);";

// runes

const char* SQLITE_INSERTRMETA = "INSERT INTO runes_meta VALUES (%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%d);";

const char* SQLITE_INSERTRMOD = "INSERT INTO runes_mods VALUES (%d,%d,%d,%d,%d);";

const char* SQLITE_UPDATECDATA = "UPDATE character_data SET respawns=%d, health=%d, maxhealth=%d, armour=%d, maxarmour=%d, nerfme=%d, adminlevel=%d, bosslevel=%d;";

const char* SQLITE_UPDATESTATS = "UPDATE game_stats SET shots=%d, shots_hit=%d, frags=%d, fragged=%d, num_sprees=%d, max_streak=%d, spree_wars=%d, broken_sprees=%d, broken_spreewars=%d, suicides=%d, teleports=%d, num_2fers=%d;";

const char* SQLITE_UPDATEUDATA = "UPDATE userdata SET title=\"%s\", playername=\"%s\", password=\"%s\", email=\"%s\", owner=\"%s\", member_since=\"%s\", last_played=\"%s\", playtime_total=%d, playingtime=%d;";

const char* SQLITE_UPDATEPDATA = "UPDATE point_data SET exp=%d, exptnl=%d, level=%d, classnum=%d, skillpoints=%d, credits=%d, weap_points=%d, resp_weapon=%d, tpoints=%d;";

const char* SQLITE_UPDATECTFSTATS = "UPDATE ctf_stats SET flag_pickups=%d, flag_captures=%d, flag_returns=%d, flag_kills=%d, offense_kills=%d, defense_kills=%d, assists=%d;";

// LOADING

// this is for tables with only one entry.
const char* SQLITE_VRXSELECT = "SELECT * FROM %s";

//az end


// az begin
//************************************************

#define QUERY(x) format=x;\
	r=sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);\
	r=sqlite3_step(statement);\
	sqlite3_finalize(statement);

void BeginTransaction(sqlite3* db)
{
	char* format;
	int r;
	sqlite3_stmt *statement;

	QUERY("BEGIN TRANSACTION;");
}

void CommitTransaction(sqlite3 *db)
{
	char* format;
	int r;
	sqlite3_stmt *statement;

	QUERY("COMMIT;")
}


// az begin
void VSF_SaveRunes(edict_t *player, char *path)
{
	// We assume two things: the player is logged in; the player is a client; and its file exists.
	sqlite3* db;
	sqlite3_stmt *statement;
	int r, i;
	int numRunes = CountRunes(player);
	char* format;

	r = sqlite3_open(path, &db);

	BeginTransaction(db);

	QUERY( "DELETE FROM runes_meta;" );


	QUERY("DELETE FROM runes_mods;");

	//begin runes
	for (i = 0; i < numRunes; ++i)
	{
		int index = FindRuneIndex(i+1, player);
		if (index != -1)
		{
			int j;

			QUERY(strdup(va(SQLITE_INSERTRMETA, 
				index,
				player->myskills.items[index].itemtype,
				player->myskills.items[index].itemLevel,
				player->myskills.items[index].quantity,
				player->myskills.items[index].untradeable,
				player->myskills.items[index].id,
				player->myskills.items[index].name,
				player->myskills.items[index].numMods,
				player->myskills.items[index].setCode,
				player->myskills.items[index].classNum)));

			free (format);

			for (j = 0; j < MAX_VRXITEMMODS; ++j)
			{
				QUERY( strdup(va(SQLITE_INSERTRMOD, 
					index,
					player->myskills.items[index].modifiers[j].type,
					player->myskills.items[index].modifiers[j].index,
					player->myskills.items[index].modifiers[j].value,
					player->myskills.items[index].modifiers[j].set)) );

				free (format);
			}
		}
	}
	//end runes

	CommitTransaction(db);
	sqlite3_close(db); // end saving.
}

//************************************************
//************************************************
qboolean VSF_SavePlayer(edict_t *player, char *path, qboolean fileexists, char* playername)
{
	sqlite3* db;
	sqlite3_stmt *statement;
	int r, i;
	int numAbilities = CountAbilities(player);
	int numWeapons = CountWeapons(player);
	int numRunes = CountRunes(player);
	char *format;

	r = sqlite3_open(path, &db);

	BeginTransaction(db);

	if (!fileexists)
	{
		// Create initial database.
		
		gi.dprintf("SQLite: creating initial database [%d]... ", r);
		for (i = 0; i < TOTAL_TABLES; i++)
		{
			QUERY(SQLITE_CREATEDBQUERY[i])
		}

		for (i = 0; i < TOTAL_INSERTONCE; i++)
		{
			QUERY(SQLITE_INSERTONCE[i]);
		}
		gi.dprintf("inserted bases.\n", r);
	}

	{ // real saving
		sqlite3_stmt *statement;

		// reset tables (remove records for reinsertion)
		for (i = 0; i < TOTAL_RESETTABLES; i++)
		{
			QUERY( SQLITE_RESETTABLES[i] );
		}

		QUERY(strdup(va(SQLITE_UPDATEUDATA, 
		 player->myskills.title,
		 playername,
		 player->myskills.password,
		 player->myskills.email,
		 player->myskills.owner,
 		 player->myskills.member_since,
		 player->myskills.last_played,
		 player->myskills.total_playtime,
 		 player->myskills.playingtime)));

		 free (format);

		// talents
		for (i = 0; i < player->myskills.talents.count; ++i)
		{
			QUERY( strdup(va(SQLITE_INSERTTALENT, player->myskills.talents.talent[i].id,
				player->myskills.talents.talent[i].upgradeLevel,
				player->myskills.talents.talent[i].maxLevel)) );
			free (format);
		}

		// abilities
	
		for (i = 0; i < numAbilities; ++i)
		{
			int index = FindAbilityIndex(i+1, player);
			if (index != -1)
			{
				format = strdup(va(SQLITE_INSERTABILITY, index, 
					player->myskills.abilities[index].level,
					player->myskills.abilities[index].max_level,
					player->myskills.abilities[index].hard_max,
					player->myskills.abilities[index].modifier,
					(int)player->myskills.abilities[index].disable,
					(int)player->myskills.abilities[index].general_skill));
				
				r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
				if (r == SQLITE_ERROR)
				{
					format = sqlite3_errmsg(db);
					gi.dprintf(format);
				}
				r = sqlite3_step(statement);
				sqlite3_finalize(statement);
				free (format); // this will be slow...
			}
		}
		// gi.dprintf("saved abilities");
		
		//*****************************
		//in-game stats
		//*****************************

		QUERY(strdup(va(SQLITE_UPDATECDATA, 
		 player->myskills.weapon_respawns,
		 player->myskills.current_health,
		 MAX_HEALTH(player),
		 player->client->pers.inventory[body_armor_index],
  		 MAX_ARMOR(player),
 		 player->myskills.nerfme,
		 player->myskills.administrator, // flags
		 player->myskills.boss)));

		free (format);

		//*****************************
		//stats
		//*****************************

		QUERY( strdup(va(SQLITE_UPDATESTATS, 
		 player->myskills.shots,
		 player->myskills.shots_hit,
		 player->myskills.frags,
		 player->myskills.fragged,
  		 player->myskills.num_sprees,
 		 player->myskills.max_streak,
		 player->myskills.spree_wars,
		 player->myskills.break_sprees,
		 player->myskills.break_spree_wars,
		 player->myskills.suicides,
		 player->myskills.teleports,
		 player->myskills.num_2fers)) );

		free (format);
		
		//*****************************
		//standard stats
		//*****************************
		
		QUERY( strdup(va(SQLITE_UPDATEPDATA, 
		 player->myskills.experience,
		 player->myskills.next_level,
         player->myskills.level,
		 player->myskills.class_num,
		 player->myskills.speciality_points,
 		 player->myskills.credits,
		 player->myskills.weapon_points,
		 player->myskills.respawn_weapon,
		 player->myskills.talents.talentPoints)) );
		
		free (format);

		//begin weapons
		for (i = 0; i < numWeapons; ++i)
		{
			int index = FindWeaponIndex(i+1, player);
			if (index != -1)
			{
				int j;
				QUERY( strdup(va(SQLITE_INSERTWMETA, 
				 index,
				 player->myskills.weapons[index].disable)));			
				
				free (format);

				for (j = 0; j < MAX_WEAPONMODS; ++j)
				{
					QUERY( strdup(va(SQLITE_INSERTWMOD, 
						index,
						j,
					    player->myskills.weapons[index].mods[j].level,
					    player->myskills.weapons[index].mods[j].soft_max,
					    player->myskills.weapons[index].mods[j].hard_max)) );
					free (format);
				}
			}
		}
		//end weapons

		//begin runes
		for (i = 0; i < numRunes; ++i)
		{
			int index = FindRuneIndex(i+1, player);
			if (index != -1)
			{
				int j;

				QUERY( strdup(va(SQLITE_INSERTRMETA, 
				 index,
				 player->myskills.items[index].itemtype,
				 player->myskills.items[index].itemLevel,
				 player->myskills.items[index].quantity,
				 player->myskills.items[index].untradeable,
				 player->myskills.items[index].id,
				 player->myskills.items[index].name,
				 player->myskills.items[index].numMods,
				 player->myskills.items[index].setCode,
				 player->myskills.items[index].classNum)));

				free (format);

				for (j = 0; j < MAX_VRXITEMMODS; ++j)
				{
					QUERY( strdup(va(SQLITE_INSERTRMOD, 
						index,
					    player->myskills.items[index].modifiers[j].type,
					    player->myskills.items[index].modifiers[j].index,
					    player->myskills.items[index].modifiers[j].value,
					    player->myskills.items[index].modifiers[j].set)) );
					
					free (format);
				}
			}
		}
		//end runes

		QUERY( strdup(va(SQLITE_UPDATECTFSTATS, 
			player->myskills.flag_pickups,
			player->myskills.flag_captures,
			player->myskills.flag_returns,
			player->myskills.flag_kills,
			player->myskills.offense_kills,
			player->myskills.defense_kills,
			player->myskills.assists)) );

		free (format);

	} // end saving

	CommitTransaction(db);

	if (player->client->pers.inventory[power_cube_index] > player->client->pers.max_powercubes)
		player->client->pers.inventory[power_cube_index] = player->client->pers.max_powercubes;

	sqlite3_close(db); // finalize
	return true;
}


qboolean VSF_LoadPlayer(edict_t *player, char* path)
{
	sqlite3* db;
	sqlite3_stmt* stmt, *stmt_mods;
	char* format;
	int numAbilities, numWeapons, numRunes;
	int i, r;

	sqlite3_open(path, &db);
	
	format = "SELECT * FROM userdata";
	
	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

    strcpy(player->myskills.title, sqlite3_column_text(stmt, 0));
	strcpy(player->myskills.player_name, sqlite3_column_text(stmt, 1));
	strcpy(player->myskills.password, sqlite3_column_text(stmt, 2));
	strcpy(player->myskills.email, sqlite3_column_text(stmt, 3));
	strcpy(player->myskills.owner, sqlite3_column_text(stmt, 4));
	strcpy(player->myskills.member_since, sqlite3_column_text(stmt, 5));
	strcpy(player->myskills.last_played, sqlite3_column_text(stmt, 6));
	player->myskills.total_playtime =  sqlite3_column_int(stmt, 7);

	player->myskills.playingtime =  sqlite3_column_int(stmt, 8);

	sqlite3_finalize(stmt);

	format = "SELECT COUNT(*) FROM talents";
	
	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

    //begin talents
	player->myskills.talents.count = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM talents";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	for (i = 0; i < player->myskills.talents.count; ++i)
	{
		//don't crash.
        if (i > MAX_TALENTS)
			return false;

		player->myskills.talents.talent[i].id = sqlite3_column_int(stmt, 0);
		player->myskills.talents.talent[i].upgradeLevel = sqlite3_column_int(stmt, 1);
		player->myskills.talents.talent[i].maxLevel = sqlite3_column_int(stmt, 2);


		if ( (r = sqlite3_step(stmt)) == SQLITE_DONE)
			break;
	}
	//end talents
	
	sqlite3_finalize(stmt);

	format = "SELECT COUNT(*) FROM abilities";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//begin abilities
	numAbilities = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM abilities";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	for (i = 0; i < numAbilities; ++i)
	{
		int index;
		index = sqlite3_column_int(stmt, 0);

		if ((index >= 0) && (index < MAX_ABILITIES))
		{
			player->myskills.abilities[index].level			= sqlite3_column_int(stmt, 1);
			player->myskills.abilities[index].max_level		= sqlite3_column_int(stmt, 2);
			player->myskills.abilities[index].hard_max		= sqlite3_column_int(stmt, 3);
			player->myskills.abilities[index].modifier		= sqlite3_column_int(stmt, 4);
			player->myskills.abilities[index].disable		= sqlite3_column_int(stmt, 5);
			player->myskills.abilities[index].general_skill = (qboolean)sqlite3_column_int(stmt, 6);

			if ( (r = sqlite3_step(stmt)) == SQLITE_DONE)
				break;
		}
		else
		{
			gi.dprintf("Error loading player: %s. Ability index not loaded correctly!\n", player->client->pers.netname);
			WriteToLogfile(player, "ERROR during loading: Ability index not loaded correctly!");
			return false;
		}
	}
	//end abilities

	sqlite3_finalize(stmt);

	format = "SELECT COUNT(*) FROM weapon_meta";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//begin weapons
    numWeapons = sqlite3_column_int(stmt, 0);
	
	sqlite3_finalize(stmt);

	format = "SELECT * FROM weapon_meta";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	for (i = 0; i < numWeapons; ++i)
	{
		int index;
		index = sqlite3_column_int(stmt, 0);

		if ((index >= 0 ) && (index < MAX_WEAPONS))
		{
			int j;
			player->myskills.weapons[index].disable = sqlite3_column_int(stmt, 1);

			format = strdup(va("SELECT * FROM weapon_mods WHERE weapon_index=%d", index));

			r = sqlite3_prepare_v2(db, format, strlen(format), &stmt_mods, NULL);
			r = sqlite3_step(stmt_mods);

			for (j = 0; j < MAX_WEAPONMODS; ++j)
			{
				
				player->myskills.weapons[index].mods[j].level = sqlite3_column_int(stmt_mods, 2);
				player->myskills.weapons[index].mods[j].soft_max = sqlite3_column_int(stmt_mods, 3);
				player->myskills.weapons[index].mods[j].hard_max = sqlite3_column_int(stmt_mods, 4);
				
				if ((r = sqlite3_step(stmt_mods)) == SQLITE_DONE)
					break;
			}

			free (format);
			sqlite3_finalize(stmt_mods);
		}
		else
		{
			gi.dprintf("Error loading player: %s. Weapon index not loaded correctly!\n", player->myskills.player_name);
			WriteToLogfile(player, "ERROR during loading: Weapon index not loaded correctly!");
			return false;
		}

		if ((r = sqlite3_step(stmt)) == SQLITE_DONE)
			break;

	}

	sqlite3_finalize(stmt);
	//end weapons

	//begin runes

	format = "SELECT COUNT(*) FROM runes_meta";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	numRunes = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM runes_meta";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	for (i = 0; i < numRunes; ++i)
	{
		int index;
		index = sqlite3_column_int(stmt, 0);
		if ((index >= 0) && (index < MAX_VRXITEMS))
		{
			int j;
			player->myskills.items[index].itemtype = sqlite3_column_int(stmt, 1);
			player->myskills.items[index].itemLevel = sqlite3_column_int(stmt, 2);
			player->myskills.items[index].quantity = sqlite3_column_int(stmt, 3);
			player->myskills.items[index].untradeable = sqlite3_column_int(stmt, 4);
			strcpy(player->myskills.items[index].id, sqlite3_column_text(stmt, 5));
			strcpy(player->myskills.items[index].name, sqlite3_column_text(stmt, 6));
			player->myskills.items[index].numMods = sqlite3_column_int(stmt, 7);
			player->myskills.items[index].setCode = sqlite3_column_int(stmt, 8);
			player->myskills.items[index].classNum = sqlite3_column_int(stmt, 9);

			format = strdup(va("SELECT * FROM runes_mods WHERE rune_index=%d", index));

			r = sqlite3_prepare_v2(db, format, strlen(format), &stmt_mods, NULL);
			r = sqlite3_step(stmt_mods);

			for (j = 0; j < MAX_VRXITEMMODS; ++j)
			{
				player->myskills.items[index].modifiers[j].type = sqlite3_column_int(stmt_mods, 1);
				player->myskills.items[index].modifiers[j].index = sqlite3_column_int(stmt_mods, 2);
				player->myskills.items[index].modifiers[j].value = sqlite3_column_int(stmt_mods, 3);
				player->myskills.items[index].modifiers[j].set = sqlite3_column_int(stmt_mods, 4);

				if ((r = sqlite3_step(stmt_mods)) == SQLITE_DONE)
					break;
			}

			free (format);
			sqlite3_finalize(stmt_mods);
		}

		if ((r = sqlite3_step(stmt)) == SQLITE_DONE)
			break;
	}

	sqlite3_finalize(stmt);
	//end runes


	//*****************************
	//standard stats
	//*****************************

	format = "SELECT * FROM point_data";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//Exp
	player->myskills.experience =  sqlite3_column_int(stmt, 0);
	//next_level
	player->myskills.next_level =  sqlite3_column_int(stmt, 1);
	//Level
	player->myskills.level =  sqlite3_column_int(stmt, 2);
	//Class number
	player->myskills.class_num = sqlite3_column_int(stmt, 3);
	//skill points
	player->myskills.speciality_points = sqlite3_column_int(stmt, 4);
	//credits
	player->myskills.credits = sqlite3_column_int(stmt, 5);
	//weapon points
	player->myskills.weapon_points = sqlite3_column_int(stmt, 6);
	//respawn weapon
	player->myskills.respawn_weapon = sqlite3_column_int(stmt, 7);
	//talent points
	player->myskills.talents.talentPoints = sqlite3_column_int(stmt, 8);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM character_data";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);


	//*****************************
	//in-game stats
	//*****************************
	//respawns
	player->myskills.weapon_respawns = sqlite3_column_int(stmt, 0);
	//health
	player->myskills.current_health = sqlite3_column_int(stmt, 1);
	//max health
	player->myskills.max_health = sqlite3_column_int(stmt, 2);
	//armour
	player->myskills.current_armor = sqlite3_column_int(stmt, 3);
	//max armour
	player->myskills.max_armor = sqlite3_column_int(stmt, 4);
	//nerfme			(cursing a player maybe?)
	player->myskills.nerfme = sqlite3_column_int(stmt, 5);

	//*****************************
	//flags
	//*****************************
	//admin flag
	player->myskills.administrator =  sqlite3_column_int(stmt, 6);
	//boss flag
	player->myskills.boss = sqlite3_column_int(stmt, 7);

	//*****************************
	//stats
	//*****************************

	sqlite3_finalize(stmt);

	format = "SELECT * FROM game_stats";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//shots fired
	player->myskills.shots = sqlite3_column_int(stmt, 0);
	//shots hit
	player->myskills.shots_hit = sqlite3_column_int(stmt, 1);
	//frags
	player->myskills.frags = sqlite3_column_int(stmt, 2);
	//deaths
	player->myskills.fragged = sqlite3_column_int(stmt, 3);
	//number of sprees
	player->myskills.num_sprees = sqlite3_column_int(stmt, 4);
	//max spree
	player->myskills.max_streak = sqlite3_column_int(stmt, 5);
	//number of wars
	player->myskills.spree_wars =  sqlite3_column_int(stmt, 6);
	//number of sprees broken
	player->myskills.break_sprees =  sqlite3_column_int(stmt, 7);
	//number of wars broken
	player->myskills.break_spree_wars = sqlite3_column_int(stmt, 8);
	//suicides
	player->myskills.suicides = sqlite3_column_int(stmt, 9);
	//teleports			(link this to "use tballself" maybe?)
	player->myskills.teleports =  sqlite3_column_int(stmt, 10);
	//number of 2fers
	player->myskills.num_2fers = sqlite3_column_int(stmt, 11);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM ctf_stats";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//CTF statistics
	player->myskills.flag_pickups =  sqlite3_column_int(stmt, 0);
	player->myskills.flag_captures =  sqlite3_column_int(stmt, 1);
	player->myskills.flag_returns =  sqlite3_column_int(stmt, 2);
	player->myskills.flag_kills =  sqlite3_column_int(stmt, 3);
	player->myskills.offense_kills =  sqlite3_column_int(stmt, 4);
	player->myskills.defense_kills =  sqlite3_column_int(stmt, 5);
	player->myskills.assists =  sqlite3_column_int(stmt, 6);
	//End CTF

	sqlite3_finalize(stmt);

	//Apply runes
	V_ResetAllStats(player);
	for (i = 0; i < 4; ++i)
		V_ApplyRune(player, &player->myskills.items[i]);

	//Apply health
	if (player->myskills.current_health > MAX_HEALTH(player))
		player->myskills.current_health = MAX_HEALTH(player);

	//Apply armor
	if (player->myskills.current_armor > MAX_ARMOR(player))
		player->myskills.current_armor = MAX_ARMOR(player);
	player->myskills.inventory[body_armor_index] = player->myskills.current_armor;

	//done
	sqlite3_close(db);
	return true;
}