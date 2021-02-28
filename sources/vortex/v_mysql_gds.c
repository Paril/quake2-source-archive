#include "g_local.h"

#ifndef NO_GDS

#include <my_global.h>
#include <mysql.h>
#include "gds.h"

#ifndef GDS_NOMULTITHREADING
#include <pthread.h>
#endif

#ifdef _WIN32
#pragma warning ( disable : 4090 ; disable : 4996 )
#endif

// *********************************
// Definitions
// *********************************

#define DEFAULT_DATABASE "127.0.0.1"
#define MYSQL_PW "vortex_default_password"
#define MYSQL_USER "vortex_user"
#define MYSQL_DBNAME "vrxcl"

/* 
These are modified versions of the sqlite version
the difference here is that these include ID insertion
which is of course, important when you can't create one database per character. 
This does open a small big can of worms though, if a player quickly
gets in and out of his character before the thread properly updates
it most likely implies that its character will get either locked
or played in several servers, etc..
-az
*/

// abilities

const char* MYSQL_INSERTABILITY = "INSERT INTO abilities VALUES (%d,%d,%d,%d,%d,%d,%d,%d);";

// talents

const char* MYSQL_INSERTTALENT = "INSERT INTO talents VALUES (%d,%d,%d,%d);";

// weapons

const char* MYSQL_INSERTWMETA = "INSERT INTO weapon_meta VALUES (%d,%d,%d);";

const char* MYSQL_INSERTWMOD = "INSERT INTO weapon_mods VALUES (%d,%d,%d,%d,%d,%d);";

// runes

const char* MYSQL_INSERTRMETA = "INSERT INTO runes_meta VALUES (%d,%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%d);";

const char* MYSQL_INSERTRMOD = "INSERT INTO runes_mods VALUES (%d,%d,%d,%d,%d,%d,%d);";

// update stuff

const char* MYSQL_UPDATECDATA = "UPDATE character_data SET respawns=%d, health=%d, maxhealth=%d, armour=%d, maxarmour=%d, nerfme=%d, adminlevel=%d, bosslevel=%d WHERE char_idx=%d;";

const char* MYSQL_UPDATESTATS = "UPDATE game_stats SET shots=%d, shots_hit=%d, frags=%d, fragged=%d, num_sprees=%d, max_streak=%d, spree_wars=%d, broken_sprees=%d, broken_spreewars=%d, suicides=%d, teleports=%d, num_2fers=%d WHERE char_idx=%d;";

const char* MYSQL_UPDATEUDATA = "UPDATE userdata SET title=\"%s\", playername=\"%s\", password=\"%s\", email=\"%s\", owner=\"%s\", member_since=\"%s\", last_played=CURDATE(), playtime_total=%d, playingtime=%d WHERE char_idx=%d;";

const char* MYSQL_UPDATEPDATA = "UPDATE point_data SET exp=%d, exptnl=%d, level=%d, classnum=%d, skillpoints=%d, credits=%d, weap_points=%d, resp_weapon=%d, tpoints=%d WHERE char_idx=%d;";

const char* MYSQL_UPDATECTFSTATS = "UPDATE ctf_stats SET flag_pickups=%d, flag_captures=%d, flag_returns=%d, flag_kills=%d, offense_kills=%d, defense_kills=%d, assists=%d WHERE char_idx=%d;";

/* 
	This global instance is used in our only thread- where everything is queued.
	I personally prefer it this way as it simplifies things,
	decreases the amount of connections to the DB and 
	makes the multithread business actually easy to sync.
	It might be slower- it IS slower, but it takes a lot of
	work to make it the kots2007 way. Anyone's welcome to do it the kots2007 way
	as long as you make it work. But do it yourself. I'm a hobbyist, not a professional. -az
*/

// MYSQL
MYSQL* GDS_MySQL = NULL;

// Queue
typedef struct
{
	edict_t *ent;
	int operation;
	int id;
	void *next;
	skills_t myskills;
} gds_queue_t;

gds_queue_t *first;
gds_queue_t *last;

gds_queue_t *free_first = NULL;
gds_queue_t *free_last = NULL;
// CVARS
// cvar_t *gds_debug; // Should I actually use this? -az
// if gds_singleserver is set then bypass the isplaying check.
cvar_t *gds_singleserver;

// Threading
#ifndef GDS_NOMULTITHREADING

pthread_t QueueThread;
pthread_attr_t attr;
pthread_mutex_t QueueMutex;
pthread_mutex_t StatusMutex;
pthread_mutex_t MemMutex_Free;
pthread_mutex_t MemMutex_Malloc;
pthread_mutex_t ThreadStatusMutex;

qboolean ThreadRunning;

#endif

// Prototypes
int V_GDS_Save(gds_queue_t *myskills, MYSQL* db);
qboolean V_GDS_Load(gds_queue_t *current, MYSQL *db);
void V_GDS_SaveQuit(gds_queue_t *current, MYSQL *db);
int V_GDS_UpdateRunes(gds_queue_t *current, MYSQL* db);
void CreateProcessQueue();
qboolean IsThreadRunning();

// Utility Functions

// va used only in the thread.
char	*myva(char *format, ...)
{
	va_list		argptr;
	static char		string[1024];
	
	va_start (argptr, format);
	vsprintf (string, format,argptr);
	va_end (argptr);

	return string;	
}

int FindRuneIndex_S(int index, skills_t *myskills)
{
	int i;
	int count = 0;

	for (i = 0; i < MAX_VRXITEMS; ++i)
	{
		if (myskills->items[i].itemtype != ITEM_NONE)
		{
			++count;
			if (count == index)
			{
				return i;
			}
		}
	}
	return -1;	//just in case something messes up
}

