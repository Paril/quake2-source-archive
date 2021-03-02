/**
 * Expert Quake2 Teamplay Code
 * Rich "Publius" Tollerton
 * Charles "Myrkul" Kendrick
 */

#include <errno.h>
#include "g_local.h"

// Array of possible teams.  The .team field in ent->client->resp 
// is an index into this array. Note -1 or any negative number 
// indicates an invalid team.
typedef struct
{
	// name of team, case insensitive
	char *teamName;
	// paths for _either_ skins or models
	char *skinPaths;
	int skinPathLength;
	int memberCount;
	// unused: meant to be pointer to list of team members
	edict_t *teamList;
} gteam_t;

gteam_t gTeams[MAX_TEAMS]; 

/* --------------------- Utils ------------------- */

/**
 * Returns true if teamplay is active (whether by model or by skin)
 * false otherwise.
 */
qboolean teamplayEnabled()
{
	// Are we using skinteams or modelteams?
	if ((int)dmflags->value & (DF_SKINTEAMS | DF_MODELTEAMS))
		return true;

	return false;
}

/**
 * If teamplay is off, always false.  If teamplay is active
 * but EXPERT_ENFORCED_TEAMS is not active, always true.
 * If EXPERT_ENFORCED_TEAMS is active, true if the player
 * has been assigned to a team.
 */
qboolean playerIsOnATeam(edict_t *player)
{
	if (teamplayEnabled()) {
		if (expflags & EXPERT_ENFORCED_TEAMS) {
			return (player->client->resp.team != NOTEAM);
		} else {
			return true;
		}
	}

	return false;
}

// Returns true if the two players are on the same team, false otherwise
qboolean onSameTeam(edict_t *player1, edict_t *player2)
{
	if (!teamplayEnabled())
		return false;

	if (!player1 || !player2) {
		gi.dprintf("onSameTeam passed null player\n");
	}

	// non-clients never on same team
	if (!player1->client || !player2->client) {
		return false;
	}

	if (expflags & EXPERT_ENFORCED_TEAMS) {
		// under enforced teams, use the team number
		if (player1->client->resp.team == player2->client->resp.team)
			return true;
	} else {
		// unenforced teams, use id's teamplay check
		return OnSameTeam(player1, player2);
	}

	return false;
}

// Returns the team name for a given team number.
// Only valid under EXPERT_ENFORCED_TEAMS
char *nameForTeam(int team) {

	return gTeams[team].teamName;
}

// Responds with -1 if a bogus team name is given
// Only valid under EXPERT_ENFORCED_TEAMS
int teamForName(char *name) {
	int entry = 0;

	for (entry = 0; entry < (int)sv_numteams->value; entry++) {
		if (Q_stricmp(name, gTeams[entry].teamName) == 0)
			return entry;
	}

	return -1;
}

// return number of players on team "team"
int memberCount(int team) {
	return gTeams[team].memberCount;
}

// print info about teamplay; player's current
// team and the list of teams and what they are wearing
// Only valid under EXPERT_ENFORCED_TEAMS
void printTeamInfo(edict_t *player) {

	int teamEntry;

	if (playerIsOnATeam(player)) {
		gi.cprintf(player, PRINT_HIGH, 
			greenText(va("You are on team %s\n", nameForTeam(player->client->resp.team)))
				);
	}
	gi.cprintf(player, PRINT_HIGH, "List of all teams:\n");
	for (teamEntry = 0; teamEntry < (int)sv_numteams->value; teamEntry++) {
		gi.cprintf(player, PRINT_HIGH, "  %s: wearing %s\n", gTeams[teamEntry].teamName,
			gTeams[teamEntry].skinPaths);
	}

	gi.cprintf(player, PRINT_HIGH, 
		greenText(va("Type \"team [teamname]\" to change team\n")));

	return;
}

/**
 * Play a sound to all players on team "team"
 */
void teamSound(int team, int soundIndex, float volume) 
{
	int i;
	edict_t *player;

	player = g_edicts + 1;
	for (i = 1; i <= game.maxclients; i++) {
		if (player->inuse && player->client != NULL &&
		    player->client->resp.team == team)
		{
			unicastSound(player, soundIndex, volume);
		}
		player = player + 1;
	}
}

/**
 * Print a message for all players on team "team"
 */
void teamPrint(int team, int priority, char *message) 
{
	int i;
	edict_t *player;

	player = g_edicts + 1;
	for (i = 1; i <= game.maxclients; i++) {
		if (player->inuse && player->client != NULL &&
		    player->client->resp.team == team)
		{
			gi.cprintf(player, priority, message);
		}
		player = player + 1;
	}
}


