#include "g_local.h"

/**
 * There can be two flags in the game per team: 
 *
 * the "baseFlag": always at the base; when there is a 
 * flag carrier or a dropped flag in the game, the 
 * homeFlag is just made non-solid (ie can't be picked up)
 * 
 * the "activeFlag": this is the flag that can currently
 * by picked up.  If the flag is at base, this is the same
 * flag as the homeFlag.  If the flag has been dropped somewhere
 * in the level, that dropped flag is the active flag, and will
 * have the DROPPED_ITEM spawnflag.  If a carrier has the flag,
 * activeFlag is NULL.  
 */

#define TEAM_DISTRIBUTION_DELAY 				2

typedef struct ctfgame_s
{
	int teamScore[TEAM2 + 1];
	int teamDistribution[TEAM2 + 1][4];
	int lastTeamDistributionCalc;
	int total1, total2; // these are only set when going into intermission!
	float last_flag_capture;
	int last_capture_team;
	edict_t *activeFlag[TEAM2 + 1], *baseFlag[TEAM2 + 1], *carrier[TEAM2 + 1];
	list_t teamSpawns[TEAM2 + 1];
} ctfgame_t;

ctfgame_t ctfgame;

char *ctf_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// Expert: disabled in CTF to free up slots for Team Distribution
/*
// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "
*/

// timer
"if 9 "
  "xv 246 "
  "num 2 10 "
  "xv 296 "
  "pic 9 "
"endif "

//  help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "

// red team
"yb -102 "
"if 17 "
//  "xr -26 "
  "xl 2 "
  "pic 17 "
"endif "
// "xr -62 "
"xl 28 "
"num 2 18 "
//joined overlay
"if 22 "
  "yb -104 "
//  "xr -28 "
  "xl 0 "
  "pic 22 "
"endif "

// blue team
"yb -75 "
"if 19 "
//  "xr -26 "
  "xl 2 "
  "pic 19 "
"endif "
//"xr -62 "
"xl 28 "
"num 2 20 "
"if 23 "
  "yb -77 "
//  "xr -28 "
  "xl 0 "
  "pic 23 "
"endif "

// have flag graph
"if 21 "
  "yb -137 "
  "xl 2 "
  "pic 21 "
"endif "

// Expert: playerid
"if 27 "
  "yb -58 "
  "xv 120 "
  "stat_string 27 "
"endif "

// Expert: Flag carrier names
// Note that carrier name appear next to the player's
// team's flag status indicator, not next to the indicator
// for enemy flag status
"xl 68 "
"if 28 "
  "yb -94 "
  "stat_string 28 "
"endif "
"if 29 "
  "yb -67 "
  "stat_string 29 "
"endif "

// Expert: Team Distribution
// Four numbers in a vertical column on the right
// side of the screen, showing the distribution of
// a player's teammates on the field
"if 30 "

// endpoint graphics
  "xr -12 "
  "yb -142 "
  "pic 7 "
  "yb -60 "
  "pic 8 "

// distribution graphics
  "xr -12 "
  "yb -78 "
  "pic 30 "
  "yb -96 "
  "pic 31 "
  "yb -114 "
  "pic 26 "
  "yb -132 "
  "pic 16 "
"endif "
;

/*--------------------------------------------------------------------------*/

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
#if 0
		if (from->solid == SOLID_NOT)
			continue;
#endif
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}

/*--------------------------------------------------------------------------*/

static gitem_t *flag1_item;
static gitem_t *flag2_item;

void InitSpawnLists();

void CTFInit(void)
{
	if (!G_Find (NULL, FOFS(classname), "item_flag_team1") ||
	    !G_Find (NULL, FOFS(classname), "item_flag_team2")) {
		gi.dprintf("Map is missing at least one flag.  Disabling CTF\n");
		gi.cvar_set("ctf", "0");
		return;
	}

	if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");
	if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");

	memset(&ctfgame, 0, sizeof(ctfgame));

	ctfgame.baseFlag[TEAM1] = G_Find (NULL, FOFS(classname), "item_flag_team1");
	ctfgame.baseFlag[TEAM2] = G_Find (NULL, FOFS(classname), "item_flag_team2");
	
	InitSpawnLists();
}

/*--------------------------------------------------------------------------*/

// utilities specific to two-team CTF

int CTFOtherTeam(int team)
{
	switch (team) {
	case TEAM1:
		return TEAM2;
	case TEAM2:
		return TEAM1;
	}
	return -1; // invalid value
}

int CTFTeamForFlagClass(char *flagClass) {
	if (Q_stricmp("item_flag_team1", flagClass) == 0) {
		return TEAM1;
	} else {
		return TEAM2;
	}
}

qboolean isCarrier(edict_t *player) {
	return (player == ctfgame.carrier[player->client->resp.team]);
}

/*------------------Spawning-----------------------------------------*/

edict_t *SelectDeathmatchSpawnPoint (edict_t *player);
float	PlayersRangeFromSpot (edict_t *spot);

/**
 * Returns straight line distance from "spot" to the enemy 
 * base with respect to "player"
 */
float RangeToEnemyFlag(edict_t *spot, int team)
{
	vec3_t		v;
	edict_t 	*enemyFlag;

	// get enemy flag ent
	if (team == TEAM1) {
		enemyFlag = ctfgame.baseFlag[TEAM2];
	} else {
		enemyFlag = ctfgame.baseFlag[TEAM1];
	}

	// return distance to enemy base
	VectorSubtract(spot->s.origin, enemyFlag->s.origin, v);

	return VectorLength (v);

}

/**
 * Find all spawns for team "team" that are not too near to enemy base
 */
list_t findLegalSpawns(int team) {

	list_t legalSpawns = listNew(0, NULL);
	edict_t* spot = NULL;
    edict_t *flag1 = ctfgame.baseFlag[TEAM1],
        *flag2 = ctfgame.baseFlag[TEAM2];
    vec3_t v;
    float distance;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		/*gi.dprintf("team %d: spawn %s at range %f to flag\n", team,
				vtos(spot->s.origin),
				RangeToEnemyFlag(spot, team));;
		*/
		if (RangeToEnemyFlag(spot, team) >= CTF_SPAWN_RANGE) 
		{
			listAppend(legalSpawns, spot);			
		}
	}

	/*gi.dprintf("Requiring at least %d spawns for %d players on a team, found %d spawns\n", 
			(int)floor(game.maxclients/4.0), 
			(int)floor(game.maxclients/2.0), 
			listSize(legalSpawns)); 
	*/

	// We found enough legal spawns by this method.
	// Enough spawns is considered to be half as
	// many spawns as players on a team
	if (listSize(legalSpawns) > floor(game.maxclients/6.0)) 
		return legalSpawns;

	gi.dprintf("Small map: picking CTF team spawns\naccording to distance between bases.\n");

	// didn't find a sufficient number of legal spawns
	// start over with an alternate algorithm
	listFree(legalSpawns);
	legalSpawns = listNew(0, NULL);

	// find distance between flags
   	VectorSubtract(flag1->s.origin, flag2->s.origin, v);
   	distance = VectorLength(v);

	// accept spots that are not near enemy base with respect to the
	// length between the two bases.  Not as safe due to wraparound maps,
	// but better than the alternative given the previous algorithm found
	// too few spots
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		/*gi.dprintf("team %d: spawn %s at range of %f of field between flags\n", 
				team,
				vtos(spot->s.origin),
				RangeToEnemyFlag(spot, team)/distance);
		*/
		if (RangeToEnemyFlag(spot, team)/distance >= CTF_SPAWN_RANGE_RATIO) 
		{
			listAppend(legalSpawns, spot);			
		}
	}
	
	if (listSize(legalSpawns) == 0) {
		// no legal spawns were found
		// normal deathmatch spawning will be used
		listFree(legalSpawns);
		return NULL;
	} else {
		return legalSpawns;
	}
}

