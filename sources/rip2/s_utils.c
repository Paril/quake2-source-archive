#include "g_local.h"

/*
==========================
s_utils.c

Misc Utility Functions
==========================
*/

void RIP_T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	gclient_t	*client;
	int			take;
	int			save;
	int			asave;
	int			psave;
	int			te_sparks;
    float       z_rel; 
    int         height; 

	if (!targ->takedamage)
		return;

    if (teamplay->value == 1)
	{
		if (PlayingSameTeam (targ, attacker) || targ == attacker)
			damage = 0;
	}
    else if (teamplay->value == 2)
	{
      if (targ != attacker)
	  {
		if (PlayingSameTeam (targ, attacker))
			damage = 0;
	  }
	}
    else if (teamplay->value == 3)
	{
		if (PlayingSameTeam (targ, attacker) || targ == attacker)
		{
			damage *= 0.5;
			mod |= MOD_FRIENDLY_FIRE;
		}
	}
    else if (teamplay->value == 4)
	{
      if (targ == attacker)
	    	damage = 0;
	  else
	  {
		  if (PlayingSameTeam (targ, attacker))
			  mod |= MOD_FRIENDLY_FIRE;
	  }
	}

	height = abs(targ->mins[2]) + targ->maxs[2]; 

	if ((targ->client) && (targ != attacker))
	{
		if ((mod == MOD_BLASTER) || (mod == MOD_SHOTGUN) || (mod == MOD_SSHOTGUN) || (mod == MOD_MACHINEGUN) || (mod == MOD_CHAINGUN) || (mod == MOD_HYPERBLASTER) || (mod == MOD_RAILGUN) || (mod == MOD_RAILBOMB) || (mod == MOD_PLASMA))
		{
			z_rel = point[2] - targ->s.origin[2]; 
			if (z_rel < LEG_DAMAGE)
			{
				//leg damage 
				damage *= 0.75;
				targ->ripstate |= STATE_LEGSHOT;
				Rip_SetSpeed (targ);
			}
			else if (z_rel < STOMACH_DAMAGE)
			{
				//stomach damage
				damage *= 0.9; 
			}
			else if (z_rel < CHEST_DAMAGE)
			{
				//chest damage 
				damage *= 1.1; 
			}
			else
			{
				//head damage 
				damage *= 100; 
			} 
		}      
	}  


    // friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if ((targ != attacker) && ((((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))))
	{
		if (OnSameTeam (targ, attacker))
		{
			if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
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

	// check for invincibility
	if ((client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		if (targ->pain_debounce_time < level.time)
		{
			gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}
		take = 0;
		save = damage;
	}

	psave = CheckPowerArmor (targ, point, normal, take, dflags);
	take -= psave;

	asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
	take -= asave;

	//treat cheat/powerup savings the same as armor
	asave += save;

    CTFCheckHurtCarrier(targ, attacker);

// do the damage
	if (take)
	{
		if ((targ->svflags & SVF_MONSTER) || (client))
		{
			SpawnDamage (TE_BLOOD, point, normal, take);
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
void RIP_T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
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
				RIP_T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
			}
		}
	}
}

