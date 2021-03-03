// g_combat.c

#include "g_local.h"

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (edict_t *targ, edict_t *inflictor)
{
	vec3_t	dest;
	trace_t	trace;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
	{
		VectorAdd (targ->absmin, targ->absmax, dest);
		VectorScale (dest, 0.5, dest);
		trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
		if (trace.ent == targ)
			return true;
		return false;
	}
	
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;


	return false;
}


/*
============
Killed
============
*/
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (targ->health < -999)
		targ->health = -999;

	targ->enemy = attacker;

	if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	{	// doors, triggers, etc
		if (targ->die)
			targ->die (targ, inflictor, attacker, damage, point);
		return;
	}

	if (targ->die)
		targ->die (targ, inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage)
{
	if (damage > 255)
		damage = 255;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
//	gi.WriteByte (damage);
	gi.WritePosition (origin);
	gi.WriteDir (normal);
	gi.multicast (origin, MULTICAST_PVS);
}


/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack
point		point at which the damage is being inflicted
normal		normal vector from that point
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

dflags		these flags are used to control how T_Damage works
	DAMAGE_IMPACT			damage is from conventional bullets
	DAMAGE_PIERCE			damage of this type does 1/2 knockback, and is used for armour piercing rounds (special armour is required)
	DAMAGE_BLAST			damage was indirect (from a nearby explosion), ignores hit-loc
	DAMAGE_ENERGY			damage is from an energy based weapon

	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
	DAMAGE_NO_HITLOC		do not apply hit-location checking
============
*/

float CalcArmourSave(float damage, float protect, float armor) {
float save;

	save = (damage * protect);
	save += (damage * protect) * ( 0.0000111 * (armor * armor) );
	if (save > damage) return damage;
	return save;
}

static int CheckArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags) {
gclient_t	*client;
int			loc, save = 0;
item_t		*item, *nitem;
supply_t	*armour;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	loc = GetHitLocation(point, ent);
	nitem = ent->inventory;
	while (nitem && (damage > 0)) {
		// Cheap hack, because this item may be removed later in this function
		item = nitem;
		nitem = nitem->next;

		if ((item->itemtype != ITEM_SUPPLY) || (ITEMSUPPLY(item)->type != SUP_ARMOUR))
			continue;
		armour = ITEMSUPPLY(item);

		if (!(dflags & DAMAGE_BLAST))
			if (!(armour->flags & loc))
				continue;

		// convert resistance values if required
		if (armour->impact_prot > 1.0)
			armour->impact_prot /= 100.0;
		if (armour->pierce_prot > 1.0)
			armour->pierce_prot /= 100.0;
		if (armour->blast_prot > 1.0)
			armour->blast_prot /= 100.0;
		if (armour->energy_prot > 1.0)
			armour->energy_prot /= 100.0;

		if (dflags & DAMAGE_IMPACT)
			save += CalcArmourSave(damage, armour->impact_prot, item->quantity);
		if (dflags & DAMAGE_PIERCE)
			save += CalcArmourSave(damage, armour->pierce_prot, item->quantity);
		if (dflags & DAMAGE_BLAST)
			save += CalcArmourSave(damage, armour->blast_prot, item->quantity);
		if (dflags & DAMAGE_ENERGY)
			save += CalcArmourSave(damage, armour->energy_prot, item->quantity);

		if (save > damage) save = damage;
		if (save > item->quantity) save = item->quantity;

		RemoveInventoryItem(ent, item, save);
		damage -= save;

		if (save) {
			switch(armour->frame) {
			case ARMOUR_SHELL:	ent->client->armour_effect |= EF_COLOR_SHELL; ent->client->armour_fx = armour->skinnum; break;
			case ARMOUR_SHIELD:	ent->client->armour_effect |= EF_POWERSCREEN; break;
			case ARMOUR_FADE:	ent->client->armour_fx |= RF_TRANSLUCENT; break;
			default:	SpawnDamage (te_sparks, point, normal, save); break;
			}
		}
	}


	return save;
}


qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
team_t *a_team, *t_team;
// DEADLODE
	if (dlsys.playmode & PM_TEAMDAMAGE)
		return false;

	if (attacker->client) a_team = attacker->client->resp.team;
	else a_team = attacker->team;

	if (targ->client) t_team = targ->client->resp.team;
	else t_team = targ->team;

	if (t_team && (t_team == a_team) && ((targ != attacker) && (targ != attacker->owner) && (targ != attacker->master)))
		return true;
// DEADLODE

	//FIXME make the next line real and uncomment this block
	// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	return false;
}

