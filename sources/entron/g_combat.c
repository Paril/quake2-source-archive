// g_combat.c

#include "g_local.h"
void kill_script(edict_t *self);
void SpawnBlood(vec3_t point, float damage, vec3_t dir);
void SpawnPool(vec3_t point);

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/

int script_pain(edict_t *ent, edict_t *other);

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
   int i = 0;
   edict_t *user0 = g_edicts;

   // Tell all enemies that we are dead.
	for (; user0 < &g_edicts[globals.num_edicts]; user0++)
	   {
      if (!user0->inuse || user0->health <= 0)
         continue;
		if (user0->client)
			if (user0->client->locked == targ)
				{
				targ->s.renderfx &= ~RF_SHELL_RED;
				user0->client->locked = NULL;
				}
		if (user0->enemy == targ)
         {
         if (user0->oldenemy != targ)
            user0->enemy = user0->oldenemy;
         else
            user0->enemy = NULL;            
			user0->oldenemy = NULL;
         if (!user0->enemy)
            if (user0->monsterinfo.stand)
               user0->monsterinfo.stand(user0);
         }
		else if (user0->oldenemy == targ)
         user0->oldenemy = NULL;
		if (user0->monsterinfo.chaser == targ)
         {
			//if (!(user0->type & TYPE_SLAVE_ACTOR))
			user0->status &= ~STATUS_MONSTER_HIDE;
         user0->monsterinfo.chaser = NULL;
         if (user0->monsterinfo.stand)
            user0->monsterinfo.stand(user0);
         }
	   }

	gi.linkentity(targ);
	if (targ->health < -999)
		targ->health = -999;

   if (targ->client)
	   targ->enemy = attacker;
   else
      targ->enemy = NULL;

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	   {
		targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
		kill_script(targ);
		targ->s.sound = 0;
		if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY))
	   	{
			level.killed_monsters++;
			if (coop->value && attacker->client)
				attacker->client->resp.score++;
		   }
	   }

	if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	   {	// doors, triggers, etc
		targ->die (targ, inflictor, attacker, damage, point);
		return;
	   }

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	   {
		targ->touch = NULL;
		monster_death_use (targ);
		if (meansOfDeath == MOD_RIFLE_HEAD)
			if (targ->monsterinfo.decap)
				{
				if (rand() & 1)
					gi.sound (targ, CHAN_BODY, gi.soundindex("organic/decap1.wav"), 1, ATTN_NORM, 0);
				else if (level.framenum & 2)
					gi.sound (targ, CHAN_BODY, gi.soundindex("organic/decap2.wav"), 1, ATTN_NORM, 0);
				targ->monsterinfo.decap (targ, damage);
				return;
				}
	   }

	targ->die (targ, inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage)
{
	//if (damage > 255)
		//damage = 255;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
	//gi.WriteByte (damage);
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
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_ENERGY			damage is from an energy based weapon
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_BULLET			damage is from a bullet (used for ricochets)
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/
static int CheckPowerArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			power_armor_type;
	int			index;
	int			damagePerCell;
	int			pa_te_type;
	int			power;
	int			power_used;

	if (!damage)
		return 0;

	client = ent->client;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	if (client)
	{
		power_armor_type = PowerArmorType (ent);
		if (power_armor_type != POWER_ARMOR_NONE)
		{
			index = ITEM_INDEX(FindItem("Plasma Slugs"));
			power = client->pers.inventory[index];
		}
	}
	else if (ent->svflags & SVF_MONSTER)
	{
		power_armor_type = ent->monsterinfo.power_armor_type;
		power = ent->monsterinfo.power_armor_power;
	}
	else
		return 0;

	if (power_armor_type == POWER_ARMOR_NONE)
		return 0;
	if (!power)
		return 0;

	if (power_armor_type == POWER_ARMOR_SCREEN)
	{
		vec3_t		vec;
		float		dot;
		vec3_t		forward;

		// only works if damage point is in front
		AngleVectors (ent->s.angles, forward, NULL, NULL);
		VectorSubtract (point, ent->s.origin, vec);
		VectorNormalize (vec);
		dot = DotProduct (vec, forward);
		if (dot <= 0.3)
			return 0;

		damagePerCell = 1;
		pa_te_type = TE_SCREEN_SPARKS;
		damage = damage / 3;
	}
	else
	{
		damagePerCell = 2;
		pa_te_type = TE_SHIELD_SPARKS;
		damage = (2 * damage) / 3;
	}

	save = power * damagePerCell;
	if (!save)
		return 0;
	if (save > damage)
		save = damage;

	SpawnDamage (pa_te_type, point, normal, save);
	ent->powerarmor_time = level.time + 0.2;

	power_used = save / damagePerCell;

	if (client)
		client->pers.inventory[index] -= power_used;
	else
		ent->monsterinfo.power_armor_power -= power_used;
	return save;
}