qboolean validSkinName(char *skinName)
{
	// FIXME: Actually check if the files exist?
	// Check for null path, zero length path, path
	// with no slash, or skinName that ends in a slash
	if (skinName == NULL || strlen(skinName) == 0 || 
	    strcspn(skinName, "/") == strlen(skinName) ||
	    strcspn(skinName, "/") == (strlen(skinName) - 1)) {
		return false;
	}

	return true;
}

// internal utility functions
// both assume a string of form [model]/[skin]
// and return either the model or skin in
// a buffer.  Each can be called only twice before 
// clobbering previous results
char *modelFromString(char *modelSkin) {
	int modelLength; 
	static char model[2][512];	// use two buffers so compares
								// work without stomping on each other
	static int whichModel;
	whichModel ^= 1;

	// length of string before /
	modelLength = strcspn(modelSkin, "/");
	// copy just up to the /
	strncpy(model[whichModel], modelSkin, modelLength);
	// add null terminator
	model[whichModel][modelLength] = '\0';

	return model[whichModel];
}

char *skinFromString(char *modelSkin) {
	static char skin[2][512];	// use two buffers so compares
								// work without stomping on each other
	static int whichSkin;
	whichSkin ^= 1;

	// doesn't have a / or ends with one.. return "" 
	if (strcspn(modelSkin, "/") == strlen(modelSkin) ||
		strcspn(modelSkin, "/") == (strlen(modelSkin) - 1)) {
		return "";
	}

	// pointer into the string, just past the /
	strcpy(skin[whichSkin], modelSkin + strcspn(modelSkin, "/") + 1);
	return skin[whichSkin];
}

// returns the sum of the score of 
// all members of team "team"
// Only valid under EXPERT_ENFORCED_TEAMS
int totalTeamScore(int team)
{
	int i, teamTotal = 0;
	edict_t *player = NULL;

	for (i = 1; i <= game.maxclients; i++) {
		player = g_edicts + i;

		// ignore unused and invalid players, and players
		// not assigned to a team
		if (player->inuse == false ||
		    player->client == NULL ||
		    player->client->resp.team != team) {
			continue;
		}
		teamTotal += player->client->resp.score;
	}
	return teamTotal;
}

// Returns which team is the weaker of the two.
// Only valid under EXPERT_ENFORCED_TEAMS
int weakerTeam(int team1, int team2) {

	// FIXME : add more and better criteria here, a weighted combination of
	// 1. difference in number of players
	// 2. difference in current score
	// 3. difference in total rate of scoring of all players on each team,
	// supplemented by ping if a scoring rate has not been established

	if (gTeams[team1].memberCount < gTeams[team2].memberCount) {
		return team1;
	} else if (gTeams[team2].memberCount < gTeams[team1].memberCount) {
		return team2;
	}
	else /* equal number of players */ {

		// if two or more teams have the same number of members,
		// give the new player to the team with the least score
		if (totalTeamScore(team1) < totalTeamScore(team2)) {
			return team1;
		} else {
			return team2;
		}
	}
}

// Returns which team is most deserving of getting another player
// Only valid under EXPERT_ENFORCED_TEAMS
int weakestTeam(void)
{
	int i = 0;
	int bestCandidateTeam = 0;
	
	// Go through the team list and find which team has the lowest number of players
	for (i = 0; i < (int)sv_numteams->value; i++) {
		bestCandidateTeam = weakerTeam(i, bestCandidateTeam);
	}

	return bestCandidateTeam;
}

// In EXPERT_ENFORCED_TEAMS mode, called from ClientBegin to assign 
// a player to a team (ClientBegin is called every level cycle or 
// initial connect).
void assignTeam(edict_t *player)
{
	assignToTeam(player, NOTEAM);
}

void assignToTeam(edict_t *player, int teamToAssign) 
{
	int team = player->client->resp.team;

	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		return;
	}

	// If the player has just connected for the first time, resp.team 
	// will be NOTEAM (set in ClientConnect).
	if (team == NOTEAM) {
		if (teamToAssign == NOTEAM) {
			// assign to the weakest team
			team = weakestTeam();
		} else {
			// assign to the team we were passed as an argument
			team = teamToAssign;
		}
		addPlayerToTeam(player, team);

		gi.bprintf(PRINT_HIGH, "%s has been assigned to team %s\n", 
	           	player->client->pers.netname, nameForTeam(team));

		// tell the player about all the teams
		printTeamInfo(player);

		// make the player use the correct skin and model
		enforceTeamModelSkin(player);
		return;
	}

	// Otherwise resp.team will indicate the team the player was 
	// on last level so that we can record the number of players 
	// on each team in the gTeams array
	gTeams[team].memberCount++;
}

