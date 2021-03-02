// g_combat.c

#include "g_local.h"
//ERASER START
#include "bot_procs.h"//ERASER
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
//ERASER END
//WF34 START
extern ctfgame_t ctfgame;

void VectorUnrotate(vec3_t in, vec3_t angles, vec3_t out);
//WF34 END
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

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
//		targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
		if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY))
		{
			level.killed_monsters++;
			if (coop->value && attacker->client)
			{	attacker->client->resp.score++;
			botDebugPrint("scored\n");}
			// medics won't heal monsters that they kill themselves
			if (strcmp(attacker->classname, "monster_medic") == 0)
				targ->owner = attacker;
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
	}
//ERASER START
///Q2 Camera Begin
	PlayerDied(targ);
///Q2 Camera End
//ERASER END
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
			index = ITEM_INDEX(FindItem("Cells"));
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
		damagePerCell = 1; // power armor is weaker in CTF
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
	botDebugPrint("M_reactTo\n");
	if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
		return;

	if (attacker == targ || attacker == targ->enemy)
		return;

	// if we are a good guy monster and our attacker is a player
	// or another good guy, do not get mad at them
	if (targ->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (attacker->client || (attacker->monsterinfo.aiflags & AI_GOOD_GUY))
			return;
	}

	// we now know that we are not both good guys

	// if attacker is a client, get mad at them because he's good and we're not
	if (attacker->client)
	{
		targ->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

		// this can only happen in coop (both new and old enemies are clients)
		// only switch if can't see the current enemy
		if (targ->enemy && targ->enemy->client)
		{
			if (visible(targ, targ->enemy))
			{
				targ->oldenemy = attacker;
				return;
			}
			targ->oldenemy = targ->enemy;
		}
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		{	botDebugPrint("!aiflags &&&&&&");
		FoundTarget (targ);}
		return;
	}

	// it's the same base (walk/swim/fly) type and a different classname and it's not a tank
	// (they spray too much), get mad at them
	if (((targ->flags & (FL_FLY|FL_SWIM)) == (attacker->flags & (FL_FLY|FL_SWIM))) &&
		 (strcmp (targ->classname, attacker->classname) != 0) &&
		 (strcmp(attacker->classname, "monster_tank") != 0) &&
		 (strcmp(attacker->classname, "monster_supertank") != 0) &&
		 (strcmp(attacker->classname, "monster_makron") != 0) &&
		 (strcmp(attacker->classname, "monster_jorg") != 0) )
	{
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}
	// if they *meant* to shoot us, then shoot back
	else if (attacker->enemy == targ)
	{
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}
	// otherwise get mad at whoever they are mad at (help our buddy) unless it is us!
	else if (attacker->enemy && attacker->enemy != targ)
	{
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker->enemy;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}
}

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
//ZOID
	if (ctf->value && targ->client && attacker->client)
		if (targ->client->resp.ctf_team == attacker->client->resp.ctf_team &&
			targ != attacker)
			return true;
//ZOID

		//FIXME make the next line real and uncomment this block
		// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	return false;
}

//WF34 S JR Stuff for artieral blood spray code
void Calc_Offset1(edict_t *self,vec3_t point,vec3_t normal)
{
	vec3_t	forward;
	vec3_t	pos1,pos2;
//	int i;

	VectorSubtract(point, self->s.origin, forward);
	VectorUnrotate(forward, self->s.angles, pos1);

	AngleVectors(normal, forward, NULL, NULL);
	VectorMA(point, 64, forward, forward);
	VectorSubtract(forward, self->s.origin, forward);
	VectorUnrotate(forward, self->s.angles, pos2);
/*///34 used this
	for(i=0;i<3;i++)
	{
		self->client->blood1_pos1[i] = pos1[i];
		self->client->blood1_pos2[i] = pos2[i];
	}
*/
}
void Calc_Offset2(edict_t *self,vec3_t point,vec3_t normal)
{
	vec3_t	forward;
	vec3_t	pos1,pos2;
//	int i;

	VectorSubtract(point, self->s.origin, forward);
	VectorUnrotate(forward, self->s.angles, pos1);

	AngleVectors(normal, forward, NULL, NULL);
	VectorMA(point, 64, forward, forward);
	VectorSubtract(forward, self->s.origin, forward);
	VectorUnrotate(forward, self->s.angles, pos2);
/*//34 used this
	for(i=0;i<3;i++)
	{
		self->client->blood2_pos1[i] = pos1[i];
		self->client->blood2_pos2[i] = pos2[i];
	}
*/
}