/**
 * Find a list of spawn points for each team that are to spawn at
 * in CTF, that is, are not to near to the enemy's flag
 */
void InitSpawnLists() {

	ctfgame.teamSpawns[TEAM1] = findLegalSpawns(TEAM1);
	ctfgame.teamSpawns[TEAM2] = findLegalSpawns(TEAM2);
}

edict_t* SelectFarthestCTFSpawnPoint (edict_t* player)
{
	list_t spawns = ctfgame.teamSpawns[player->client->resp.team];

	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;
	int     spawnCount, i;

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;

	// go through legal spawns and find the spawn 
	// which is the furthest from any player
	spawnCount = listSize(spawns);
	for (i = 0; i < spawnCount; i++) 
	{
		spot = (edict_t*)listElementAt(spawns, i);
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t* SelectRandomCTFSpawnPoint (edict_t* player)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0, selection, spawnCount, i;
	float	range, range1, range2;
	list_t  spawns = ctfgame.teamSpawns[player->client->resp.team];

	int nearFlag = 0;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	spawnCount = listSize(spawns);
	//gi.dprintf("%d legal spawns\n", spawnCount);
	// if there are more than two legal spawns, locate the two 
	// spawns with players nearest them so we can avoid spawning there
	if (spawnCount > 2) {
		//gi.dprintf("Finding spawn nearest players\n");
		for (i = 0; i < spawnCount; i++) 
		{
			spot = (edict_t*)listElementAt(spawns, i);
			range = PlayersRangeFromSpot(spot);
			if (range < range1)
			{
				range1 = range;
				spot1 = spot;
			}
			else if (range < range2)
			{
				range2 = range;
				spot2 = spot;
			}
		}
	}

	do
	{
		selection = rand() % spawnCount;
		//gi.dprintf("Selected spawn %d\n", selection);
	
		spot = (edict_t*)listElementAt(spawns, selection);
	} while (spot == spot1 || spot == spot2);

	//gi.dprintf("Final selection %d\n", selection);
	return spot;
}

/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectCTFSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	// only use special spawning at the beginning of the game
	if (ent->client->resp.ctf_state != CTF_STATE_START) {
		if (ent->client->resp.team == NOTEAM ||
			!ctfgame.teamSpawns[ent->client->resp.team])
		{	
			// player not on a team, or no valid spawns
			// punt to the normal deathmatch spawn routines
			return SelectDeathmatchSpawnPoint(ent);
		}
	
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestCTFSpawnPoint (ent);
		else
			return SelectRandomCTFSpawnPoint (ent);
	}

	ent->client->resp.ctf_state = CTF_STATE_PLAYING;

	switch (ent->client->resp.team) {
	case TEAM1:
		cname = "info_player_team1";
		break;
	case TEAM2:
		cname = "info_player_team2";
		break;
	default:
		return SelectDeathmatchSpawnPoint(ent);
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return SelectDeathmatchSpawnPoint(ent);

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*------------------------------------------------------------------------*/

/**
 * Give out bonuses for advancing the team goal of capturing the flag.
 */
void ExpertCTFScoring(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	int i;
	edict_t *ent;
	gitem_t *flag_item, *enemy_flag_item;
	int otherteam;
	edict_t *baseFlag, *activeFlag, *carrier;
	vec3_t v1, v2;
	qboolean gotCarrierDefenseBonus = false;

	if (!targ || !attacker) {
		gi.dprintf("Expert CTF scoring passed NULL attacker or targ\n");
		return;
	}

	// bonuses only for client-to-client 
	// kills on players on the enemy team
	if (!targ->client || !attacker->client || 
	    targ == attacker || onSameTeam(attacker, targ))
		return;

	otherteam = CTFOtherTeam(targ->client->resp.team);
	if (otherteam < 0)
		return; // whoever died isn't on a team

	otherteam = CTFOtherTeam(attacker->client->resp.team);
	if (otherteam < 0)
		return; // killer isn't on a team

	// set up flag_item and enemy_flag_item relative
	// to the dying player's team
	if (targ->client->resp.team == TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} else {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	// CARRIER KILL bonus
	if (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) {
		int bonus, carryTime;

		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		// if the carrier is killed almost immediately after
		// getting flag, award the equivalent of a flag defense;
		// players shouldn't be encouraged to manipulate the timing of 
		// the carrier kill to get more points.
		carryTime = level.time - targ->client->resp.ctf_flagsince; 
		if (carryTime < CTF_IMMEDIATE_CARRIER_KILL_TIMEOUT) {
			bonus = CTF_FLAG_DEFENSE_BONUS;
		} else {
			bonus = floor(carryTime * CTF_CARRIER_KILL_BONUS_RATE);
			if (bonus > CTF_CARRIER_KILL_BONUS_MAX) bonus = 
					CTF_CARRIER_KILL_BONUS_MAX;
		}
		gi.bprintf(PRINT_MEDIUM, "BONUS %s: CARRIER KILL: %d points\n",
		           attacker->client->pers.netname,
		           bonus + 1); // bonus plus the frag for the kill
		attacker->client->resp.score += bonus;
		gsLogScore(attacker, GS_TYPE_CTF_CARRIER_KILL, bonus);

		// Clear ctf_lastHurtcarrier field on the enemy team;
		// The flag carrier just died, so there should be no
		// bonuses for defending the carrier against players
		// who recently hit the carrier. 
		ent = g_edicts + 1;
		for (i = 1; i <= maxclients->value; i++) {
			if (ent->inuse && ent->client->resp.team == otherteam) {
				ent->client->resp.ctf_lasthurtcarrier = 0;
			}
			ent = ent + 1;
		}

	}

	// CARRIER SAVE bonus.  Mutually exclusive with CARRIER DEFENSE;
	// CARRIER SAVE _must_ be worth more points, since "attacker" has
	// defended his carrier against and enemy who has not only obviously
	// noticed the carrier, but has done damage to him 
	if (level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT &&
	    !attacker->client->pers.inventory[ITEM_INDEX(flag_item)]) // attacker not the carrier
	{ 
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
		gi.bprintf(PRINT_MEDIUM, "BONUS %s: %s CARRIER SAVE\n",
			attacker->client->pers.netname, 
			nameForTeam(attacker->client->resp.team));
		gsLogScore(attacker, GS_TYPE_CTF_CARRIER_SAVE, CTF_CARRIER_DANGER_PROTECT_BONUS);
		// prevent further (radius-based) carrier defense bonuses
		gotCarrierDefenseBonus = true;
	}

	// Radius-based flag and carrier defense bonuses: check if either
	// the target or attacker is in radius of some object of importance
	// in ctf: flags, bases, carriers.  If either the target or attacker
	// has line-of-sight to the object being defended, there is always a
	// bonus at any distance.

	// FIXME: radius checks can work poorly if the bases are adjacent.
	// Detect this case in init, set a flag, and use it to alter scoring
	// heuristics?

	activeFlag = ctfgame.activeFlag[attacker->client->resp.team];
	baseFlag = ctfgame.baseFlag[attacker->client->resp.team];

	// BASE DEFENSE: a defender made a kill while either he or the player
	// he killed were near the home base position of his flag.  This kind
	// of defensive bonus can be received even if the defender's flag is
	// not at base.
	VectorSubtract(targ->s.origin, baseFlag->s.origin, v1);
	VectorSubtract(attacker->s.origin, baseFlag->s.origin, v2);

	if ( (VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS ||
	      VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS) ||
	     (loc_CanSee(baseFlag, targ) || 
	      loc_CanSee(baseFlag, attacker)) ) 
	{
		attacker->client->resp.score += CTF_BASE_DEFENSE_BONUS;
		gi.bprintf(PRINT_MEDIUM, "BONUS %s: %s BASE DEFENSE\n",
			attacker->client->pers.netname, 
			nameForTeam(attacker->client->resp.team));
		gsLogScore(attacker, GS_TYPE_CTF_BASE_DEFENSE, CTF_BASE_DEFENSE_BONUS);

		// FLAG HOLDING: Additional bonuses given to carriers when they make
		// kills near their base.  Only given out if the carrier's flag is not
		// at base.
		if (baseFlag != activeFlag && attacker == ctfgame.carrier[attacker->client->resp.team]) {
			attacker->client->resp.score += CTF_FLAG_HOLDING_BONUS;
			gi.bprintf(PRINT_MEDIUM, "BONUS %s: FLAG HOLDING\n",
				attacker->client->pers.netname);
			gsLogScore(attacker, GS_TYPE_CTF_FLAG_HOLDING, CTF_FLAG_HOLDING_BONUS);
		}
	}

	// if activeFlag == NULL, a carrier has 
	// the flag, no flag defense bonus possible
	if (activeFlag) {
		// FLAG DEFENSE: a defender made a kill while either he or the player
		// he killed were near the active flag.  If the flag is at base, this 
		// bonus can be cumulative with base defense (it's extremely important
		// to defend a flag at base, and only somewhat important to defend a
		// base with no flag). 
		VectorSubtract(targ->s.origin, activeFlag->s.origin, v1);
		VectorSubtract(attacker->s.origin, activeFlag->s.origin, v2);

		if ( (VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS ||
		      VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS) ||
		     (loc_CanSee(activeFlag, targ) || 
		      loc_CanSee(activeFlag, attacker)) ) 
		{
			attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
			gi.bprintf(PRINT_MEDIUM, "BONUS %s: %s FLAG DEFENSE\n",
				attacker->client->pers.netname, 
				nameForTeam(attacker->client->resp.team));
			gsLogScore(attacker, GS_TYPE_CTF_FLAG_DEFENSE, CTF_FLAG_DEFENSE_BONUS);
		}
	}

	// the target's (dying player's) flag.
	activeFlag = ctfgame.activeFlag[targ->client->resp.team];

	// if activeFlag == NULL, a carrier has the flag
	if (activeFlag) {
		// DEFENDER KILL: the reverse of a FLAG DEFENSE.  The attacker has
		// killed the target near the target's flag.  Awarded during base
		// onslaughts as the attackers whittle away at base defenders,
		// or during mad rushes for a dropped flag, as attackers prevent
		// the flag from being returned.  Carriers can't get a DEFENDER
		// KILL since the defender's flag must be on the ground.
		VectorSubtract(targ->s.origin, activeFlag->s.origin, v1);
		VectorSubtract(attacker->s.origin, activeFlag->s.origin, v2);

		if ( (VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS ||
		      VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS) ||
		     (loc_CanSee(activeFlag, targ) || 
		      loc_CanSee(activeFlag, attacker)) ) 
		{
			attacker->client->resp.score += CTF_DEFENDER_KILL_BONUS;
			gi.bprintf(PRINT_MEDIUM, "BONUS %s: %s DEFENDER KILL\n",
				attacker->client->pers.netname, 
				nameForTeam(targ->client->resp.team));
			gsLogScore(attacker, GS_TYPE_CTF_DEFENDER_KILL, CTF_DEFENDER_KILL_BONUS);
		}
	}

	if (gotCarrierDefenseBonus) {
		// already got a carrier defense bonus
		return;
	}

	// attacker's team's flag carrier
	carrier = ctfgame.carrier[attacker->client->resp.team];

	// CARRIER DEFENSE, radius based.  Given for attacker or target
	// being near the flag carrier.  Note no LOS checks, since the
	// carrier is fleeing and likely to get behind cover.  Carriers
	// are inelegible for CARRIER DEFENSE, since their priority should
	// be returning to base.
	if (carrier && carrier != attacker) { 
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if ( (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS ||
		      VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS) ||
		     (loc_CanSee(carrier, targ) || 
		      loc_CanSee(carrier, attacker)) ) 
		{
			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			gi.bprintf(PRINT_MEDIUM, "BONUS %s: %s CARRIER DEFENSE\n",
				attacker->client->pers.netname, 
				nameForTeam(attacker->client->resp.team));
			gsLogScore(attacker, GS_TYPE_CTF_CARRIER_DEFENSE, CTF_CARRIER_PROTECT_BONUS);
		}
	}
}

void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker)
{
	if (!targ->client || !attacker->client)
		return;

	if (isCarrier(targ) && 
		targ->client->resp.team != attacker->client->resp.team) 
	{
		attacker->client->resp.ctf_lasthurtcarrier = level.time;
	}
}

/**
 * Counts up number of players on a single team that are
 * at various positions in the map; in the immediate vicinity 
 * of each base, or if not in the immediate vicinity of a base,
 * which base they are closer to
 */
#define		HOME_BASE		0
#define		NEAR_HOME		1
#define		NEAR_ENEMY		2
#define		ENEMY_BASE		3

#define		CTF_DISTRIBUTION_BASE_RADIUS		1500

// count the number of players in each general area of the map
void countPlayers() {

	int i, team;
	edict_t *ent;
	vec3_t toHome, toEnemy;
	float homeDistance, enemyDistance;

	// clear out old distribution data
	memset (ctfgame.teamDistribution, 0, sizeof(ctfgame.teamDistribution));

	// loop through all players
	ent = g_edicts;
	for (i = 1; i <= maxclients->value; i++) {
		ent += 1;
		if (!ent->inuse)
			continue;

		// ignore players not on a team
		if (!ent->client || ent->client->resp.team == NOTEAM)
			continue;

		// don't count dead players
		if (ent->health <= 0)
			continue;

		team = ent->client->resp.team;

		// determine distance to each team's flag
		VectorSubtract(ent->s.origin, ctfgame.baseFlag[team]->s.origin, toHome);
		homeDistance = VectorLength(toHome);
		VectorSubtract(ent->s.origin, ctfgame.baseFlag[CTFOtherTeam(team)]->s.origin, toEnemy);
		enemyDistance = VectorLength(toEnemy);

		if (homeDistance < enemyDistance) {
			// nearer home base
			if (homeDistance < CTF_DISTRIBUTION_BASE_RADIUS) {
				//gi.bprintf(PRINT_HIGH, "Player %s at his base\n", ent->client->pers.netname);
				// player at his team's base
				ctfgame.teamDistribution[team][HOME_BASE]++;
				continue;
			} else {
				//gi.bprintf(PRINT_HIGH, "Player %s near his base\n", ent->client->pers.netname);
				// just near home base
				ctfgame.teamDistribution[team][NEAR_HOME]++;
			}
		} else { // neglible bias toward showing up as nearer enemy base
			// nearer enemy base
			if (enemyDistance < CTF_DISTRIBUTION_BASE_RADIUS) {
				//gi.bprintf(PRINT_HIGH, "Player %s at enemy base\n", ent->client->pers.netname);
				// player at enemy team's base
				ctfgame.teamDistribution[team][ENEMY_BASE]++;
				continue;
			} else {
				//gi.bprintf(PRINT_HIGH, "Player %s near enemy base\n", ent->client->pers.netname);
				// just near enemy base
				ctfgame.teamDistribution[team][NEAR_ENEMY]++;
			}
		}
	}

/*
	gi.bprintf(PRINT_HIGH, "Dist: H:%d NH:%d NE:%d E:%d\n",
			ctfgame.teamDistribution[team][HOME_BASE],
			ctfgame.teamDistribution[team][NEAR_HOME],
			ctfgame.teamDistribution[team][NEAR_ENEMY],
			ctfgame.teamDistribution[team][ENEMY_BASE]);
*/

}

// convert player counts to imageindexes for pics showing 
// concentration of players at each location
void convertToPics() {

	int team, location;
	int numMembers;
	float concentration;

	// translate player counts per area into player concentrations
	for (team = TEAM1; team <= TEAM2; team++) {
		// grab the number of players in the game on this team
		numMembers = memberCount(team);
		if (numMembers == 0) {
			numMembers = 1;
		}
//		gi.bprintf(PRINT_HIGH, "Conc: "); 
		for (location = HOME_BASE; location <= ENEMY_BASE; location++) 
		{
			// concentration = fraction of team in area of map
			concentration = ctfgame.teamDistribution[team][location] / (float)numMembers;

//			gi.bprintf(PRINT_HIGH, "%.2f, ", concentration); 

			// translate concentration into one of 5 pics 
			// that range from black to green in intensity
			if (ctfgame.teamDistribution[team][location] == 0) {
				// special case: zero players in area
				ctfgame.teamDistribution[team][location] = gi.imageindex("i_etd0");
			} else if (concentration <= 0.25) {
				ctfgame.teamDistribution[team][location] = gi.imageindex("i_etd25");
			} else if (concentration <= 0.50) {
				ctfgame.teamDistribution[team][location] = gi.imageindex("i_etd50");
			} else if (concentration <= 0.75) {
				ctfgame.teamDistribution[team][location] = gi.imageindex("i_etd75");
			} else {
				ctfgame.teamDistribution[team][location] = gi.imageindex("i_etd100");
			}
		}
//		gi.bprintf(PRINT_HIGH, "\n");
	}
}

void calcTeamDistribution() {
	countPlayers();
	convertToPics();
}

/*------------------------------------------------------------------------*/

void CTFResetFlag(int flagTeam)
{
	edict_t *activeFlag, *baseFlag;

	// grab this team's flags
	baseFlag = ctfgame.baseFlag[flagTeam];
	activeFlag = ctfgame.activeFlag[flagTeam];

	// get rid of a dropped flag, if there is one
	if (activeFlag && activeFlag->spawnflags & DROPPED_ITEM) {
		G_FreeEdict(activeFlag);
	}

	// enable the flag at base
	baseFlag->svflags &= ~SVF_NOCLIENT;
	baseFlag->solid = SOLID_TRIGGER;
	gi.linkentity(baseFlag);
	baseFlag->s.event = EV_ITEM_RESPAWN;

	// active flag becomes the flag at base
	ctfgame.activeFlag[flagTeam] = ctfgame.baseFlag[flagTeam];
}

void CTFResetFlags(void)
{
	CTFResetFlag(TEAM1);
	CTFResetFlag(TEAM2);
}

/**
 * Capture a flag
 */
void CTFCapture(edict_t *flag, edict_t *player,
                gitem_t *flag_item, gitem_t *enemy_flag_item)
{
	int i;
	float heldTime = level.time - player->client->resp.ctf_flagsince;
	edict_t *p;

	// announce to all players
	gi.bprintf(PRINT_HIGH, "%s %s the %s flag! (held %.1f seconds)\n",
	           player->client->pers.netname, 
	           GREENTEXT_CAPTURED,
	           nameForTeam(CTFOtherTeam(player->client->resp.team)),
	           heldTime);

	// track total held time for scoreboard
	player->client->resp.totalHeld += heldTime;

	// remove flag from inventory
	player->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;

	// track global game state
	ctfgame.last_flag_capture = level.time;
	ctfgame.last_capture_team = flag->flagTeam;
	ctfgame.teamScore[flag->flagTeam]++;

	// player who just captured no longer a carrier
	ctfgame.carrier[flag->flagTeam] = NULL;

	// sound for capturing team
	teamSound(flag->flagTeam, gi.soundindex("ctf/flagcap.wav"), 1);
	// sound for other team
	teamSound(CTFOtherTeam(flag->flagTeam), gi.soundindex("world/x_alarm.wav"), 1);

	// score for capture
	player->client->resp.score += CTF_CAPTURE_BONUS;
	gsLogScore(player, GS_TYPE_CTF_CAPTURE, CTF_CAPTURE_BONUS);

	// announce capture to players, award assists
	p = g_edicts;
	for (i = 1; i <= maxclients->value; i++) {
		p += 1;
		if (!p->inuse)
			continue;

		if (p->client->resp.team != player->client->resp.team) {
			// reset last hurt carrier on enemy team 
			// since carrier has already scored
			p->client->resp.ctf_lasthurtcarrier = -5;
		} else if (p->client->resp.team == player->client->resp.team) {
			// award extra points for capture assists
			if (p->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time) {
				// returned team's flag
				gi.bprintf(PRINT_HIGH, "%s %s: %s FLAG RETURN\n",
				           GREENTEXT_ASSIST,
				           p->client->pers.netname,
				           nameForTeam(p->client->resp.team));
				p->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
				gsLogScore(p, GS_TYPE_CTF_RETURN_FLAG_ASSIST, CTF_RETURN_FLAG_ASSIST_BONUS);
			}
			if (p->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time) {
				// killed enemy carrier
				gi.bprintf(PRINT_HIGH, "%s %s: %s CARRIER KILL\n",
				           GREENTEXT_ASSIST,
				           p->client->pers.netname,
				           nameForTeam(CTFOtherTeam(p->client->resp.team)));
				p->client->resp.score += CTF_CARRIER_KILL_ASSIST_BONUS;
				gsLogScore(p, GS_TYPE_CTF_CARRIER_KILL_ASSIST, CTF_CARRIER_KILL_ASSIST_BONUS);
			}
		}
	}
	// reset both flags
	CTFResetFlags();
}

/**
 * Return home team's flag
 */
void CTFReturnFlag(edict_t *flag, edict_t *player,
                   gitem_t *flag_item, gitem_t *enemy_flag_item) 
{

	// anounce return
	gi.bprintf(PRINT_HIGH, "%s %s the %s flag!\n", 
		player->client->pers.netname, 
		GREENTEXT_RETURNED,
		nameForTeam(flag->flagTeam));

	// score for return
	player->client->resp.score += CTF_RECOVERY_BONUS;
	gsLogScore(player, GS_TYPE_CTF_RETURN, CTF_RECOVERY_BONUS);

	// mark return time for assists
	player->client->resp.ctf_lastreturnedflag = level.time;

	// sound for team who's flag was returned
	teamSound(flag->flagTeam, gi.soundindex("world/train2.wav"), 1);
	// sound for other team
	teamSound(CTFOtherTeam(flag->flagTeam), gi.soundindex("world/fuseout.wav"), 1);

	// reset the flag to base
	CTFResetFlag(flag->flagTeam);
}

/** 
 * Pickup the enemy flag
 */
void CTFPickupFlag(edict_t *flag, edict_t *player, 
                   gitem_t *flag_item, gitem_t *enemy_flag_item) 
{

	// announce pickup
	gi.bprintf(PRINT_HIGH, "%s %s the %s flag!\n",
		player->client->pers.netname,
		GREENTEXT_GRABBED,
		nameForTeam(flag->flagTeam));

	// score for pickup
	player->client->resp.score += CTF_FLAG_BONUS;
	gsLogScore(player, GS_TYPE_CTF_FLAG_PICKUP, CTF_FLAG_BONUS);

	// sound for team who's flag was stolen
	teamSound(flag->flagTeam, gi.soundindex("ctf/flagtk.wav"), 1);
	// sound for team stealing flag
	teamSound(CTFOtherTeam(flag->flagTeam), gi.soundindex("world/fusein.wav"), 1);

	// keep track of the carrier
	ctfgame.carrier[player->client->resp.team] = player;

	// put item in inventory
	player->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;

	// mark pickup time
	player->client->resp.ctf_flagsince = level.time;

	// team has no active flag - player carrying it
	ctfgame.activeFlag[flag->flagTeam] = NULL;

	// if this is a flag at home base, just
	// make it disappear
	if (!(flag->spawnflags & DROPPED_ITEM)) {
		flag->flags |= FL_RESPAWN;
		flag->svflags |= SVF_NOCLIENT;
		flag->solid = SOLID_NOT;
	}
}

/**
 * Touch function of item_flag's
 * May lead to pickup, return or capture
 */
qboolean CTFTouchFlag(edict_t *flag, edict_t *player)
{
	gitem_t *flag_item, *enemy_flag_item;

	if (flag->flagTeam == TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} else {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	// player touched his own flag
	if (flag->flagTeam == player->client->resp.team) {

		if (flag->spawnflags & DROPPED_ITEM) {
			// player touched his own dropped flag: flag return
			CTFReturnFlag(flag, player, flag_item, enemy_flag_item);
			// indicate didn't pickup flag
			return false;
		} else {
			// player touched his own flag at base
			if (player->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) {
				// player was carrying enemy flag - CAPTURE
				CTFCapture(flag, player, flag_item, enemy_flag_item);
			}
			// indicate didn't pick up 
			return false; 
		}	
	}

	// player touched enemy flag: pickup
	CTFPickupFlag(flag, player, flag_item, enemy_flag_item);
	// indicate picked up flag
	return true;
	// if this is a dropped flag, caller G_FreeEdict()'s it
	// if it's a flag at base, CTFPickupFlag makes it 
	// invisible and non-touchable
}

static void CTFDropFlagTouch(edict_t *flag, edict_t *player, cplane_t *plane, csurface_t *surf)
{
	// owner (who dropped us) can't touch for two secs
	if (player == flag->owner && 
		flag->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT-2)
	{
		return;
	}

	Touch_Item (flag, player, plane, surf);
}

static void CTFDropFlagThink(edict_t *flag)
{
	// auto return the flag
	gi.bprintf(PRINT_HIGH, "The %s flag has %s!\n",
	           nameForTeam(flag->flagTeam),
	           GREENTEXT_RETURNED);

	// sound for team who's flag was returned
	teamSound(flag->flagTeam, gi.soundindex("world/train2.wav"), 1);
	// sound for other team
	teamSound(CTFOtherTeam(flag->flagTeam), gi.soundindex("world/fuseout.wav"), 1);

	// reset flag will remove the flag entity
	CTFResetFlag(flag->flagTeam);
}

// Called from PlayerDie, to drop the flag from a dying player
void CTFDeadDropFlag(edict_t *self)
{
	edict_t *dropped = NULL;
	gitem_t *flag_item;

	if (!flag1_item || !flag2_item)
		CTFInit();

	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
		flag_item = flag1_item;
	} else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
		flag_item = flag2_item;
	} else {
		// had no flag
		return;
	}

	// drop the flag
	dropped = Drop_Item(self, flag_item);

	// dropped flag needs to know it's team
	dropped->flagTeam = CTFOtherTeam(self->client->resp.team);

	// carrier died
	ctfgame.carrier[self->client->resp.team] = NULL;

	// active flag for the enemy team becomes the dropped flag (was NULL)
	ctfgame.activeFlag[CTFOtherTeam(self->client->resp.team)] = dropped;

	// remove from player's inventory
	self->client->pers.inventory[ITEM_INDEX(flag_item)] = 0;

	// announce flag drop
	gi.bprintf(PRINT_HIGH, "%s %s the %s flag!\n",
	           self->client->pers.netname, 
	           GREENTEXT_LOST,
	           nameForTeam(CTFOtherTeam(self->client->resp.team)));

	// set up autoreturn and pickup function
	dropped->think = CTFDropFlagThink;
	dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
	dropped->touch = CTFDropFlagTouch;
}

qboolean CTFDrop_Flag(edict_t *ent, gitem_t *item)
{
	gi.cprintf(ent, PRINT_HIGH, "CTF is about carrier survival under fire\n\n"
	                            "If you want to pass off the flag, you must do "
	                            "it by killing yourself in a situation where a "
	                            "teammate can grab the flag safely.\n");
	return false;
}

void flag_sanity (edict_t *flag) {

	edict_t *redCarrier = ctfgame.carrier[TEAM1];
	edict_t *blueCarrier = ctfgame.carrier[TEAM2];
	
	if (redCarrier) {
		if (redCarrier->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
			gi.dprintf("%s has the blue flag\n", redCarrier->client->pers.netname);
		} else {
			gi.dprintf("********************************\n"
						"%s is supposed to have the blue flag, but doesn't!!!!!!!!\n", 
						redCarrier->client->pers.netname);
		}
	} else {
		if (ctfgame.baseFlag[TEAM2] == ctfgame.activeFlag[TEAM2]) {
			gi.dprintf("Blue flag is at base\n");
		} else {
			gi.dprintf("Blue flag is dropped somewhere\n");
		}
	}

	if (blueCarrier) {
		if (blueCarrier->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
			gi.dprintf("%s has the red flag\n", blueCarrier->client->pers.netname);
		} else {
			gi.dprintf("********************************\n"
						"%s is supposed to have the red flag, but doesn't!!!!!!!!\n", 
						blueCarrier->client->pers.netname);
		}
	} else {
		if (ctfgame.baseFlag[TEAM1] == ctfgame.activeFlag[TEAM1]) {
			gi.dprintf("Red flag is at base\n");
		} else {
			gi.dprintf("Red flag is dropped somewhere\n");
		}
	}
}


static void CTFFlagThink(edict_t *ent)
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	// Flag Sanity checking
	/*
	if (ent->flagTeam == TEAM1 && ((level.framenum & 31) == 31)) {
		flag_sanity(ent);
	}
	*/
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_teleporter_dest(edict_t *ent);

void CTFFlagSetup (edict_t *ent)
{
	trace_t		tr;
	float		*v;
	vec3_t		dest;
	edict_t 	*stand;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("CTFFlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	// store flag's team on flag
	ent->flagTeam = CTFTeamForFlagClass(ent->classname);

	// store newly spawned flag as both 
	// active and home flag for it's team
	ctfgame.baseFlag[ent->flagTeam] = ent;
	ctfgame.activeFlag[ent->flagTeam] = ent;

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;

	// create a "stand" for the flag, that always remains at the flag
	// position as a marker even when the flag is not there.
	stand = G_Spawn();

	// looks like a DM spawn pad..
	SP_misc_teleporter_dest(stand);
	gi.unlinkentity(stand);
	VectorCopy(ent->s.origin, stand->s.origin);
	stand->s.origin[2] += 8;
	// only non-solid
	stand->solid = SOLID_NOT;
	// quad or pent effect according to team
	stand->s.effects |= (ent->flagTeam == TEAM2) ? EF_QUAD : EF_PENT;
	gi.linkentity(stand);

}

void CTFEffects(edict_t *player)
{
	player->s.effects &= (EF_FLAG1 | EF_FLAG2);
	if (player->health > 0) {
		// FIXME : must remove that stupid particle trail but preserve glow
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
			player->s.effects |= EF_FLAG1;
		}
		if (player->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
			player->s.effects |= EF_FLAG2;
		}
	}

	if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag1.md2");
	else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");
	else
		player->s.modelindex3 = 0;
}

// called when we enter the intermission
void CTFCalcScores(void)
{
	int i;

	ctfgame.total1 = ctfgame.total2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.team == TEAM1)
			ctfgame.total1 += game.clients[i].resp.score;
		else if (game.clients[i].resp.team == TEAM2)
			ctfgame.total2 += game.clients[i].resp.score;
	}

}