void addPlayerToTeam(edict_t *player, int newTeam)
{
	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		return;
	}

	// reset score on team switch
	player->client->resp.score = 0;
	player->client->resp.team = newTeam;
	gTeams[player->client->resp.team].memberCount++;
} 

void removePlayerFromTeam(edict_t *player)
{
	int i, playersLeft, playerGap, oldTeam;
	char *oldTeamName;

	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		return;
	}

	gTeams[player->client->resp.team].memberCount--;
	playersLeft = gTeams[player->client->resp.team].memberCount;

	oldTeam = player->client->resp.team;
	player->client->resp.team = NOTEAM;

	// if players are allowed to switch teams..
	if (expflags & EXPERT_NO_TEAM_SWITCH) {
		return;
	}

	// then inform the team with more players of 	
	// the possible team imbalance, so they can 
	// switch to balance teams
	oldTeamName = nameForTeam(oldTeam);

	for (i = 0; i < (int)sv_numteams->value; i++) {
		playerGap = gTeams[i].memberCount - playersLeft;
		if (playerGap > 1) {
			// computer update sound (not voice)
			teamSound(i, gi.soundindex("misc/comp_up.wav"), 1);
			// prominent printout
			teamPrint(i, PRINT_HIGH, greenText(va(
                                    ":::: Your team now has %d more\n" 
			                        "players than Team %s. ::::\n", 
			                        playerGap,
			                        oldTeamName)));
			
		}
	}
}

// Player disconnected.  Remove from team.
void teamDisconnect(edict_t *player)
{
	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		return;
	}

	if (playerIsOnATeam(player)) {
		gi.bprintf(PRINT_HIGH, "%s has left team %s\n", 
   	        	player->client->pers.netname, nameForTeam(player->client->resp.team));
		removePlayerFromTeam(player);
	}
}

/**
 * Switch a player to a new team, killing him
 * and resetting his team.
 */ 
void killAndSwitchTeam(edict_t *player, int desiredTeam) {

	// kill the player before team switch if already playing,
	// so there is no carryover of power/carried items
	// FIXME : creates bogus death message
	player->svflags = 0;
	player->flags &= ~FL_GODMODE;
	player->health = 0;
	player_die (player, player, player, 100000, vec3_origin);
	player->deadflag = DEAD_DEAD;

	// do the team change
	removePlayerFromTeam(player);
	addPlayerToTeam(player, desiredTeam);

	// spawn in base
	player->client->resp.ctf_state = CTF_STATE_START;

	// note respawn() should occur 
	// only once player joins new team
	respawn (player);

	// enforce the skin/models of the new team
	enforceTeamModelSkin(player);

	// log the team change
	if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING)
		gsTeamChange(player->client->pers.netname, nameForTeam(desiredTeam));

}

// Cmd_Team_f: Team utility function. Shows available teams and allows switching.
void Cmd_Team_f(edict_t *player)
{
	char *teamName;
	int desiredTeam;
		
	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		if (teamplayEnabled()) {
			// explain teamplay mode
			gi.cprintf(player, PRINT_HIGH, "Teams are not enforced\n");
			if ((int)dmflags->value & DF_SKINTEAMS) {
				gi.cprintf(player, PRINT_HIGH, "Team is determined by skin\n");
			} else if ((int)dmflags->value & DF_SKINTEAMS) {
				gi.cprintf(player, PRINT_HIGH, "Team is determined by model\n");
			}
		} else {
			gi.cprintf(player, PRINT_HIGH, "Teamplay not enabled\n");
		}
		return;
	}

	teamName = gi.args();

	// no argument  
	if (!*teamName) {
		printTeamInfo(player);
		return;
	}

	// Expert Floodprot
	if (floodProt(player)) {
		return;
	}

	// player is trying to switch teams
	desiredTeam = teamForName(teamName);

	// bogus team name	
	if (desiredTeam == -1) {
		gi.cprintf(player, PRINT_HIGH, "Unknown team \"%s\"\n", teamName);
		return;
	}

	// player is not assigned to a team
	if (!playerIsOnATeam(player)) {
		if (IsObserver(player)) {
			// if all teams have equal numbers of players, observers can join any
			// team they want.  This is to allow player swaps between even teams
			// via two players switching into observer mode then switching to each
			// other's teams.

			qboolean allEqual = true;
			int i;
			int firstTeam = gTeams[0].memberCount;
	
			for (i = 1; i < (int)sv_numteams->value; i++) {
				if (firstTeam != gTeams[i].memberCount) {
					allEqual = false;
					break;
				}
			}
			if (!allEqual) {
				// not all teams have equal numbers of players, assign to weakest team
				gi.cprintf(player, PRINT_HIGH, 
						"Teams are not even, so you are being\n"
						"assigned to the team that needs you most\n");
				ObserverToPlayer(player);
			} else {
				// all teams have equal numbers of players, give the joiner his
				// choice of team
				ObserverToTeam(player, desiredTeam);
			}
			return;
		}
		// if not an observer, player must have sent a command before 
		// being connected enough to be assigned to a team, ignore
		return;
	}

	// already on desired team
	if (player->client->resp.team == desiredTeam) {
		gi.cprintf(player, PRINT_HIGH, "You are already on team %s\n",
			nameForTeam(player->client->resp.team));
		return;
	}

	// all team switching disallow
	if (expflags & EXPERT_NO_TEAM_SWITCH) {
		gi.cprintf(player, PRINT_HIGH, "Team switching is disabled on this server"); 
		return;
	}

	// if in fair teams mode, only allow changing to a weaker team 
	if (expflags & EXPERT_FAIR_TEAMS) {
		if (!(gTeams[desiredTeam].memberCount < gTeams[player->client->resp.team].memberCount)) {
			gi.cprintf(player, PRINT_HIGH, "You can only change to a team with less players\n");
			return;
		}
	}

	// all criteria met, do the team change
	killAndSwitchTeam(player, desiredTeam);

	gi.bprintf(PRINT_HIGH, "%s switched to team %s\n",
		player->client->pers.netname, nameForTeam(desiredTeam));
}

