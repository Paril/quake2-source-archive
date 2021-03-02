#include "g_local.h"

void Cure_Disease(edict_t *self)
{
	if (self->owner)
	{
		self->owner->disease = 0;
		self->owner->s.effects &= ~EF_FLIES;
	}
	G_FreeEdict(self);
}

void Disease_Think (edict_t *self)
{
	edict_t *ent;
	vec3_t dir;
	trace_t tr;
	int damage;
	float	points;
	vec3_t	v;
	float rnum;
	rnum = random();

	if (!self->owner)
	{
		Cure_Disease(self);
		return;
	}

	//stop disease after the person dies - GREGG
	if(self->owner->health < 1)
	{
		Cure_Disease(self);
		return;
	}

	//If disease flag is clear, then they are cured
	if(!self->owner->disease)
	{
		Cure_Disease(self);
		return;
	}

	//Move the entity to where the target is
	VectorAdd (self->target_ent->mins, self->target_ent->maxs, v);
	VectorMA (self->target_ent->s.origin, 0.5, v, v);
	VectorSubtract (self->s.origin, v, v);
	VectorSubtract (self->owner->s.origin, self->s.origin, dir);
	VectorCopy(self->owner->s.origin,self->s.origin);

	//Random amount of damage
	damage = rndnum (4,11);
	points = damage - 0.5 * (VectorLength (v));
	if (points > wf_game.grenade_damage[GRENADE_TYPE_PLAGUE] ) points = wf_game.grenade_damage[GRENADE_TYPE_PLAGUE] ;

	T_Damage (self->owner, self, self->target_ent, dir, self->owner->s.origin,vec3_origin, damage, (int)points, DAMAGE_NO_KNOCKBACK, MOD_DISEASE);

	//Throw up 10% of the time //JR thought I should raise this chance then
	if(rnum < 0.10)
	{
		ThrowUpNow(self->owner);
		T_Damage (self->owner, self, self->target_ent, dir, self->owner->s.origin,vec3_origin, damage, (int)points, DAMAGE_NO_KNOCKBACK,MOD_DISEASE);
	}

	self->nextthink = level.time + 1.5;
	ent = NULL;

	//Infect others who are close
	while ((ent = findradius(ent, self->s.origin, 128)) && ent != NULL)
	{
		if (!ent->client)		//only infect other players
			continue;

		if (self->owner == ent)	//don't infect self
			continue;

		//Don't go through walls
		if (!visible(self, ent))
             continue;

		if (!ent->takedamage)
			continue;

		if (!CanDamage (ent, self))
			continue;

		if (!CanDamage (ent, self->target_ent))
			continue;

		tr = gi.trace (self->s.origin, NULL, NULL, ent->s.origin, self, MASK_SOLID);
		if (tr.fraction != 1.0)
			continue;

		infect_person(ent,self->target_ent);
	}
}

void infect_person(edict_t *target, edict_t *owner)
{
	edict_t	*plague;

	//Valid entities?
	if (!target) return;
	if (!owner) return;

	//Ignore if either entity went away
	if (!target->inuse) return;
	if (!owner->inuse) return;

	//Only infect players
	if (!target->client)
		return;
	if (!owner->client)
		return;

	// ignore if on same team
	if (target->wf_team == owner->wf_team)
	{
		return;
	}

	//Don't disease again
	if (target->disease)
	{
		return;
	}
	
	//Can't infect a nurse
	//Shouldn't test the class number.  Instead, don't infect them 
	//if the can create a biosentry or healing depot
	if ((target->client->player_special & SPECIAL_HEALING) ||
	    (target->client->player_special & SPECIAL_BIOSENTRY) )
		return;

    safe_cprintf(target, PRINT_HIGH, "You've been infected by %s.\n",	owner->client->pers.netname);
	safe_cprintf(owner, PRINT_HIGH, "You have infected %s!\n",target->client->pers.netname);

	target->disease++;
	target->diseased_by = owner;
	target->s.effects |= EF_FLIES;

	plague = G_Spawn();
	plague->movetype = MOVETYPE_NOCLIP;
	plague->clipmask = MASK_SHOT;
	plague->solid = SOLID_NOT;
	VectorClear (plague->mins);
	VectorClear (plague->maxs);
	plague->owner = target;
	plague->target_ent = owner;
	plague->s.modelindex = gi.modelindex ("sprites/s_bubble.sp2");
	plague->nextthink = level.time + 1.5;
	plague->think = Disease_Think;
	plague->s.effects |= EF_GIB;
	plague->s.sound = gi.soundindex ("infantry/inflies1.wav");
	plague->classname = "Disease";
	VectorCopy(target->s.origin,plague->s.origin);
	gi.linkentity (plague);
}
