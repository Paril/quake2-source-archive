#include "g_local.h"
#include "ctf.h"
#include "runes.h"
#include "flag.h"
#include "teamplay.h"
#include "id.h"

// ZOID
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

// spectator
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "

//tech
"yb -129 "
"if 26 "
  "xr -26 "
  "pic 26 "
"endif "

// red team
"yb -102 "
//joined overlay
"if 24 "
  "xr -77 "
  "pic 24 "
  "xr -76 "
  "pic 24 "
"endif "
"if 19 "
  "xr -26 "
  "pic 19 "
"endif "
"xr -78 "
"num 3 20 "

// blue team
"yb -75 "
"if 25 "
  "xr -77 "
  "pic 25 "
  "xr -76 "
  "pic 25 "
"endif "
"if 21 "
  "xr -26 "
  "pic 21 "
"endif "
"xr -78 "
"num 3 22 "

// have flag graph
"if 23 "
  "yt 26 "
  "xr -24 "
  "pic 23 "
"endif "

// id view state
"if 27 "
  "xv 0 "
  "yb -58 "
  "string \"Viewing\" "
  "xv 64 "
  "stat_string 27 "
"endif "
;

// ZOID
void ctf_frag_bonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{

	edict_t		*flag, *carrier;
	vec3_t		v1, v2;

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;

	// whoever died isn't on a team
	if (!(IS_TEAM1(targ) || IS_TEAM2(targ)))
		return;

	// did the attacker frag the person carrying his team's flag?
	if (flag_has_flag(targ) == attacker->client->resp.ctf_team) {
		attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
		gi.cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n",
			CTF_FRAG_CARRIER_BONUS);
		return;
	}

	// find the attacker's flag
	flag = NULL;
	while ((flag = G_Find (flag, FOFS(classname), flag_classnameforteam[attacker->client->resp.ctf_team]))) {
		if (!(flag->spawnflags & DROPPED_ITEM))
			break;
	}

	if (!flag)
		return; // can't find attacker's flag

	// find attacker's team's flag carrier
	carrier = NULL;
	while (carrier = G_Find(carrier, FOFS(classname), "player"))
	{
		if (carrier->client && flag_has_flag(carrier) && OnSameTeam(carrier, attacker))
			break;
	}

	if (!carrier)
		return;

	// ok we have the attackers flag and a pointer to the carrier

	// check to see if we are defending the base's flag
	VectorSubtract(targ->s.origin, flag->s.origin, v1);
	VectorSubtract(attacker->s.origin, flag->s.origin, v2);

	if (VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS ||
		VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS ||
		CanDamage(flag, targ) || CanDamage(flag, attacker)) {
		// we defended the base flag
		attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
		if (flag->solid == SOLID_NOT)
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s base.\n",
				attacker->client->pers.netname, 
				team_nameforteam[attacker->client->resp.ctf_team]);
		else
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n",
				attacker->client->pers.netname, 
				team_nameforteam[attacker->client->resp.ctf_team]);
		return;
	}

	// see if defending our flag carrier
	if (carrier && carrier != attacker) {
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v2);

		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS ||
			VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS ||
			CanDamage(carrier, targ) || CanDamage(carrier, attacker)) {
			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
				attacker->client->pers.netname, 
				team_nameforteam[attacker->client->resp.ctf_team]);
			return;
		}
	}
}

void stuffcmd(edict_t *e, char *s) {
	// Why q2 calls ClientUserinfoChanged during the opening
	// scenes, I'll never know
	if (strcmp(level.level_name, "") == 0) return;
	gi.WriteByte (11);
	gi.WriteString (s);
	gi.unicast (e, true);
}