// Debugging command. Looks at a particular
// entry from gTeams.
void Cmd_Examine_Teams(edict_t *player)
{
	int entry;

	entry = atoi (gi.argv(1));

	if (entry < 0 || entry >= (int)sv_numteams)
	{
		gi.cprintf(player, PRINT_HIGH, "Invalid team number.\n");
		return;
	}

	gi.cprintf(player, PRINT_HIGH, "Team Number %d:\n", entry);
	gi.cprintf(player, PRINT_HIGH, "  Name: %s\n", gTeams[entry].teamName);
	gi.cprintf(player, PRINT_HIGH, "  Skins: %s\n", gTeams[entry].skinPaths);
	gi.cprintf(player, PRINT_HIGH, "  Members: %d\n", gTeams[entry].memberCount);
	//gi.cprintf(player, PRINT_HIGH, "  teamList is unused - not printed\n");
}

/* --------------- Team enforcement ------------------ */

void setSkinAndModel(edict_t *player, char *modelSkin) {
	int playernum = player - g_edicts - 1;
	char *model = modelFromString(modelSkin);

	if (modelSkin == NULL) {
		gi.dprintf("setSkinAndModel passed null model/skin path\n");
		return;
	}

	// set skin and model as configstring
	gi.configstring(CS_PLAYERSKINS + playernum,
		va("%s\\%s", player->client->pers.netname, modelSkin));
	
	// set the model
	player->model = va("players/%s/tris.md2", model);
}

// Rates how well two paths match for models/skins. Higher
// ratings indicate a better match.
static int getPathRating(char *allowedModelAndSkin, char *chosenModelAndSkin)
{
	// if the two strings match completely, give a 3.
	if (Q_stricmp(allowedModelAndSkin, chosenModelAndSkin) == 0)
		return 3;
	else {
		char *allowedModel, *chosenModel, *allowedSkin, *chosenSkin;
	
		allowedModel = modelFromString(allowedModelAndSkin);
		chosenModel = modelFromString(chosenModelAndSkin);

		// Models match, give a 2.
		if (Q_stricmp(allowedModel, chosenModel) == 0) {
			return 2;
		}

		allowedSkin = skinFromString(allowedModelAndSkin);
		chosenSkin = skinFromString(chosenModelAndSkin);

		// Skins match, give a 1.
		if (Q_stricmp(allowedSkin, chosenSkin) == 0) {
			return 1;
		}
	}

	// Give a 0 if they don't match at all
	return 0;
}