int V_WeaponUpgradeVal_S(skills_t *myskills, int weapnum)
{
    //Returns an integer value, usualy from 0-100. Used as a % of maximum upgrades statistic

	int	i;
	float iMax, iCount;
	float val;
	
	iMax = iCount = 0.0f;

	if ((weapnum >= MAX_WEAPONS) || (weapnum < 0))
		return -666;	//BAD weapon number

	for (i=0; i<MAX_WEAPONMODS;++i)
	{
		iCount += myskills->weapons[weapnum].mods[i].current_level;
		iMax += myskills->weapons[weapnum].mods[i].soft_max;
	}

	if (iMax == 0)
		return 0;

	val = (iCount / iMax) * 100;
	
	return (int)val;
}

int CountWeapons_S(skills_t *myskills)
{
	int i;
	int count = 0;
	for (i = 0; i < MAX_WEAPONS; ++i)
	{
		if (V_WeaponUpgradeVal_S(myskills, i) > 0)
			count++;
	}
	return count;
}


int CountRunes_S(skills_t* myskills)
{
	int count = 0;
	int i;

	for (i = 0; i < MAX_VRXITEMS; ++i)
	{
		if (myskills->items[i].itemtype != ITEM_NONE)
			++count;
	}
	return count;
}

int CountAbilities_S(skills_t *myskills)
{
	int i;
	int count = 0;
	for (i = 0; i < MAX_ABILITIES; ++i)
	{
		if (!myskills->abilities[i].disable)
			++count;
	}
	return count;
}

int FindAbilityIndex_S(int index, skills_t *myskills)
{
    int i;
	int count = 0;
	for (i = 0; i < MAX_ABILITIES; ++i)
	{
		if (!myskills->abilities[i].disable)
		{
			++count;
			if (count == index)
				return i;
		}
	}
	return -1;	//just in case something messes up
}

int FindWeaponIndex_S(int index, skills_t *myskills)
{
	int i;
	int count = 0;
	for (i = 0; i < MAX_WEAPONS; ++i)
	{
		if (V_WeaponUpgradeVal_S(myskills, i) > 0)
		{
			count++;
			if (count == index)
				return i;
		}
	}
	return -1;	//just in case something messes up
}


// *********************************
// QUEUE functions
// *********************************
void V_GDS_FreeQueue_Add(gds_queue_t *current)
{
	if (current)
	{
		if (!free_first)
		{
			free_last = free_first = current;
		}
		else
		{
			free_last->next = current;
			free_last = current;
			free_last->next = NULL;
		}
	}
}

void V_GDS_FreeMemory_Queue()
{
	gds_queue_t *next;
	
	while (free_first)
	{
		next = free_first->next;
		V_Free (free_first);
		free_first = next;
	}

	free_last = free_first = NULL;
}

void V_GDS_Queue_Push(gds_queue_t *current, qboolean lock)
{
#ifndef GDS_NOMULTITHREADING
	if (lock)
		pthread_mutex_lock(&QueueMutex);
#endif
	
	if (current)
	{
		last->next = current;
		last = current;
		last->next = NULL;
	}

#ifndef GDS_NOMULTITHREADING
	if (lock)
		pthread_mutex_unlock(&QueueMutex);
#endif
}

void V_GDS_Queue_Add(edict_t *ent, int operation)
{
	int createque = 0;
	if ((!ent || !ent->client) && operation != GDS_EXITTHREAD)
	{
		gi.dprintf("V_GDS_Queue_Add: Null Entity or Client!\n");
	}

	if (!GDS_MySQL)
	{
		gi.cprintf(ent, PRINT_HIGH, "It seems that there's no connection to the database. Contact an admin ASAP.\n");
		return;
	}

	if (operation != GDS_SAVE && 
		operation != GDS_LOAD && 
		operation != GDS_SAVECLOSE &&
		operation != GDS_SAVERUNES &&
		operation != GDS_EXITTHREAD)
	{
		gi.dprintf("V_GDS_Queue_Add: Called with wrong operation!\n");
		return;
	}

	if (operation == GDS_EXITTHREAD && ent)
	{
		//if (gds_debug->value)
			gi.dprintf("V_GDS_Queue_Add: Called with an entity on an exit thread operation?\n");
	}

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&QueueMutex);
#endif

	if (!first)
	{
		first = V_Malloc(sizeof(gds_queue_t), TAG_GAME);
		last = first;
		first->ent = NULL;
		if (!IsThreadRunning())
			createque = 1; // Create a thread- likely there's nothing processing.
	}
	
	if (first->ent) // warning: we assume first is != NULL!
	{
		gds_queue_t *newest; // Start by the last slot

		// Use this empty next node 
		// for a new node
		newest = V_Malloc(sizeof(gds_queue_t), TAG_GAME); 

		V_GDS_Queue_Push(newest, false); // false since we're already locked
	}

	last->ent = ent;
	last->next = NULL; // make sure last node has null pointer 
	last->operation = operation;
	last->id = ent? ent->PlayerID : 0;

	if (operation == GDS_SAVE || 
		operation == GDS_SAVECLOSE ||
		operation == GDS_SAVERUNES)
	{
		memcpy(&last->myskills, &ent->myskills, sizeof(skills_t));
	}

	if (createque && !IsThreadRunning()) // double check nothing is getting processed
		CreateProcessQueue(); // Let's assume the queue is done since there are no elements.
	
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&QueueMutex);
#endif
}

