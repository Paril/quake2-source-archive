#include "g_local.h"

/*
================
generic_goal - most goals run this. It just sets up owners, allies etc...
================
*/
void generic_goal (edict_t *ent)
{
        goals_exist = true;

        Set_Forteam_n_Ally (ent);

        if (!ent->goal_state)
                ent->goal_state = GOAL_OFF;
        ent->original_goal_state = ent->goal_state; // for round resetting

        if (!ent->fire_when)
                ent->fire_when = GOAL_ON;
        if (!ent->group_fire_when)
                ent->group_fire_when = GOAL_ON;

        if (st.noise)
                ent->noise_index = gi.soundindex (st.noise);

        if (st.globalnoise)
                ent->noise_index2 = gi.soundindex (st.globalnoise);

        if (st.item)
                ent->item = FindItemByClassname (st.item);

        ent->flags |= FL_RESPAWN;
	ent->svflags = SVF_NOCLIENT;

        if (!ent->count)
                ent->count = 0; // re-stating the already stated?
        ent->kills_in_a_row = 0;
}

/*
================
trigger_goal_touch - checks to see if the player has the right item etc
================
*/
void trigger_goal_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int     i;

        i = goalCheckCrits (ent, other);
        if (i != CRITS_PASS)
        {
                if (i == CRITS_FAIL)
                        goalFailed (ent, other);
                return;
        }

        ent->activator = other;

        DoGoalStuff (ent, GOAL_ON);
}

/*
======================
trigger_goal_tofloor - "Throw him, to the fwoor!!!"
                          - Julius Caesar (?), Life Of Bryan
======================
*/
void trigger_goal_restart (edict_t *ent);

void trigger_goal_tofloor (edict_t *ent)
{
	if (ent->model)
        {
		gi.modelindex (ent->model);
		gi.setmodel (ent, ent->model);
                if (ent->count > 0)
                        ent->s.skinnum = ent->count;
                g_droptofloor (ent);
        }

        trigger_goal_restart (ent);
}

/*
======================
trigger_goal - touchable, sometimes visible goal
        You shouldn't target trigger_goal's... target target_goal's
        That's what they're their for =P
======================
*/
void reset_trigger_goal (edict_t *ent)
{
        ent->owner = NULL;
        ent->activator = NULL;
        if (ent->model)
        {
                ent->svflags &= ~SVF_NOCLIENT;

                if (ent->goal_type & G_SOLID)
                        ent->solid = SOLID_BBOX;
                else            
                        ent->solid = SOLID_TRIGGER;
        }
        gi.linkentity(ent);
}

void trigger_goal_restart (edict_t *ent)
{
        ent->goal_state = ent->original_goal_state;
        if (ent->model)
        {
                ent->svflags &= ~SVF_NOCLIENT;
                ent->touch = trigger_goal_touch;
                if (ent->goal_state == GOAL_DISABLED)
                {
                        ent->solid = SOLID_NOT;
                        ent->svflags |= SVF_NOCLIENT;
                }
        }
        else
                ent->svflags |= SVF_NOCLIENT;
        gi.linkentity (ent);
}

void SP_trigger_goal (edict_t *ent)
{
        generic_goal (ent);
        ent->goal_type |= G_TOUCH;

        ent->think = trigger_goal_tofloor;
        ent->nextthink = level.time + 0.2;

        ent->use = NULL;
        ent->restart = trigger_goal_restart;
        ent->reset = reset_trigger_goal;
        ent->solid = SOLID_NOT;
        ent->svflags |= SVF_NOCLIENT;
        gi.linkentity (ent);
}


/*
======================
use_target_goal
======================
*/
void use_target_goal (edict_t *ent, edict_t *other, edict_t *activator)
{
        int     i;

        i = goalCheckCrits (ent, activator);
        if (i != CRITS_PASS)
        {
                //gi.dprintf ("use_target_goal - didn't pass crits\n");
                if (i == CRITS_FAIL)
                        goalFailed (ent, activator);
                return;
        }

        //gi.dprintf ("use_target_goal - passed crits\n");

        ent->activator = activator;

        DoGoalStuff (ent, GOAL_ON);
}

/*
======================
target_goal - non-visible goal, that must be targeted to be fired....
======================
*/
void target_goal_restart (edict_t *ent)
{
        ent->kills_in_a_row = 0;
        ent->goal_state = ent->original_goal_state;
}

void SP_target_goal (edict_t *ent)
{
        if ((!ent->targetname) && (!ent->group))
        {
                gi.dprintf ("target_goal - No targetname or group!\n");
                G_FreeEdict(ent);
                return;
        }

        generic_goal (ent);

        ent->use = use_target_goal;
        ent->restart = target_goal_restart;
}



