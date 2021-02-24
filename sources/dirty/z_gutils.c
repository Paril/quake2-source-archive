// z_gutils.c -- misc utility functions for goal entities
#include "g_local.h"

/*
=============
OkOnOurTeam - Checks to see if 'player' owns us
=============
*/
qboolean OkOnOurTeam (edict_t *ent, edict_t *player)
{
        if ((!ent->goal_owner) && (!ent->goal_antiowner))
                return true;

        if (!player->client)
                return false;

        if (!player->client->resp.team)
                return false;

        if (ent->goal_antiowner)
        {
                if (player->client->resp.team == ent->goal_antiowner)
                        return false;
        }

        if (ent->goal_owner)
        {
                if (player->client->resp.team != ent->goal_owner)
                        return false;
        }

        return true;
}


/*
==============================
CheckGroupState - Checks if all goals the 'group'
        are in the same state as 'state'.
==============================
*/
qboolean CheckGroupState (char *gname, int state)
{
        edict_t *goal = NULL;
        int     total, matching;

        if (!gname)
                return false;

        total = matching = 0;

        while ((goal = G_Find (goal, FOFS(group), gname)) != NULL)
        {
                total++;
                if (goal->goal_state == state)
                        matching++;
        }

        if (matching >= total)
                return true;

        return false;
}


/*
================
FindEntBy - looks through all entities for the one that matches the classname
        then it checks for both reqs, if defined
================
*/
edict_t *FindEntBy (char *cname, edict_t *owner, gitem_t *item, gitem_t *item2)
{
        edict_t *goal;
        edict_t *final;

        goal = NULL;
        final = NULL;
        while ((goal = G_Find (goal, FOFS(classname), cname)) != NULL)
        {
                if (owner)
                {
                        if ((!goal->owner) || (goal->owner != owner))
                                continue;
                }

                if (item)
                {
                        if ((!goal->item) || (goal->item != item))
                                continue;
                }

                if (item2)
                {
                        if ((!goal->item2) || (goal->item2 != item2))
                                continue;
                }

                final = goal;
        }

        if (!final)
                return NULL;

        return final;
}

/*
================
Set_Forteam_n_Ally - sets up goal_owner, goal_antiowner etc
================
*/
void Set_Forteam_n_Ally (edict_t *ent)
{
        teams_t *team = NULL;
        int     i;

        if (ent->forteam < 0)
        {
                i = (ent->forteam * -1);
                team = GetTeamByIndex (i);
                if (!team)
                {
                        gi.dprintf ("%s - Invalid forteam!\n", ent->classname);
                        G_FreeEdict (ent);
                        return;
                }
                ent->goal_owner = NULL;
                ent->goal_antiowner = team;
        }
        else if (ent->forteam > 0)
        {
                team = GetTeamByIndex (ent->forteam);
                if (!team)
                {
                        gi.dprintf ("%s - Invalid forteam!\n", ent->classname);
                        G_FreeEdict (ent);
                        return;
                }
                ent->goal_owner = team;
                ent->goal_antiowner = NULL;
        }
        else
        {
                ent->goal_owner = NULL;
                ent->goal_antiowner = NULL;
        }

        team = NULL;
        if (ent->ally < 0)
        {
                i = (ent->ally * -1);
                team = GetTeamByIndex (i);
                if (!team)
                {
                        gi.dprintf ("%s - Invalid ally!\n", ent->classname);
                        G_FreeEdict (ent);
                        return;
                }
                ent->goal_ally = NULL;
                ent->goal_antially = team;
        }
        else if (ent->ally > 0)
        {
                team = GetTeamByIndex (ent->ally);
                if (!team)
                {
                        gi.dprintf ("%s - Invalid ally!\n", ent->classname);
                        G_FreeEdict (ent);
                        return;
                }
                ent->goal_ally = team;
                ent->goal_antially = NULL;
        }
        else
        {
                ent->goal_ally = NULL;
                ent->goal_antially = NULL;
        }
}