/*------------------- Overlay/Scoreboard ------------------*/

char *flagStatusIcon(int team)
{
	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped

	if (!ctfgame.activeFlag[team]) {
		// taken
		return (team == TEAM1 ? "i_ctf1t" : "i_ctf2t");
	} else if (ctfgame.baseFlag[team] == ctfgame.activeFlag[team]) {
		// active flag at base
		return (team == TEAM1 ? "i_ctf1" : "i_ctf2");
	} else {
		// dropped
		return (team == TEAM1 ? "i_ctf1d" : "i_ctf2d");
	}
}

// Returns the configstring # of who is carrying a given flag.
int carrierName(int teamnum)
{
	if (ctfgame.carrier[teamnum] == NULL) {
		return 0;
	}

	return (CS_PLAYERNAMES + ctfgame.carrier[teamnum] - g_edicts - 1);
}

void SetCTFStats(edict_t *ent)
{
	// Expert: TeamDistribution
	int team;
/*
	gi.dprintf("SetCTFStats: team1 %d team2 %d\n", 
		ctfgame.teamScore[TEAM1], 
		ctfgame.teamScore[TEAM2]); 
*/

	// logo headers for the frag display
	ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("ctfsb1");
	ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("ctfsb2");

	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8)) { // blink 1/8th second
		// note that ctfgame.total[12] is set when we go to intermission
		if (ctfgame.teamScore[TEAM1] > ctfgame.teamScore[TEAM2])
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.teamScore[TEAM2] > ctfgame.teamScore[TEAM1])
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.total2 > ctfgame.total1) 
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else { // tie game!
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		}
	}

	// flag status icons
	ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = gi.imageindex(flagStatusIcon(TEAM1));
	ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = gi.imageindex(flagStatusIcon(TEAM2));

	// blink status indicator after capture
	if (ctfgame.last_flag_capture && 
	    level.time - ctfgame.last_flag_capture < 5 &&
	    level.framenum & 8)
	{
		if (ctfgame.last_capture_team == TEAM1) {
			ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = 0;
		} else {
			ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = 0;
		}
	}

	// team score in captures
	ent->client->ps.stats[STAT_CTF_TEAM1_CAPS] = ctfgame.teamScore[TEAM1];
	ent->client->ps.stats[STAT_CTF_TEAM2_CAPS] = ctfgame.teamScore[TEAM2];

	// Expert: Team Distribution
	// User can set "notd" override to turn Team Distribution off
	if (expflags & EXPERT_CTF_TEAM_DISTRIBUTION &&
	    (Info_ValueForKey(ent->client->pers.userinfo, "notd")[0] == '\0' ||
	     Info_ValueForKey(ent->client->pers.userinfo, "notd")[0] == '0'))
	{
		if (level.time > ctfgame.lastTeamDistributionCalc + TEAM_DISTRIBUTION_DELAY) 
		{
			calcTeamDistribution(); 
			ctfgame.lastTeamDistributionCalc = level.time;
		}
		team = ent->client->resp.team;
		ent->client->ps.stats[STAT_HOME_BASE] = ctfgame.teamDistribution[team][HOME_BASE];
		ent->client->ps.stats[STAT_NEAR_HOME] = ctfgame.teamDistribution[team][NEAR_HOME];
		ent->client->ps.stats[STAT_NEAR_ENEMY] = ctfgame.teamDistribution[team][NEAR_ENEMY];
		ent->client->ps.stats[STAT_ENEMY_BASE] = ctfgame.teamDistribution[team][ENEMY_BASE];
	
		if (team == TEAM1) 
		{
			ent->client->ps.stats[STAT_TD_TOP_ICON] = gi.imageindex("sbfctf2");
			ent->client->ps.stats[STAT_TD_BOTTOM_ICON] = gi.imageindex("sbfctf1");
		}
		else 
		{
			ent->client->ps.stats[STAT_TD_TOP_ICON] = gi.imageindex("sbfctf1");
			ent->client->ps.stats[STAT_TD_BOTTOM_ICON] = gi.imageindex("sbfctf2");
		}
	}

	// blinking indicator for carriers
	ent->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;
	if (isCarrier(ent) &&
	    level.framenum & 8)
	{
		if (ent->client->resp.team == TEAM1) {
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("i_ctf2");
		} else {
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("i_ctf1");
		}
	}

	// outline home team flag status indicator
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
	if (ent->client->resp.team == TEAM1)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = gi.imageindex ("i_ctfj");
	else if (ent->client->resp.team == TEAM2)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = gi.imageindex ("i_ctfj");

	// Expert: Show carrier names
	ent->client->ps.stats[STAT_CTF_CARRIER1] = carrierName(TEAM1);
	ent->client->ps.stats[STAT_CTF_CARRIER2] = carrierName(TEAM2);

}

