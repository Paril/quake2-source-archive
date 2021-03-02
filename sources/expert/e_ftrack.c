#include "g_local.h"

#define		FLAGTRACK_RESPAWN_DELAY							15
#define		FLAGTRACK_INACTIVE_TIMEOUT						90
#define		FLAGTRACK_TOSS_SPEED							100

#define		FLAGTRACK_CARRIER_DANGER_PROTECT_TIMEOUT		5

#define		FLAGTRACK_CAPTURE_BONUS							5
#define		FLAGTRACK_ASSIST_BONUS							5
#define		FLAGTRACK_CARRIER_KILL_BONUS					2
#define		FLAGTRACK_CARRIER_DANGER_PROTECT_BONUS			1

list_t flags;

// defined in p_client.c
edict_t *SelectFarthestDeathmatchSpawnPoint(edict_t *player);

char *flagtrack_statusbar =
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

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

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
/*
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
// team indicator
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
*/
// have flag graph
"if 21 "
//  "yt 26 "
//  "xr -24 "
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

;

char *modelForTeam(int team) {
	// FIXME two team dependancy
	return (team == TEAM1) ? "players/male/flag1.md2" : "players/male/flag2.md2";
}

void FlagTrackEffects(edict_t *player) {

	player->s.effects &= ~(EF_FLAG1 | EF_FLAG2);
	if (player->health > 0) {
		if (player->client->flagCarried == TEAM1) {
			player->s.effects |= EF_FLAG1;
			player->s.modelindex3 = gi.modelindex("players/male/flag1.md2");
		} else if (player->client->flagCarried == TEAM2) {
			player->s.effects |= EF_FLAG2;
			player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");
		} else {
			player->s.modelindex3 = 0;
		}
	}

}

void touchFlag(edict_t *flag, edict_t *player, cplane_t *plane, csurface_t *surf);
edict_t *spawnFlag(int team);

void respawnInactiveFlag(edict_t *flag)
{
	spawnFlag(flag->flagTeam);
	G_FreeEdict(flag);
}

void activateFlag(edict_t *flag) 
{
	//gi.dprintf("Flag activating for team %d\n", flag->flagTeam);
	// make visible and touchable
	flag->solid = SOLID_TRIGGER;
	flag->touch = touchFlag;
	flag->movetype = MOVETYPE_TOSS;

	// respawn flags when not touched for a long time
	flag->think = respawnInactiveFlag;
	flag->nextthink = level.time + FLAGTRACK_INACTIVE_TIMEOUT;

	// make flags easy to see
	flag->s.effects |= EF_ROTATE;
	flag->s.renderfx |= RF_FULLBRIGHT;

	gi.linkentity(flag);
}

edict_t *createFlag(int team) {

	edict_t *flag;
	float		*v;

	flag = G_Spawn();

	flag->classname = "item_flag";

	// set bounding box
	v = tv(-15,-15,-15);
	VectorCopy (v, flag->mins);
	v = tv(15,15,15);
	VectorCopy (v, flag->maxs);

	flag->flagTeam = team;
	// set model
	flag->model = modelForTeam(flag->flagTeam);
	gi.setmodel (flag, flag->model);
	gi.modelindex (flag->model);

	// FIXME two team dependancy
	flag->s.effects = (team == TEAM1) ? EF_FLAG1 : EF_FLAG2;

	return flag;

}

edict_t *spawnFlag(int team)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;
	edict_t *spawnSpot, *flag; 

	flag = createFlag(team);

	flag->solid = SOLID_NOT;

	// make solid and touchable after a delay
	// in the meantime, this team's players can track this flag spawn position
	flag->nextthink = level.time + FLAGTRACK_RESPAWN_DELAY;
	flag->think = activateFlag;

	// place flag on DM spawn spot furthest from players
	spawnSpot = SelectFarthestDeathmatchSpawnPoint(flag);
	VectorCopy (spawnSpot->s.origin, flag->s.origin);

	// equivalent of droptofloor() in g_items.c
	v = tv(0,0,-128);
	VectorAdd (flag->s.origin, v, dest);
	tr = gi.trace (flag->s.origin, flag->mins, flag->maxs, dest, flag, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("FlagTrack: flag startsolid at %s\n", vtos(flag->s.origin));
		G_FreeEdict (flag);
		return NULL;
	}
	VectorCopy (tr.endpos, flag->s.origin);

	//gi.dprintf("Spawning a flag for team %d at %s\n", team, vtos(flag->s.origin));

	listAppend(flags, flag);

	// note: initially unlinked, so invisible and not able to be picked up 

	return flag;
}