/*
================
g_droptofloor - just places "ent" on ground, with the generic
        15 unit bbox, makes it solidtrigger and movetype_toss.
================
*/
void g_droptofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

        if ((ent->mins[0] == 0) && (ent->mins[1] == 0) && (ent->mins[2] == 0))
                VectorSet (ent->mins, -15, -15, -15);
        if ((ent->maxs[0] == 0) && (ent->maxs[1] == 0) && (ent->maxs[2] == 0))
                VectorSet (ent->maxs, 15, 15, 15);

	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  

        v = tv (0, 0, -128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
                gi.dprintf ("g_droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	gi.linkentity (ent);
        //gi.dprintf ("g_droptofloor: %s maxs,  %s mins\n", vtos(ent->maxs), vtos(ent->mins));
}

/*
==============================
FireGroup - Fire the group targets...
==============================
*/
void FireGroup (edict_t *ent)
{
        edict_t *t;

        if (!ent->group)
                return;

        if (!ent->activator)
                ent->activator = ent;

        if (ent->group_target)
        {
                t = NULL;
                while ((t = G_Find (t, FOFS(targetname), ent->group_target)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else if (t->use)
                                t->use (t, ent, ent->activator);
                }
        }

        if (ent->group_targetgroup)
        {
                t = NULL;
                while ((t = G_Find (t, FOFS(group), ent->group_targetgroup)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else if (t->use)
                                t->use (t, ent, ent->activator);
                }
        }
}


/*
==============================
FireGoalTargets - Fire our individual targets, set dis/act/deact targets...
==============================
*/
void FireGoalTargets (edict_t *ent)
{
        edict_t *t;
        //gi.dprintf ("FireGoalTargets -");

        if (!ent->activator)
                ent->activator = ent;

        if (ent->dis_target)
        {
                //gi.dprintf (" ent->dis_target,");
                t = NULL;
                while ((t = G_Find (t, FOFS(targetname), ent->dis_target)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else
                                DoGoalStuff (t, GOAL_DISABLED);
                }
        }

        if (ent->dis_targetgroup)
        {
                //gi.dprintf ("ent->dis_targetgroup, ");
                t = NULL;
                while ((t = G_Find (t, FOFS(group), ent->dis_targetgroup)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else
                                DoGoalStuff (t, GOAL_DISABLED);
                }
        }

        if (ent->act_target)
        {
                //gi.dprintf (", ent->act_target");
                t = NULL;
                while ((t = G_Find (t, FOFS(targetname), ent->act_target)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else
                                DoGoalStuff (t, GOAL_ON);
                }
        }

        if (ent->act_targetgroup)
        {
                //gi.dprintf ("ent->act_targetgroup, ");
                t = NULL;
                while ((t = G_Find (t, FOFS(group), ent->act_targetgroup)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else
                                DoGoalStuff (t, GOAL_ON);
                }
        }

        if (ent->deact_target)
        {
                //gi.dprintf ("ent->deact_target, ");
                t = NULL;
                while ((t = G_Find (t, FOFS(targetname), ent->deact_target)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else
                                DoGoalStuff (t, GOAL_OFF);
                }
        }

        if (ent->deact_targetgroup)
        {
                //gi.dprintf ("ent->deact_targetgroup, ");
                t = NULL;
                while ((t = G_Find (t, FOFS(group), ent->deact_targetgroup)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else
                                DoGoalStuff (t, GOAL_OFF);
                }
        }

        if (ent->target)
        {
                //gi.dprintf ("ent->target, ");
                t = NULL;
                while ((t = G_Find (t, FOFS(targetname), ent->target)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else if (t->use)
                                t->use (t, ent, ent->activator);
                }
        }

        if (ent->targetgroup)
        {
                //gi.dprintf ("ent->targetgroup, ");
                t = NULL;
                while ((t = G_Find (t, FOFS(group), ent->targetgroup)) != NULL)
                {
			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
                        else if (t->use)
                                t->use (t, ent, ent->activator);
                }
        }

        //gi.dprintf ("\n");

        if (ent->goal_type & G_ROF)
        {
                ent->goal_state = ent->original_goal_state;
                if (ent->reset)
                        ent->reset (ent);
        }

        if (ent->goal_type & G_DOF)
        {
                ent->goal_state = GOAL_DISABLED;
                if (ent->disable)
                        ent->disable (ent);
        }
}

/*
================
GoalAwardPoints - award points to appropriate team or the activator
================
*/
void GoalAwardPoints (edict_t *ent)
{
        teams_t *team = NULL;
        int     i;

        if (!ent->points)
                return;
                
        if (ent->goal_owner)
                AdjustTeamScore (ent->goal_owner, ent->points, false);
        else if (ent->goal_antiowner)
        {
                for (i = 1; i <= max_teams; i++)
                {
                        team = GetTeamByIndex(i);
                        if (team == NULL)
                        {
                                gi.dprintf ("GoalAwardPoints - No team at index %i.\n", i);
                                break;
                        }
                        if (team != ent->goal_antiowner)
                                AdjustTeamScore (team, ent->points, false);
                }
        }
        else if (ent->activator->client->resp.team)
                AdjustTeamScore (ent->activator->client->resp.team, ent->points, false);
        else
                AdjustScore (ent->activator, ent->points, false);
}



/*
================
GoalCommunicates - Checks for message, sounds etc, and send them to who
        they're supposed to be sent too.
================
*/
void GoalCommunicates (edict_t *ent)
{
        edict_t *activator;
        edict_t *other = NULL;
        int     i;

        activator = ent->activator;
        if (!activator->client)
                return;

        if (ent->noise_index)
                gi.sound (ent, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);

        if (ent->noise_index2) // global noise
                gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_AUTO, ent->noise_index2, 1, ATTN_NONE, 0);

        if (ent->message)
                gi.centerprintf (activator, "%s", ent->message);

        for (i = 1; i <= game.maxclients; i++)
	{
                other = &g_edicts[i];

		if (!other->inuse)
			continue;

		if (!other->client)
			continue;

                if ((other->client->resp.state != CS_PLAYING) || (other->deadflag))
                        continue;

                if (activator && (other == activator))
                        continue;

                if (ent->broadcast)
                        gi.cprintf (other, PRINT_HIGH, "%s\n", ent->broadcast);

                if (ent->team_broadcast && OnSameTeam (activator, other))
                        gi.cprintf (other, PRINT_HIGH, "%s\n", ent->team_broadcast);

                if (ent->nonteam_broadcast && (!OnSameTeam (activator, other)))
                        gi.cprintf (other, PRINT_HIGH, "%s\n", ent->nonteam_broadcast);

                if (ent->netname_broadcast)
                        gi.cprintf (other, PRINT_HIGH, "%s %s\n", activator->client->pers.netname, ent->netname_broadcast);

                if (ent->netname_team_broadcast && (OnSameTeam (activator, other)))
                        gi.cprintf (other, PRINT_HIGH, "%s %s\n", activator->client->pers.netname, ent->netname_team_broadcast);

                if (ent->netname_nonteam_broadcast && (!OnSameTeam (activator, other)))
                        gi.cprintf (other, PRINT_HIGH, "%s %s\n", activator->client->pers.netname, ent->netname_nonteam_broadcast);

                if (ent->ourteam_broadcast && OkOnOurTeam (ent, other))
                        gi.cprintf (other, PRINT_HIGH, "%s\n", ent->ourteam_broadcast);

                if (ent->netname_ourteam_broadcast && (OkOnOurTeam (ent, other)))
                        gi.cprintf (other, PRINT_HIGH, "%s %s\n", activator->client->pers.netname, ent->netname_ourteam_broadcast);
	}
}


/*
================
DoGoalStuff - check our state...
        if we're supposed to fire in that state, fire
        if our group is supposed to fire in that state,
        and all the group is in that state, fire the group
================
*/
void DoGoalStuff (edict_t *ent, int newstate)
{
        int     oldstate;

        if (ent->goal_state == newstate) // do nothing (coz nothings changed?)
                return;

        oldstate = ent->goal_state;
        ent->goal_state = newstate;

        if ((oldstate == GOAL_DISABLED) && (newstate != GOAL_DISABLED))
        {
                if (ent->reset)
                        ent->reset (ent);
        }
        else if (newstate == GOAL_DISABLED)
        {
                if (ent->disable)
                        ent->disable (ent);
        }

        if (ent->goal_type & G_ITEM)
        {
                if ((oldstate == GOAL_ON) && (newstate == GOAL_OFF))
                        ent->reset (ent);
        }

        if (ent->goal_state == ent->fire_when)
        {
                if (ent->points)
                        GoalAwardPoints (ent);
                GoalCommunicates (ent);
                FireGoalTargets (ent);
        }

        if (ent->group && (ent->goal_state == ent->group_fire_when))
        {
                if (CheckGroupState (ent->group, ent->goal_state))
                {
                        FireGroup (ent);
                        if (ent->fire_when == GOAL_GROUP)
                        {
                                if (ent->points)
                                        GoalAwardPoints (ent);
                                GoalCommunicates (ent);
                                FireGoalTargets (ent);
                        }
                }
        }
}


/*
================
goalFailed - Crits failed, so check for a fail_target and fire it.
        for example : if you made it so you needed a certain key card
                to open up a door, but someone who didn't have the key
                card tried to open it. Well, then you could make it
                fire off a warning message
================
*/
void goalFailed (edict_t *ent, edict_t *other)
{
        edict_t *t;

        if (!ent->fail_target)
                return;

        if (!ent->activator)
                ent->activator = ent;

        t = NULL;
        while ((t = G_Find (t, FOFS(targetname), ent->fail_target)) != NULL)
        {
                if (t == ent)
                        gi.dprintf ("WARNING: Entity used itself.\n");
                else if (t->use)
                        t->use (t, ent, ent->activator);
        }
}


/*
================
goalCheckClientHasItem - Looks for item or item2 on the player,
        which ever you specify.
================
*/
qboolean goalCheckClientHasItem (edict_t *ent, edict_t *other, qboolean itemtwo)
{
        edict_t *ohgi = NULL;
        gitem_t *item;

        if (!other->client)
                return false;

        if (itemtwo)
        {
                if (!ent->item2)
                        return true;
                item = ent->item2;
        }
        else if (!ent->item)
                return true;
        else
                item = ent->item;

        if (!other->client->pers.inventory[ITEM_INDEX(item)])
                return false;

        if (itemtwo)
                ohgi = FindEntBy ("item_goal", other, NULL, item);
        else
                ohgi = FindEntBy ("item_goal", other, item, NULL);

        if (ohgi)
        {
                if (!(ohgi->goal_type & G_ITEM_RETAIN))
                {
                        other->client->pers.inventory[ITEM_INDEX(item)]--;
                        RemoveItemWeight (other, item);
                        ValidateSelectedItem (other);
                }

                if (ohgi->goal_type & G_ITEM_ROD)
                        ohgi->reset (ent);
                else
                        DoGoalStuff (ohgi, GOAL_OFF);
        }

        return true;
}


/*
================
goalCheckCrits - find out if all a goals criterias are met.
        If an important crit is failed, it returns CRITS_FAIL.
        If crits can't be check (disabled etc) is failed, it returns CRITS_NA.
        If it all passes, it returns CRITS_PASS.
================
*/
int goalCheckCrits (edict_t *ent, edict_t *other)
{
        if (ent->goal_state == GOAL_DISABLED)
                return CRITS_NA;

        if ((other->health < 1) || (!other->client))
                return CRITS_NA; // non-client or dead 

        if (!AllyCheckOk (ent, other))
                return CRITS_NA; // they're not allow to interact with us

        if (ent->goal_type & G_ITEM)
        {
                if (ent->item2)
                {
                        if (!goalCheckClientHasItem (ent, other, true))
                                return CRITS_FAIL;
                }

                if (ent->goal_type & G_ITEM_IS_BIG) // needs to be on back
                {
                        if (other->client->pers.largeweapon->pickup_name)
                                return CRITS_NA;
                }
        }
        else if (ent->item)
        {
                if (!goalCheckClientHasItem (ent, other, false))
                        return CRITS_FAIL;
        }

        return CRITS_PASS;
}