/*------------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_info_player_team1(edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self)
{
}

/*
==================
CTFScoreboardMessage
==================
*/
// FIXME : new Scoreboard
// team, name, frags, time connected, captures, total held time
void CTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
	int		score, total[2], totalscore[2];
	int		last[2];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	// sort the clients by team and by score within team
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (!playerIsOnATeam(cl_ent)) {
			continue;
		}

		team = game.clients[i].resp.team;
		score = game.clients[i].resp.score;

		// determine player's rank within team so far
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		// copy all entries higher than current
		// player's rank further into the array
		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		// insert current player's stats
		sorted[team][j] = i;
		sortedscores[team][j] = score;

		// track total team info
		totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// team one
	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 18 "
		"if 25 xv 168 yv 8 pic 25 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 256 yv 12 num 2 20 ",
		totalscore[0], total[0],
		totalscore[1], total[1]);
	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

#if 0 //ndef NEW_SCORE
		// set up y
		sprintf(entry, "yv %d ", 42 + i * 8);
		if (maxsize - len > strlen(entry)) {
			strcat(string, entry);
			len = strlen(string);
		}
#else
		*entry = 0;
#endif

		// left side
		if (i < total[0]) {
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 0 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				strcat(entry, "xv 56 picn sbfctf2 ");
#else
			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				sprintf(entry + strlen(entry), "xv 56 yv %d picn sbfctf2 ",
					42 + i * 8);
#endif

			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[0] = i;
			}
		}

		// right side
		if (i < total[1]) {
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 160 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				strcat(entry, "xv 216 picn sbfctf1 ");

#else

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfctf1 ",
					42 + i * 8);