// ZOID
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

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			team = 0;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			team = 1;
		else
			continue; // unknown team?

		score = game.clients[i].resp.score;
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// team one
	sprintf(string, "xv 8 yv 8 picn k_redkey "
		"xv 40 yv 28 string \"%-3d\" "
		"xv 74 yv 12 num 3 20 "
		"xv 168 yv 8 picn k_bluekey "
		"xv 200 yv 28 string \"%-3d\" "
		"xv 232 yv 12 num 3 22 ",
		/*totalscore[0],*/ total[0],
		/*totalscore[1],*/ total[1]);
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

			if (flag_has_flag(cl_ent))
				sprintf(entry + strlen(entry), "xv 56 yv %d string \"-\" ",
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

			if (flag_has_flag(cl_ent))
				sprintf(entry + strlen(entry), "xv 216 yv %d string \"-\" ",
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
				cl_ent->client->resp.ctf_team != CTF_NOTEAM)
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

void ctf_update_sbar(edict_t *ent)
{
	int	i;
	int p1, p2;
	int	state;
	int	redscore, bluescore;

	// tech name
	ent->client->ps.stats[STAT_CTF_TECH] = 0;
	if (i = rune_has_a_rune(ent))
		ent->client->ps.stats[STAT_CTF_TECH] = gi.imageindex(rune_iconfornum[i]);

	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped

	state = flag_state(CTF_TEAM1, NULL);
	switch (state)
	{
		case FLAG_ATBASE:
			p1 = gi.imageindex ("k_redkey");
			break;
		default:
			p1 = 0;
	}

	state = flag_state(CTF_TEAM2, NULL);
	switch (state)
	{
		case FLAG_ATBASE:
			p2 = gi.imageindex ("k_bluekey");
			break;
		default:
			p2 = 0;
	}

	ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;

	team_scores(&redscore, &bluescore, NULL, NULL);

	ent->client->ps.stats[STAT_CTF_TEAM1_SCORE] = redscore;
	ent->client->ps.stats[STAT_CTF_TEAM2_SCORE] = bluescore;

	state = flag_has_flag(ent);
	ent->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;
	if (state == CTF_TEAM1)
		ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("k_redkey");
	else if (state == CTF_TEAM2)
		ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("k_bluekey");


	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
	if (IS_TEAM1(ent))
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = gi.imageindex ("field_3");
	else if (IS_TEAM2(ent))
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = gi.imageindex ("field_3");

	if (ent->client->resp.id_state)
		id_update_sbar(ent);
	else
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;

	if (ent->client->showmotd && ent->client->pers.health > 0)
		ent->client->ps.stats[STAT_LAYOUTS] |= 1;		// will this work?
}

void Cmd_CTFHelp_f (edict_t *ent) {
	gi.cprintf (ent, PRINT_HIGH,
		"Listing of CTF-related commands:\n"
		"'ctfhelp'     this list of commands\n"
		"'drop tech'   drops your rune\n"
		"'flagstat'    status of the flags\n"
		"'id'          toggles id display\n"
		"'team blue'   switch to blue team\n"
		"'team red'    switch to red team\n"
		"'use grapple' selects the hook\n"
		"'+hook'       use offhand hook\n");
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

	while ((what = findradius(what, who->s.origin, 1024)) != NULL) {
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		if (!loc_names[i].classname)
			continue;
		// something we can see get priority over something we can't
		cansee = CanDamage(what, who);
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
			hotsee = CanDamage(hot, who);
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
				nearteam = CTF_TEAM1;
			else if (hotdist > newdist)
				nearteam = CTF_TEAM2;
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

	if (nearteam == CTF_TEAM1)
		strcat(buf, "the red ");
	else if (nearteam == CTF_TEAM2)
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

static void CTFSay_Team_Tech(edict_t *who, char *buf)
{
	int i;
	// see if the player has a tech powerup
	if (i = rune_has_a_rune(who))
	{
		sprintf(buf, "the %s", rune_namefornum[i]);
		return;
	}
	strcpy(buf, "no powerup");
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
// CTF
			!CanDamage(targ, who) ||
			(targ->client && targ->client->resp.spectator))
// End CTF			
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

void Cmd_CTFSay_f(edict_t *who)
{
	char outmsg[1024];
	char buf[1024];
	int i;
	char *p;
	edict_t *cl_ent;
	char *msg = gi.args();

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
				case 't' :
				case 'T' :
					CTFSay_Team_Tech(who, buf);
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

// CTF - fix buffer overflow problem
	if ( strlen(outmsg) > 150)
		outmsg[150] = 0;
// End CTF

	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team == who->client->resp.ctf_team)
			gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", 
				who->client->pers.netname, outmsg);
	}
}

void ScoreHolder_Cleanup(edict_t *self)
{
	gi.TagFree(self->target);
	G_FreeEdict(self);
}

void CTF_Disconnect(edict_t *ent)
{
	edict_t *sh;
	sh = G_Spawn();
	sh->classname = "score_holder";
	sh->count = ent->client->resp.score;
	sh->target = gi.TagMalloc(16, TAG_LEVEL);
	strcpy(sh->target, ent->client->pers.netname);
	sh->spawnflags = ent->client->resp.ctf_team;
	sh->think = ScoreHolder_Cleanup;
	sh->nextthink = level.time + 5 * 60;
}

qboolean CTF_Reconnect(edict_t *ent)
{
	edict_t *sh = NULL;
	while (sh = G_Find(sh, FOFS(classname), "score_holder"))
	{
		if (strcmp(sh->target, Info_ValueForKey(ent->client->pers.userinfo, "name")) == 0)
		{
			ent->client->resp.score = sh->count;
			ent->client->resp.ctf_team = sh->spawnflags;
			ScoreHolder_Cleanup(sh);
			return(true);
		}
	}
	return(false);
}

void CTF_EndLevel()
{
	int	redscore = 0, bluescore = 0;

    if (!ctf->value)
		return;

	team_scores(&redscore, &bluescore, NULL, NULL);

	if (redscore > bluescore) {
		gi.bprintf(PRINT_HIGH, "Red team won the level with %d points\n", redscore);
		gi.bprintf(PRINT_HIGH, "Blue team lost the level with %d points\n", bluescore);
	} else if (bluescore > redscore) {
		gi.bprintf(PRINT_HIGH, "Blue team won the level with %d points\n", bluescore);
		gi.bprintf(PRINT_HIGH, "Red team lost the level with %d points\n", redscore);
	} else {
		gi.bprintf(PRINT_HIGH, "Red and Blue teams tied the level with %d points\n", redscore);
	}
}


qboolean CTF_CheckRules()
{
	if (ctf->value && teamscorelimit->value)
	{
		int redscore, bluescore;

		team_scores(&redscore, &bluescore, NULL, NULL);

		if (redscore >= teamscorelimit->value || bluescore >= teamscorelimit->value)
		{
			gi.bprintf (PRINT_HIGH, "Teamscorelimit hit.\n");
			return (true);
		}
	}
	return(false);
}