gds_queue_t *V_GDS_Queue_PopFirst()
{
	gds_queue_t *node;

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&QueueMutex);
#endif
	node = first; // save node on top to delete
	
	if (first && first->operation) // does our node have something to do?
		first = first->next; // set first to next node

	// give me the assgined entity of the first node
	// node must be freed by caller
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&QueueMutex);
#endif
	return node; 
}

// Find a latest save operation where the character name = current one
gds_queue_t *V_GDS_FindSave(gds_queue_t *current)
{
	gds_queue_t *iterator, *previous_element = NULL;
	gds_queue_t *delete_queue_start = NULL, *delete_queue_end = NULL;

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&QueueMutex);
#endif

	/* We can safely assume the first is not already in use. */
	iterator = first;

	while (iterator) // We got a valid node
	{
		if (!strcmp(iterator->ent->client->pers.netname, current->myskills.player_name))
		{ // The name is the same..
			if (iterator->operation == GDS_SAVE || iterator->operation == GDS_SAVECLOSE)
			{ // And the operation is a save- use it and pop it out.

				if (previous_element != NULL) // we got an element that came before us
				{
					// set it to what comes after iterator, skipping this node.
					/* 
						note to self: if iterator->next is also a save of the same player then what happens?
						
						a save node (aka bad node) is left pointing to a node, 
						while the real first node that doesn't satisfy
						the condition of being a save of this player is left pointing to this node, or at least
						a node that should be skipped.

						We should iterate to the last of them, add them to the delete que, and leave
						previous_element to be the one that comes after the last followed element
						that is a save.

						It should be so rare that this is good enough. Hopefully. -az
					*/
					previous_element->next = iterator->next; 
				}

				if (!delete_queue_start) // No deleting queue?
				{
					delete_queue_start = delete_queue_end = iterator; // Create it using the popped element
				}else // There's at least one element
				{
					delete_queue_end->next = iterator; // Add this last popped element into this queue
					delete_queue_end = iterator; // assign it to the last popeed element
				}

			}
		}
		
		previous_element = iterator; // Set "previous element" to the current one
		iterator = iterator->next; // iterate to next element in queue.
	}

	if (delete_queue_start != delete_queue_end) // The delete list does exist
	{
		gds_queue_t *current = delete_queue_start;
		while (current != delete_queue_end) // While we iterate through the nodes that are in fact, redundant
		{
			V_GDS_FreeQueue_Add(current); // Add this node that is not the one we're returning to the free queue.
			current = current->next; // iterate through.
		}
	}

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&QueueMutex);
#endif

	return delete_queue_end;
}

// *********************************
// Database Thread
// *********************************

qboolean IsThreadRunning()
{
#ifndef GDS_NOMULTITHREADING
	qboolean temp;
	pthread_mutex_lock(&ThreadStatusMutex);

	temp = ThreadRunning;

	pthread_mutex_unlock(&ThreadStatusMutex);
	return temp;
#else
	return true;
#endif
}

void SetThreadRunning(qboolean running)
{
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&ThreadStatusMutex);

	ThreadRunning = running;

	pthread_mutex_unlock(&ThreadStatusMutex);
#endif
}

void *ProcessQueue(void *unused)
{
	gds_queue_t *current = V_GDS_Queue_PopFirst();

	while (current)
	{
#ifndef GDS_NOMULTITHREADING
		if (!current)
		{
			SetThreadRunning(false);
			pthread_exit(NULL);
			continue;
		}
#endif
		
		if (current->operation == GDS_LOAD)
		{
			V_GDS_Load(current, GDS_MySQL);
		}else if (current->operation == GDS_SAVE)
		{
			V_GDS_Save(current, GDS_MySQL);
		}else if (current->operation == GDS_SAVECLOSE)
		{
			V_GDS_SaveQuit(current, GDS_MySQL);
		}else if (current->operation == GDS_SAVERUNES)
		{
			V_GDS_UpdateRunes(current, GDS_MySQL);
		}
		
		V_GDS_FreeQueue_Add(current);
		current = V_GDS_Queue_PopFirst();
	}

#ifndef GDS_NOMULTITHREADING
	SetThreadRunning(false);
	pthread_exit(NULL);
#endif
	return NULL;
}

// *********************************
// MYSQL functions
// *********************************

#define QUERY(a, ...) format = strdup(myva(a, __VA_ARGS__));\
	mysql_query(db, format);\
	free (format);

#define GET_RESULT result = mysql_store_result(db);\
	row = mysql_fetch_row(result);

#define FREE_RESULT mysql_free_result(result);