// Assigns a model and skin to a player on a team,
// trying to match whatever the player had set as
// closely as possible from the set of skins and
// models allowed for a team.
void enforceTeamModelSkin(edict_t *player)
{
	char allowedSkins[1000];
	char *bestAllowedModelAndSkin, *allowedModelAndSkin;
	int bestrating, currentrating;
	char *chosenModelAndSkin = Info_ValueForKey (player->client->pers.userinfo, "skin");
	char *chosenModel, *allowedModel, *chosenSkin, *allowedSkin;
	int numEqualSkins;

	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		return;
	}

	if (player->client->resp.team == NOTEAM) {
		// Player isn't on a team (yet)
		gi.dprintf("enforceTeamModelSkin called with teamless player %s\n",
			player->client->pers.netname);
		return;
	}

	strncpy(allowedSkins, gTeams[player->client->resp.team].skinPaths, 
			gTeams[player->client->resp.team].skinPathLength);

	// of all the model/skin paths allowed for this team, pick
	// the one that most closely matches the user's skin and model setting
	bestAllowedModelAndSkin = strtok(allowedSkins, ";");
	bestrating = getPathRating(bestAllowedModelAndSkin, chosenModelAndSkin);
	numEqualSkins = 1;
	while ((allowedModelAndSkin = strtok(NULL, ";")) != NULL) {
		currentrating = getPathRating(allowedModelAndSkin, chosenModelAndSkin);
		if (currentrating > bestrating) {
			bestAllowedModelAndSkin = allowedModelAndSkin;
			bestrating = currentrating;
			numEqualSkins = 1;
		} else if (currentrating == bestrating) {
			numEqualSkins++;
			// Randomly pick between the two choices if they are of
			// equal rating with respect to matching the player's
			// choice of model and skin

			// In order to avoid favoring skins earlier or later in
			// the list, the probability of picking the skin we have just
			// encountered is the likelihood that we would pick it if we
			// were picking at random out of all the skins of equal
			// rating we have discovered so far.
			if ((double) rand()/RAND_MAX < (float) 1/numEqualSkins)
				bestAllowedModelAndSkin = allowedModelAndSkin;
		}
	}

	// in skin teams, enforce both model and skin
	if ((int)dmflags->value & DF_SKINTEAMS) {
		setSkinAndModel(player, bestAllowedModelAndSkin);
		if (strlen(player->client->resp.oldskin) > 0) {
			gi.dprintf("Player %s had set skin %s.\nSetting skin to %s\n", 
				player->client->pers.netname, chosenModelAndSkin, bestAllowedModelAndSkin);
		}
		return;
	}
	
	// otherwise model teams: if the user chose a model that is valid 
	// for the team he was assigned to, let him keep the skin he was 
	// using with that model.

	chosenModel = modelFromString(chosenModelAndSkin);
	allowedModel = modelFromString(bestAllowedModelAndSkin);
	
	chosenSkin = skinFromString(chosenModelAndSkin);
	allowedSkin = skinFromString(bestAllowedModelAndSkin);

	if (Q_stricmp(chosenModel, allowedModel) == 0) {
		setSkinAndModel(player, chosenModelAndSkin);
		gi.dprintf("Player %s had set model/skin %s.\nSince model teams and user chosen model\n"
			   "already matched team model, not changing user setting\n", 
			   player->client->pers.netname, chosenModelAndSkin);
	} else {
		setSkinAndModel(player, bestAllowedModelAndSkin);
		// otherwise, force both model and skin to the model and skin listed 
		// in teams.txt.  This is done just to ensure a valid model and skin pair.
		gi.dprintf("Player %s had set skin %s.\nOverriding skin and model to %s\n"
			   "since model %s not allowed for team\n", player->client->pers.netname,
			   chosenModelAndSkin, bestAllowedModelAndSkin, 
			   chosenModel);
	}
}

/* --------------- Team loading and setup ------------------ */

static int precacheCounter = 0;

void hackPrecacheModelSkin(char *modelSkin) {
	// set skin and model as configstring
	gi.configstring(CS_PLAYERSKINS + 128 + precacheCounter++,
		va("expert\\%s", modelSkin));
}

// setTeamEntry: parses the team name/path string into the team array.
// Returns true if a team was successfully processed.
static qboolean setTeamEntry(char *inputString, int currentTeam)
{
	char *teamName, *teamPath;

	trimWhitespace(inputString);

	// line is a comment or zero length after whitespace trim
	if (inputString[0] == '#' || strlen(inputString) == 0) {
		return false;
	}

	// If there's no '=' char in the string, malformed string, ignore
	if (strcspn(inputString, "=") == strlen(inputString)) {
		gi.dprintf("ERROR: Malformed team string detected, "
				"team name but no skins: %s\n", inputString);
		return false;
	}

	// The first token is the team name
	teamName = strtok(inputString, "=");

	// Store the teamName to the global gTeams array
	gTeams[currentTeam].teamName = malloc(strlen(teamName) + 1);
	strcpy(gTeams[currentTeam].teamName, teamName);
	trimWhitespace(gTeams[currentTeam].teamName);

	// now parse the rest of the line looking for valid paths to skins
	while ((teamPath = strtok(NULL, "; \t")) != NULL) {

		// Stop if it's a comment
		if (*teamPath == '#') {
			break;
		}

		if (validSkinName(teamPath))
		{
			if (gTeams[currentTeam].skinPaths == NULL) {
				int skinPathLength = strlen(teamPath) + 2;
				gTeams[currentTeam].skinPaths = malloc(skinPathLength);
				strcpy(gTeams[currentTeam].skinPaths, teamPath);
				gTeams[currentTeam].skinPathLength = skinPathLength;
			}
			else {
				int skinPathLength = strlen(gTeams[currentTeam].skinPaths) +
									strlen(teamPath) + 2;
				gTeams[currentTeam].skinPaths = realloc(gTeams[currentTeam].skinPaths, skinPathLength);
				strcat(gTeams[currentTeam].skinPaths, ";");
				strcat(gTeams[currentTeam].skinPaths, teamPath);
				gTeams[currentTeam].skinPathLength = skinPathLength;
			}
			hackPrecacheModelSkin(teamPath);
		}
		else
		{
			gi.dprintf("Skipping invalid skin/model path %s\n", teamPath);
		}
	}

	// If no valid paths were actually found, return false
	if (gTeams[currentTeam].skinPaths == NULL) {
		free(gTeams[currentTeam].teamName);
		return false;
	}

	return true;
}