#endif
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[1] = i;
			}
		}
	}

	// put in spectators if we have enough room
	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];
	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50) {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.team != NOTEAM)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Spectators\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			sprintf(entry+strlen(entry),
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}

	if (total[0] - last[0] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
	if (total[1] - last[1] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*
======================================================================

SAY_TEAM

======================================================================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct {
	char *classname;
	int priority;
} loc_names[] = 
{
	{	"item_flag_team1",			1 },
	{	"item_flag_team2",			1 },
	{	"item_quad",				2 }, 
	{	"item_invulnerability",		2 },
	{	"weapon_bfg",				3 },
	{	"weapon_railgun",			4 },
	{	"weapon_rocketlauncher",	4 },
	{	"weapon_hyperblaster",		4 },
	{	"weapon_chaingun",			4 },
	{	"weapon_grenadelauncher",	4 },
	{	"weapon_machinegun",		4 },
	{	"weapon_supershotgun",		4 },
	{	"weapon_shotgun",			4 },
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
	{	"item_armor_body",			6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
	{	"item_silencer",			7 },
	{	"item_breather",			7 },
	{	"item_enviro",				7 },
	{	"item_adrenaline",			7 },
	{	"item_bandolier",			8 },
	{	"item_pack",				8 },
	{ NULL, 0 }
};


static void CTFSay_Team_Location(edict_t *who, char *buf)
{
	edict_t *what = NULL;
	edict_t *hot = NULL;
	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item;
	int nearteam = -1;
	edict_t *flag1, *flag2;
	qboolean hotsee = false;
	qboolean cansee;

	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		if (!loc_names[i].classname)
			continue;
		// something we can see get priority over something we can't
		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee) {
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}
		// if we can't see this, but we have something we can see, skip it
		if (hotsee && !cansee)
			continue;
		if (hotsee && hotindex < loc_names[i].priority)
			continue;
		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || 
			(cansee && loc_names[i].priority < hotindex)) {
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}

	if (!hot) {
		strcpy(buf, "nowhere");
		return;
	}

	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL) {
		if (what == hot)
			continue;
		// if we are here, there is more than one, find out if hot
		// is closer to red flag or blue flag
		if ((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL &&
			(flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL) {
			VectorSubtract(hot->s.origin, flag1->s.origin, v);
			hotdist = VectorLength(v);
			VectorSubtract(hot->s.origin, flag2->s.origin, v);
			newdist = VectorLength(v);
			if (hotdist < newdist)
				nearteam = TEAM1;
			else if (hotdist > newdist)
				nearteam = TEAM2;
		}
		break;
	}

	if ((item = FindItemByClassname(hot->classname)) == NULL) {
		strcpy(buf, "nowhere");
		return;
	}

	// in water?
	if (who->waterlevel)
		strcpy(buf, "in the water ");
	else
		*buf = 0;

	// near or above
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
		if (v[2] > 0)
			strcat(buf, "above ");
		else
			strcat(buf, "below ");
	else
		strcat(buf, "near ");

	if (nearteam == TEAM1)
		strcat(buf, "the red ");
	else if (nearteam == TEAM2)
		strcat(buf, "the blue ");
	else
		strcat(buf, "the ");

	strcat(buf, item->pickup_name);
}

static void CTFSay_Team_Armor(edict_t *who, char *buf)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	*buf = 0;

	power_armor_type = PowerArmorType (who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells)
			sprintf(buf+strlen(buf), "%s with %i cells ",
				(power_armor_type == POWER_ARMOR_SCREEN) ?
				"Power Screen" : "Power Shield", cells);
	}

	index = ArmorIndex (who);
	if (index)
	{
		item = GetItemByIndex (index);
		if (item) {
			if (*buf)
				strcat(buf, "and ");
			sprintf(buf+strlen(buf), "%i units of %s",
				who->client->pers.inventory[index], item->pickup_name);
		}
	}

	if (!*buf)
		strcpy(buf, "no armor");
}