int V_GDS_GetID(gds_queue_t *current, MYSQL *db)
{
	char *format;
	MYSQL_ROW row;
	MYSQL_RES *result;
	char escaped[32];
	int id;

	mysql_real_escape_string(db, escaped, current->myskills.player_name, strlen(current->myskills.player_name));

	QUERY ("CALL GetCharID(\"%s\", @PID);", escaped);

	mysql_query (db, "SELECT @PID;"); 

	GET_RESULT;

	id = atoi(row[0]);

	FREE_RESULT;

	return id;
}
// GDS_Save except it only deals with runes.
int V_GDS_UpdateRunes(gds_queue_t *current, MYSQL* db)
{
	int numRunes = CountRunes_S(&current->myskills);
	char* format;
	int id;
	int i;

	id = V_GDS_GetID(current, db);
	mysql_autocommit(db, false);

	mysql_query(db, "START TRANSACTION");

	QUERY("DELETE FROM runes_meta WHERE char_idx=%d", id);
	QUERY("DELETE FROM runes_mods WHERE char_idx=%d", id);
	
	//begin runes
	for (i = 0; i < numRunes; ++i)
	{
		int index = FindRuneIndex_S(i+1, &current->myskills);
		if (index != -1)
		{
			int j;

			QUERY (MYSQL_INSERTRMETA, id, 
				index,
				current->myskills.items[index].itemtype,
				current->myskills.items[index].itemLevel,
				current->myskills.items[index].quantity,
				current->myskills.items[index].untradeable,
				current->myskills.items[index].id,
				current->myskills.items[index].name,
				current->myskills.items[index].numMods,
				current->myskills.items[index].setCode,
				current->myskills.items[index].classNum);
			for (j = 0; j < MAX_VRXITEMMODS; ++j)
			{
				QUERY(MYSQL_INSERTRMOD, id, index,
					current->myskills.items[index].modifiers[j].type,
					current->myskills.items[index].modifiers[j].index,
					current->myskills.items[index].modifiers[j].value,
					current->myskills.items[index].modifiers[j].set);
			}
		}
	}
	//end runes
	
	mysql_query(db, "COMMIT"); // hopefully this will avoid shit breaking
	mysql_autocommit(db, true);

	return 0;
}

int V_GDS_Save(gds_queue_t *current, MYSQL* db)
{
	int i;
	int id; // character ID
	int numAbilities = CountAbilities_S(&current->myskills);
	int numWeapons = CountWeapons_S(&current->myskills);
	int numRunes = CountRunes_S(&current->myskills);
	char escaped[32];
	char *format;
	MYSQL_ROW row;
	MYSQL_RES *result;

	if (!db)
	{
/*		if (gds_debug->value)
			gi.dprintf("DB: NULL database (V_GDS_Save())\n"); */
		return -1;
	}

	mysql_autocommit(db, false);

	mysql_real_escape_string(db, escaped, current->myskills.player_name, strlen(current->myskills.player_name));

	QUERY ("CALL CharacterExists(\"%s\", @exists);", escaped);
	
	QUERY ("SELECT @exists;", escaped);

	GET_RESULT;

	if (!strcmp(row[0], "0"))
		i = 0;
	else
		i = 1;
	
	FREE_RESULT;

	if (!i) // does not exist
	{
		// Create initial database.
		gi.dprintf("DB: Creating character \"%s\"!\n", current->myskills.player_name);
		QUERY ("CALL FillNewChar(\"%s\");", escaped);
	}

	id = V_GDS_GetID(current, db);

	{ // real saving
		mysql_query(db, "START TRANSACTION");
		// reset tables (remove records for reinsertion)
		QUERY("CALL ResetTables(\"%s\");", escaped);

		QUERY (MYSQL_UPDATEUDATA, 
		 current->myskills.title,
		 current->myskills.player_name,
		 current->myskills.password,
		 current->myskills.email,
		 current->myskills.owner,
 		 current->myskills.member_since,
		 // current->myskills.last_played,
		 current->myskills.total_playtime,
 		 current->myskills.playingtime, id);

		// talents
		for (i = 0; i < current->myskills.talents.count; ++i)
		{
			QUERY (MYSQL_INSERTTALENT, id, current->myskills.talents.talent[i].id,
				current->myskills.talents.talent[i].upgradeLevel,
				current->myskills.talents.talent[i].maxLevel);
		}

		// abilities
	
		for (i = 0; i < numAbilities; ++i)
		{
			int index = FindAbilityIndex_S(i+1, &current->myskills);
			if (index != -1)
			{
				QUERY (MYSQL_INSERTABILITY, id, index, 
					current->myskills.abilities[index].level,
					current->myskills.abilities[index].max_level,
					current->myskills.abilities[index].hard_max,
					current->myskills.abilities[index].modifier,
					(int)current->myskills.abilities[index].disable,
					(int)current->myskills.abilities[index].general_skill);
			}
		}
		// gi.dprintf("saved abilities");
		
		//*****************************
		//in-game stats
		//*****************************

		QUERY (MYSQL_UPDATECDATA,
		 current->myskills.weapon_respawns,
		 current->myskills.current_health,
		 MAX_HEALTH(current->ent),
		 current->ent->client ? current->ent->client->pers.inventory[body_armor_index] : 0,
  		 MAX_ARMOR(current->ent),
 		 current->myskills.nerfme,
		 current->myskills.administrator, // flags
		 current->myskills.boss,  id); // last param WHERE char_idx = %d

		//*****************************
		//stats
		//*****************************

		QUERY (MYSQL_UPDATESTATS, 
		 current->myskills.shots,
		 current->myskills.shots_hit,
		 current->myskills.frags,
		 current->myskills.fragged,
  		 current->myskills.num_sprees,
 		 current->myskills.max_streak,
		 current->myskills.spree_wars,
		 current->myskills.break_sprees,
		 current->myskills.break_spree_wars,
		 current->myskills.suicides,
		 current->myskills.teleports,
		 current->myskills.num_2fers, id);
		
		//*****************************
		//standard stats
		//*****************************
		
		QUERY(MYSQL_UPDATEPDATA, 
		 current->myskills.experience,
		 current->myskills.next_level,
         current->myskills.level,
		 current->myskills.class_num,
		 current->myskills.speciality_points,
 		 current->myskills.credits,
		 current->myskills.weapon_points,
		 current->myskills.respawn_weapon,
		 current->myskills.talents.talentPoints, id);

		//begin weapons
		for (i = 0; i < numWeapons; ++i)
		{
			int index = FindWeaponIndex_S(i+1, &current->myskills);
			if (index != -1)
			{
				int j;
				QUERY (MYSQL_INSERTWMETA, 
					id, 
					index,
				    current->myskills.weapons[index].disable);			

				for (j = 0; j < MAX_WEAPONMODS; ++j)
				{
					QUERY (MYSQL_INSERTWMOD, id, index, j,
					    current->myskills.weapons[index].mods[j].level,
					    current->myskills.weapons[index].mods[j].soft_max,
					    current->myskills.weapons[index].mods[j].hard_max);
				}
			}
		}
		//end weapons

		//begin runes
		for (i = 0; i < numRunes; ++i)
		{
			int index = FindRuneIndex_S(i+1, &current->myskills);
			if (index != -1)
			{
				int j;

				QUERY (MYSQL_INSERTRMETA, id, 
				 index,
				 current->myskills.items[index].itemtype,
				 current->myskills.items[index].itemLevel,
				 current->myskills.items[index].quantity,
				 current->myskills.items[index].untradeable,
				 current->myskills.items[index].id,
				 current->myskills.items[index].name,
				 current->myskills.items[index].numMods,
				 current->myskills.items[index].setCode,
				 current->myskills.items[index].classNum);

				for (j = 0; j < MAX_VRXITEMMODS; ++j)
				{
					QUERY(MYSQL_INSERTRMOD, id, index, j,
					    current->myskills.items[index].modifiers[j].type,
					    current->myskills.items[index].modifiers[j].index,
					    current->myskills.items[index].modifiers[j].value,
					    current->myskills.items[index].modifiers[j].set);
				}
			}
		}
		//end runes

		QUERY (MYSQL_UPDATECTFSTATS, 
			current->myskills.flag_pickups,
			current->myskills.flag_captures,
			current->myskills.flag_returns,
			current->myskills.flag_kills,
			current->myskills.offense_kills,
			current->myskills.defense_kills,
			current->myskills.assists, id);

	} // end saving

	mysql_query(db, "COMMIT;");

	if (current->ent->client)
		if (current->ent->client->pers.inventory[power_cube_index] > current->ent->client->pers.max_powercubes)
			current->ent->client->pers.inventory[power_cube_index] = current->ent->client->pers.max_powercubes;

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&StatusMutex);
#endif

	if (current->ent->PlayerID == current->id)
		current->ent->ThreadStatus = 3;

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&StatusMutex);
#endif

	mysql_autocommit(db, true);

	return id;
}