edict_t *tossFlag(edict_t *player) {
	vec3_t forward, right, offset;
	trace_t	trace;
	edict_t *flag;

	if (player->client->flagCarried == NOTEAM) {
		return NULL;
	}

	flag = createFlag(player->client->flagCarried);

	// code stolen from Drop_Item in g_items.c
	// place flag a bit forward of player but not into solid
	AngleVectors (player->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 0, -16);
	G_ProjectSource (player->s.origin, offset, forward, right, flag->s.origin);
	trace = gi.trace (player->s.origin, flag->mins, flag->maxs,
		flag->s.origin, player, CONTENTS_SOLID);
	VectorCopy (trace.endpos, flag->s.origin);

	// throw flag forward and up
	VectorScale (forward, 100, flag->velocity);
	flag->velocity[2] = 300;

	// flag being dropped - thrown from player 
	// and immediately touchable
	flag->solid = SOLID_TRIGGER;
	flag->touch = touchFlag;
	flag->movetype = MOVETYPE_TOSS;

	// easy to see
	flag->s.renderfx |= RF_FULLBRIGHT;

	// respawn flags when not touched for a long time
	flag->think = respawnInactiveFlag;
	flag->nextthink = level.time + FLAGTRACK_INACTIVE_TIMEOUT;

	player->client->flagCarried = NOTEAM;
	flag->owner = player;
	flag->drop_time = level.time;

	listAppend(flags, flag);

	gi.linkentity(flag);

	return flag;
}

void FlagTrackDeadDropFlag(edict_t *targ, edict_t *attacker)
{
	edict_t *flag;

	// drop the flag
	flag = tossFlag(targ);

	// player no longer has flag
	targ->client->flagCarried = NOTEAM;

	// scoring information: if the player was a carrier, record
	// player who killed this carrier, if it was a player on the enemy team
	if (flag &&
	    attacker->client && 
	    attacker->client->resp.team != targ->client->resp.team) {
		flag->assist = attacker;
	}
}

void Cmd_Drop_Flag(edict_t *player)
{
	edict_t *flag;

	if (player->client->flagCarried == -1) {
		gi.cprintf(player, PRINT_HIGH, "You aren't carrying a flag\n");
	}

	if (player->client->flagCarried != player->client->resp.team) {
		flag = tossFlag(player);
		player->client->flagCarried = NOTEAM;
		// scoring: mark the flag with the player who dropped it
		flag->assist = player;
	} else {
		gi.cprintf(player, PRINT_HIGH, "You can only toss an enemy flag\n");
	}
}

void pickupFlag(edict_t *flag, edict_t *player) 
{
	// text message
	if (player->client->resp.team == flag->flagTeam) {
		gi.cprintf(player, PRINT_HIGH, "You picked up your team's flag!\n");
	} else {
		gi.cprintf(player, PRINT_HIGH, "You picked up an enemy flag!\n");
	}

	player->client->flagCarried = flag->flagTeam;

	G_FreeEdict(flag);
}

// ASSISTS
// There are two ways to get an assist in FlagTrack: 
// 1. assister kills an enemy carrier causing a flag to drop, which a friendly
// carrier *other than the assister* captures as soon as he touches it, before 
// any other player touches it.
// 2. assister drops a flag, which a friendly carrier *other than the assister*
// captures as soon as he touches it, before any other player touches it.
//
// There are no timelimits on assists.
//
// Note that if carriers could assist themselves in FlagTrack, they could score
// freebie assists through various combinations of flag dropping and grabbing.
//
// Note that anytime a flag is picked up, any assist-waiting-to-happen is
// invalidated (since the flag is free'd). 
//
// Note also that flag->assist can be an enemy player (if an enemy player 
// dropped the flag), in which case no assist is scored.
void scoreAssist(edict_t *flag, edict_t *player)
{
	if (flag->assist && 
	    flag->assist->inuse &&
		flag->assist->client->pers.connected &&
	    flag->assist->client && 
		flag->assist != player &&
	    flag->assist->client->resp.team == player->client->resp.team) 
	{
		gi.bprintf(PRINT_HIGH, "ASSIST: %s [%s]\n", 	
				flag->assist->client->pers.netname,
				nameForTeam(flag->assist->client->resp.team));
		flag->assist->client->resp.score += FLAGTRACK_ASSIST_BONUS;
	}

}