/**
 * Teams-by-models and teams-by-skins (as set in dmflags) are handled the same way.
 * Each team has a list of model/skin files that a player is allowed to use while
 * on that team.  This routine loads a series of paths to models/skins relative
 * to "gamedir", and stores the paths in the gTeams array of gteam_t.
 */ 
void loadTeams(void)
{
	FILE *teamfile;
	char readString[1000] = {0};
	int i = 0, currentTeam = 0;

	// If teamplay is disabled, return.
	if (!teamplayEnabled()) { 
		// If Enforced Teams was set, disable it
		if (expflags & EXPERT_ENFORCED_TEAMS) {
			gi.dprintf("Enforced Teams enabled but neither skin "
			           "teams nor\nmodel teams set.  Disabling "
			           "Enforced Teams.\n");
			gi.cvar_set("expflags", va("%d", expflags & ~EXPERT_ENFORCED_TEAMS));
		}
		return;
	}

	// don't load team definitions if teams are not enforced
	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		return;
	}

	if ((int)sv_numteams->value < 2) {
		gi.dprintf("Teamplay enabled but cvar \"numteams\" set less than 2" 
		           ", disabling teamplay\n");
		gi.cvar_set("dmflags", va("%d",
			(int)dmflags->value & ~(DF_SKINTEAMS | DF_MODELTEAMS)));
		return;
	}

	// Initialize the array of teams 
	memset(gTeams, 0, MAX_TEAMS * sizeof(gteam_t));
	precacheCounter = 0;

	gi.dprintf("\n~~~~~~~~~~~~~~~~~~~~~~\nExpert Quake2 Teamplay\nLoading team definitions...\n");

	// Open the file
	teamfile = OpenGamedirFile(gamedir->string, va("%s/%s", levelCycle->string, TEAM_FILENAME), "r");
	
	if (teamfile == NULL) {
		// No models or skins, so no teams..
		gi.dprintf("ERROR: Couldn't open %s.  Teamplay disabled.\n", 
		           TEAM_FILENAME);
		gi.cvar_set("dmflags", va("%d",
			(int)dmflags->value & ~(DF_SKINTEAMS | DF_MODELTEAMS)));
		return;
	}

	// This is the main team-reading loop.
	while (fgets(readString, 1000, teamfile) != NULL) {

		// If the maximum number of entries have been read, stop.
		if (currentTeam == MAX_TEAMS) {
			gi.dprintf("Hit max team limit while reading team file: %s",
			           TEAM_FILENAME);
			break;
		}

		if (setTeamEntry(readString, currentTeam) == true)
			currentTeam++;
	}

	fclose(teamfile);

	// If there aren't enough teams, then disable teamplay
	if (currentTeam < 1) {
		gi.dprintf("ERROR: Unable to load 2 team definitions from %s.\n"
				   "Disabling teamplay.\n", TEAM_FILENAME);
		gi.cvar_set("dmflags", va("%d", 
			(int)dmflags->value & ~(DF_SKINTEAMS | DF_MODELTEAMS)));
		return;
	}
	
	// If there aren't enough teams to supply the requested number of teams,
	// reduce the requested number of teams
	if (currentTeam < (int)sv_numteams->value) {
		gi.dprintf("Unable to load %d team definitions from %s.\n"
				   "Reducing the number of teams to %d.\n",
				   (int)sv_numteams->value, TEAM_FILENAME, currentTeam);
		gi.cvar_set("numteams", va("%d", currentTeam));
	}

	if (ctf->value && (int)sv_numteams->value > 2) {
		gi.dprintf("Too many teams for CTF, reducing to 2 teams\n");
		gi.cvar_set("numteams", "2");
	}

	gi.dprintf("Using %d teams out of %d defined in %s\n~~~~~~~~~~~~~~~~~~~~~~\n\n",
		   (int)sv_numteams->value, currentTeam, TEAM_FILENAME);
}