void V_GDS_SetPlaying(gds_queue_t *current, MYSQL *db)
{
	char escaped[32];
	char *format;

	if (current->ent->ThreadStatus == 1)
	{
		mysql_real_escape_string(db, escaped, current->ent->client->pers.netname, strlen(current->ent->client->pers.netname));

		QUERY("UPDATE userdata SET isplaying = 1 WHERE playername=\"%s\"", escaped);
	}
}

qboolean V_GDS_Load(gds_queue_t *current, MYSQL *db)
{
	char* format;
	int numAbilities, numWeapons, numRunes;
	int i, exists;
	int id;
	edict_t *player = current->ent;
	char escaped[32];
	gds_queue_t *otherq;
	MYSQL_ROW row;
	MYSQL_RES *result, *result_b;

	if (!db)
	{
		return false;
	}

	if (player->PlayerID != current->id)
		return false; // Heh.

	mysql_real_escape_string(db, escaped, current->ent->client->pers.netname, strlen(current->ent->client->pers.netname));

	QUERY ("CALL CharacterExists(\"%s\", @Exists);", escaped);

	mysql_query (db, "SELECT @Exists;"); 

	GET_RESULT;

	exists = atoi(row[0]);

	if (exists == 0)
	{
#ifndef GDS_NOMULTITHREADING
		pthread_mutex_lock(&StatusMutex);
#endif
		player->ThreadStatus = 2;
#ifndef GDS_NOMULTITHREADING
		pthread_mutex_unlock(&StatusMutex);
#endif
		return false;
	}

	FREE_RESULT;

	QUERY ("CALL GetCharID(\"%s\", @PID);", escaped);
	
	mysql_query (db, "SELECT @PID;"); 

	GET_RESULT;

	id = atoi(row[0]);

	FREE_RESULT;

	if (exists) // Exists? Then is it able to play?
	{
		if (otherq = V_GDS_FindSave(current)) // Got a save in the que, use that instead.
		{  
			memcpy(&current->ent->myskills, &otherq->myskills, sizeof(skills_t));

#ifndef GDS_NOMULTITHREADING
			pthread_mutex_lock(&StatusMutex);
#endif

			if ( (i = canJoinGame(player)) == 0) 
			{
				if (player->PlayerID == current->id)
				{
					player->ThreadStatus = 1; // You can play.

					QUERY ("CALL CanPlay(%d, @IsAble);", id);
					mysql_query (db, "SELECT @IsAble;");
					GET_RESULT;

					if (atoi(row[0]) == 0 && !gds_singleserver->value)
					{
						player->ThreadStatus = 4; // Already playing.
						FREE_RESULT;
						return false;
					}

					FREE_RESULT;

					V_GDS_SetPlaying(current, db);
					V_GDS_FreeQueue_Add(otherq); // Remove this, now.
				}
			}else
			{
				V_GDS_Queue_Push(otherq, true); // push back into the que.
				player->ThreadStatus = i;
			}
#ifndef GDS_NOMULTITHREADING
			pthread_mutex_unlock(&StatusMutex);
#endif
			return i == 0; // success.
		} // No save in q? proceed as before


		QUERY ("CALL CanPlay(%d, @IsAble);", id);
		mysql_query (db, "SELECT @IsAble;");
		GET_RESULT;

		if (row && row[0])
		{
			if (atoi(row[0]) == 0 && !gds_singleserver->value)
			{
#ifndef GDS_NOMULTITHREADING
				pthread_mutex_lock(&StatusMutex);
#endif
				player->ThreadStatus = 4; // Already playing.
#ifndef GDS_NOMULTITHREADING
				pthread_mutex_unlock(&StatusMutex);
#endif
				FREE_RESULT;
				return false;
			}
		}else
		{
			char* error = mysql_error(GDS_MySQL);
			if (error)
				gi.dprintf("DB: %s\n", error);
		}

		FREE_RESULT;
	}

	QUERY( "SELECT * FROM userdata WHERE char_idx=%d", id );

	GET_RESULT;

	if (row)
	{

		strcpy(player->myskills.title, row[1]);
		strcpy(player->myskills.player_name, row[2]);
		strcpy(player->myskills.password, row[3]);
		strcpy(player->myskills.email, row[4]);
		strcpy(player->myskills.owner, row[5]);
		strcpy(player->myskills.member_since, row[6]);
		strcpy(player->myskills.last_played, row[7]);
		player->myskills.total_playtime =  atoi(row[8]);

		player->myskills.playingtime = atoi(row[9]);
	}
	else return false;

	FREE_RESULT;

	if (player->PlayerID != current->id)
		return false; // Don't waste time...

	QUERY( "SELECT COUNT(*) FROM talents WHERE char_idx=%d", id );
	
	GET_RESULT;
    //begin talents
	player->myskills.talents.count = atoi(row[0]);

	FREE_RESULT;

	QUERY( "SELECT * FROM talents WHERE char_idx=%d", id );

	GET_RESULT;

	for (i = 0; i < player->myskills.talents.count; ++i)
	{
		//don't crash.
        if (i > MAX_TALENTS)
			return false;

		player->myskills.talents.talent[i].id = atoi(row[1]);
		player->myskills.talents.talent[i].upgradeLevel = atoi(row[2]);
		player->myskills.talents.talent[i].maxLevel = atoi(row[3]);

		row = mysql_fetch_row(result);
		if ( !row )
			break;
	}

	FREE_RESULT;
	//end talents

	if (player->PlayerID != current->id)
		return false; // Still here.

	QUERY( "SELECT COUNT(*) FROM abilities WHERE char_idx=%d", id );

	GET_RESULT;

	//begin abilities
	numAbilities = atoi(row[0]);
	
	FREE_RESULT;

	QUERY( "SELECT * FROM abilities WHERE char_idx=%d", id );

	GET_RESULT

	for (i = 0; i < numAbilities; ++i)
	{
		int index;
		index = atoi(row[1]);

		if ((index >= 0) && (index < MAX_ABILITIES))
		{
			player->myskills.abilities[index].level			= atoi(row[2]);
			player->myskills.abilities[index].max_level		= atoi(row[3]);
			player->myskills.abilities[index].hard_max		= atoi(row[4]);
			player->myskills.abilities[index].modifier		= atoi(row[5]);
			player->myskills.abilities[index].disable		= atoi(row[6]);
			player->myskills.abilities[index].general_skill = atoi(row[7]);
			
			row = mysql_fetch_row(result);
			if ( !row )
				break;
		}
	}
	//end abilities

	FREE_RESULT;

	if (player->PlayerID != current->id)
		return false; // Patient enough...

	QUERY( "SELECT COUNT(*) FROM weapon_meta WHERE char_idx=%d", id );

	GET_RESULT;

	//begin weapons
    numWeapons = atoi(row[0]);
	
	FREE_RESULT;

	QUERY( "SELECT * FROM weapon_meta WHERE char_idx=%d", id );

	result_b = mysql_store_result(db);
	row = mysql_fetch_row(result_b);

	for (i = 0; i < numWeapons; ++i)
	{
		int index;
		index = atoi(row[1]);

		if ((index >= 0 ) && (index < MAX_WEAPONS))
		{
			int j;
			player->myskills.weapons[index].disable = atoi(row[2]);

			QUERY ("SELECT * FROM weapon_mods WHERE weapon_index=%d AND char_idx=%d", index, id);

			GET_RESULT;

			if (row)
			{
				for (j = 0; j < MAX_WEAPONMODS; ++j)
				{
				
					player->myskills.weapons[index].mods[atoi(row[2])].level = atoi(row[3]);
					player->myskills.weapons[index].mods[atoi(row[2])].soft_max = atoi(row[4]);
					player->myskills.weapons[index].mods[atoi(row[2])].hard_max = atoi(row[5]);
				
					row = mysql_fetch_row(result);
					if (!row)
						break;
				}
			}

			FREE_RESULT;

			
		}

		row = mysql_fetch_row(result_b);
		if (!row)
			break;

	}

	mysql_free_result(result_b);
	//end weapons

	if (player->PlayerID != current->id)
		return false; // Why quit now?

	//begin runes

	QUERY ("SELECT COUNT(*) FROM runes_meta WHERE char_idx=%d", id);

	GET_RESULT;

	numRunes = atoi(row[0]);

	FREE_RESULT;

	QUERY( "SELECT * FROM runes_meta WHERE char_idx=%d", id );

	GET_RESULT;

	for (i = 0; i < numRunes; ++i)
	{
		int index;
		index = atoi(row[1]);
		if ((index >= 0) && (index < MAX_VRXITEMS))
		{
			int j;
			player->myskills.items[index].itemtype = atoi(row[2]);
			player->myskills.items[index].itemLevel = atoi(row[3]);
			player->myskills.items[index].quantity = atoi(row[4]);
			player->myskills.items[index].untradeable = atoi(row[5]);
			strcpy(player->myskills.items[index].id, row[6]);
			strcpy(player->myskills.items[index].name, row[7]);
			player->myskills.items[index].numMods = atoi(row[8]);
			player->myskills.items[index].setCode = atoi(row[9]);
			player->myskills.items[index].classNum = atoi(row[10]);

			QUERY ("SELECT * FROM runes_mods WHERE rune_index=%d AND char_idx=%d", index, id);

			result_b = mysql_store_result(db);
			
			if (result_b)
				row = mysql_fetch_row(result_b);
			else
				row = NULL;

			if (row)
			{
				for (j = 0; j < MAX_VRXITEMMODS; ++j)
				{
					player->myskills.items[index].modifiers[atoi(row[2])].type = atoi(row[3]);
					player->myskills.items[index].modifiers[atoi(row[2])].index = atoi(row[4]);
					player->myskills.items[index].modifiers[atoi(row[2])].value = atoi(row[5]);
					player->myskills.items[index].modifiers[atoi(row[2])].set = atoi(row[6]);

					row = mysql_fetch_row(result_b);
					if (!row)
						break;
				}
			}

			mysql_free_result(result_b);
		}

		row = mysql_fetch_row(result);
		if (!row)
			break;
	}

	FREE_RESULT;
	//end runes

	if (player->PlayerID != current->id)
		return false; // Almost there.


	//*****************************
	//standard stats
	//*****************************

	QUERY("SELECT * FROM point_data WHERE char_idx=%d", id);

	GET_RESULT;

	//Exp
	player->myskills.experience =  atoi(row[1]);
	//next_level
	player->myskills.next_level =  atoi(row[2]);
	//Level
	player->myskills.level =  atoi(row[3]);
	//Class number
	player->myskills.class_num = atoi(row[4]);
	//skill points
	player->myskills.speciality_points = atoi(row[5]);
	//credits
	player->myskills.credits = atoi(row[6]);
	//weapon points
	player->myskills.weapon_points = atoi(row[7]);
	//respawn weapon
	player->myskills.respawn_weapon = atoi(row[8]);
	//talent points
	player->myskills.talents.talentPoints = atoi(row[9]);

	FREE_RESULT;

	QUERY("SELECT * FROM character_data WHERE char_idx=%d", id);

	GET_RESULT;


	//*****************************
	//in-game stats
	//*****************************
	//respawns
	player->myskills.weapon_respawns = atoi(row[1]);
	//health
	player->myskills.current_health = atoi(row[2]);
	//max health
	player->myskills.max_health = atoi(row[3]);
	//armour
	player->myskills.current_armor = atoi(row[4]);
	//max armour
	player->myskills.max_armor = atoi(row[5]);
	//nerfme			(cursing a player maybe?)
	player->myskills.nerfme = atoi(row[6]);

	//*****************************
	//flags
	//*****************************
	//admin flag
	player->myskills.administrator = atoi(row[7]);
	//boss flag
	player->myskills.boss = atoi(row[8]);


	FREE_RESULT;

	//*****************************
	//stats
	//*****************************

	QUERY( "SELECT * FROM game_stats WHERE char_idx=%d", id );

	GET_RESULT;

	//shots fired
	player->myskills.shots = atoi(row[1]);
	//shots hit
	player->myskills.shots_hit = atoi(row[2]);
	//frags
	player->myskills.frags = atoi(row[3]);
	//deaths
	player->myskills.fragged = atoi(row[4]);
	//number of sprees
	player->myskills.num_sprees = atoi(row[5]);
	//max spree
	player->myskills.max_streak = atoi(row[6]);
	//number of wars
	player->myskills.spree_wars = atoi(row[7]);
	//number of sprees broken
	player->myskills.break_sprees = atoi(row[8]);
	//number of wars broken
	player->myskills.break_spree_wars = atoi(row[9]);
	//suicides
	player->myskills.suicides = atoi(row[10]);
	//teleports			(link this to "use tballself" maybe?)
	player->myskills.teleports = atoi(row[11]);
	//number of 2fers
	player->myskills.num_2fers = atoi(row[12]);

	FREE_RESULT;

	QUERY( "SELECT * FROM ctf_stats WHERE char_idx=%d", id);

	GET_RESULT;

	//CTF statistics
	player->myskills.flag_pickups =  atoi(row[1]);
	player->myskills.flag_captures =  atoi(row[2]);
	player->myskills.flag_returns =  atoi(row[3]);
	player->myskills.flag_kills =  atoi(row[4]);
	player->myskills.offense_kills =  atoi(row[5]);
	player->myskills.defense_kills =  atoi(row[6]);
	player->myskills.assists =  atoi(row[7]);
	//End CTF

	FREE_RESULT;

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
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&StatusMutex);
#endif
	
	if ( (i = canJoinGame(player)) == 0)
	{
		if (player->PlayerID == current->id)
		{
			player->ThreadStatus = 1; // You can play! :)
			V_GDS_SetPlaying(current, db);
		}
	}else
		player->ThreadStatus = i;

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&StatusMutex);
#endif

	return true;
}

