/**
 * e_game.c
 */ 

#include "g_local.h"
#include "m_player.h"

#define ENT_READSIZE			10240
#define ENT_HUNKSIZE			ENT_READSIZE*3

props_t *gProperties = NULL;

// String equivalents of settings
char e_bits[NUM_SETTINGS][25] = 
{
"expert weapons",
"balanced items",
"free gear",
"powerups",
"no powerups",
"expert hook",
"no hacks",
"playerid",
"enforced teams",
"fair teams",
"no team switch",
"pogo",
"slow hook",
"sky solid",
"no plats",
"ctf team distribution",
"alternate restore",
"ammo regen"
};

// indexes for various items
int shell_index;
int bullet_index;
int grenade_index;
int rocket_index;
int slug_index;
int cell_index;

// Expert: additional cvars
cvar_t *version;			// DLL version
cvar_t *gamedir;			// Game directory

// bitflag settings cvars
cvar_t *ctf;				// CTF flags
cvar_t *flagtrack;			// FlagTrack flags
cvar_t *sv_expflags;		// Expert flags
cvar_t *sv_utilflags;		// Obituary/GibStat/Utility flags
cvar_t *sv_arenaflags;		// Expert Arena flags

// value settings cvars
cvar_t *capturelimit;		// Captures before level ends
cvar_t *levelCycle;			// Directory to get level-specific .cfgs from
cvar_t *sv_numteams;		// Number of teams
cvar_t *sv_pace;			// Game pace multiplier
cvar_t *sv_lethality;		// Damage multiplier

// Expert: Obituary/Context CVars
cvar_t	*sv_giblog;			// GibStats Log Filename

// What action to take when a bot is detected
cvar_t  *botaction;

// Paused cvar
cvar_t	*sv_paused;

// Player forwarding
cvar_t *forwardAddress;
cvar_t *forwardMessage;

#ifndef AMIGA
// Solaris support
int main(int argc, char *argv[]) { return 0; }
#endif

/**
 * Called from InitGame at game DLL load time.  Set up cvars used by Expert, etc
 */
void ExpertGameInits() {

	// Expert: Custom cvars

	// options bitvectors
	sv_expflags = gi.cvar("expflags", "0", CVAR_SERVERINFO);
	sv_utilflags = gi.cvar("utilflags", "0", CVAR_SERVERINFO);
	sv_arenaflags = gi.cvar("arenaflags", "0", CVAR_SERVERINFO);
	
	// lethality: scales amount of damage everything in the game does
	sv_lethality = gi.cvar("lethality", "1", CVAR_SERVERINFO);
	
	// pace: scales the speeds of the player, projectiles, and other velocities
	sv_pace = gi.cvar("pace", "1", CVAR_SERVERINFO);

	// number of requested teams
	sv_numteams = gi.cvar("numteams", "0", CVAR_SERVERINFO);

	// for level cycles
	levelCycle = gi.cvar("cycle", "", CVAR_SERVERINFO);

	// gibstats
	sv_giblog = gi.cvar("giblog", EXPERT_GIB_FILENAME, 0);

	// what action to take on detecting a bot
	botaction = gi.cvar("botaction", "kick", 0);

	// for server browsing
	gi.cvar("expert", EXPERT_VERSION, CVAR_SERVERINFO|CVAR_NOSET);
	gi.cvar("mode", "deathmatch", CVAR_SERVERINFO);

	// for dir access
	gamedir = gi.cvar("game", "", CVAR_NOSET);
	
	// CTF
	ctf = gi.cvar("ctf", "0", CVAR_SERVERINFO);
	capturelimit = gi.cvar ("capturelimit", "7", CVAR_SERVERINFO);

	// FlagTrack
	flagtrack = gi.cvar("flagtrack", "0", CVAR_SERVERINFO);

	// Pausing
	sv_paused = gi.cvar("sv_paused", "0", CVAR_SERVERINFO);

	// Player forwarding
	forwardAddress = gi.cvar("forwardAddress", "127.0.0.1", 0);
	forwardMessage = gi.cvar("forwardMessage", 
			"This server is full.  You are being forwarded to another server", 0);

	// global properties table
	gProperties = newProps();
}

void ExpertLevelScripting(char *mapname)
{
	// Expert: level scripting

	// exec "eachlev.cfg" every level
	if (strlen(levelCycle->string) > 0) {
		gi.AddCommandString(va("exec %s/eachlev.cfg\n", levelCycle->string));
		// in case last command in .cfg doesn't have a CR
		gi.AddCommandString("\n");
		// Exec a .cfg file with the same name as the level we are about to go to
		gi.AddCommandString(va("exec %s/%s.cfg\n", levelCycle->string, mapname));
		gi.AddCommandString("\n");
	} else {
		// same except no path separator
		gi.AddCommandString(va("exec eachlev.cfg\n", levelCycle->string));
		gi.AddCommandString("\n");
		gi.AddCommandString(va("exec %s.cfg\n", levelCycle->string, mapname));
		gi.AddCommandString("\n");
	}
}

/**
 * Called by SpawnEntities at the beginning of each level, after 
 * tagmalloc'd memory from the previous level has been freed.
 * SpawnEntities is called by the engine.
 */