static int CheckArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
	gclient_t	*client;
	int			save;
	int			index;
	gitem_t		*armor;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	index = ArmorIndex (ent);
	if (!index)
		return 0;

	armor = GetItemByIndex (index);

	if (dflags & DAMAGE_ENERGY)
		save = ceil(((gitem_armor_t *)armor->info)->energy_protection*damage);
	else
		save = ceil(((gitem_armor_t *)armor->info)->normal_protection*damage);
	if (save >= client->pers.inventory[index])
		save = client->pers.inventory[index];

	if (!save)
		return 0;

	client->pers.inventory[index] -= save;
	SpawnDamage (te_sparks, point, normal, save);

	return save;
}

void M_ReactToDamage (edict_t *targ, edict_t *attacker)
{

   if (targ->type & TYPE_MONSTER_TEAMMATE)
      return;
   if (attacker && attacker != targ && attacker->takedamage)
      {
      if (targ->enemy != attacker)
         {
         if (targ->oldenemy != targ->enemy)
            targ->oldenemy = targ->enemy;
         targ->enemy = attacker;
         }
		if (targ->scr.on_pain < 0)
			kill_script(targ);
      }
}

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
		//FIXME make the next line real and uncomment this block
		// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	return false;
}

void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t inflictdir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	gclient_t	*client;
	int			take;
	int			save;
	int			asave;
	int			psave;
	int			te_sparks;
	vec3_t		dir;				
	VectorCopy(inflictdir, dir);	

	if (!targ->takedamage)
      return;

	if (attacker && attacker->client)
		if (attacker->client->beserk > level.framenum)
			damage *= 3;
	if (targ->client)
		{
		if (targ->client->invincible > level.framenum)
			return;
		if (targ->client->beserk > level.framenum)
			damage /= 3;
		}

	if (!deathmatch->value && attacker != targ)
		if (attacker && targ->team_data == attacker->team_data && targ->team_data)
			return;
	
   // friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value))
	   {
		if (OnSameTeam (targ, attacker))
		   {
			if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
		   }
	   }

   
   if ((targ->svflags & SVF_MONSTER || targ->client) 
      && !(targ->type & TYPE_MONSTER_FLAT))
   {
   take = (targ->maxs[2] - targ->mins[2]) / 4;
   if (mod == MOD_RIFLE)
      {
      if (targ->s.origin[2] + 3 * take < point[2])
         {
         //gi.bprintf(PRINT_CHAT, "Head Shot\n");
			if (targ->type & TYPE_MONSTER_ORGANIC)
				{
				if (rand() & 1)
					gi.sound (targ, CHAN_BODY, gi.soundindex("organic/himpact1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (targ, CHAN_BODY, gi.soundindex("organic/himpact2.wav"), 1, ATTN_NORM, 0);
				}	
			else if (targ->type & TYPE_MONSTER_METALIC)
				{
				if (rand() & 1)
					gi.sound (targ, CHAN_BODY, gi.soundindex("metallic/himpact1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (targ, CHAN_BODY, gi.soundindex("metallic/himpact2.wav"), 1, ATTN_NORM, 0);
				}	
         damage *= 10;
         mod = MOD_RIFLE_HEAD;
         }
      else if (targ->s.origin[2] > point[2])
         {
         //gi.bprintf(PRINT_CHAT, "Leg Shot\n");
         damage /= 2;
         mod = MOD_RIFLE_LEG;
         }
      else if (targ->s.origin[2] + take > point[2])
         {
         //gi.bprintf(PRINT_CHAT, "Groin Shot\n");
         mod = MOD_RIFLE_GROIN;
         }
      else
         {
         //gi.bprintf(PRINT_CHAT, "Chest Shot\n");
         damage *= 1.5;
         mod = MOD_RIFLE_TORSO;
         }
      }
   }

	meansOfDeath = mod;

	// easy mode takes half damage
	if (skill->value == 0 && deathmatch->value == 0 && targ->client)
	   {
		damage *= 0.5;
		if (!damage)
			damage = 1;
	   }

	client = targ->client;

	if (dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	VectorNormalize(dir);

// bonus damage for suprising a monster
	if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) && (attacker->client) && (!targ->enemy) && (targ->health > 0))
		damage *= 2;

	if (targ->flags & FL_NO_KNOCKBACK)
		knockback = 0;

	// figure momentum add
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	   {
		if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP))
		   {
			vec3_t	kvel;
			float	mass;

			if (targ->mass < 50)
				mass = 50;
			else
				mass = targ->mass;

			if (targ->client  && attacker == targ)
				VectorScale (dir, 1600.0 * (float)knockback / mass, kvel);	// the rocket jump hack...
			else
				VectorScale (dir, 500.0 * (float)knockback / mass, kvel);

			VectorAdd (targ->velocity, kvel, targ->velocity);
		   }
	   }

	take = damage;
	save = 0;

	// check for godmode
	if ( (targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION) )
	   {
		take = 0;
		save = damage;
		SpawnDamage (te_sparks, point, normal, save);
	   }


	psave = CheckPowerArmor (targ, point, normal, take, dflags);
	take -= psave;

	asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
	take -= asave;

	//treat cheat/powerup savings the same as armor
	asave += save;

	// team damage avoidance
	if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage (targ, attacker))
		return;