void Calc_Offset3(edict_t *self,vec3_t point,vec3_t normal)
{
	vec3_t	forward;
	vec3_t	pos1,pos2;
//	int i;

	VectorSubtract(point, self->s.origin, forward);
	VectorUnrotate(forward, self->s.angles, pos1);

	AngleVectors(normal, forward, NULL, NULL);
	VectorMA(point, 64, forward, forward);
	VectorSubtract(forward, self->s.origin, forward);
	VectorUnrotate(forward, self->s.angles, pos2);
/*//34 used this
	for(i=0;i<3;i++)
	{
		self->client->sblood1_pos1[i] = pos1[i];
		self->client->sblood1_pos2[i] = pos2[i];
	}
*/
}
void Calc_Offset4(edict_t *self,vec3_t point,vec3_t normal)
{
	vec3_t	forward;
	vec3_t	pos1,pos2;
//	int i;

	VectorSubtract(point, self->s.origin, forward);
	VectorUnrotate(forward, self->s.angles, pos1);

	AngleVectors(normal, forward, NULL, NULL);
	VectorMA(point, 64, forward, forward);
	VectorSubtract(forward, self->s.origin, forward);
	VectorUnrotate(forward, self->s.angles, pos2);
/*//34 used this
	for(i=0;i<3;i++)
	{
		self->client->sblood2_pos1[i] = pos1[i];
		self->client->sblood2_pos2[i] = pos2[i];
	}
*/
}
//WF34 E JR end
//Gregg This should fix some problems if it screws other stuff up uncomment this t_damage and recomment the next one
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	gclient_t	*client;
	int			take;
	int			save;
	int			asave;
	int			psave;
	int			te_sparks;
	float		factor;//WF34
	int			delta;//WF34
//	float		i;//WF34
//WF34 START
if (wfdebug)
{
gi.dprintf("T_Damage: dmg = %d\n", damage);
if (targ) gi.dprintf("Target=%s\n ",targ->classname);
if (targ) gi.dprintf("Targets health=%i\n ",targ->health);
if (targ && targ->creator) gi.dprintf("Targets owner=%s \n",targ->creator->classname);
if (targ && targ->creator) gi.dprintf("Target owner health=%i \n",targ->creator->health);
if (inflictor) gi.dprintf("Inflictor=%s \n",inflictor->classname);
if (attacker) gi.dprintf("Attacker=%s \n",attacker->classname);
gi.dprintf(",mod = %d\n", mod);
}

//WF34 END
//WF24 S
	//Don't allow negative damage
	if (damage < 0)
	{
		damage = -damage;
	}
//WF24 E

	if (!targ->takedamage)
		return;
//ERASER START
	if (!attacker)
		return;

	if (level.intermissiontime)
		return;

	//acrid 3/99 attackers with a freeze gun in inventory cant damage
	// frozen targ
	if(targ->frozen && attacker->client &&
			attacker->client->pers.inventory[ITEM_INDEX(FindItem("Freezer"))])
		return;

//ERASER END
	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value))
	{
		//If these are both clients, see if they are on the same team
		if (OnSameTeam (targ, attacker))
		{
			if (((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE))
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
		}
	}

//WF24 S See if the entity allows team damage
	if ((targ->noteamdamage == true) && (attacker->wf_team == targ->wf_team))
	{
		//If this is an created entity (like sentry gun), allow
		//the creator to damage the entity.  Teammates can't.
		if (targ->creator != attacker)
			damage = 0;
	}
//WF24 E
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

//WF24 S  bonus damage for suprising a monster
	if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) && (attacker->client) && (!targ->enemy) && (targ->health > 0))
		damage *= 2;