void ExpertLevelInits() {

	int i;

	shell_index = ITEM_INDEX(FindItem("Shells"));
	bullet_index = ITEM_INDEX(FindItem("Bullets"));
	grenade_index = ITEM_INDEX(FindItem("Grenades"));
	rocket_index = ITEM_INDEX(FindItem("Rockets"));
	slug_index = ITEM_INDEX(FindItem("Slugs"));
	cell_index = ITEM_INDEX(FindItem("Cells"));

	// determine the gametype for various printouts
	if (ctf->value) 
	{
		gametype = GAME_CTF;
	} 
	else if (((int)dmflags->value) & DF_SKINTEAMS || ((int)dmflags->value) & DF_MODELTEAMS) 
	{
		gametype = GAME_TEAMPLAY;
	}
	else
	{
		gametype = GAME_DM;
	}
	
	// set the "mode" cvar to the current gametype for server browsers
	gi.cvar_set("mode", MODESTRING);

	// To allow players more air time after hook releases,
	// and allow longer range arcing projectiles.
	// FIXME : causes an engine crash
	if (expflags & EXPERT_WEAPONS) {
		//gi.cvar_set("sv_gravity", 650);
	}

	// Expert: Reconnect detection
	// On level change, set all players to "disconnected" state.
	// The players who are actually connected will be set back to
	// CONNECTED state when they spawn.
	// Zombies left behind by players who drop connection without
	// ClientDisconnect() being called will no longer trigger 
	// reconnect detection after a level change.
	for (i=0 ; i<game.maxclients ; i++)
	{
		game.clients[i].pers.connectState = DISCONNECTED;
	}

	// Init CTF
	if (ctf->value) {
		CTFInit();
	}
	
	// Initialize FlagTrack
	if (flagtrack->value) {
		FlagTrackInit();
	}

	// Initialize Arena
	if (arenaflags & EXPERT_ENABLE_ARENA) {
		arenaInitLevel();
	}

	// Initialize the teamplay system
	if (teamplayEnabled()) {
		loadTeams();
	}

	// Initialize MOTD. (can't be called from InitGame because tags are freed above)
	if ( !(utilflags & EXPERT_DISABLE_MOTD) ) {
		InitMOTD();
	}

	// Initialize Obituary (can't be called from InitGame for same reason)
	if ( !(utilflags & EXPERT_DISABLE_CLIENT_OBITUARIES) ) {
		InitExpertObituary();
	} 

	// Initialize TeamAudio
	if (teamplayEnabled())
		InitTeamAudio();

	// Start GibStat logging if applicable.
	if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING)
	{
		gsStopLogging();
		gsStartLogging();
		gsLogLevelStart();
		gsEnumConnectedClients();
	}

}

// Delayed init that occurs every level, 5 seconds after completed connection (ClientBegin)
void ExpertPlayerDelayedInits(edict_t *player) 
{
	// These stuffcmds are huge enough to cause unreliable dumps or even overflows
	// if sent during connect.

	// Some likely attempts at getting help
	StuffCmd(player, "alias help \"cmd motd\";alias settings \"cmd settings\";\n");
	StuffCmd(player, "alias motd \"cmd motd\";alias serverhelp \"cmd motd\";\n");

	// SwitchFire aliases
	StuffCmd(player, "alias back \"-attack;cmd weaplast\";\n");
	StuffCmd(player, "alias +sg \"use shotgun;+attack\";alias -sg back;\n");
	StuffCmd(player, "alias +ssg \"use super shotgun;+attack\";alias -ssg back;\n");
	StuffCmd(player, "alias +mg \"use machinegun;+attack\";alias -mg back;\n");
	StuffCmd(player, "alias +cg \"use chaingun;+attack\";alias -cg back;\n");
	StuffCmd(player, "alias +gl \"use grenade launcher;+attack\";alias -gl back;\n");
	StuffCmd(player, "alias +rl \"use rocket launcher;+attack\";alias -rl back;\n");
	StuffCmd(player, "alias +hb \"use hyperblaster;+attack\";alias -hb back;\n");
	StuffCmd(player, "alias +rg \"use railgun;+attack\";alias -rg back;\n");
	StuffCmd(player, "alias +bfg \"use bfg10k;+attack\";alias -bfg back;\n");
	StuffCmd(player, "alias +hg \"use grenades;+attack\";alias -hg back;\n");

	// for Expert-specific bindings in CTF and other game modes
	StuffCmd(player, "exec expauto.cfg\n");

	player->client->resp.delayedInit = true;
}

// InitCmds: Sends commands to the client every level load.
void InitCmds(edict_t *player)
{
	// Player forwarding
	if (utilflags & EXPERT_ENABLE_PLAYER_FORWARDING &&
		player->client->resp.toBeForwarded)
	{
		gi.cprintf(player, PRINT_HIGH, "%s\nConnecting you to %s\n", 
				forwardMessage->string, forwardAddress->string);
		StuffCmd(player, va("\nconnect %s\n", forwardAddress->string));
		return;
	}

	// Add aliases for the grappling hook and for pogo
	StuffCmd(player, "alias +hook +use; alias -hook -use;\n");
	StuffCmd(player, "alias +pogo +use; alias -pogo -use;\n");
}