void V_GDS_SaveQuit(gds_queue_t *current, MYSQL *db)
{
	int id = V_GDS_Save(current, db);
	char* format;
	if (id != -1)
		QUERY ("UPDATE userdata SET isplaying = 0 WHERE char_idx = %d;", id);
}

// Start Connection to GDS/MySQL

void CreateProcessQueue()
{
	int rc;

	// gi.dprintf ("DB: Creating thread...");
	rc = pthread_create(&QueueThread, &attr, ProcessQueue, NULL);
	
	if (rc)
	{
		gi.dprintf(" Failure creating thread! %d\n", rc);
		return;
	}/*else
		gi.dprintf(" Done!\n");*/
	SetThreadRunning(true);
}

qboolean V_GDS_StartConn()
{
	int rc;
	char* database, *user, *pw, *dbname;

	gi.dprintf("DB: Initializing connection... ");

	gds_singleserver = gi.cvar("gds_single", "1", 0); // default to a single server using sql.

	database = Lua_GetStringSetting("dbaddress");
	user = Lua_GetStringSetting("username");
	pw = Lua_GetStringSetting("dbpass");
	dbname = Lua_GetStringSetting("databasename");

	if (!database)
		database = DEFAULT_DATABASE;

	if (!user)
		user = MYSQL_USER;

	if (!pw)
		pw = MYSQL_PW;

	if (!dbname)
		dbname = MYSQL_DBNAME;

	if (!GDS_MySQL)
	{
		GDS_MySQL = mysql_init(NULL);
		if (mysql_real_connect(GDS_MySQL, database, user, pw, dbname, 0, NULL, 0) == NULL)
		{
			gi.dprintf("Failure: %s\n", mysql_error(GDS_MySQL));
			mysql_close(GDS_MySQL);
			GDS_MySQL = NULL;
			return false;
		}
	}else
	{
		if (GDS_MySQL)
		{
			gi.dprintf("DB: Already connected\n");
		}
		return false;
	}

	/*gds_debug = gi.cvar("gds_debug", "0", 0);*/

	gi.dprintf("Success!\n");
	
#ifndef GDS_NOMULTITHREAD

	pthread_mutex_init(&QueueMutex, NULL);
	pthread_mutex_init(&StatusMutex, NULL);
	if (rc = pthread_mutex_init(&ThreadStatusMutex, NULL))
		gi.dprintf("mutex creation err: %d", rc);

	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

#endif

	return true;
}

