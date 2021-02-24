#include "g_local.h"
void RPI_Think (edict_t *ent);

/*
================
CTFBaseSetup
================
*/
void target_laser_on (edict_t *self);
void target_laser_off (edict_t *self);
void target_laser_think (edict_t *self);

extern vec3_t MOVEDIR_DOWN;

/*
================
CreateBase - called after flag has spawned to create pretty lasers
================
*/
void PreBaseLaserThink (edict_t *self) 
{
        //gi.dprintf("create_base: laser on\n");
        target_laser_on (self);
	self->think = target_laser_think;
}

void CreateBaseLaser (edict_t *ent, int team)
{
        edict_t *laser;
        vec3_t  roof;
        trace_t tr;
        int     this_one;
        int     colours[] = {
                        0xf2f2f0f0,    // red
                        0xf3f3f1f1,    // blue
                        0xd0d1d2d3,    // green
                        0xdcdddedf,    // yellow
                        0xe0e1e2e3     // bitty yellow strobe
                        };

        roof[0]=ent->s.origin[0];
        roof[1]=ent->s.origin[1];
        roof[2]=ent->s.origin[2]+8192;  

        tr = gi.trace (ent->s.origin, NULL, NULL, roof, ent, MASK_SOLID);

        // Line complete ? (ie. no collision)
        if (tr.fraction == 1.0)
        {
                gi.dprintf ("CreateBase: Too far from roof.\n");
                return;
        }

        laser = G_Spawn();

        laser->movetype     = MOVETYPE_NONE;
        laser->solid        = SOLID_NOT;
        laser->s.renderfx   = RF_BEAM|RF_TRANSLUCENT;
        laser->s.modelindex = 1;        // must be non-zero
        laser->s.sound      = gi.soundindex ("world/laser.wav");
        laser->classname    = "base_laser";

        // beam diameter
        laser->s.frame      = 2;
        laser->owner        = laser;

        // Choose colour based on team
        this_one = (team - 1);
        if (this_one > 4)
                this_one = 4;
        laser->s.skinnum    = colours[this_one];

        laser->dmg          = 0;
        laser->touch        = NULL;
        laser->think        = PreBaseLaserThink;
        //laser->delay       = level.time + LASER_TIME;

        VectorCopy(tr.endpos, laser->s.origin);
        vectoangles(tr.plane.normal, laser->s.angles);

        // GRIM - MOVEDIR_DOWN is used (done by derek (?)) incase the
        //        roof/floor is angled.
        //G_SetMovedir (self->s.angles, self->movedir);
        VectorCopy (MOVEDIR_DOWN, laser->movedir);

        VectorSet (laser->mins, -8, -8, -8);
        VectorSet (laser->maxs, 8, 8, 8);
        // GRIM - Always wanted a think laser :)

        gi.linkentity (laser);

        // start off...and then come on auto-like afta everythn's init'd
        target_laser_off (laser);
        laser->nextthink = level.time + 2;
}