// Called during ClientBegin, which is called for each player
// every level change and on initial connect.
// NOTE: this function is basically ClientEnterPlay plus some per-level stuff
void ExpertPlayerLevelInits(edict_t *player) {

	// mark as having entered play
	player->client->pers.connectState = CONNECTED;

	// Expert Matrix: Add the player
	//ExpandMatrix (ent);

	// Expert: now ok to switch into observer mode
	player->client->resp.canObserve = true;

	// Expert: begin bot testing
	player->client->resp.botflags = 0;

	// Expert: clear floodprot
	clearFloodSamples(player);

	// Expert: Display MOTD
	if ( !(utilflags & EXPERT_DISABLE_MOTD) )
		DisplayMOTD(player);

	// Expert: Send any necessary commands to the client
	InitCmds(player);

	// Expert: If teamplay is enabled, assign the player to a team
	if (teamplayEnabled()) { 
		assignTeam(player);
	}

	// Expert: GibStats Logging
	if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING)
		gsLogClientConnect(player);

	// Expert Arena: add the player to the arena 
	// spectator queue or the arena itself
	if ((int)sv_arenaflags->value & EXPERT_ENABLE_ARENA) {
		arenaConnect(player);
	}
}

void ClientLeavePlay(edict_t *player) {

	// Expert Arena: remove the player from the arena queue
	if (arenaflags & EXPERT_ENABLE_ARENA) {
		arenaDisconnect(player);
	}

	// Expert CTF
	if (ctf->value)
		CTFDeadDropFlag(player);

	// Expert FlagTrack
	if (flagtrack->value) {
		tossFlag(player);
	}

	// Expert Teamplay
	if (teamplayEnabled()) {
		teamDisconnect(player);
	}
}

// called on player disconnect
void ExpertPlayerDisconnect(edict_t *player) {

	ClientLeavePlay(player);

	// Expert: GibStats Logging
	if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING)
		gsLogClientDisconnect(player);

	// Expert Matrix: Remove the player
	//ContractMatrix(player);

	markNotObserver(player);
}

/**
 *  Inhibit additional entities in various modes.  Returning
 *  true indicates the entity should be freed.
 */
qboolean ExpertInhibit(edict_t *ent) {

	// if any non-blank property exists named after this weapon's pickup
	// name, don't spawn it
	gitem_t *item = FindItemByClassname(ent->classname);
	if (item && getProp(gProperties, item->pickup_name)) {
		return true;
	}

	// Inhibit all weapons in free gear mode
	if (expflags & EXPERT_FREE_GEAR &&
	    strstr(ent->classname, "weapon")) {
		return true;
	}

	if (expflags & EXPERT_AMMO_REGEN &&
	    strstr(ent->classname, "ammo")) 
	{
		return true;
	}

	// Inhibit critical all powerups in no powerups mode
	if (expflags & EXPERT_NO_POWERUPS && 
	    (strstr(ent->classname, "item_power") ||
	     strcmp(ent->classname, "item_quad") == 0 ||
	     strcmp(ent->classname, "item_invulnerability") == 0)) 
	{
		return true;
	}

	// All health and armor restoration removed in Alternate Restore
	if (expflags & EXPERT_ALTERNATE_RESTORE &&
	    (strstr(ent->classname, "item_armor") ||
		 strstr(ent->classname, "item_health") ||
	     strcmp(ent->classname, "item_ancient_head") == 0 ||
	     strcmp(ent->classname, "item_adrenaline") == 0)) 
	{
		return true;
	}

	// Inhibit plats in No Plats mode.  This is intended for games
	// with the hook or pogo, where plats aren't necessary for getting
	// around, and instead get in the way.  Also useful to alter level
	// dynamics such as in Capture Showdown (q2ctf5).  Should generally 
	// be enabled on a level-by-level basis. 
	// Note: we could remove doors, too, but some doors are areaportals.
	// We could leave the areaportals open all the time at a performance penalty.
	if (expflags & EXPERT_NO_PLATS &&
	    Q_stricmp(ent->classname, "func_plat") == 0)
	{
		return true;
	}

	// Other on-the-fly changes that are not inhibits

	// Under Expert weapons, transmogrify BFG into an SSG.  
	// BFG is ok for one on one, but too effective in larger games
	if (expflags & EXPERT_WEAPONS) {
		if (strcmp(ent->classname, "weapon_bfg") == 0) {
			ent->classname = "weapon_supershotgun";
		}
	}

	// Translate LMCTF flag and team spawn points
	if (!strcmp(ent->classname, "info_player_red")) {
		ent->classname = "info_player_team1";
	} else if (!strcmp(ent->classname, "info_player_blue")) {
		ent->classname = "info_player_team2";
	} else if (!strcmp(ent->classname, "info_flag_red")) {
		ent->classname = "info_flag_team1";
	} else if (!strcmp(ent->classname, "info_flag_blue")) {
		ent->classname = "info_flag_team2";
	}

	return false;

}