void Mem_PrepareMutexes()
{
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_init(&MemMutex_Malloc, NULL);
	pthread_mutex_init(&MemMutex_Free, NULL);
#endif
}

qboolean CanUseGDS()
{
	return GDS_MySQL != NULL;
}

#ifndef GDS_NOMULTITHREADING

void HandleStatus(edict_t *player)
{
	if (!player)
		return;

	if (!player->inuse)
		return;

	if (savemethod->value != 2)
		return;

	if (!G_IsSpectator(player)) // don't handle plyers that are already logged in!
		return;
		
	if (!GDS_MySQL)
		return;

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&StatusMutex);
#endif

	if (player->ThreadStatus == 0)
	{
#ifndef GDS_NOMULTITHREADING
		pthread_mutex_unlock(&StatusMutex);
#endif
		return;
	}

	switch (player->ThreadStatus)
	{
	case 4:
		gi.centerprintf(player, "You seem to be already playing in this or another server.\nIf you're not, wait until tomorrow or ask an admin\nto free your character.\n");
	case 3:
		/*if (player->inuse)
			gi.cprintf(player, PRINT_LOW, "Character saved!\n");*/
		break;
	case 2: // does not exist?
		gi.centerprintf(player, "Creating a new Character!\n");
		newPlayer(player);
		OpenModeMenu(player);
		break;
	case 1:
		gi.centerprintf(player, "Your character was loaded succesfully.");
		OpenModeMenu(player);
		break;
	case 0:
		break;
	default:
		CanJoinGame(player, player->ThreadStatus); //Sends out correct message.
	}
	if (player->ThreadStatus != 1)
		player->ThreadStatus = 0;
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&StatusMutex);
#endif
}