// do the damage
	if (take)
	   {
		if (targ->type & TYPE_MONSTER_ORGANIC || client)
			{
			if (level.framenum & 1)
				gi.sound (targ, CHAN_BODY, gi.soundindex("organic/bimpact1.wav"), 1, ATTN_NORM, 0);
			else if (level.framenum & 2)
				gi.sound (targ, CHAN_BODY, gi.soundindex("organic/bimpact2.wav"), 1, ATTN_NORM, 0);
			else 
				gi.sound (targ, CHAN_BODY, gi.soundindex("organic/bimpact3.wav"), 1, ATTN_NORM, 0);
			//SpawnBlood(point, take, dir);
			}	
		else if (targ->type & TYPE_MONSTER_METALIC)
			{
			if (level.framenum & 1)
				gi.sound (targ, CHAN_BODY, gi.soundindex("metallic/bimpact1.wav"), 1, ATTN_NORM, 0);
			else if (level.framenum & 2)
				gi.sound (targ, CHAN_BODY, gi.soundindex("metallic/bimpact2.wav"), 1, ATTN_NORM, 0);
			else 
				gi.sound (targ, CHAN_BODY, gi.soundindex("metallic/bimpact3.wav"), 1, ATTN_NORM, 0);
			}

		if (client)
			SpawnDamage (TE_BLOOD, point, normal, take);
		else if (targ->svflags & SVF_MONSTER)
         {
         if (targ->type & TYPE_MONSTER_ORGANIC)
            SpawnDamage (TE_BLOOD, point, normal, take);
         else
            SpawnDamage (te_sparks, point, normal, take);
         }
      else
			SpawnDamage (te_sparks, point, normal, take);

		targ->health = targ->health - take;
			
		if (targ->health <= 0)
		   {
			if ((targ->svflags & SVF_MONSTER) || (client))
				targ->flags |= FL_NO_KNOCKBACK;
			Killed (targ, inflictor, attacker, take, point);
			return;
		   }
	   }

	if (targ->svflags & SVF_MONSTER)
	   {
      if (targ->scr.commands && targ->scr.on_pain >= 0)
         if (targ->scr.pain_time < level.time)
            {
            if (script_pain(targ, attacker))
               return;
            }
		M_ReactToDamage (targ, attacker);
		if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take))
		   {
			targ->pain (targ, attacker, knockback, take);
			// nightmare mode monsters don't go into pain frames often
			if (skill->value == 3)
				targ->pain_debounce_time = level.time + 5;
		   }
	   }
	else if (client)
	   {
		if (!(targ->flags & FL_GODMODE) && (take))
			targ->pain (targ, attacker, knockback, take);
	   }
	else if (take)
	   {
		if (targ->pain)
			targ->pain (targ, attacker, knockback, take);
	   }

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if (client)
	   {
		client->damage_parmor += psave;
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
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
			}
		}
	}
}