static void CTFSay_Team_Health(edict_t *who, char *buf)
{
	if (who->health <= 0)
		strcpy(buf, "dead");
	else
		sprintf(buf, "%i health", who->health);
}

static void CTFSay_Team_Weapon(edict_t *who, char *buf)
{
	if (who->client->pers.weapon)
		strcpy(buf, who->client->pers.weapon->pickup_name);
	else
		strcpy(buf, "none");
}

static void CTFSay_Team_Sight(edict_t *who, char *buf)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];

	*s = *s2 = 0;
	for (i = 1; i <= maxclients->value; i++) {
		targ = g_edicts + i;
		if (!targ->inuse || 
			targ == who ||
			!loc_CanSee(targ, who))
			continue;
		if (*s2) {
			if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {
				if (n)
					strcat(s, ", ");
				strcat(s, s2);
				*s2 = 0;
			}
			n++;
		}
		strcpy(s2, targ->client->pers.netname);
	}
	if (*s2) {
		if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {
			if (n)
				strcat(s, " and ");
			strcat(s, s2);
		}
		strcpy(buf, s);
	} else
		strcpy(buf, "no one");
}

void CTFSay_Team(edict_t *who, char *msg)
{
	char outmsg[1024];
	char buf[1024];
	int i;
	char *p;
	edict_t *cl_ent;

	// Expert Floodprot
	if (floodProt(who)) {
		return;
	}

	outmsg[0] = 0;

	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++) {
		if (*msg == '%') {
			switch (*++msg) {
				case 'l' :
				case 'L' :
					CTFSay_Team_Location(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'a' :
				case 'A' :
					CTFSay_Team_Armor(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'h' :
				case 'H' :
					CTFSay_Team_Health(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'w' :
				case 'W' :
					CTFSay_Team_Weapon(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				case 'n' :
				case 'N' :
					CTFSay_Team_Sight(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				default :
					*p++ = *msg;
			}
		} else
			*p++ = *msg;
	}
	*p = 0;

	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.team == who->client->resp.team)
			gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", 
				who->client->pers.netname, outmsg);
	}
}

/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/tris.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_ctf_small_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/small.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}