void GDS_FinishThread()
{
	void *status;
	int rc;

	if (GDS_MySQL)
	{
		V_GDS_Queue_Add(NULL, GDS_EXITTHREAD);

		gi.dprintf("DB: Finishing thread... ");
		rc = pthread_join(QueueThread, &status);
		gi.dprintf(" Done.\n", rc);

		if (rc)
			gi.dprintf("pthread_join: %d\n", rc);

		rc = pthread_mutex_destroy(&QueueMutex);
		if (rc)
			gi.dprintf("pthread_mutex_destroy: %d\n", rc);

		rc = pthread_mutex_destroy(&StatusMutex);
		if (rc)
			gi.dprintf("pthread_mutex_destroy: %d\n", rc);

		V_GDS_FreeMemory_Queue();
		mysql_close(GDS_MySQL);
		GDS_MySQL = NULL;
	}
}
#else

void GDS_FinishThread () {}
void HandleStatus () {}

#endif

// az end

#endif NO_GDS

void *V_Malloc(size_t Size, int Tag)
{
	void *Memory;

#if (!defined GDS_NOMULTITHREADING) && (!defined NO_GDS)
	pthread_mutex_lock(&MemMutex_Malloc);
#endif
	Memory = gi.TagMalloc (Size, Tag);

#if (!defined GDS_NOMULTITHREADING) && (!defined NO_GDS)
	pthread_mutex_unlock(&MemMutex_Malloc);
#endif

	return Memory;
}

void V_Free(void *mem)
{
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&MemMutex_Free);
#endif

	gi.TagFree (mem);

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&MemMutex_Free);
#endif
}