/*
================
FlagsRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float FlagsRangeFromSpot (edict_t *spot)
{
        edict_t *other_spot;
        char    *c;
        teams_t *team;
        float   bestflagdistance;
        float   flagdistance;
        int     count;
	vec3_t	v;

        bestflagdistance = 9999999;

        other_spot = NULL;
        count = 0;
        while (count < 4)
        {
                count++;
                team = GetTeamByIndex(count);
                c = FlagClassname(team);
                other_spot = G_Find(NULL, FOFS(classname), c);
                if (other_spot)
                {
                        VectorSubtract(spot->s.origin, other_spot->s.origin, v);
                        flagdistance = VectorLength(v);
                        if (flagdistance < bestflagdistance)
                                bestflagdistance = flagdistance;
                }
        }
        return bestflagdistance;
}

edict_t *SelectFarthestFlagSpotSpawnPoint (void)
{
	edict_t	*bestspot;
        float   bestdistance, bestflagdistance;
	edict_t	*spot;

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
                bestflagdistance = FlagsRangeFromSpot(spot);

                if (bestflagdistance > bestdistance)
		{
			bestspot = spot;
                        bestdistance = bestflagdistance;
		}
	}

	if (bestspot)
		return bestspot;

	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

void PutFlagHere (edict_t *spot, int team_index)
{
	edict_t *ent;
        teams_t *team;

        if (!spot)
        {
                gi.dprintf ("No spot specified\n");
                return;
        }
        team = GetTeamByIndex(team_index);

        ent = G_Spawn();

        ent->classname = FlagClassname(team);
        VectorCopy(spot->s.origin, ent->s.origin);
        ent->s.origin[2] += 16;
        SpawnItem(ent, FindItemByClassname(ent->classname));

        G_FreeEdict(spot); // Remove DM point
}

void CTFBaseInit(void)
{
	edict_t *base1 = NULL, *base2 = NULL;
        edict_t *spot;
        int i;

        // Dirty
        if (goals_exist)
                return;
        // Dirty

	base1 = G_Find(NULL, FOFS(classname), "item_flag_team1");
	base2 = G_Find(NULL, FOFS(classname), "item_flag_team2");

        if (base1 && base2)
		return;
	else if (base1)
                spot = base1;
        else if (base2)
                spot = base2;
        else // Find random dm point
        {
                spot = SelectRandomDMSpot();
                PutFlagHere (spot, 1);
	}

        i = teams->value;
        // Currently ownly support 4 teams
        if (i > 4)
                i = 4;

        max_teams = i; // Dirty - Set global variable

        // No need for team 1 coz already done that above...
        while (i > 1)
        {
                spot = SelectFarthestFlagSpotSpawnPoint();
                if (spot == NULL)
                {
                        gi.dprintf ("while(i < 0) No spot found\n");
                        return;
                }
                PutFlagHere (spot, i);
                i--;
	}
}

// Paranoid
void SingleFlagThink (edict_t *ent)
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	ent->nextthink = level.time + FRAMETIME;

        if (ent->goal_owner->score < 0)
                ent->goal_owner->score = 0;

        ent->goal_owner->score++;

        if (ent->goal_owner->score >= 1500)
        {
                gi.bprintf(PRINT_HIGH, "%s captured The Flag!\n", ent->master->client->pers.netname);
                gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("teams/t5_fcp.wav"), 1, ATTN_NONE, 0);

                ent->goal_owner->score = 0;
                if (turns_on && (level.match_state == MATCH_DURING))
                {
                        level.match_state = MATCH_FINISHED;
                        level.match_time = level.time + 3.5;
                        ent->nextthink = 0;
                }
                else
                {
                        AdjustTeamScore(ent->master->client->resp.team, 1, false);
                        ent->think = RPI_Think;
                }
        }
}

void SingleFlagPlace (edict_t *ent, edict_t *spot)
{
        edict_t         *flag;
	trace_t		tr;
	vec3_t		dest;
	float		*v;

        ent->client->pers.has_flag = NULL;

        flag = G_Spawn();

        flag->classname = "item_flag_generic";
        VectorCopy(spot->s.origin, flag->s.origin);
        flag->s.origin[2] += 16;

	v = tv(-15,-15,-15);
        VectorCopy (v, flag->mins);
	v = tv(15,15,15);
        VectorCopy (v, flag->maxs);

        flag->item = FindItem("The Flag");
        gi.setmodel (flag, flag->item->world_model);

        flag->solid = SOLID_TRIGGER;
        flag->movetype = MOVETYPE_TOSS;  
        flag->touch = Touch_Item;

	v = tv(0,0,-128);
        VectorAdd (flag->s.origin, v, dest);

        tr = gi.trace (flag->s.origin, flag->mins, flag->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
                gi.dprintf ("SingleFlagPlace: %s startsolid at %s\n", flag->classname, vtos(flag->s.origin));
                //G_FreeEdict (flag);
                //return;
	}

        VectorCopy (tr.endpos, flag->s.origin);

        flag->nextthink = level.time + FRAMETIME;
        flag->think = SingleFlagThink;
        flag->goal_owner = ent->client->resp.team;
        flag->master = ent;

        flag->s.frame = 173;

        flag->s.effects |= EF_BLASTER;
        gi.linkentity (flag);

        gi.bprintf(PRINT_HIGH, "The Flag has been placed by %s!\n", ent->client->pers.netname);
        gi.sound (flag, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("teams/t5_frt.wav"), 1, ATTN_NONE, 0);
}
// Paranoid