void captureFlag(edict_t *flag, edict_t *player)
{
	int capturedFlag;

	// play sound from capturing player only
	gi.sound (player, CHAN_RELIABLE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NORM, 0);

	// score capture and assist, if any
	player->client->resp.score += FLAGTRACK_CAPTURE_BONUS;
	scoreAssist(flag, player);
	
	// determine team of capturedFlag
	if (player->client->resp.team != player->client->flagCarried) {
		// flag player was holding was captured
		// (player picks up flag on ground)
		capturedFlag = player->client->flagCarried;
	} else {
		// flag player touched was captured
		capturedFlag = flag->flagTeam;
	}

	// replace captured flag
	spawnFlag(capturedFlag);
	
	// announce capture
	gi.bprintf(PRINT_HIGH, "%s [%s] CAPTURED a %s flag\n", 
		va("%s", greenText(player->client->pers.netname)),
		nameForTeam(player->client->resp.team),
		nameForTeam(capturedFlag));

	// flag on ground is freed in either case
	G_FreeEdict(flag);

	// player ends up with his own flag
	player->client->flagCarried = player->client->resp.team;
}

void touchFlag(edict_t *flag, edict_t *player, cplane_t *plane, csurface_t *surf)
{
	if (!player->client)
		return;
	if (player->health < 1)
		return;		// dead people can't pickup
	// prevent dropping play from instantly re-touching flag
	if (flag->owner == player && level.time - flag->drop_time < 1)
		return;

	if (player->client->flagCarried == NOTEAM) {
		// player has no flag, so this is always a pickup
		pickupFlag(flag, player);
		return;
	}

	// player is carrying a flag

	if (flag->flagTeam == player->client->flagCarried) {
		// flag touched is the same team as flag carried, do nothing
		return;
	}

	// This criteria only required for 3 or more teams
	/*
	if (flag->flagTeam != player->client->resp.team &&
	    player->client->flagCarried != player->client->resp.team) {
		// neither the carried flag nor the flag on the ground is of the
		// player's team - no capture
		return;
	}
	*/
	
	// player has flag, flags on carrier and flag on ground are from 
	// different teams, and one flag is from player's team - CAPTURE
	captureFlag(flag, player);
}

/**
 * Bonus scoring for FlagTrack.
 */
void FlagTrackScoring(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
/*
	int i;
	edict_t *ent;
*/
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

	// Carrier kill
	if (targ->client->flagCarried != NOTEAM) {
		gi.bprintf(PRINT_MEDIUM, "BONUS %s: CARRIER KILL: %d points\n",
		           attacker->client->pers.netname,
		           FLAGTRACK_CARRIER_KILL_BONUS + 1); // bonus plus the frag for the kill
		attacker->client->resp.score += FLAGTRACK_CARRIER_KILL_BONUS;
	}

	// FIXME : for carrier saves, players need to have to use a list of carriers 
	// that they recently hurt, so that: 
	// 1. You don't get a carrier save for defending yourself
	// 2. When a carrier dies, they can be removed from the list of carriers 
	// recently defended without affecting bonuses for saving other carriers
/*
	// CARRIER SAVE bonus, for killing an enemy player who 
	// has recently hurt a friendly carrier  
	if (level.time - targ->client->resp.ctf_lasthurtcarrier < FLAGTRACK_CARRIER_DANGER_PROTECT_TIMEOUT &&
	    !attacker->client->pers.inventory[ITEM_INDEX(flag_item)]) // attacker not the carrier
	{ 
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		attacker->client->resp.score += FLAGTRACK_CARRIER_DANGER_PROTECT_BONUS;
		gi.bprintf(PRINT_MEDIUM, "BONUS %s: %s CARRIER SAVE\n",
			attacker->client->pers.netname, 
			nameForTeam(attacker->client->resp.team));
		// prevent further (radius-based) carrier defense bonuses
		gotCarrierDefenseBonus = true;
	}
*/
	
}

void FlagTrackInit() {
	int i;
	int flagsPerTeam = ceil(maxclients->value / 4);

	// list of all FlagTrack flags
	flags = listNew(0, NULL);

	for (i = 0; i < flagsPerTeam; i++) {
		// FIXME two team dependancy
		spawnFlag(TEAM1);
		spawnFlag(TEAM2);
	}

}