/**
 * Various changes to the itemlist. Must be done on the fly at
 * level load time in order to support the changes as optional.  We do
 * this once, immediately before "worldspawn" is spawned, since worldspawn
 * sets up configstrings using the itemlist.
 */
void ExpertItemListChanges() {

	// Expert: Switch pickup names when Expert powerups are used
	if (expflags & EXPERT_POWERUPS)
	{
		itemlist[ITEM_INDEX(FindItemByClassname("item_quad"))].pickup_name = "Vampirism";
		itemlist[ITEM_INDEX(FindItemByClassname("item_invulnerability"))].pickup_name = "Mutant Jump";
	}

	// Expert: Switch pickup names when balanced items is in effect
	if (expflags & EXPERT_BALANCED_ITEMS)
	{
		itemlist[ITEM_INDEX(FindItemByClassname("item_power_shield"))].pickup_name = "Inertial Screen";
	}
	
	// Expert: Alternate armor stats
	if (expflags & EXPERT_BALANCED_ITEMS)
	{
		itemlist[ITEM_INDEX(FindItem("Body Armor"))].info = &balanced_bodyarmor_info;
		itemlist[ITEM_INDEX(FindItem("Combat Armor"))].info = &balanced_combatarmor_info;
		itemlist[ITEM_INDEX(FindItem("Jacket Armor"))].info = &balanced_jacketarmor_info;
	}

}

void sendBotCheck(edict_t *player, unsigned int bit) 
{
	// zbot specific
	if (bit == BOT_CHECK_BANG_STUFFCMD) {
		StuffCmd(player, va("!%d", random()));
	}

	// other tests to possible be implemented later
/*
	// proxy checking tests

	// a random stuffcmd prefixed with !
	StuffCmd(player, va("!%d", random()));

	// some tests that could be used to detect 
	// full custom clients

	// the server should never see this command since 
	// it refers to a client cvar
	StuffCmd(player, "cl_particles");
	// should appear in userinfo
	StuffCmd(player, "set notabot 1 u");
	// test that aliasing facilities are present on client
	StuffCmd(player, "alias test \"cmd !zbot\";test");
*/
}

void ExpertBotDetect(edict_t *player)
{
	unsigned int bits;
	unsigned int checkBit;
	int i;

	// passed all tests
	if (player->client->resp.botflags & BOT_CHECK_FLAGS_MASK)
		return;

	// retry every 3 seconds, with 4 retries
	// if client fails 4 retries, boot
	if (player->client->resp.botRetries > 4) {
		BootPlayer(player, "Proxy bots not allowed.",
				"Appears to be using a bot");
		gi.dprintf("WARNING: Client %s appears to be using a bot;\n"
					" did not pass through stuffcmd prefixed with ! character\n"
					"IP is %s\n", player->client->pers.netname, 
					Info_ValueForKey (player->client->pers.userinfo, "ip"));
	}
	
	// hasn't passed all checks, but not time to retry yet
	if (level.time < player->client->resp.nextBotRetry) {
		return;
	}

	// retry all not yet passed tests
	bits = numberOfBitsSet(BOT_CHECK_FLAGS_MASK);
	for (i = 0; i < bits; i ++) {
		checkBit = 1 << i;
		if (!(checkBit & player->client->resp.botflags))
		{
			sendBotCheck(player, checkBit);
		}
	}

	// set up to do a retry of all checks that haven't been passed
	player->client->resp.nextBotRetry = level.time + 3;
	player->client->resp.botRetries++;
}

void ShowGun(edict_t *ent)
{
        int nIndex;
        char *pszIcon;

        // No weapon?
        if (!ent->client->pers.weapon)
        {
                ent->s.modelindex2 = 0;
                return;
        }

        // Determine the weapon's precache index.

        nIndex = 0;
        pszIcon = ent->client->pers.weapon->icon;

        if ( strcmp( pszIcon, "w_blaster") == 0)
                nIndex = 1;
        else if ( strcmp( pszIcon, "w_shotgun") == 0)
                nIndex = 2;
        else if ( strcmp( pszIcon, "w_sshotgun") == 0)
                nIndex = 3;
        else if ( strcmp( pszIcon, "w_machinegun") == 0)
                nIndex = 4;
        else if ( strcmp( pszIcon, "w_chaingun") == 0)
                nIndex = 5;
        else if ( strcmp( pszIcon, "a_grenades") == 0)
                nIndex = 6;
        else if ( strcmp( pszIcon, "w_glauncher") == 0)
                nIndex = 7;
        else if ( strcmp( pszIcon, "w_rlauncher") == 0)
                nIndex = 8;
        else if ( strcmp( pszIcon, "w_hyperblaster") == 0)
                nIndex = 9;
        else if ( strcmp( pszIcon, "w_railgun") == 0)
                nIndex = 10;
        else if ( strcmp( pszIcon, "w_bfg") == 0)
                nIndex = 11;
        else if ( strcmp( pszIcon, "w_grapple") == 0)
                nIndex = 12;

        // Clear previous weapon model.
        ent->s.skinnum &= 255;

        // Set new weapon model.
        ent->s.skinnum |= (nIndex << 8);
        ent->s.modelindex2 = 255;
}