//WF24 E
//ZOID
//strength tech
	damage = CTFApplyStrength(attacker, damage);
//ZOID

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


//WF24 S
	if (targ->client)	//Make sure it's a client before we check protecttime
	{

		//If client has spawn protect and the means of death is *not* MOD_TELEFRAG, return
		//If the mod type is MOD_TELEFRAG, and the means of death *is* MOD_TELEFRAG,
		//we've gotten here because both the telefragger and telefragge have spawn protection
		//If both have spawn protection, we kill the telefraggee anyway
                if (K2_IsProtected(targ) && mod != MOD_TELEFRAG && mod != MOD_REVERSE_TELEFRAG)
		{
			gi.sound(targ,CHAN_ITEM,gi.soundindex("items/protect3.wav"),1, ATTN_NORM, 0);
			return;
		}

	}
//WF24 E

//ZOID
//team armor protect//ERASER ADDED ATTACKER
	if (ctf->value && targ->client && attacker && attacker->client &&
		targ->client->resp.ctf_team == attacker->client->resp.ctf_team &&
		targ != attacker && ((int)dmflags->value & DF_ARMOR_PROTECT)) {
		psave = asave = 0;
	}
	else if (mod != WEAPON_ARMORDART)	//ignore armor for this one
	{
//ZOID
		psave = CheckPowerArmor (targ, point, normal, take, dflags);
		take -= psave;

		asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
		take -= asave;
	}
	else//WF24 S
	{
		psave = asave = 0;
	}//WF24 E

	//treat cheat/powerup savings the same as armor
	asave += save;


//ZOID
//resistance tech
	take = CTFApplyResistance(targ, take);
//ZOID

	// team damage avoidance
	if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage (targ, attacker) )
		return;


//ZOID
	CTFCheckHurtCarrier(targ, attacker);
//ZOID

//WF24 S

	if (take < 0)
	{
		take = -take;
	}
//WF24 E

