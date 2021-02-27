#include "g_local.h"

void fire_think (edict_t *self)
{
	edict_t *ed=NULL;
	qboolean quench = false;
	int i;
	int damage;

	// fire self-terminates if
	if (!G_EntIsAlive(self->enemy)
		|| !G_EntIsAlive(self->owner)//owner dies
		|| (level.time > self->delay))	// enemy dies								//duration expires
		//|| que_findtype(self->enemy->curses, NULL, HEALING) != NULL)	//3.0 when player is blessed with healing
	{
		que_removeent(self->enemy->curses, self, true);
		return;
	}

	//3.0 quench the flames if the player is in possesion of a flame stopping item
	if (!(self->enemy->waterlevel || self->waterlevel))
	{
		for (i = 3; i < MAX_VRXITEMS; ++i)
		{
			if (self->enemy->myskills.items[i].itemtype & ITEM_FIRE_RESIST)
			{
				quench = true;
				break;
			}
		}
	}

	if (self->enemy->waterlevel || self->waterlevel || quench)
	{
		//if item and not water stopped the fire
		if (quench)
		{
			//Consume an item charge
			if (!(self->enemy->myskills.items[i].itemtype & ITEM_UNIQUE))
				self->enemy->myskills.items[i].quantity -= 1;
			if(self->enemy->myskills.items[i].quantity == 0)
			{
				int count = 0;
				safe_cprintf(self->enemy, PRINT_HIGH, "Your burn resistant clothing has been destroyed!\n");
				//erase the item
				V_ItemClear(&self->enemy->myskills.items[i]);
				//Tell the user if they have any left
				for (i = 3; i < MAX_VRXITEMS; ++i)
					if (self->enemy->myskills.items[i].itemtype & ITEM_FIRE_RESIST)
						count++;
				if (count) safe_cprintf(self->enemy, PRINT_HIGH, "You have %d left.\n", count);
			}
		}
		//water did it, so play a hissing dound
		else gi.sound (self->enemy, CHAN_WEAPON, gi.soundindex ("world/airhiss1.wav"), 1, ATTN_NORM, 0);

		que_removeent(self->enemy->curses, self, true);
		return;
	}

	damage = self->dmg;	

	VectorCopy(self->enemy->s.origin,self->s.origin);
	if (self->PlasmaDelay < level.time)
	{
		T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, 
			vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK, MOD_BURN);
		self->PlasmaDelay = level.time + 1;
	}
	self->nextthink = level.time + FRAMETIME;
}

void burn_person (edict_t *target, edict_t *owner, int damage)
{
	edict_t	*flame;
	que_t *slot=NULL;

	if (level.time < pregame_time->value)
		return;
	if (que_typeexists(target->curses, CURSE_FROZEN))
		return; // attacker is frozen!
	if (!G_ValidTarget(owner, target, false))
		return;
	while ((slot = que_findtype(target->curses, slot, CURSE_BURN)) != NULL)
	{
		if (slot->time-level.time >= 9)
			return; // only allow 1 burn per second
	}

	flame = G_Spawn();
	flame->movetype = MOVETYPE_NOCLIP;
	flame->solid = SOLID_NOT;
	VectorClear(flame->mins);
	VectorClear(flame->maxs);
	flame->owner = owner;
	flame->enemy = target;
	flame->mtype = CURSE_BURN;
	flame->delay = level.time + 5;
	flame->nextthink = level.time + FRAMETIME;
	flame->PlasmaDelay = level.time + FRAMETIME;
	flame->think = fire_think;
	flame->classname = "fire";
	flame->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
	flame->dmg = damage;
	VectorCopy(target->s.origin, flame->s.origin);
	gi.linkentity (flame);

	if (!que_addent(target->curses, flame, 10))
	{
		G_FreeEdict(flame);
		return;
	}

	gi.sound (target, CHAN_ITEM, gi.soundindex ("misc/needlite.wav"), 1, ATTN_NORM, 0);
}