// ShardPoints: Returns how many armor points to add to a player's armor type.
int ShardPoints(int armor_index)
{
	gitem_armor_t *armor = (gitem_armor_t *) itemlist[armor_index].info;

	if (armor == NULL || !(itemlist[armor_index].flags & IT_ARMOR))
	{
		gi.error ("Bad armor given to ShardPoints\n");
		return 2;
	}

	if (expflags & EXPERT_BALANCED_ITEMS)
	{
		// God bless floating-point division rounding errors
		return (BALANCED_SHARD_POINTS / armor->normal_protection) + 0.6;
	}
	else
		return 2;
}

/**
 * Give a client the "free gear" set of weapons and ammo.
 * Returns the weapon to start with.
 */
gitem_t *giveFreeGear(gclient_t *client) {

	gitem_t		*item;

	// give weapons 

	// if any non-blank property exists named after a weapon's pickup
	// name, don't give it out on spawn

	if (!getProp(gProperties, "Shotgun")) {
		item = FindItem("Shotgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}

	if (!getProp(gProperties, "Super Shotgun")) {
		item = FindItem("Super Shotgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}

	if (!getProp(gProperties, "Machinegun")) {
		item = FindItem("Machinegun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	
	if (!getProp(gProperties, "Chaingun")) {
		item = FindItem("Chaingun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	
	if (!getProp(gProperties, "Grenade Launcher")) {
		item = FindItem("Grenade Launcher");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	
	if (!getProp(gProperties, "Rocket Launcher")) {
		item = FindItem("Rocket Launcher");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
		
	if (!getProp(gProperties, "HyperBlaster")) {
		item = FindItem("HyperBlaster");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}

	if (!getProp(gProperties, "Railgun")) {
		item = FindItem("Railgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}

	// give ammo
	item = FindItem("Shells");
	client->pers.inventory[ITEM_INDEX(item)] = 25;

	item = FindItem("Bullets");
	client->pers.inventory[ITEM_INDEX(item)] = 100;

	item = FindItem("Cells");
	client->pers.inventory[ITEM_INDEX(item)] = 40;

	item = FindItem("Grenades");
	client->pers.inventory[ITEM_INDEX(item)] = 5;

	item = FindItem("Slugs");
	// Expert : more ammo for balanced version of railgun
	if (expflags & EXPERT_WEAPONS) {
		client->pers.inventory[ITEM_INDEX(item)] = 15;
	} else {
		client->pers.inventory[ITEM_INDEX(item)] = 5;
	}
	
	item = FindItem("Rockets");
	client->pers.inventory[ITEM_INDEX(item)] = 10;

	// weapon to start with
	return FindItem("Rocket Launcher");
		
}

// Alternate Restore system
// Regenerate health if in combat or recently in combat
#define REGEN_DELAY							2	
#define REGEN_HEALTH						10	
#define KILL_INSTANT_HEALTH					25
#define HIT_REGEN_RATIO						0.2

void alternateRestoreKill(edict_t *attacker, edict_t *target) {

	if (attacker == target || attacker->deadflag) {
		return;
	}

	// give health
	if (attacker->health < attacker->max_health) {
		attacker->health += KILL_INSTANT_HEALTH;
		if (attacker->health > attacker->max_health) {
			attacker->health = attacker->max_health;
		}
	}
}


void damageRestore(edict_t *attacker, edict_t *targ, int damage) {

	// take - the actual damage taken not considering going below 0
	int take = (targ->health < 0) ? damage - targ->health : damage;

	// both attacker and targ are players, attacker and targ
	// are different players, attacker is not dead, and 
	// attacker and targ are not on the same team if teamplay 
	// is enabled
	if (!attacker->client || !targ->client ||
		attacker == targ || attacker->deadflag || 
		onSameTeam(attacker, targ)) {
		return;
	}

	// In Alternate Restore mode give health back on every hit
	if (expflags & EXPERT_ALTERNATE_RESTORE) {
		if (attacker->health < attacker->max_health) {
			attacker->health += ceil(HIT_REGEN_RATIO * damage);
			if (attacker->health > attacker->max_health) {		
				attacker->health = attacker->max_health;
			}
		}
	}

	// Attacker has Vampire Artifact
	if (attacker->client->quad_framenum > level.framenum && 
		expflags & EXPERT_POWERUPS)
	{
		if (expflags & EXPERT_ALTERNATE_RESTORE) {
			// steal less health, since all damage is health (no armor)
			// higher max, since health max is already higher
			attacker->health += take/3;
			if (attacker->health > ALTERNATE_MAX_VAMPIRE_HEALTH)
				attacker->health = ALTERNATE_MAX_VAMPIRE_HEALTH;
		} else {
			attacker->health += take/2;
			if (attacker->health > MAX_VAMPIRE_HEALTH)
				attacker->health = MAX_VAMPIRE_HEALTH;
		}
	}

}
	

// Ammo Regen mode is intended to limit ammo just enough to 
// prevent abuses like continuously filling a corridor with 
// greandes or holding a corridor with repeated rocket fire.

#define 	AMMO_REGEN_DELAY 	3	

void giveAmmo(edict_t *player) {

	gclient_t *client = player->client;
	
	// give ammo
	client->pers.inventory[shell_index] += 2;
	if (client->pers.inventory[shell_index] > 20)
		client->pers.inventory[shell_index] = 20;

	client->pers.inventory[bullet_index] += 20;
	if (client->pers.inventory[bullet_index] > 200)
		client->pers.inventory[bullet_index] = 200;

	// grenades in large quantities are extremely easy 
	// to abuse, so the limit is very low.  
	// Note: Under Expert Weapons with Ammo Regen, hand grenades take a
	// fraction of the ammo that grenades launched from the grenade
	// launcher take, since large amounts of hand grenades under Expert 
	// weapons can't be abused the way grenades lauched from the grenade
	// launcher can
	client->pers.inventory[grenade_index] += 1;
	if (client->pers.inventory[grenade_index] > 4)
		client->pers.inventory[grenade_index] = 4;

	client->pers.inventory[rocket_index] += 1;
	if (client->pers.inventory[rocket_index] > 12)
		client->pers.inventory[rocket_index] = 12;

	if (expflags & EXPERT_WEAPONS) {
		client->pers.inventory[slug_index] += 3;
		if (client->pers.inventory[slug_index] > 18)
			client->pers.inventory[slug_index] = 18;
	} else {
		client->pers.inventory[slug_index] += 1;
		if (client->pers.inventory[slug_index] > 6)
			client->pers.inventory[slug_index] = 6;
	}

	client->pers.inventory[cell_index] += 7;
	if (client->pers.inventory[cell_index] > 80)
		client->pers.inventory[cell_index] = 80;

}

// periodic regeneration of both ammo and health
void regen(edict_t *player) {

	int healthAmount;
	float rate;

	if (player->deadflag) {
		return;
	}

	// regenerate ammo
	if (expflags & EXPERT_AMMO_REGEN) {
		if (level.time - player->client->lastAmmo > AMMO_REGEN_DELAY) {
			giveAmmo(player);
			player->client->lastAmmo = level.time;
		}
	}

	// In Alternate Restore mode, there is no health or armor on the map, 
	// so players regenerate at a rate proportional to how injured they are.  
	if (!(expflags & EXPERT_ALTERNATE_RESTORE) ||
		player->health < 0 || // is dead
		level.time - player->client->lastRestore < REGEN_DELAY || // isn't time to regen yet
		player->health >= player->max_health) // doesn't need health
	{
		return;
	}

	// the more injured you are, the faster you regenerate
	rate = 1.0 - (float)player->health/(float)player->max_health;
	if (rate <= 0) return;
	healthAmount = ceil(rate * REGEN_HEALTH);

	// give health
	if (player->health < player->max_health) {
		player->health += healthAmount;
		if (player->health > player->max_health) {
			player->health = player->max_health;
		}
		// FIXME play different sounds for different amounts?
		// play a regen sound if regaining health, only for the player being regen'd
		// Note fractional volume based on rate of regen, max is 1
		unicastSound(player, gi.soundindex ("items/n_health.wav"), rate);
	}
	player->client->lastRestore = level.time;

}

// utility: return the index into the itemlist for the gitem_t of an armor 
// of type "armor". #defines of armor types are in g_local.h, and are
// stored in the gitem_armor_t->armor field
int indexForArmor(int armor) {

	switch (armor)
	{
		case ARMOR_JACKET: return jacket_armor_index; break;
		case ARMOR_COMBAT: return combat_armor_index; break;
		case ARMOR_BODY: return body_armor_index; break;
		default: return jacket_armor_index; break;
	}
}

// ---------- Modified pickup routines

void ExpertAddToDroppedWeapon(edict_t *drop, edict_t *self) {

	// If Expert Powerups AND balanced items, powerups are placed into the 
	// dropped weapon, and the weapon glows according to the powerups it holds
	// (balanced items needs to be set since only then is item dropping
	// guaranteed on every death)
	if (expflags & EXPERT_BALANCED_ITEMS &&
	    expflags & EXPERT_POWERUPS)
	{
		drop->included_invincibility = 0;
		drop->included_quad = 0;

		if (self->client->quad_framenum > level.framenum)
		{
			drop->included_quad = self->client->quad_framenum;
			drop->s.effects |= 0x80000000;
		}

		if (self->client->invincible_framenum > level.framenum)
		{
			drop->included_invincibility = self->client->invincible_framenum;
			drop->s.effects |= EF_PENT;
		}
	}

	// Expert: Don't rotate dropped items to 
	// help distinguish them from normal items.
	if (expflags & EXPERT_NO_HACKS) {
		drop->s.effects &= ~EF_ROTATE;
	}
}
	
// Give a player an armor shard
void giveShard(edict_t *player) {
	int armor_index;
	gitem_armor_t *armorinfo;

	armor_index = ArmorIndex (player);
	// if no armor, give jacket armor
	if (!armor_index) {
		armor_index = jacket_armor_index;
	}
	armorinfo = (gitem_armor_t *) itemlist[armor_index].info;
	// give different amounts of points according to armor type
	player->client->pers.inventory[armor_index] += ShardPoints(armor_index);

	// cap at armor max
	if (player->client->pers.inventory[armor_index] > armorinfo->max_count) {
		player->client->pers.inventory[armor_index] = armorinfo->max_count;
	}
}


void ExpertPickupDroppedWeapon(edict_t *ent, edict_t *other) {

	// If balanced items, then give the player the health, 
	// armor and powerups stored in the dropped weapon
	if (expflags & EXPERT_BALANCED_ITEMS) {

		// 30 restorative health in all dropped weapons
		if (other->health < other->max_health) {
			// In alternate restore, health is also given out immediately on kill
			if (expflags & EXPERT_ALTERNATE_RESTORE) {
				other->health += 20;
			} else {
				other->health += 30;
			}
			if (other->health > other->max_health) {		
				other->health = other->max_health;
			}
		}

		// no armor in Alternate Restore
		if (!(expflags & EXPERT_ALTERNATE_RESTORE)) {
			// give the player the equivalent of an armor shard
			giveShard(other);
		}

		// If both Balanced Items and Expert powerups,
		// powerups are included in the dropped weapon
		if (expflags & EXPERT_POWERUPS) {
			if (ent->included_quad > level.framenum)
			{
				if (other->client->quad_framenum < ent->included_quad)
				{
					gi.centerprintf(other, "You got the Vampire Artifact!\n\nYou receive as life\n"
						"points half the health\ndamage you do!\n");
					other->client->quad_framenum = ent->included_quad;
				}
				else // player had powerup already
					other->client->quad_framenum += ent->included_quad - level.framenum;
			}
		
			if (ent->included_invincibility > level.framenum)
			{
				if (other->client->invincible_framenum < ent->included_invincibility)
				{
					gi.centerprintf(other, "You got the Mutant Jump!\n\nNow you can jump like a Mutant!\n"
						"You are also invulnerable to slime,\nlava, and falling!");
					other->client->invincible_framenum = ent->included_invincibility;
				}
				else
					other->client->invincible_framenum += ent->included_invincibility - level.framenum;
			}
		}	
	}
}

// Ark of Life restores both armor and health.
// Only if you have both full armor and full health will you
// not pick up the Ark.
qboolean canPickupArkOfLife(edict_t *player) {

	if (player->health >= player->max_health) 
	{
		if (player->client->pers.inventory[jacket_armor_index] >= 160 || 
		    player->client->pers.inventory[combat_armor_index] >= 96 ||
		    player->client->pers.inventory[body_armor_index] >= 80)
		{
			return false;
		}
	}
}

void pickupArkOfLife(edict_t *ark, edict_t *player) {

	player->health += 30;
	// cap health.
	if (player->health > player->max_health) {
		player->health = player->max_health;
	}
	
	// give the equivalent of an armor shard
	giveShard(player);

	// respawn like a normal health pack
	SetRespawn (ark, 30);
}

// ---------- Dropped powerup management

void ItemEffects(edict_t *dropped)
{
//	gi.bprintf(PRINT_HIGH, "Called ItemEffects\n");
	
	// If either powerup has expired, remove it's effect
	if (dropped->s.effects & 0x80000000&& dropped->included_quad <= level.framenum)
	{
//		gi.bprintf(PRINT_HIGH, "Turning off quad..\n");
		dropped->s.effects &= ~0x80000000;
	}

	if (dropped->s.effects & EF_PENT && dropped->included_invincibility <= level.framenum)
	{
//		gi.bprintf(PRINT_HIGH, "Turning off wings..\n");
		dropped->s.effects &= ~EF_PENT;
	}

	// If either powerup remains, set a timer for when the earliest
	// powerup will expire.
	if (dropped->s.effects & 0x80000000 || dropped->s.effects & EF_PENT) 
	{
		if (dropped->s.effects & 0x80000000) {
//			gi.bprintf(PRINT_HIGH, "Setting nextthink to remove quad effect...\n");
			// quad effect should be removed when the player can no
			// longer get quad from the weapon.  One second buffer
			// to avoid a bunch of extra thinks
			dropped->nextthink = dropped->included_quad/10 + 1;
		}
		if (dropped->s.effects & EF_PENT)
		{
			float pentTimeout;

//			gi.bprintf(PRINT_HIGH, "Setting nextthink to remove pent effect..\n");
	
			pentTimeout = dropped->included_invincibility/10 + 1;
			// pent will expire before quad
			if (pentTimeout < dropped->nextthink) {
				dropped->nextthink = pentTimeout;
			}
		}
	}
	else
	{
//		gi.bprintf(PRINT_HIGH, "Setting nextthink to free item..\n");
		// note: this code becomes wrong if powerups last more than 30 seconds
		dropped->nextthink = dropped->drop_time + BALANCED_DROPPED_ITEM_TIME;
		dropped->think = G_FreeEdict;
	}
}

/** SwitchFire */

int firstIdleFrameForWeapon(gitem_t *weapon);

gitem_t *weaponForNumber(int weaponNum)
{
	switch (weaponNum) {
	case 1:
		return FindItem("blaster");
		break;
	case 2:
		return FindItem("shotgun");
		break;
	case 3:
		return FindItem("super shotgun");
		break;
	case 4:
		return FindItem("machinegun");
		break;
	case 5:
		return FindItem("chaingun");
		break;
	case 6:
		return FindItem("grenade launcher");
		break;
	case 7:
		return FindItem("rocket launcher");
		break;
	case 8:
		return FindItem("hyperblaster");
		break;
	case 9:
		return FindItem("railgun");
		break;
	case 10:
		return FindItem("bfg10k");
		break;
	}

}

void Cmd_SwitchFire_f (edict_t *player) 
{

	int i = 1;
	int weaponNum;
	gitem_t *weaponItem;

	if (!expflags & EXPERT_WEAPONS) {
		gi.cprintf(player, PRINT_HIGH, "Expert Weapons must be enabled for SwitchFire to be used\n");
		return;
	}

	if (gi.argc() < 2) {
		gi.cprintf(player, PRINT_HIGH, "Usage: sw [weapon] ([weapon] [weapon] ...)\n");
		return;
	}

	while (i < gi.argc())
	{
		weaponNum = atoi(gi.argv(i));
		if (weaponNum <= 0 || weaponNum > 10) 
		{
			gi.cprintf(player, PRINT_HIGH, "Invalid weapon number specified as weapon %d in chain.  "
					"Weapons are numbered from 1 (blaster) to 10 (bfg)\n", i);
			i++;
			continue;
		}
		// check possession of weapon
		weaponItem = weaponForNumber(weaponNum);
		if (!player->client->pers.inventory[ITEM_INDEX(weaponItem)]) 
		{
			// no error message if there are more weapons in the chain
			gi.cprintf(player, PRINT_HIGH, "You don't have the %s\n", weaponItem->pickup_name);
			i++;
			continue;		
		}
		// check ammo
		if (weaponNum != 1 &&
		    !player->client->pers.inventory[ITEM_INDEX(FindItem(weaponItem->ammo))])
		{
			gi.cprintf(player, PRINT_HIGH, "No %s for the %s\n", weaponItem->ammo, weaponItem->pickup_name);
			i++;
			continue;
		}
		// switch to weapon, fire weapon and break
		player->client->newweapon = weaponItem;
		ChangeWeapon(player);
		player->client->weaponstate = WEAPON_READY;
		player->client->ps.gunframe = firstIdleFrameForWeapon(player->client->pers.weapon);
		player->client->switchfiring = 1;
		player->client->pers.weapon->weaponthink (player);
		break;
	}

}

void wave(edict_t *ent, int waveNum) {

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (waveNum)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}


/*
 * Entmap loading code
 */

void LoadCustomEntmap(char *mapname, char **entities)
{
	FILE *fp;
	char filename[MAX_QPATH]={0};
	char buffer[ENT_READSIZE];
	char *tmpmap=NULL, *tmpmap2;
	int numbytes = 0, bytesread = 0;
	int bytehunk = 0;

	// Build filename
	strcat(filename, gamedir->string);
	strcat(filename, "/maps/");
	strcat(filename, mapname);
	strcat(filename, ".ent");


	// Try to load the entmap
	if ((fp = fopen(filename, "r")) == NULL) {
		return;
	}

	gi.cprintf(NULL, PRINT_HIGH, "Loading entmap from %s...\n", filename);
	
	// Read in all the text
	// FIXME: fgets'ing several hundred lines of text is not very efficient.
	// This needs to be rewritten to use something that ignores newlines.
	// fread() doesn't work now but it might with some added character translation
	while ((bytesread = fread(buffer, 1, ENT_READSIZE-1, fp)) != 0) {
		// Put the trailing NUL on the buffer because we read it in binary
		buffer[bytesread] = 0;

		numbytes += bytesread + 1;
		if (numbytes > bytehunk)
			bytehunk += ENT_HUNKSIZE;
		
		if (tmpmap == NULL) {
			// The string must first be initialized to 0 to be usuable.
			// Thus calloc is used. Also note that to avoid overwriting
			// the previous pointer value when NULL is returned, a temp
			// pointer is used to get the return value.
			if ((tmpmap2 = calloc(bytehunk, 1)) == NULL) {
				gi.dprintf("Error - can't allocate memory for entmap, releasing memory..\n");
				free(tmpmap);
				fclose(fp);
				return;
			}
		} else {
			if ((tmpmap2 = realloc(tmpmap, bytehunk)) == NULL) {
				gi.dprintf("Error - can't allocate memory for entmap, releasing memory..\n");
				free(tmpmap);
				fclose(fp);
				return;
			}
		}

		tmpmap = tmpmap2;
		strcat(tmpmap, buffer);
	}
	
	if (feof(fp) == 0)
		gi.dprintf("Error on fread: %s\n", ferror(fp));

	fclose(fp);

//	E_LogAppend("fread entmap", "entmap:\n%s\n", tmpmap);

	// Don't do anything if no data was read
	if (tmpmap == NULL)
		return;

	// Allocate tagged space for the entmap
	if ((tmpmap2 = gi.TagMalloc(numbytes, TAG_LEVEL)) == NULL) {
		gi.dprintf("Error - can't allocate memory for entmap, releasing memory..\n");
		free(tmpmap);
		return;
	}

	strcpy(tmpmap2, tmpmap);
	free(tmpmap);

	// Swap the entmaps
	*entities = tmpmap2;
}