// do the damage
	if (take)
	{
		if ((targ->svflags & SVF_MONSTER) || (client))
			SpawnDamage (TE_BLOOD, point, normal, take);
		else
			SpawnDamage (te_sparks, point, normal, take);

		//Artieral Blood spray code JR 9/25/98
/*
		if(targ->client)
		{
			//A large hit somewhere maybe
			if(take>50)
			{
				//Check to seee if the first major blood spot is free
				if(targ->client->blood1_amount=0)
				{
					i=random();
					//Did they hit a major blood vessel
					if(i>0.45)
					{
						//Yes they did!!! =-)
						//Set it up here for the spraying
						VectorCopy(point,targ->client->blood1_point);
						VectorCopy(normal,targ->client->blood1_normal);
						Calc_Offset1(targ,targ->client->blood1_point,targ->client->blood1_normal);

						targ->client->blood1_amount=(take-20-take%5);

					}
				}
				//Check to see if the second major blood spot is free
				//if it isn't there won't be any blood pouring out
				//most likely they are dead anyways unless they had
				//a lot of health.  Also for lag reasons
				else if(targ->client->blood2_amount=0)
				{
					i=random();
					//Chances are a little harder
					if(i>0.3333)
					{
						//Oh my gosh its another hit
						targ->client->blood2_amount=(take-20-take%5);
						VectorCopy(point,targ->client->blood2_point);
						VectorCopy(normal,targ->client->blood2_normal);
						Calc_Offset2(targ,targ->client->blood2_point,targ->client->blood2_normal);
					}
				}
			}
			//For the smaller blood vessels I guess
			//The amount sprayed out is half the amount of a major hit
			//per spray for the equal amount number
			else if(take>25)
			{
				//is the first small blood point being used
				if(targ->client->sblood1_amount=0)
				{
					//It isn't time to see if they are lucky
					i=random();
					if(i>0.15)
					{
						//Yes they are
						targ->client->sblood1_amount=(take-10-take%5);
						VectorCopy(point,targ->client->sblood1_point);
						VectorCopy(normal,targ->client->sblood1_normal);
						Calc_Offset3(targ,targ->client->sblood1_point,targ->client->sblood1_normal);
					}
				}
				//checking the second blood point
				else if(targ->client->sblood2_amount=0)
				{
					//Random numbers blah blah blah...
					i=random();
					if(i>0.1)
					{
						//Imagine if all the blood spots are taken
						//How gruesome
						targ->client->sblood2_amount=(take-10-take%5);
						VectorCopy(point,targ->client->sblood2_point);
						VectorCopy(normal,targ->client->sblood2_normal);
						Calc_Offset4(targ,targ->client->sblood2_point,targ->client->sblood2_normal);
					}
				}
			}
		}
*/
		//end artieral blood spray code JR 9/25/98

//WF34  ++TeT start team balancing code
		if (((int)wfflags->value & WF_AUTO_TEAM_BALANCE)
			&& ((ctfgame.team1 > 2) || (ctfgame.team2 > 2)))
		{
			if (targ->wf_team == CTF_TEAM1)
			{
				delta = (ctfgame.team1 - ctfgame.team2);
			}
			else
			{
				delta = (ctfgame.team2 - ctfgame.team1);
			}
			// never let them take less then 20%
			if (delta < -8)
			{
				delta = -8;
			}
			factor = (10.0 + (float)delta) / 10.0;
			take = (float)take * factor;
		}
//WF34 E --TeT end team balancing code

		targ->health = targ->health - take;

		if (targ->health <= 0)
		{
			if ((targ->svflags & SVF_MONSTER) || (client))
				targ->flags |= FL_NO_KNOCKBACK;
			Killed (targ, inflictor, attacker, take, point);
			return;
		}
	}

	if ((targ->svflags & SVF_MONSTER) && (!targ->bot_client))//ERASER ADDED && TBC
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
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	edict_t	*prev_ent = NULL;//WF34
	vec3_t	v;
	vec3_t	dir;
	int i = 0;//WF34

if (wfdebug)
{
	gi.dprintf("T_RadiusDamage: dmg = %d; inflictor = %s\n", (int)damage, inflictor->classname);
}

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		++i;//WF34
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
if (wfdebug)
{
	gi.dprintf("Calling t_damage.  Damage = %d, Points = %d, vlen = %d\n", (int)damage, (int)points, (int) VectorLength (v));
}
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
//ERASER START, don't drop jump nodes if rocket jumping
				if (ent->jump_ent && ent->client && !ent->bot_client && ent->velocity[2] > 0)
				{
					G_FreeEdict(ent->jump_ent);
					ent->jump_ent = NULL;
				}
//ERASER END
			}
		}
		prev_ent = ent;//WF34
	}
}


/*
===========================================================
T_Radius2Damage - modified to include kickback flags
===========================================================
*/
void T_Radius2Damage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int kickback, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	edict_t	*prev_ent = NULL;//WF34
	vec3_t	v;
	vec3_t	dir;
	int i = 0;//WF34

if (wfdebug)
{
	gi.dprintf("T_RadiusDamage: dmg = %d\n", (int)damage);
}

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		++i;//WF34
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
if (wfdebug) gi.dprintf("Calling t_damage.  Damage = %d, Points = %d, vlen = %d\n", (int)damage, (int)points, (int) VectorLength (v));
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, kickback, DAMAGE_RADIUS, mod);
//ERASER START, don't drop jump nodes if rocket jumping
				if (ent->jump_ent && ent->client && !ent->bot_client && ent->velocity[2] > 0)
				{
					G_FreeEdict(ent->jump_ent);
					ent->jump_ent = NULL;
				}
//ERASER END
			}
		}
		prev_ent = ent;//WF34
	}
}