/*-----------------------------------------------------------------------*/

void CTFJoinTeam(edict_t *ent, int desired_team)
{
	return;
}
/*
	char *s;

	PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	PutClientInServer (ent);
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
		ent->client->pers.netname, nameForTeam(desired_team));
}
*/

void CTFJoinTeam1(edict_t *ent, pmenu_t *p)
{
	CTFJoinTeam(ent, TEAM1);
}

void CTFJoinTeam2(edict_t *ent, pmenu_t *p)
{
	CTFJoinTeam(ent, TEAM2);
}

void CTFReturnToMain(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	CTFOpenJoinMenu(ent);
}

void CTFCredits(edict_t *ent, pmenu_t *p);

void DeathmatchScoreboard (edict_t *ent);

void CTFShowScores(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->client->showscores = true;
	ent->client->showinventory = false;
	DeathmatchScoreboard (ent);
}

pmenu_t creditsmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Expert Quake2",					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*The Expert Programming Team:",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Charles \"Myrkul\" Kendrick",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Rich \"Publius\" Tollerton",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Tim \"Blitherakt!\" Adamec",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Michael \"Smeagol\" Buttrey",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Based on Threewave CTF by",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ "David \"Zoid\" Kirsch",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, NULL, CTFReturnToMain },
};


pmenu_t joinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Expert CTF",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Red Team",		PMENU_ALIGN_LEFT, NULL, CTFJoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Blue Team",		PMENU_ALIGN_LEFT, NULL, CTFJoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
//	{ "Chase Camera",		PMENU_ALIGN_LEFT, NULL, ChaseCam },
	{ "Credits",			PMENU_ALIGN_LEFT, NULL, CTFCredits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

int CTFUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

	joinmenu[4].text = "Join Red Team";
	joinmenu[4].SelectFunc = CTFJoinTeam1;
	joinmenu[6].text = "Join Blue Team";
	joinmenu[6].SelectFunc = CTFJoinTeam2;

	if (ent->client->chase_target)
		joinmenu[8].text = "Leave Chase Camera";
	else
		joinmenu[8].text = "Chase Camera";

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

	num1 = num2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.team == TEAM1)
			num1++;
		else if (game.clients[i].resp.team == TEAM2)
			num2++;
	}

	sprintf(team1players, "  (%d players)", num1);
	sprintf(team2players, "  (%d players)", num2);

	joinmenu[2].text = levelname;
	if (joinmenu[4].text)
		joinmenu[5].text = team1players;
	else
		joinmenu[5].text = NULL;
	if (joinmenu[6].text)
		joinmenu[7].text = team2players;
	else
		joinmenu[7].text = NULL;
	
	if (num1 > num2)
		return TEAM1;
	else if (num2 > num1)
		return TEAM1;
	return (rand() & 1) ? TEAM1 : TEAM2;
}