void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod, char *sndset)
{
	gclient_t	*client;
	int			take, save, asave;
	int			te_sparks, loc;

	if (!targ->takedamage)
		return;

	if (damage <= 0) {
		if (targ->client) {
			targ->health = targ->health - damage;
			targ->client->bonus_alpha = 0.25;	
			if (targ->health > targ->max_health)
				targ->health = targ->max_health;
		}
		return;
	}

	// Half-damage playmode
	if (dlsys.playmode & PM_HALFDAMAGE)
		damage *= 0.5;

	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if ((targ != attacker) && (targ->health > 0)) {
		if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(targ, attacker)) 
			damage = 0;
		else
			mod |= MOD_FRIENDLY_FIRE;
	}
	meansOfDeath = mod;
	VectorCopy (point, hitpos);

	client = targ->client;
	// Location based damage
	if (!(dflags & DAMAGE_NO_HITLOC) && client) {
		loc = GetHitLocation(point, targ);
		switch (loc)
		{
		case LOC_HEAD:
			damage *= 2;
			break;
		case LOC_LEGS:
			damage *= 0.5;
			break;
		case LOC_CHEST:
		default:
			break;
		}
	}

	if (dflags & (DAMAGE_IMPACT|DAMAGE_PIERCE))
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	VectorNormalize(dir);

	// DAMAGE_PIERCE does 1/2 knockback
	if (dflags & DAMAGE_PIERCE)
		knockback *= 0.5;

	// automatic 1/2 damage and 1/4 knockback if player is ducked due to reduced exposure area
	if (targ->client && (dflags & DAMAGE_BLAST) && (targ->client->ps.pmove.pm_flags & PMF_DUCKED)) {
		damage *= 0.5;
		knockback *= 0.25;
	}

	if (targ->flags & FL_NO_KNOCKBACK)
		knockback = 0;

	// figure momentum add from knockback
	if (!(dflags & DAMAGE_NO_KNOCKBACK)) {
		if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) &&
			(targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP)) {
			vec3_t	kvel;
			float	mass;

			if (targ->mass < 50)
				mass = 50;
			else
				mass = targ->mass;

			VectorScale (dir, 500.0 * (float)knockback / mass, kvel);
			// team-knockback clips at 100
			if (CheckTeamDamage(targ, attacker)) {
				if (VectorLength(kvel) > 250) {
					VectorNormalize(kvel);
					VectorScale(kvel, 250.0, kvel);
				}
			}

			targ->pusher = attacker;
			targ->pushframe = 3; // targ must die in .3 seconds for attacker to get credit
			VectorAdd (targ->velocity, kvel, targ->velocity);
		}
	}		

	take = damage;
	save = 0;

	// check for godmode
	if ((targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION)) {
		take = 0;
		save = damage;
		SpawnDamage (te_sparks, point, normal, save);
	}

	// Apply client states
	if (attacker->client && attacker->client->dmg_time > level.time)
		damage *= attacker->client->dmg_factor;

	if ((client && client->prot_time > level.time) && !(dflags & DAMAGE_NO_PROTECTION)) {
		targ->s.effects |= EF_COLOR_SHELL;
		targ->s.renderfx |= RF_SHELL_HALF_DAM;
		save = damage * client->prot_factor;
		take -= save;
	}

/*
	// check for invincibility
	if ((client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION)) {
		if (targ->pain_debounce_time < level.time)
		{
			gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}
		take = 0;
		save = damage;
	}
*/
	asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
	take -= asave;


	//treat cheat/powerup savings the same as armor
	asave += save;

	// team damage avoidance
	if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage (targ, attacker))
		return;

//ZOID
//	CTFCheckHurtCarrier(targ, attacker);
//ZOID

// do the damage
	if (take)
	{
		// DEADLODE
		if (attacker->team == targ->team)
			CheckDisguise(attacker);
		// DEADLODE

		if (client)
			SpawnDamage (TE_BLOOD, point, normal, take);
		else
			SpawnDamage (te_sparks, point, normal, take);


		targ->health = targ->health - take;
			
		if ((targ->health <= 0))
		{
			if (client)
				targ->flags |= FL_NO_KNOCKBACK;
			Killed (targ, inflictor, attacker, take, point);
			return;
		}
	}

	if (client)	{
		if (sndset && (level.time > targ->pain_debounce_time) && (targ->health - take > 0)) {
		char wav[MAX_QPATH];
			// play custom pain sound effects
			sub_string(sndset, wav, random() * sub_count(sndset));
			if (!strstr(wav, ".wav"))
				strcat(wav, ".wav");
			gi.sound (targ, CHAN_VOICE, gi.soundindex(wav), 1, ATTN_NORM, 0);

			targ->pain_debounce_time = level.time + 0.6;
		}

		if (!(targ->flags & FL_GODMODE) && (take))
			targ->pain (targ, attacker, knockback, take);
	} else if (take) {
		if (targ->pain)
			targ->pain (targ, attacker, knockback, take);
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if (client) {
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy (point, client->damage_from);
	}
}


/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		points = damage - 0.5 * VectorLength (v);
		if (ent == attacker)
			points = points * 0.5;
		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_BLAST, mod, NULL);
			}
		}
	}
}
