#include "g_local.h"

#include <sys/stat.h>
#include "sqlite3.h"


#ifdef _WIN32
#pragma warning ( disable : 4090 ; disable : 4996 )
#endif

// *********************************
// Definitions
// *********************************
#define TOTAL_TABLES 11
#define TOTAL_INSERTONCE 5
#define TOTAL_RESETTABLES 6

const char* VSFU_CREATEDBQUERY[TOTAL_TABLES] = 
{
	{"CREATE TABLE [abilities] ( [char_idx] INTEGER,[index] INTEGER, [level] INTEGER, [max_level] INTEGER, [hard_max] INTEGER, [modifier] INTEGER,   [disable] INTEGER,   [general_skill] INTEGER)"},
	{"CREATE TABLE [ctf_stats] ( [char_idx] INTEGER,  [flag_pickups] INTEGER,   [flag_captures] INTEGER,   [flag_returns] INTEGER,   [flag_kills] INTEGER,   [offense_kills] INTEGER,   [defense_kills] INTEGER,   [assists] INTEGER)"},
	{"CREATE TABLE [game_stats] ([char_idx] INTEGER,  [shots] INTEGER,   [shots_hit] INTEGER,   [frags] INTEGER,   [fragged] INTEGER,   [num_sprees] INTEGER,   [max_streak] INTEGER,   [spree_wars] INTEGER,   [broken_sprees] INTEGER,   [broken_spreewars] INTEGER,   [suicides] INT,   [teleports] INTEGER,   [num_2fers] INTEGER)"},
	{"CREATE TABLE [point_data] ([char_idx] INTEGER,  [exp] INTEGER,   [exptnl] INTEGER,   [level] INTEGER,   [classnum] INTEGER,   [skillpoints] INTEGER,   [credits] INTEGER,   [weap_points] INTEGER,   [resp_weapon] INTEGER,   [tpoints] INTEGER)"},
	{"CREATE TABLE [runes_meta] ([char_idx] INTEGER,[index] INTEGER, [itemtype] INTEGER, [itemlevel] INTEGER, [quantity] INTEGER, [untradeable] INTEGER, [id] CHAR(16), [name] CHAR(24), [nummods] INTEGER, [setcode] INTEGER, [classnum] INTEGER)"},
	{"CREATE TABLE [runes_mods] ([char_idx] INTEGER,  [rune_index] INTEGER, [type] INTEGER, [mod_index] INTEGER, [value] INTEGER, [set] INTEGER)"},
	{"CREATE TABLE [talents] (   [char_idx] INTEGER,[id] INTEGER, [upgrade_level] INTEGER, [max_level] INTEGER)"},
	{"CREATE TABLE [userdata] (  [char_idx] INTEGER,[title] CHAR(24), [playername] CHAR(64), [password] CHAR(24), [email] CHAR(64), [owner] CHAR(24), [member_since] CHAR(30), [last_played] CHAR(30), [playtime_total] INTEGER,[playingtime] INTEGER)"},
	{"CREATE TABLE [weapon_meta] ([char_idx] INTEGER,[index] INTEGER, [disable] INTEGER)"},
	{"CREATE TABLE [weapon_mods] ([char_idx] INTEGER,[weapon_index] INTEGER, [modindex] INTEGER, [level] INTEGER, [soft_max] INTEGER, [hard_max] INTEGER)"},
	{"CREATE TABLE [character_data] ([char_idx] INTEGER,  [respawns] INTEGER,   [health] INTEGER,   [maxhealth] INTEGER,   [armour] INTEGER,   [maxarmour] INTEGER,   [nerfme] INTEGER,   [adminlevel] INTEGER,   [bosslevel] INTEGER)"}
};

// SAVING