void CTFOpenJoinMenu(edict_t *ent)
{
	int team;

	team = CTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == TEAM1)
		team = 4;
	else
		team = 6;
	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t));
}

void CTFCredits(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t));
}

qboolean CTFCheckRules(void)
{
	if (capturelimit->value && 
		(ctfgame.teamScore[TEAM1] >= capturelimit->value ||
		 ctfgame.teamScore[TEAM2] >= capturelimit->value)) {
		gi.bprintf (PRINT_HIGH, "Capturelimit hit.\n");
		return true;
	}
	return false;
}

/*--------------------------------------------------------------------------
 * just here to help old map conversions
 *--------------------------------------------------------------------------*/

static void old_teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;
	vec3_t		forward;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
//	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	VectorCopy (dest->s.angles, other->client->ps.viewangles);
	VectorCopy (dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors (other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200, other->velocity);

	// kill anything at the destination
	if (!KillBox (other))
	{
	}

	gi.linkentity (other);
}

/*QUAKED trigger_teleport (0.5 0.5 0.5) ?
Players touching this will be teleported
*/
void SP_trigger_teleport (edict_t *ent)
{
	edict_t *s;
	int i;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = old_teleporter_touch;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

	// noise maker and splash effect dude
	s = G_Spawn();
	ent->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = ent->mins[i] + (ent->maxs[i] - ent->mins[i])/2;
	s->s.sound = gi.soundindex ("world/hum1.wav");
	gi.linkentity(s);
	
}

/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)
Point trigger_teleports at these.
*/
void SP_info_teleport_destination (edict_t *ent)
{
	ent->s.origin[2] += 16;
}