/*
======================
KillGoalCheck - Finds all goals that require the death of a
                team/team member/person, than checks to see if its
                requirements are met...
======================
*/
void KillGoalCheck (edict_t *ent)
{
        teams_t *team = NULL;
        edict_t *goal = NULL;

        if (!ent->client)
                return;

        if (ent->client->resp.team)
                team = ent->client->resp.team;

        while ((goal = G_Find (goal, FOFS(classname), "kill_goal")) != NULL)
	{
                if (goal->goal_state == GOAL_DISABLED)
                        continue;

                if ((goal->goal_antially || goal->goal_ally) && (!team))
                        continue;

                if (goal->item) // must have this item to qualify
                {
                        if (!ent->client->pers.inventory[ITEM_INDEX(goal->item)])
                                continue;
                }

                if (goal->goal_ally)
                {
                        if ((!team) || (goal->goal_ally != team))
                                continue;

                        if (goal->count > 0)
                        {
                                goal->kills_in_a_row++;
                                if (goal->kills_in_a_row >= goal->count)
                                {
                                        goal->activator = ent;
                                        DoGoalStuff (goal, GOAL_ON);
                                }
                        }
                        else if (team->living < 1)
                        {
                                goal->activator = ent;
                                DoGoalStuff (goal, GOAL_ON);
                        }
                }
                else if (goal->goal_antially)
                {
                        if ((!team) || (goal->goal_antially == team))
                                continue;

                        if (level.living == 1) // only our team left
                        {
                                goal->activator = ent;
                                DoGoalStuff (goal, GOAL_ON);
                        }
                        else if ((team->living < 1) && (goal->count > 0))
                        {
                                goal->kills_in_a_row++;
                                if (goal->kills_in_a_row >= goal->count)
                                {
                                        goal->activator = ent;
                                        DoGoalStuff (goal, GOAL_ON);
                                }
                        }
                }
                else if (level.living == 1) // only person left
                {
                        goal->activator = ent;
                        DoGoalStuff (goal, GOAL_ON);
                }
                else if (goal->count > 0)
                {
                        goal->kills_in_a_row++;
                        if (goal->kills_in_a_row >= goal->count)
                        {
                                goal->activator = ent;
                                DoGoalStuff (goal, GOAL_ON);
                        }
                }
	}
}


/*
================
kill_goal - invis n' non-solid goal that will fire once it's
        ally team has been killed or all it's anti-ally teams have
        been killed.  Can't be targeted...
================
*/
void kill_goal_restart (edict_t *ent)
{
        ent->kills_in_a_row = 0;
        ent->goal_state = ent->original_goal_state;
}

void SP_kill_goal (edict_t *ent)
{
        ent->classname = "kill_goal";
        generic_goal (ent);
        ent->goal_type |= G_KILL;

        ent->use = NULL;
        ent->nextthink = 0;
        ent->restart = kill_goal_restart;
}


/*
================
target_endgame - if match->value, resets it on firing.
                if not, the the level ends if it has been fired enough
                as well as reseting the round
================
*/
void use_endgame (edict_t *ent, edict_t *other, edict_t *activator)
{
        if (!MatchInProgress())
                return;

        GoalCommunicates (ent);
        GoalAwardPoints (ent);

        if ((!match->value) && (ent->count > 0))
        {
                ent->kills_in_a_row++;
                if (ent->kills_in_a_row >= ent->count)
                {
                        EndDMLevel ();
                        return;
                }
        }

        level.match_time = level.time + ent->wait;
        level.match_state = MATCH_FINISHED;
}

void endgame_restart (edict_t *ent)
{
        if (match->value)
                ent->kills_in_a_row = 0;
}

void SP_target_endgame (edict_t *ent)
{
        if ((!ent->targetname) && (!ent->group))
        {
                gi.dprintf ("target_endgame - No targetname or group!\n");
                G_FreeEdict(ent);
                return;
        }

        goals_exist = true;

        ent->classname = "target_endgame";
        ent->goal_type |= G_ENDGAME;
        ent->flags |= FL_RESPAWN;
        ent->svflags = SVF_NOCLIENT;

        ent->nextthink = 0;

        ent->restart = endgame_restart;
        ent->use = use_endgame;
        if (!ent->wait)
                ent->wait = 3.5;

        ent->kills_in_a_row = 0;
        if (!ent->count)
                ent->count = 0; // re-stating the already stated?

        // Turns must be on to support target_endgame's of this type
        if (ent->count != 0)
                turns_on = true;

        if (st.noise)
                ent->noise_index = gi.soundindex (st.noise);

        if (st.globalnoise)
                ent->noise_index2 = gi.soundindex (st.globalnoise);

}