// Deallocates any used memory for the teamplay code.
void shutdownTeamplay(void)
{
	int i;

	// gTeams is only set up in ENFORCED_TEAMS
	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		return;
	}

	for (i=0; i<MAX_TEAMS; i++) {
		if (gTeams[i].teamName)
			free(gTeams[i].teamName);
		if (gTeams[i].skinPaths)
			free(gTeams[i].skinPaths);
		if (gTeams[i].teamList)
			free(gTeams[i].teamList);
	}
}

/*
 * TeamAudio code
 */

void printAudioCmds(edict_t *player) {

	gi.cprintf(player, PRINT_HIGH, "To send any message, type \n"
									"\"ta [message number]\"\n"
									"at the console\n\n");

	if (ctf->value) {
		gi.cprintf(player, PRINT_HIGH, "Team-Wide Radio Messages:\n\n");

		gi.cprintf(player, PRINT_HIGH, "incoming \"enemy units incoming\"\n");
		gi.cprintf(player, PRINT_HIGH, "overrun \"base is overrun, available units pull back\"\n");
	}

	gi.cprintf(player, PRINT_HIGH, "Short Range Radio Messages:\n");

	gi.cprintf(player, PRINT_HIGH, "staying \"I have this covered\"\n");
	gi.cprintf(player, PRINT_HIGH, "hold \"Hold this position\"\n");
	gi.cprintf(player, PRINT_HIGH, "follow \"Follow me\"\n");
	gi.cprintf(player, PRINT_HIGH, "cover \"Cover me, I need an escort\"\n");
	gi.cprintf(player, PRINT_HIGH, "ok \"You got it\"\n");
	gi.cprintf(player, PRINT_HIGH, "no \"No can do\"\n");
	
	if (ctf->value) {
		gi.cprintf(player, PRINT_HIGH, "base \"Get back to base\"\n");
		gi.cprintf(player, PRINT_HIGH, "flag \"Find the enemy carrier\"\n");
		gi.cprintf(player, PRINT_HIGH, "escort \"Find our carrier and cover him\"\n");
	}
}

void audioBind(edict_t *player) {

	if (ctf->value) {
		StuffCmd(player, "bind f2 \"ta base\"\n");
		StuffCmd(player, "bind f3 \"ta flag\"\n");
		StuffCmd(player, "bind f4 \"ta escort\"\n");

		StuffCmd(player, "bind f11 \"ta incoming\"\n");
		StuffCmd(player, "bind f12 \"ta overrun\"\n");
	}

	StuffCmd(player, "bind f5 \"ta staying\"\n");
	StuffCmd(player, "bind f6 \"ta hold\"\n");
	StuffCmd(player, "bind f7 \"ta follow\"\n");
	StuffCmd(player, "bind f8 \"ta cover\"\n");
	StuffCmd(player, "bind f9 \"ta ok\"\n");
	StuffCmd(player, "bind f10 \"ta no\"\n");
}

/**
 * Print an equivalent text message for the player
 */
//void printAudioMsg(edict_t *player) {
//	
//}

// defined in p_client.c
qboolean IsFemale(edict_t *player);

char *sexString(edict_t *player) {
	return IsFemale(player) ? "female" : "male";
}

char *soundPath(edict_t *player, char *filename) {
	return va("player/%s/%s.wav", sexString(player), filename);
}

#define TEAM_AUDIO_LOCAL_RADIUS					400

void localRadio(edict_t *player, char *audioFile) {
	edict_t	*ent = NULL;
	int i;
	vec3_t v1;

	ent = g_edicts + 1;
	for (i = 1; i <= maxclients->value; i++) 
	{
		// found a player on the team of the sending player
		if (ent->inuse && 
		    ent->client->resp.team == player->client->resp.team) 
		{
			// players can either be within a radius or in line of sight
			VectorSubtract(player->s.origin, ent->s.origin, v1);
			if (VectorLength(v1) < TEAM_AUDIO_LOCAL_RADIUS ||
			    loc_CanSee(player, ent))
			{
				// sound unicasted to target players
				unicastSound(ent, gi.soundindex(soundPath(player, audioFile)), 1);
				// target players see hyperblaster muzzleflash on sender
				gi.WriteByte (svc_muzzleflash);
				gi.WriteShort (ent-g_edicts);
				gi.WriteByte (MZ_HYPERBLASTER);
				gi.unicast (ent, true);
			}
		}
		ent = ent + 1;
	}
	// spam prevention timer
	player->client->lastTeamAudio = level.time;
}

void globalRadio(edict_t *player, char *audioFile) {
	// send sound
	teamSound(player->client->resp.team,
	          gi.soundindex(soundPath(player, audioFile)), 1);
	player->client->lastTeamAudio = level.time;
	// FIXME send text
}

