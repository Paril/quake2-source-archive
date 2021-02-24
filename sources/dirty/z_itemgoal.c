#include "g_local.h"

/*
================
The reset function for item_goal's
================
*/
void reset_goalitem (edict_t *ent)
{
        ent->owner = NULL;
        ent->activator = NULL;
        ent->svflags &= ~SVF_NOCLIENT;
        ent->solid = SOLID_TRIGGER;
        gi.linkentity(ent);

        DoGoalStuff (ent, GOAL_OFF);

        ent->s.event = EV_ITEM_RESPAWN;
}


/*
================
Handling of Dropped goal items...
================
*/
void dropped_goalitem_think (edict_t *ent)
{
        if (ent->owner)
                reset_goalitem (ent->owner);

        G_FreeEdict (ent);
}

void item_goal_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void drop_goalitem (edict_t *ent, gitem_t *item)
{
        edict_t *dropped = NULL;
        edict_t *goal = NULL;

        if (!ent->client)
                return;

        goal = FindEntBy ("item_goal", ent, item, NULL);

        dropped = Drop_Item (ent, item, ITEM_UNDEF);

        if (goal)
        {
                gi.dprintf ("drop_goalitem - goal found\n");
                goal->owner = NULL;
                goal->nextthink = 0;
                ent->s.modelindex3 = 0;
                if (goal->goal_type & G_ITEM_IS_BIG)
                {
                        SetSlot (ent, "Large Weapon Slot", NULL, 0, 0, NULL, 0, NULL, 0);
                        SetLargeWeaponHUD (ent);
                }
                dropped->owner = goal;
                dropped->think = dropped_goalitem_think;
                dropped->nextthink = level.time + goal->wait;
        }
        else
        {
                gi.dprintf ("drop_goalitem - goal NOT found\n");
                dropped->owner = NULL;
                dropped->think = G_FreeEdict;
                dropped->nextthink = level.time + 0.1; //GOAL_DROPPED_TIMEOUT;
        }

        dropped->touch = item_goal_touch;
        ent->client->pers.inventory[ITEM_INDEX(item)]--;

        RemoveItemWeight (ent, item);
        ValidateSelectedItem (ent);
}

/*
================
Handling of picked up item_goal's
================
*/
void goalitem_think (edict_t *ent)
{
        if (ent->owner && (ent->owner->health < 1))
        {
                if (ent->goal_type & G_ITEM_DWD)
                        drop_goalitem (ent->owner, ent->item);
                else
                        ent->owner->client->pers.inventory[ITEM_INDEX(ent->item)]--;
                        
                ent->owner = NULL;
                ent->think = NULL;
                ent->nextthink = 0;
                gi.linkentity (ent);
                return;
        }
        
        ent->nextthink = level.time + FRAMETIME;
}

/*
================
item_goal touch function
================
*/
void item_goal_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        edict_t         *ent;
        int             i;

        if (!other->client)
                return;

        if (other->client->pers.grabbing < 1)
                return;

        if (self->goal_type & DROPPED_PLAYER_ITEM)
        {
                if (!self->owner)
                {
                        G_FreeEdict (self);
                        return;
                }

                if (gi.pointcontents (self->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
                {
                        self->nextthink = level.time + 0.1;
                        return;
                }

                ent = self->owner;
        }
        else
                ent = self;

        i = goalCheckCrits (ent, other);
        if (i != CRITS_PASS)
        {
                if (i == CRITS_FAIL)
                        goalFailed (ent, other);
                return;
        }

        /*
        else if (ent->goal_type & ITEM_INHAND)
        {
                slot = other->client->pers.goodhand;
                if (slot->pickup_name)
                {
                        slot = other->client->pers.offhand;
                        if (slot->pickup_name)
                                return; // no free hands
                }
        }
        */

        if (ent->goal_type & G_ITEM_IS_BIG) // needs to be on back
        {
                SetSlot (other, "Large Weapon Slot", ent->item->pickup_name, 0, 0, NULL, 0, NULL, 0);
                SetLargeWeaponHUD (other);
        }

        other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

        ent->activator = other;
        DoGoalStuff (ent, GOAL_ON);

        ent->owner = other;
        ent->think = goalitem_think;
        ent->nextthink = level.time + FRAMETIME;

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;

        if (ent->vwepmodel)
        {
                if (ent->blindTime == -1) // Not proper Vwep...
                        other->s.modelindex3 = gi.modelindex(ent->vwepmodel);
                else if (other->model)
                        other->s.modelindex3 = gi.modelindex(va("%s/%s", other->model, ent->vwepmodel));
        }

	gi.linkentity (ent);

        if (ent != self)
                G_FreeEdict (self);
}


/*
================
item_goal set up
================
*/
void item_goal_restart (edict_t *ent);

void place_item_goal (edict_t *ent)
{
        g_droptofloor (ent);

        PrecacheItem (ent->item);

        ent->s.effects = ent->item->world_model_flags;

        if (!((int)realflags->value & RF_NO_GLOWING))
                ent->s.renderfx = RF_GLOW;

        ent->touch = item_goal_touch;

	if (ent->model)
        {
		gi.modelindex (ent->model);
		gi.setmodel (ent, ent->model);
        }
        else
		gi.setmodel (ent, ent->item->world_model);

        item_goal_restart (ent);
}

void item_goal_restart (edict_t *ent)
{
        ent->goal_state = ent->original_goal_state;
        ent->svflags &= ~SVF_NOCLIENT;
        if (ent->goal_state != GOAL_OFF)
        {
                ent->solid = SOLID_NOT;
                ent->svflags |= SVF_NOCLIENT;
        }
        gi.linkentity (ent);
}

gitem_t *FlagItem(teams_t *team);
void SP_item_goal (edict_t *ent)
{
        generic_goal (ent);

        ent->classname = "item_goal";
        ent->goal_type |= G_ITEM;

        ent->think = place_item_goal;
        ent->nextthink = level.time + 0.2;
        ent->reset = reset_goalitem;
        ent->restart = item_goal_restart;

        ent->item = NULL;
        if (st.item)
                ent->item = FindItemByClassname (st.item);

        ent->item2 = NULL;
        if (st.item2)
                ent->item2 = FindItemByClassname (st.item2);

        if (!ent->item)
        {
                if (ent->goal_owner)
                        ent->item = FlagItem (ent->goal_owner);
                else if (ent->goal_antiowner)
                        ent->item = FlagItem (ent->goal_antiowner);
        }

        if (!ent->item)
                ent->item = FindItemByClassname ("item_flag_generic");

        ent->item->drop = drop_goalitem;

        if (!ent->wait)
                ent->wait = GOAL_DROPPED_TIMEOUT;

        /*
        if (ent->vwepmodel)
        {
                if ((strstr (s, "\") || (strstr (s, "/"))
                {
                        gi.modelindex (ent->vwepmodel);
                        ent->blindTime = -1; // Not proper Vwep...
                        // Tis like flags (one size fits all)
                }
        }
        */
}
