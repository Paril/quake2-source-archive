#include "g_local.h"

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

// static
qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);

	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

// GRIM
gitem_t *FlagItem(teams_t *team)
{
        gitem_t *flag_item;
        int team_index;

        team_index = TEAM_INDEX(team);

        // Paranoid
        if ((team_index > 4) || (team_index < 1))
        {
                flag_item = FindItemByClassname("item_flag_generic");
                return flag_item;
        }
//                return NULL;
        // Paranoid

        switch (team_index)
        {
                case 1:
                        flag_item = FindItemByClassname("item_flag_team1");
                        break;
                case 2:
                        flag_item = FindItemByClassname("item_flag_team2");
                        break;
                case 3:
                        flag_item = FindItemByClassname("item_flag_team3");
                        break;
                case 4:
                        flag_item = FindItemByClassname("item_flag_team4");
                        break;
        }
        return flag_item;
}

char *FlagClassname(teams_t *team)
{
        int team_index;

        team_index = TEAM_INDEX(team);

        if (team_index > 4)
                return "UNKNOWN";

        switch (team_index)
        {
                case 1:
                        return "item_flag_team1";
                case 2:
                        return "item_flag_team2";
                case 3:
                        return "item_flag_team3";
                case 4:
                        return "item_flag_team4";
        }
        return "UNKNOWN";
}

// GRIM

/*--------------------------------------------------------------------------*/

/*
CTFFragBonuses

Calculate the bonuses for flag defense, flag carrier defense, etc.
Note that bonuses are not cumaltive.  You get one, they are in importance
order.
*/
// GRIM
void CTFClearLastHurt(teams_t *team)
{
	int i;
        edict_t *player;
        for (i = 1; i <= maxclients->value; i++)
        {
                player = g_edicts + i;
                if (player->inuse && player->client->resp.team == team)
                {
                        player->client->resp.ctf_lasthurtcarrier = 0;
                        player->client->resp.ctf_lasthurtcarrier_flag = NULL;
                }
        }
}

void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	edict_t *flag, *carrier;
        teams_t *team;
	vec3_t v1, v2;
	char *c;
	int i;

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;

        if (!targ->client->resp.team)
		return; // whoever died isn't on a team

        // Did the attacker frag a flag carrier?
        if (targ->client->pers.has_flag)
        {
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
                attacker->client->resp.ctf_lastfraggedcarrier_flag = targ->client->pers.has_flag;

                // Did the attacker frag a flag carrier carrying their flag?
                if (targ->client->pers.has_flag == attacker->client->resp.team)
                {
                        AdjustScore(attacker, CTF_FRAG_YOUR_CARRIER_BONUS, false);
                        //attacker->client->resp.score += CTF_FRAG_YOUR_CARRIER_BONUS;
                        gi.cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier with your flag.\n", CTF_FRAG_YOUR_CARRIER_BONUS);
                }
                else
                {
                        AdjustScore(attacker, CTF_FRAG_CARRIER_BONUS, false);
                        //attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
                        gi.cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging an enemy flag carrier.\n", CTF_FRAG_CARRIER_BONUS);
                }

                // The target had the flag, clear the hurt carrier
		// field on the other team
                CTFClearLastHurt(targ->client->resp.team);
		return;
	}

        team = targ->client->resp.team;

        if (targ->client->resp.ctf_lasthurtcarrier_flag
         && (targ->client->resp.ctf_lasthurtcarrier_flag != attacker->client->pers.has_flag)
          && (level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT))
        {
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
                AdjustScore(attacker, CTF_CARRIER_DANGER_PROTECT_BONUS, false);
                //attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;

                gi.bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n", attacker->client->pers.netname, team->name);
		return;
	}

	// flag and flag carrier area defense bonuses

	// we have to find the flag and carrier entities

	// find the flag
        c = FlagClassname(attacker->client->resp.team);

	flag = NULL;
	while ((flag = G_Find (flag, FOFS(classname), c)) != NULL)
        {
                if ((!(flag->spawnflags & DROPPED_ITEM)) && (!(flag->spawnflags & DROPPED_PLAYER_ITEM)))
			break;
	}

	if (!flag)
		return; // can't find attacker's flag

	// find attacker's team's flag carrier
	for (i = 1; i <= maxclients->value; i++)
        {
		carrier = g_edicts + i;
                if (carrier->inuse && carrier->client->pers.has_flag)
			break;
		carrier = NULL;
	}

	// ok we have the attackers flag and a pointer to the carrier

	// check to see if we are defending the base's flag
	VectorSubtract(targ->s.origin, flag->s.origin, v1);
	VectorSubtract(attacker->s.origin, flag->s.origin, v2);

        if ((VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS)
         || (VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS)
          || loc_CanSee(flag, targ) || loc_CanSee(flag, attacker))
        {
                // Paranoid
		if (flag->solid == SOLID_NOT)
                        gi.bprintf(PRINT_MEDIUM, "%s defends the %s base.\n", attacker->client->pers.netname, team->name);
                else if (flag->item && (flag->item->quantity == 5))
                        return; // No defense bonus for flag not at base
                else
                        gi.bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n", attacker->client->pers.netname, team->name);

		// we defended the base flag
                AdjustScore(attacker, CTF_FLAG_DEFENSE_BONUS, false);
                //attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
                // Paranoid
		return;
	}

	if (carrier && carrier != attacker)
        {
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS
                 || VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS
                  || loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker))
                {
                        AdjustScore(attacker, CTF_CARRIER_PROTECT_BONUS, false);
                        //attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
                        gi.bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n", attacker->client->pers.netname, team->name);
			return;
		}
	}
}