const char *CA = "INSERT INTO character_data VALUES (%d,0,0,0,0,0,0,0,0)";
const char *CB = "INSERT INTO ctf_stats VALUES (%d,0,0,0,0,0,0,0)";
const char *CC = "INSERT INTO game_stats VALUES (%d,0,0,0,0,0,0,0,0,0,0,0,0)";
const char *CD = "INSERT INTO point_data VALUES (%d,0,0,0,0,0,0,0,0,0)";
const char *CE = "INSERT INTO userdata VALUES (%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",0,0)";

const char* VSFU_RESETTABLES[TOTAL_RESETTABLES] =
{
	{"DELETE FROM abilities WHERE char_idx=%d;"},
	{"DELETE FROM talents WHERE char_idx=%d;"},
	{"DELETE FROM runes_meta WHERE char_idx=%d;"},
	{"DELETE FROM runes_mods WHERE char_idx=%d;"},
	{"DELETE FROM weapon_meta WHERE char_idx=%d;"},
	{"DELETE FROM weapon_mods WHERE char_idx=%d;"}
};

// ab/talent

const char* VSFU_INSERTABILITY = "INSERT INTO abilities VALUES (%d,%d,%d,%d,%d,%d,%d,%d);";

const char* VSFU_INSERTTALENT = "INSERT INTO talents VALUES (%d,%d,%d,%d);";

// weapons

const char* VSFU_INSERTWMETA = "INSERT INTO weapon_meta VALUES (%d,%d,%d);";

const char* VSFU_INSERTWMOD = "INSERT INTO weapon_mods VALUES (%d,%d,%d,%d,%d,%d);";

// runes

const char* VSFU_INSERTRMETA = "INSERT INTO runes_meta VALUES (%d,%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%d);";

const char* VSFU_INSERTRMOD = "INSERT INTO runes_mods VALUES (%d,%d,%d,%d,%d,%d);";

const char* VSFU_INSERTRMODEX = "INSERT INTO runes_mods VALUES (%d,%d,0,%d,%d,%d,%d);";


const char* VSFU_UPDATECDATA = "UPDATE character_data SET respawns=%d, health=%d, maxhealth=%d, armour=%d, maxarmour=%d, nerfme=%d, adminlevel=%d, bosslevel=%d WHERE char_idx=%d;";

const char* VSFU_UPDATESTATS = "UPDATE game_stats SET shots=%d, shots_hit=%d, frags=%d, fragged=%d, num_sprees=%d, max_streak=%d, spree_wars=%d, broken_sprees=%d, broken_spreewars=%d, suicides=%d, teleports=%d, num_2fers=%d WHERE char_idx=%d;";

const char* VSFU_UPDATEUDATA = "UPDATE userdata SET title=\"%s\", playername=\"%s\", password=\"%s\", email=\"%s\", owner=\"%s\", member_since=\"%s\", last_played=\"%s\", playtime_total=%d, playingtime=%d WHERE char_idx=%d;";

const char* VSFU_UPDATEPDATA = "UPDATE point_data SET exp=%d, exptnl=%d, level=%d, classnum=%d, skillpoints=%d, credits=%d, weap_points=%d, resp_weapon=%d, tpoints=%d WHERE char_idx=%d;";

const char* VSFU_UPDATECTFSTATS = "UPDATE ctf_stats SET flag_pickups=%d, flag_captures=%d, flag_returns=%d, flag_kills=%d, offense_kills=%d, defense_kills=%d, assists=%d WHERE char_idx=%d;";

#define rck() if(r!=SQLITE_OK){ if(r != SQLITE_ROW && r != SQLITE_OK && r != SQLITE_DONE){gi.dprintf("sqlite error %d: %s\n", r, sqlite3_errmsg(db));return;}}
#define rck2() if(r!=SQLITE_OK){ if(r != SQLITE_ROW && r != SQLITE_OK && r != SQLITE_DONE){gi.dprintf("sqlite error %d: %s\n", r, sqlite3_errmsg(db));}}

#define QUERY(x) format=x;\
	r=sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);\
	rck()\
	r=sqlite3_step(statement);\
	rck()\
	sqlite3_finalize(statement);