#define		SCOPE_LOCAL				0
#define		SCOPE_GLOBAL			1

#define		PROP_GLOBAL_FLAG		8

/**
 * Send team radio from player "player".
 */
void sendRadio(edict_t *player, char *cmd) {

	// each sound may have a gesture associated with it,
	// and may be sent either team-wide or only to teammates
	// in the immediate vicinity

	// defaults: no gesture, local scope (nearby teammates)
	int gesture = -1;
	int scope = SCOPE_LOCAL;
	char *audioFile = NULL;

	// ctf only
	if (ctf->value) {
		if (!Q_stricmp(cmd, "incoming")) {
			audioFile = "incoming";
			scope = SCOPE_GLOBAL;
		} else if (!Q_stricmp(cmd, "overrun")) {
			audioFile = "overrun";
			scope = SCOPE_GLOBAL;
		} else if (!Q_stricmp(cmd, "secure")) {
			audioFile = "secure";
			scope = SCOPE_GLOBAL;
		} else if (!Q_stricmp(cmd, "base")) {
			audioFile = "base";
			gesture = WAVE_POINT;
		} else if (!Q_stricmp(cmd, "escort")) {
			audioFile = "escort";
			gesture = WAVE_POINT;
		} else if (!Q_stricmp(cmd, "flag")) {
			audioFile = "flag";
			gesture = WAVE_POINT;
		}
	}
	
	// generic teamplay
	if (!Q_stricmp(cmd, "staying")) {
		audioFile = "staying";
	} else if (!Q_stricmp(cmd, "hold")) {
		audioFile = "hold";
		gesture = WAVE_POINT;
	} else if (!Q_stricmp(cmd, "follow")) {
		audioFile = "follow";
		gesture = WAVE_WAVE;
	} else if (!Q_stricmp(cmd, "cover")) {
		audioFile = "cover";
		gesture = WAVE_WAVE;
	} else if (!Q_stricmp(cmd, "ok")) {
		audioFile = "ok";
		gesture = WAVE_SALUTE;
	} else if (!Q_stricmp(cmd, "no")) {
		audioFile = "no";
	}

	if (audioFile == NULL) {
		// not in the built-in set of sounds

		// server admins can set properties to enable additional
		// sounds.  The property should have the name of the sound
		// to be played and value of a gesture to accompany the
		// sound, if any, plus PROP_GLOBAL_FLAG (8) for a global sound
		int propValue;

		char *audioProp = getProp(gProperties, cmd);
		if (audioProp != NULL) {
			// server-added sound
			errno = 0;
			propValue = atoi(audioProp);
			if (errno == 0) {
				// admin adds PROP_GLOBAL_FLAG for team-wide radio sounds,
				// default is global 
				scope = (propValue & PROP_GLOBAL_FLAG) ? SCOPE_GLOBAL : SCOPE_LOCAL;
				propValue &= ~PROP_GLOBAL_FLAG;
				// value after removing team-wide flag is the number of a gesture
				if (propValue >= WAVE_FLIPOFF && propValue <= WAVE_POINT) {
					gesture = propValue;
				}
			} else {
				gi.dprintf("Bad audio property: name %s, value %s\n"
				           "Assuming local radio, no gesture\n", cmd, audioProp);
			}
			audioFile = cmd;
		} else { 
			// allow any sound to go through as a stuffcmd?
			gi.cprintf(player, PRINT_HIGH, "Bad TeamAudio sound name\n");
			//teamStuff(player->client->resp.team, 
			//          va("play %s\n", soundPath(player, audioCmd)));
			return;
		}
	}
	if (scope == SCOPE_LOCAL) {
		localRadio(player, audioFile);
		if (gesture != -1) {
			wave(player, gesture);
		}
	} else {
		globalRadio(player, audioFile);
	}
}

void Cmd_TeamAudio_f(edict_t *player) {

	char *audioCmd;

	if (!teamplayEnabled()) {
		gi.dprintf("Teamplay not enabled\n");
		return;
	}

	audioCmd = gi.args();

	// no argument  
	if (!*audioCmd) {
		printAudioCmds(player);
		return;
	}

	// bind FKeys to audio commands
	if (Q_stricmp(audioCmd, "bind") == 0) {
		audioBind(player);
		return;
	}

	// it's the name of a sound, send it
	if (level.time - player->client->lastTeamAudio >= 1.0) {
		sendRadio(player, audioCmd);
	} else {
		// spam prevention
		gi.cprintf(player, PRINT_HIGH, "You can only use TeamAudio once per second\n");
	}

}

// Initialize TeamAudio subsystem
// this will probably involve precaching a known list of sounds
void InitTeamAudio(void)
{
	// Read in a list of sounds from a file and precache them
}