void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker)
{
	if (!targ->client || !attacker->client)
		return;

        if ((targ->client->resp.team != attacker->client->resp.team) && targ->client->pers.has_flag)
        {
                attacker->client->resp.ctf_lasthurtcarrier = level.time;
                attacker->client->resp.ctf_lasthurtcarrier_flag = targ->client->pers.has_flag;
        }
}
// GRIM

/*------------------------------------------------------------------------*/

// GRIM
// This whole are is editing to shit aswell :)
void CTFResetFlag(teams_t *team)
{
	edict_t *ent;
        char    *c;

        c = FlagClassname(team);

	ent = NULL;
        while ((ent = G_Find (ent, FOFS(classname), c)) != NULL)
        {
                if (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
                        G_FreeEdict(ent);
                else
                {
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

void CTFFlagCaptured(edict_t *capturer, teams_t *flags_team)
{
        edict_t *player;
        int     i;

        CTFResetFlag(flags_team); // Carried flag back

        if (capturer->client->pers.inventory[ITEM_INDEX(FlagItem(flags_team))])
        {
                capturer->client->pers.inventory[ITEM_INDEX(FlagItem(flags_team))] = 0;
                RemoveItemWeight (capturer, FlagItem(flags_team));
        }
        capturer->client->pers.has_flag = NULL;

        // Dirty - Goes in Large Weapon slot now
        SetSlot (capturer, "Large Weapon Slot", NULL, 0, 0, NULL, 0, NULL, 0);
        SetLargeWeaponHUD (capturer);
        // Dirty

        if (flags_team->members < 1)
        {
                //gi.sound (capturer, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("comment/laugh3.wav"), 1, ATTN_NONE, 0);
                gi.sound (capturer, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("laugh3.wav"), 1, ATTN_NONE, 0);
                if ((int)bflags->value & BF_CTF_TOUCH)
                {
                        if (level.time - capturer->last_taken_time > 1)
                                gi.cprintf(capturer, PRINT_HIGH, "Can't capture the %s flag. That team is memberless!\n", flags_team->name);
                }
                else if (random() < 0.5)
                        gi.bprintf(PRINT_HIGH, "%s got the %s flag for nothing. That team is memberless!\n", capturer->client->pers.netname, flags_team->name);
                else
                        gi.bprintf(PRINT_HIGH, "%s didn't capture the %s flag, no one is on that team!\n", capturer->client->pers.netname, flags_team->name);
                return;
        }

        gi.bprintf(PRINT_HIGH, "%s captured the %s flag!\n", capturer->client->pers.netname, flags_team->name);

        if (turns_on)
        {
                i = TeamGoal(flags_team);
                capturer->client->resp.team->score += i;
        }
        else
                AdjustTeamScore(capturer->client->resp.team, 1, false);

        //capturver->client->resp.team->score++;
        AdjustScore(capturer, CTF_CAPTURE_BONUS, false);
        //capturer->client->resp.score += CTF_CAPTURE_BONUS;

        teamgame.last_score_team = capturer->client->resp.team;
        teamgame.last_score = level.time;
        MatchOverCheck(); // Dirty

        gi.sound (capturer, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_AUTO, gi.soundindex(va("teams/t%i_fcp.wav", TEAM_INDEX(capturer->client->resp.team))), 1, ATTN_NONE, 0);

        // Ok, let's do the player loop, hand out the bonuses
        for (i = 1; i <= maxclients->value; i++)
        {
                player = &g_edicts[i];
                if (!player->inuse)
                        continue;
                if (player->client->resp.team != capturer->client->resp.team)
                        player->client->resp.ctf_lasthurtcarrier = -5;
                else if (player->client->resp.team == capturer->client->resp.team)
                {
                        if (player != capturer)
                                AdjustScore(player, CTF_TEAM_BONUS, false);
                                //player->client->resp.score += CTF_TEAM_BONUS;
                        // award extra points for capture assists
                        if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time)
                        {
                                gi.bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);
                                AdjustScore(player, CTF_RETURN_FLAG_ASSIST_BONUS, false);
                                //player->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
                        }
                        if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time)
                        {
                                gi.bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
                                AdjustScore(player, CTF_FRAG_CARRIER_ASSIST_BONUS, false);
                                //player->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS;
                        }
                }

                // FIX ME - Work on this...
                // GRIM - Put everyone (But our hero) back into their
                // bases or at a random point if no spawn point available
                // inside their base...
                /*
                if (player != capturer)
                {
                        player->client->resp.ctf_state = CTF_STATE_RESTART;
                        SelectReStartPoint (player);
                }
                */
                // GRIM
                // FIX ME
        }
}

void CTF_FlagReturned (edict_t *ent)
{

        if (ent->client->pers.inventory[ITEM_INDEX(FlagItem(ent->client->resp.team))])
        {
                ent->client->pers.inventory[ITEM_INDEX(FlagItem(ent->client->resp.team))] = 0;
                RemoveItemWeight (ent, FlagItem(ent->client->resp.team));
        }
        if (ent->client->pers.has_flag == ent->client->resp.team)
                ent->client->pers.has_flag = NULL;

        AdjustScore(ent, CTF_RECOVERY_BONUS, false);
        //ent->client->resp.score += CTF_RECOVERY_BONUS;
        ent->client->resp.ctf_lastreturnedflag = level.time;

        // Dirty
        //gi.bprintf(PRINT_HIGH, "%s returned the %s flag!\n", ent->client->pers.netname, ent->client->resp.team->name);
        //gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex(va("teams/t%i_frt.wav", TEAM_INDEX(ent->client->resp.team))), 1, ATTN_NONE, 0);
        // Dirty

        CTFResetFlag (ent->client->resp.team);
}

// GRIM

void CTFResetAllFlags(void)
{
	edict_t *player;
	int i;

        i = teamgame.flag_items;
        while (i > 0)
        {
                CTFResetFlag(GetTeamByIndex(i));
                i--;
        }

        for (i = 1; i <= maxclients->value; i++)
        {
                player = &g_edicts[i];
                if (!player->inuse)
                        continue;

                if (player->client->pers.has_flag)
                {
                        player->client->pers.inventory[ITEM_INDEX(FlagItem(player->client->pers.has_flag))] = 0;
                        player->client->pers.has_flag = NULL;
                }
        }
}

qboolean CTFPickup_Flag(edict_t *ent, edict_t *other)
{
        teams_t *flags_team;
        weapons_t       *slot; // Dirty

        // Paranoid
        if (turns_on && (level.match_state != MATCH_DURING))
                return false;

        if (!other->client->resp.team)
                return false; // No team

        if (ent->goal_owner && (ent->goal_owner == other->client->resp.team))
                return false; // Been placed...
        // Paranoid

        flags_team = GetTeamByIndex(ent->item->quantity);

        if (flags_team == NULL)
        {
		gi.cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

        if (flags_team == other->client->resp.team)
        {
                // Flags at home base...
                if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
                {
                        // This guys has someones elses flag! They scored!
                        if (other->client->pers.has_flag && (other->client->pers.has_flag != flags_team))
                                CTFFlagCaptured(other, other->client->pers.has_flag);
                        // It's home already, leave it there...
                        // FIX ME - Add more modes...
                        return false;
		}

                // If flags stay where they fall is set, cannot be picked up
                if ((int)bflags->value & BF_FLAGS_STAY)
                        return false;

                // Not "Flags Must Be Carried Back" and we're not home,
                // so auto teleport return type thingy (Reset)
                if (!((int)bflags->value & BF_FLAGS_MBCB))
                {
                        CTF_FlagReturned (other);
                        return false;
                }
        }
        else if (turns_on && (other->client->resp.team->score & TeamGoal(flags_team)))
        {
                if (level.time - other->last_taken_time > 1)
                        gi.cprintf (other, PRINT_HIGH, "Your team has already captured this teams flag!\n");
                return false;
        }
        else if ((int)bflags->value & BF_CTF_TOUCH)
        {       // CTF Apon touch...good for match games.
                CTFFlagCaptured(other, flags_team);
                return false;
        }
        // Dirty
        /*
        else if (other->client->pers.has_flag == NULL)
                gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex(va("teams/t%i_ftk.wav", TEAM_INDEX(flags_team))), 1, ATTN_NONE, 0);
        else // Already carrying a flag...
        */
        else if (other->client->pers.has_flag)
		return false;
        // Dirty

        // Dirty
        slot = other->client->pers.largeweapon;
        // Large weapon slot full
        if (slot->pickup_name)
        {
                if (level.time - other->last_taken_time > 1)
                        gi.cprintf (other, PRINT_HIGH, "Weapon on back, can't carry Flag...\n");
                return false;
        }

        SetSlot (other, slot->slot_name, ent->item->pickup_name, 0, 0, NULL, 0, NULL, 0);
        SetLargeWeaponHUD (other);
        // Dirty

        // Its not our flag
        //      or
        // It is and we need to carry it back, pick it up
        // Paranoid
        // Dirty
        /*
        if (ent->item->quantity == 5)
        {
                if (ent->goal_owner)
                       gi.bprintf(PRINT_HIGH, "%s took The Flag off the %s flagbase!\n", other->client->pers.netname, ent->goal_owner->name);
                else
                       gi.bprintf(PRINT_HIGH, "%s got The Flag!\n", other->client->pers.netname);
        }
        else
                gi.bprintf(PRINT_HIGH, "%s got the %s flag!\n", other->client->pers.netname, flags_team->name);
        */
        // Dirty
        // Paranoid

        // Give bonus if not our own teams flag...
        // Paranoid
        // Dirty
        //if ((flags_team != other->client->resp.team) && (ent->item->quantity != 5))
        //        AdjustScore(other, CTF_FLAG_BONUS, false);
                //other->client->resp.score += CTF_FLAG_BONUS;
        // Dirty
        // Paranoid

        other->client->pers.inventory[ITEM_INDEX(FlagItem(flags_team))] = 1;
        other->client->pers.has_flag = flags_team;
	other->client->resp.ctf_flagsince = level.time;

	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
        if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
        {
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	return true;
}

void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        // GRIM - If in Lava etc, reset automatically.
        // Paranoid
        if (gi.pointcontents(ent->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
                ent->nextthink = level.time + 0.1;
                //CTFDropFlagThink(ent);
        // Paranoid
        else
                Touch_Item (ent, other, plane, surf);
}

void CTFDropFlagThink(edict_t *ent)
{
        teams_t *team;

        // auto return the flag.  reset flag will remove ourselves
        team = GetTeamByIndex(ent->item->quantity);

        CTFResetFlag(team);
        if (level.match_state != MATCH_PRE_START)
                gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n", team->name);
}

// Called from PlayerDie, to drop the flag from a dying player
void RPI_Think (edict_t *ent);

void CTFDeadDropFlag(edict_t *ent)
{
	edict_t *dropped = NULL;

        // Haven't got a fucking flag
        if (ent->client->pers.has_flag == NULL)
                return;

        ent->client->pers.inventory[ITEM_INDEX(FlagItem(ent->client->pers.has_flag))] = 0;

        dropped = Drop_Item(ent, FlagItem(ent->client->pers.has_flag), ITEM_UNDEF);

        // Paranoid
        if (TEAM_INDEX(ent->client->pers.has_flag) == 5)
        {
                gi.bprintf(PRINT_HIGH, "%s lost The Flag!\n", ent->client->pers.netname);
                dropped->think = RPI_Think;
        }
        else
        {
                gi.bprintf(PRINT_HIGH, "%s lost the %s flag!\n", ent->client->pers.netname, ent->client->pers.has_flag->name);
                dropped->think = CTFDropFlagThink;
        }
        // Paranoid
        ent->client->pers.has_flag = NULL;

        // Flag stays where it is, so wait longer (4 minutes)
        if ((int)bflags->value & BF_FLAGS_STAY)
                dropped->nextthink = level.time + (CTF_AUTO_FLAG_RETURN_TIMEOUT * 6);
        else
                dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;

        dropped->touch = CTFDropFlagTouch;

        // Dirty - Goes in Large Weapon slot now
        SetSlot (ent, "Large Weapon Slot", NULL, 0, 0, NULL, 0, NULL, 0);
        // Dirty
}

void CTFDrop_Flag(edict_t *ent, gitem_t *item)
{
	edict_t *dropped = NULL;
        teams_t *team;

        team = GetTeamByIndex(item->quantity);
        gi.bprintf(PRINT_HIGH, "%s dropped the %s flag!\n", ent->client->pers.netname, team->name);

        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        ent->client->pers.has_flag = NULL;
        RemoveItemWeight (ent, item);

        dropped = Drop_Item(ent, item, ITEM_UNDEF);
        dropped->think = CTFDropFlagThink;

        // Flag stays where it is, so wait longer (4 minutes)
        if ((int)bflags->value & BF_FLAGS_STAY)
                dropped->nextthink = level.time + (CTF_AUTO_FLAG_RETURN_TIMEOUT * 6);
        else
                dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;

        dropped->touch = CTFDropFlagTouch;

        ValidateSelectedItem (ent);

        // Dirty - Goes in Large Weapon slot now
        SetSlot (ent, "Large Weapon Slot", NULL, 0, 0, NULL, 0, NULL, 0);
        SetLargeWeaponHUD (ent);
        // Dirty

        /* Good old message :)
	if (rand() & 1)
		gi.cprintf(ent, PRINT_HIGH, "Only lusers drop flags.\n");
	else
		gi.cprintf(ent, PRINT_HIGH, "Winners don't drop flags.\n");
        }
        */
}
// GRIM

static void CTFFlagThink(edict_t *ent)
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	ent->nextthink = level.time + FRAMETIME;
}

// GRIM
void SP_misc_teleporter_dest (edict_t *ent);

void CTFFlagSpotTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (!other->client)
                return;

	if (other->health < 1)
                return; // Corpse CTF...I don't think so.

        // Not carrying a flag or not on a team
        if ((other->client->pers.has_flag == NULL) || (other->client->resp.team == NULL))
                return;

        // Spot is for no team
        if ((TEAM_INDEX(self->goal_owner) > 4) || (TEAM_INDEX(self->goal_owner) == 0))
        {
                gi.cprintf(other, PRINT_HIGH, "Don't know what team the spot is for!\n");
                return;
	}

        // Not on spots team (Pickup_Flag handles that)
        if (other->client->resp.team != self->goal_owner)
                return;

        // Spot and the flag the player is carrying are on same team,
        // then this person just brought their own flag back...
        if (self->goal_owner == other->client->pers.has_flag)
                CTF_FlagReturned (other);
        else if ((int)bflags->value & BF_CTF_ANYTIME)
                CTFFlagCaptured (other, other->client->pers.has_flag);
        // Paranoid
        // Dirty - FIX ME?
        //else if (ctf->value < 2)
        //        SingleFlagPlace (other, self);
        // Paranoid
        // else do nothing...
}

void CTFMakeFlagSpot (edict_t *ent, int team)
{
        edict_t *flagspot;

        flagspot = G_Spawn();
        flagspot->classname = "flag_spot";

        VectorCopy(ent->s.origin, flagspot->s.origin);
        flagspot->s.origin[2] += 8;
        SP_misc_teleporter_dest(flagspot);
        flagspot->s.skinnum = 1;
        flagspot->goal_owner = GetTeamByIndex(team);

        // Dirty - Unlimited lives, unless Capture on Touch
        if (flagspot->goal_owner && ctf->value && match->value
         && (!((int)bflags->value & BF_CTF_TOUCH)))
                flagspot->goal_owner->start_lives = -1;
        // Dirty

        // Dirty - Not in Dirty
        /*
        flagspot->s.effects |= EF_COLOR_SHELL;
        switch (team)
        {
                case 1:
                        flagspot->s.renderfx |= (RF_SHELL_RED);
                        break;
                case 2:
                        flagspot->s.renderfx |= (RF_SHELL_BLUE);
                        break;
                case 3:
                        flagspot->s.renderfx |= (RF_SHELL_GREEN);
                        break;
                case 4:
                        flagspot->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN);
                        break;
                default:
                        flagspot->s.effects &= ~EF_COLOR_SHELL;
                        break;
        }
        */
        // Dirty - Not in Dirty
        flagspot->touch = CTFFlagSpotTouch;

        gi.linkentity (flagspot);
}
// GRIM

void CTFFlagSetup (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

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

	gi.linkentity (ent);

// GRIM
        // Make a laser for it
        // NOTE: ent->item->quantity preset to team index :)
        // Dirty - Not in Dirty
        //CreateBaseLaser (ent, ent->item->quantity);
        // Dirty

        // Make a base for it (Used in CTF Anytime etc)
        CTFMakeFlagSpot (ent, ent->item->quantity);
// GRIM
	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;

        // Paranoid
        // Dirty - FIX ME? Use this?
        /*
        if (ctf->value < 2)
        {
                ent->nextthink = level.time + 3;
                ent->think = G_FreeEdict;
        }
        else
        */
        // Dirty
                teamgame.flag_items++;
        // Paranoid
}

// GRIM
// Paranoid - BraZen copy me
void CTFEffects(edict_t *player)
{
        int     team_index;
        gitem_t *flag_item;

        if (player->health < 1)
                return;

        if (player->client->pers.has_flag)
        {
                team_index = TEAM_INDEX(player->client->pers.has_flag);
                // Dirty - Not in Dirty
                /*
                player->s.effects &= (EF_FLAG1 | EF_FLAG2 | EF_GREENGIB | EF_BLASTER);


                if (team_index == 1)
			player->s.effects |= EF_FLAG1;
                else if (team_index == 2)
                        player->s.effects |= EF_FLAG2;
                else if (team_index == 3)
                        player->s.effects |= EF_GREENGIB; // Whats green?
                else if (team_index == 4)
                        player->s.effects |= EF_GREENGIB;
                else if (team_index == 5)
                        player->s.effects |= EF_BLASTER;
                */
                if (player->s.modelindex3 == 0)
                {
                        flag_item = FlagItem(player->client->pers.has_flag);
                        player->s.modelindex3 = gi.modelindex(flag_item->world_model);
                }
        }
        else if (player->s.modelindex3)
                player->s.modelindex3 = 0;
}
// Paranoid - BraZen copy me

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