#define LQUERY(x) format=x;\
	r=sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);\
	rck2()\
	r=sqlite3_step(statement);\
	rck2()\

#define DEFAULT_PATH va("%s/settings/characters.db", game_path->string)
sqlite3 *db = NULL;


void BeginTransaction(sqlite3* db);

void CommitTransaction(sqlite3 *db);


// az begin
void VSFU_SaveRunes(edict_t *player)
{
	// We assume two things: the player is logged in; the player is a client; and its file exists.
	sqlite3_stmt *statement;
	int r, i, id;
	int numRunes = CountRunes(player);
	char* format;

	V_VSFU_StartConn();

	LQUERY(va("SELECT char_idx FROM userdata WHERE playername=\"%s\"", player->client->pers.netname));

	id = sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);

	BeginTransaction(db);

	QUERY( va("DELETE FROM runes_meta WHERE char_idx=%d;", id) );


	QUERY( va("DELETE FROM runes_mods WHERE char_idx=%d;", id) );

	//begin runes
	for (i = 0; i < numRunes; ++i)
	{
		int index = FindRuneIndex(i+1, player);
		if (index != -1)
		{
			int j;

			QUERY(strdup(va(VSFU_INSERTRMETA, 
				id,
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
				char* query;
				// cant remove columns with sqlite.
				// ugly hack to work around that
				if (Lua_GetIntVariable("useMysqlTablesOnSQLite", 0))
					query = VSFU_INSERTRMODEX;
				else
					query = VSFU_INSERTRMOD;

				QUERY( strdup(va(query, 
					id,
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
	V_VSFU_Cleanup();
}

int VSFU_GetID(char *playername)
{	
	sqlite3_stmt *statement;
	int r, id;
	char* format;

	LQUERY(va("SELECT char_idx FROM userdata WHERE playername=\"%s\"", playername));

	if (r == SQLITE_ROW) // character exists
	{
		id = sqlite3_column_int(statement, 0);
	}else
		id = -1;

	sqlite3_finalize(statement);
	return id;
}

int VSFU_NewID()
{
	sqlite3_stmt *statement;
	int r, ncount;
	char* format;

	LQUERY("SELECT COUNT(*) FROM userdata");

	ncount = sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);
	return ncount;
}

//************************************************
//************************************************
void VSFU_SavePlayer(edict_t *player)
{
	sqlite3_stmt *statement;
	int r, i, id;
	int numAbilities = CountAbilities(player);
	int numWeapons = CountWeapons(player);
	int numRunes = CountRunes(player);
	char *format;

	V_VSFU_StartConn();

	id = VSFU_GetID(player->client->pers.netname);

	BeginTransaction(db);

	if (id == -1)
	{

		// Create initial database.
		id = VSFU_NewID();
		
		gi.dprintf("SQLite (single mode): creating initial data for player id %d..", id);
		
		if (!Lua_GetIntVariable("useMysqlTablesOnSQLite", 0))
		{
			/* sorry about this :( -az*/
				QUERY(va (CA, id));
				QUERY(va (CB, id));
				QUERY(va (CC, id));
				QUERY(va (CD, id));
				QUERY(va (CE, id));
		}else
		{
			QUERY(va (CA, id));
			QUERY(va (CB, id));
			QUERY(va (CC, id));
			QUERY(va (CD, id));
			QUERY(va("INSERT INTO userdata VALUES (%d,\"\",\"\",\"\",\"\",\"\",\"\",\"\",0,0,0)", id));
		}
		gi.dprintf("inserted bases.\n", r);
	}

	{ // real saving
		sqlite3_stmt *statement;

		// reset tables (remove records for reinsertion)
		for (i = 0; i < TOTAL_RESETTABLES; i++)
		{
			QUERY( va(VSFU_RESETTABLES[i], id) );
		}

		QUERY(strdup(va(VSFU_UPDATEUDATA, 
		 player->myskills.title,
		 player->client->pers.netname,
		 player->myskills.password,
		 player->myskills.email,
		 player->myskills.owner,
 		 player->myskills.member_since,
		 player->myskills.last_played,
		 player->myskills.total_playtime,
 		 player->myskills.playingtime, id)));

		 free (format);

		// talents
		for (i = 0; i < player->myskills.talents.count; ++i)
		{
			QUERY( strdup(va(VSFU_INSERTTALENT, id, player->myskills.talents.talent[i].id,
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
				format = strdup(va(VSFU_INSERTABILITY, id, index, 
					player->myskills.abilities[index].level,
					player->myskills.abilities[index].max_level,
					player->myskills.abilities[index].hard_max,
					player->myskills.abilities[index].modifier,
					(int)player->myskills.abilities[index].disable,
					(int)player->myskills.abilities[index].general_skill));
				
				r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
				r = sqlite3_step(statement);
				sqlite3_finalize(statement);
				free (format); // this will be slow...
			}
		}
		// gi.dprintf("saved abilities");
		
		//*****************************
		//in-game stats
		//*****************************

		QUERY(strdup(va(VSFU_UPDATECDATA, 
		 player->myskills.weapon_respawns,
		 player->myskills.current_health,
		 MAX_HEALTH(player),
		 player->client->pers.inventory[body_armor_index],
  		 MAX_ARMOR(player),
 		 player->myskills.nerfme,
		 player->myskills.administrator, // flags
		 player->myskills.boss, id)));

		free (format);

		//*****************************
		//stats
		//*****************************

		QUERY( strdup(va(VSFU_UPDATESTATS, 
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
		 player->myskills.num_2fers, id)) );

		free (format);
		
		//*****************************
		//standard stats
		//*****************************
		
		QUERY( strdup(va(VSFU_UPDATEPDATA, 
		 player->myskills.experience,
		 player->myskills.next_level,
         player->myskills.level,
		 player->myskills.class_num,
		 player->myskills.speciality_points,
 		 player->myskills.credits,
		 player->myskills.weapon_points,
		 player->myskills.respawn_weapon,
		 player->myskills.talents.talentPoints, id)) );
		
		free (format);

		//begin weapons
		for (i = 0; i < numWeapons; ++i)
		{
			int index = FindWeaponIndex(i+1, player);
			if (index != -1)
			{
				int j;
				QUERY( strdup(va(VSFU_INSERTWMETA, id, 
				 index,
				 player->myskills.weapons[index].disable)));			
				
				free (format);

				for (j = 0; j < MAX_WEAPONMODS; ++j)
				{
					QUERY( strdup(va(VSFU_INSERTWMOD, id, 
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

				QUERY( strdup(va(VSFU_INSERTRMETA, id, 
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
					char* query;
					// cant remove columns with sqlite.
					// ugly hack to work around that
					if (Lua_GetIntVariable("useMysqlTablesOnSQLite", 0))
						query = VSFU_INSERTRMODEX;
					else
						query = VSFU_INSERTRMOD;

					QUERY( strdup(va(query, 
					 id, 
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

		QUERY( strdup(va(VSFU_UPDATECTFSTATS, 
			player->myskills.flag_pickups,
			player->myskills.flag_captures,
			player->myskills.flag_returns,
			player->myskills.flag_kills,
			player->myskills.offense_kills,
			player->myskills.defense_kills,
			player->myskills.assists, id)) );

		free (format);

	} // end saving

	CommitTransaction(db);

	if (player->client->pers.inventory[power_cube_index] > player->client->pers.max_powercubes)
		player->client->pers.inventory[power_cube_index] = player->client->pers.max_powercubes;

	V_VSFU_Cleanup();
}


qboolean VSFU_LoadPlayer(edict_t *player)
{
	sqlite3_stmt* statement, *statement_mods;
	char* format;
	int numAbilities, numWeapons, numRunes;
	int i, r, id;

	V_VSFU_StartConn();

	id = VSFU_GetID(player->client->pers.netname);

	if (id == -1)
		return false;
	
	LQUERY(va("SELECT * FROM userdata WHERE char_idx=%d", id));

    strcpy(player->myskills.title, sqlite3_column_text(statement, 1));
	strcpy(player->myskills.player_name, sqlite3_column_text(statement, 2));
	strcpy(player->myskills.password, sqlite3_column_text(statement, 3));
	strcpy(player->myskills.email, sqlite3_column_text(statement, 4));
	strcpy(player->myskills.owner, sqlite3_column_text(statement, 5));
	strcpy(player->myskills.member_since, sqlite3_column_text(statement, 6));
	strcpy(player->myskills.last_played, sqlite3_column_text(statement, 7));
	player->myskills.total_playtime =  sqlite3_column_int(statement, 8);

	player->myskills.playingtime =  sqlite3_column_int(statement, 9);

	sqlite3_finalize(statement);

	format = va("SELECT COUNT(*) FROM talents WHERE char_idx=%d", id);
	
	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

    //begin talents
	player->myskills.talents.count = sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);

	format = va("SELECT * FROM talents WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	for (i = 0; i < player->myskills.talents.count; ++i)
	{
		//don't crash.
        if (i > MAX_TALENTS)
			return false;

		player->myskills.talents.talent[i].id = sqlite3_column_int(statement, 1);
		player->myskills.talents.talent[i].upgradeLevel = sqlite3_column_int(statement, 2);
		player->myskills.talents.talent[i].maxLevel = sqlite3_column_int(statement, 3);


		if ( (r = sqlite3_step(statement)) == SQLITE_DONE)
			break;
	}
	//end talents
	
	sqlite3_finalize(statement);

	format = va("SELECT COUNT(*) FROM abilities WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	//begin abilities
	numAbilities = sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);

	format = va("SELECT * FROM abilities WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	for (i = 0; i < numAbilities; ++i)
	{
		int index;
		index = sqlite3_column_int(statement, 1);

		if ((index >= 0) && (index < MAX_ABILITIES))
		{
			player->myskills.abilities[index].level			= sqlite3_column_int(statement, 2);
			player->myskills.abilities[index].max_level		= sqlite3_column_int(statement, 3);
			player->myskills.abilities[index].hard_max		= sqlite3_column_int(statement, 4);
			player->myskills.abilities[index].modifier		= sqlite3_column_int(statement, 5);
			player->myskills.abilities[index].disable		= sqlite3_column_int(statement, 6);
			player->myskills.abilities[index].general_skill = (qboolean)sqlite3_column_int(statement, 7);

			if ( (r = sqlite3_step(statement)) == SQLITE_DONE)
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

	sqlite3_finalize(statement);

	format = va("SELECT COUNT(*) FROM weapon_meta WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	//begin weapons
	numWeapons = sqlite3_column_int(statement, 0);
	
	sqlite3_finalize(statement);

	format = va("SELECT * FROM weapon_meta WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	for (i = 0; i < numWeapons; ++i)
	{
		int index;
		index = sqlite3_column_int(statement, 1);

		if ((index >= 0 ) && (index < MAX_WEAPONS))
		{
			int j;
			player->myskills.weapons[index].disable = sqlite3_column_int(statement, 2);

			format = strdup(va("SELECT * FROM weapon_mods WHERE weapon_index=%d AND char_idx=%d", index, id));

			r = sqlite3_prepare_v2(db, format, strlen(format), &statement_mods, NULL);
			r = sqlite3_step(statement_mods);

			for (j = 0; j < MAX_WEAPONMODS; ++j)
			{
				
				player->myskills.weapons[index].mods[j].level = sqlite3_column_int(statement_mods, 3);
				player->myskills.weapons[index].mods[j].soft_max = sqlite3_column_int(statement_mods, 4);
				player->myskills.weapons[index].mods[j].hard_max = sqlite3_column_int(statement_mods, 5);
				
				if ((r = sqlite3_step(statement_mods)) == SQLITE_DONE)
					break;
			}

			free (format);
			sqlite3_finalize(statement_mods);
		}
		else
		{
			gi.dprintf("Error loading player: %s. Weapon index not loaded correctly!\n", player->myskills.player_name);
			WriteToLogfile(player, "ERROR during loading: Weapon index not loaded correctly!");
			return false;
		}

		if ((r = sqlite3_step(statement)) == SQLITE_DONE)
			break;

	}

	sqlite3_finalize(statement);
	//end weapons

	//begin runes

	format = va("SELECT COUNT(*) FROM runes_meta WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	numRunes = sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);

	format = va("SELECT * FROM runes_meta WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	for (i = 0; i < numRunes; ++i)
	{
		int index;
		index = sqlite3_column_int(statement, 1);
		if ((index >= 0) && (index < MAX_VRXITEMS))
		{
			int j;
			player->myskills.items[index].itemtype = sqlite3_column_int(statement, 2);
			player->myskills.items[index].itemLevel = sqlite3_column_int(statement, 3);
			player->myskills.items[index].quantity = sqlite3_column_int(statement, 4);
			player->myskills.items[index].untradeable = sqlite3_column_int(statement, 5);
			strcpy(player->myskills.items[index].id, sqlite3_column_text(statement, 6));
			strcpy(player->myskills.items[index].name, sqlite3_column_text(statement, 7));
			player->myskills.items[index].numMods = sqlite3_column_int(statement, 8);
			player->myskills.items[index].setCode = sqlite3_column_int(statement, 9);
			player->myskills.items[index].classNum = sqlite3_column_int(statement, 10);

			format = strdup(va("SELECT * FROM runes_mods WHERE rune_index=%d AND char_idx=%d", index, id));

			r = sqlite3_prepare_v2(db, format, strlen(format), &statement_mods, NULL);
			r = sqlite3_step(statement_mods);

			for (j = 0; j < MAX_VRXITEMMODS; ++j)
			{
				if (Lua_GetIntVariable("useMysqlTablesOnSQLite", 0))
				{
					player->myskills.items[index].modifiers[j].type = sqlite3_column_int(statement_mods, 3);
					player->myskills.items[index].modifiers[j].index = sqlite3_column_int(statement_mods, 4);
					player->myskills.items[index].modifiers[j].value = sqlite3_column_int(statement_mods, 5);
					player->myskills.items[index].modifiers[j].set = sqlite3_column_int(statement_mods, 6);
				}else
				{
					player->myskills.items[index].modifiers[j].type = sqlite3_column_int(statement_mods, 2);
					player->myskills.items[index].modifiers[j].index = sqlite3_column_int(statement_mods, 3);
					player->myskills.items[index].modifiers[j].value = sqlite3_column_int(statement_mods, 4);
					player->myskills.items[index].modifiers[j].set = sqlite3_column_int(statement_mods, 5);
				}

				if ((r = sqlite3_step(statement_mods)) == SQLITE_DONE)
					break;
			}

			free (format);
			sqlite3_finalize(statement_mods);
		}

		if ((r = sqlite3_step(statement)) == SQLITE_DONE)
			break;
	}

	sqlite3_finalize(statement);
	//end runes


	//*****************************
	//standard stats
	//*****************************

	format = va("SELECT * FROM point_data WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	//Exp
	player->myskills.experience =  sqlite3_column_int(statement, 1);
	//next_level
	player->myskills.next_level =  sqlite3_column_int(statement, 2);
	//Level
	player->myskills.level =  sqlite3_column_int(statement, 3);
	//Class number
	player->myskills.class_num = sqlite3_column_int(statement, 4);
	//skill points
	player->myskills.speciality_points = sqlite3_column_int(statement, 5);
	//credits
	player->myskills.credits = sqlite3_column_int(statement, 6);
	//weapon points
	player->myskills.weapon_points = sqlite3_column_int(statement, 7);
	//respawn weapon
	player->myskills.respawn_weapon = sqlite3_column_int(statement, 8);
	//talent points
	player->myskills.talents.talentPoints = sqlite3_column_int(statement, 9);

	sqlite3_finalize(statement);

	format = va("SELECT * FROM character_data WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);


	//*****************************
	//in-game stats
	//*****************************
	//respawns
	player->myskills.weapon_respawns = sqlite3_column_int(statement, 1);
	//health
	player->myskills.current_health = sqlite3_column_int(statement, 2);
	//max health
	player->myskills.max_health = sqlite3_column_int(statement, 3);
	//armour
	player->myskills.current_armor = sqlite3_column_int(statement, 4);
	//max armour
	player->myskills.max_armor = sqlite3_column_int(statement, 5);
	//nerfme			(cursing a player maybe?)
	player->myskills.nerfme = sqlite3_column_int(statement, 6);

	//*****************************
	//flags
	//*****************************
	//admin flag
	player->myskills.administrator =  sqlite3_column_int(statement, 7);
	//boss flag
	player->myskills.boss = sqlite3_column_int(statement, 8);

	//*****************************
	//stats
	//*****************************

	sqlite3_finalize(statement);

	format = va("SELECT * FROM game_stats WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	//shots fired
	player->myskills.shots = sqlite3_column_int(statement, 1);
	//shots hit
	player->myskills.shots_hit = sqlite3_column_int(statement, 2);
	//frags
	player->myskills.frags = sqlite3_column_int(statement, 3);
	//deaths
	player->myskills.fragged = sqlite3_column_int(statement, 4);
	//number of sprees
	player->myskills.num_sprees = sqlite3_column_int(statement, 5);
	//max spree
	player->myskills.max_streak = sqlite3_column_int(statement, 6);
	//number of wars
	player->myskills.spree_wars =  sqlite3_column_int(statement, 7);
	//number of sprees broken
	player->myskills.break_sprees =  sqlite3_column_int(statement, 8);
	//number of wars broken
	player->myskills.break_spree_wars = sqlite3_column_int(statement, 9);
	//suicides
	player->myskills.suicides = sqlite3_column_int(statement, 10);
	//teleports			(link this to "use tballself" maybe?)
	player->myskills.teleports =  sqlite3_column_int(statement, 11);
	//number of 2fers
	player->myskills.num_2fers = sqlite3_column_int(statement, 12);

	sqlite3_finalize(statement);

	format = va("SELECT * FROM ctf_stats WHERE char_idx=%d", id);

	r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
	r = sqlite3_step(statement);

	//CTF statistics
	player->myskills.flag_pickups =  sqlite3_column_int(statement, 1);
	player->myskills.flag_captures =  sqlite3_column_int(statement, 2);
	player->myskills.flag_returns =  sqlite3_column_int(statement, 3);
	player->myskills.flag_kills =  sqlite3_column_int(statement, 4);
	player->myskills.offense_kills =  sqlite3_column_int(statement, 5);
	player->myskills.defense_kills =  sqlite3_column_int(statement, 6);
	player->myskills.assists =  sqlite3_column_int(statement, 7);
	//End CTF

	sqlite3_finalize(statement);

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

	V_VSFU_Cleanup();
	return true;
}

// Start Connection to SQLite

void V_VSFU_StartConn()
{
	char *dbname = Lua_GetStringSetting("SQLitePath");
	struct stat mybuf;
	int i, r;
	char* format;
	sqlite3_stmt *statement;
	qboolean build_db = false;

	if (db)
		return;

	if (!dbname)
		dbname = DEFAULT_PATH;
	
	if (stat(dbname, &mybuf))
	{
		build_db = true;
	}

	sqlite3_open(dbname, &db);

	if (build_db)
	{
		for (i = 0; i < TOTAL_TABLES; i++)
		{
			QUERY(va (VSFU_CREATEDBQUERY[i]))
 		}
	}

}
	
void V_VSFU_Cleanup()
{
	sqlite3_close(db);
	db = NULL;
}
