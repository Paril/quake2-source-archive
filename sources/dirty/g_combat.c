// g_combat.c

#include "g_local.h"

// GRIM - For monster testing...
void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
// GRIM - For monster testing...

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

        if ((targ->svflags & SVF_MONSTER) && (!targ->deadflag))
	{
//		targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
		if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY))
		{
                        // GRIM
                        if (!targ->deadflag)
                                level.killed_monsters++;
                        // GRIM
			if (coop->value && attacker->client)
				attacker->client->resp.score++;
			// medics won't heal monsters that they kill themselves
                        /* GRIM - I don't care.
			if (strcmp(attacker->classname, "monster_medic") == 0)
				targ->owner = attacker;
                        */ 
		}

                // GRIM - Monster obituary...tempish, but fun!
                /*
                if (!targ->deadflag)
                        ClientObituary (targ, inflictor, attacker);
                */
                // GRIM
	}

	if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	{	// doors, triggers, etc
                targ->enemy = attacker; // GRIM - For some reason, must be specified again...
		targ->die (targ, inflictor, attacker, damage, point);
		return;
	}

        if ((targ->svflags & SVF_MONSTER) && (!targ->deadflag))
	{
		targ->touch = NULL;
                //GRIM
                // MONSTER CODE
                //monster_death_use (targ);
                //GRIM
	}

        // GRIM
        if ((attacker->client) && ((targ->svflags & SVF_MONSTER) || (targ->client)) && (!targ->deadflag))
                KilledMakeEvent (attacker, targ);
        // GRIM

        targ->die (targ, inflictor, attacker, (int)damage, point);
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

        // GRIM
        if (dflags & TOD_NO_ARMOR)
		return 0;
        // GRIM

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

// GRIM
static int CheckArmor (edict_t *ent, vec3_t point, vec3_t normal, float damage, int te_sparks, int dflags)
{
	gclient_t	*client;
        gitem_t         *armor;
        gitem_armor_t   *armor_info;
        int             armor_index;
        float           save, i, acheck, n;

        if ((!damage) || (ent->deadflag))
		return 0;

	client = ent->client;

	if (!client)
		return 0;

        if (!client->pers.armor)
		return 0;

        if (dflags & TOD_NO_ARMOR)
		return 0;

        armor_index = client->pers.armor_index;

        save = 0;

        armor = client->pers.armor;
        armor_info = (gitem_armor_t *)armor->info;

        if ((HitLocation & HIT_HEAD) && (!(armor_info->areas & HIT_HEAD)))
        {
                if ((HitLocation & HIT_UPPER_HEAD) && (armor_info->areas & HIT_UPPER_HEAD))
                        save = damage;
                else
                        return 0;
        }
        else if ((HitLocation & HIT_RIGHT_LEG) && (!(armor_info->areas & HIT_RIGHT_LEG)))
        {
                if ((HitLocation & HIT_UPPER_LEGS) && (armor_info->areas & HIT_UPPER_LEGS))
                        save = damage;
                else
                        return 0;
        }
        else if ((HitLocation & HIT_LEFT_LEG) && (!(armor_info->areas & HIT_LEFT_LEG)))
        {
                if ((HitLocation & HIT_UPPER_LEGS) && (armor_info->areas & HIT_UPPER_LEGS))
                        save = damage;
                else
                        return 0;
        }
        else if ((HitLocation & HIT_RIGHT_ARM) && (!(armor_info->areas & HIT_RIGHT_ARM)))
        {
                if ((HitLocation & HIT_UPPER_ARMS) && (armor_info->areas & HIT_UPPER_ARMS))
                        save = damage;
                else
                        return 0;
        }
        else if ((HitLocation & HIT_LEFT_ARM) && (!(armor_info->areas & HIT_LEFT_ARM)))
        {
                if ((HitLocation & HIT_UPPER_ARMS) && (armor_info->areas & HIT_UPPER_ARMS))
                        save = damage;
                else
                        return 0;
        }
        else
                save = damage;

        if (save > 0)
        {
                i = 1 + (damage / 10);

                // GRIM - 12/01/99
                // AP round go through armor like cheese....
                if (dflags & TOD_AP)
                        acheck = ((float)armor_index * 0.25);
                // HV rounds go through armor easier too....
                else if (dflags & TOD_HV)
                        acheck = ((float)armor_index * 0.5);
                else
                        acheck = (float)armor_index;
                                 
                // If greater then how we treat the armor vs damage type
                if (save > acheck)
                {       // Now against how we treat it vs normal

                        n = (save - (float)armor_index);
                        HitLocation |= HIT_SLOWED; // Dirty
                        //if (n > 0)
                        //        i += n;
                        save = acheck;
                }
                else
                {
                        HitLocation |= HIT_SOAKED;
                        // Bruise damage...
                        save -= (i * 0.5);
                }

                if (armor_index > armor_info->min_count)
                        client->pers.armor_index -= i;

                // Save modifier vs explosive force
                if (dflags & TOD_RADIUS)
                        save *= armor_info->explosive_protection;
        }

        if (client->pers.armor_index < 1)
        {
                // Dirty
                if (!max_teams)
                        client->pers.armor_index = armor_info->min_count;
                else
                {
                        RemoveItemWeight (ent, armor);
                        client->pers.inventory[ITEM_INDEX(armor)]--;
                        client->pers.armor = NULL;
                        client->pers.armor_index = 0;
                        client->ps.stats[STAT_ARMOR_ICON] = 0;
                }
                // Dirty
        }
        else
                SetArmorHUD(ent);

        if (save < 1)
		return 0;

        SpawnDamage (te_sparks, point, normal, save);

	return save;
}
// GRIM

void M_ReactToDamage (edict_t *targ, edict_t *attacker)
{
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
                //GRIM
                // MONSTER CODE
                //if (!(targ->monsterinfo.aiflags & AI_DUCKED))
                //        FoundTarget (targ);

                //GRIM
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
                //GRIM
                // MONSTER CODE
                //if (!(targ->monsterinfo.aiflags & AI_DUCKED))
                //        FoundTarget (targ);

                //GRIM
	}
	// if they *meant* to shoot us, then shoot back
	else if (attacker->enemy == targ)
	{
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
                //GRIM
                // MONSTER CODE
                //if (!(targ->monsterinfo.aiflags & AI_DUCKED))
                //        FoundTarget (targ);

                //GRIM
	}
	// otherwise get mad at whoever they are mad at (help our buddy) unless it is us!
	else if (attacker->enemy && attacker->enemy != targ)
	{
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker->enemy;
                //GRIM
                // MONSTER CODE
                //if (!(targ->monsterinfo.aiflags & AI_DUCKED))
                //        FoundTarget (targ);

                //GRIM
	}
}

qboolean AllyCheck (edict_t *targ, edict_t *attacker)
{
        if ((!max_teams) && (!coop->value))
                return false;

        if ((!targ->client) || (!attacker->client))
                return false;

        if ((targ->client->resp.team == NULL) || (attacker->client->resp.team == NULL))
                return false;

        if (!attacker->client->resp.team->ally)
                return false;

        if (targ->client->resp.team != attacker->client->resp.team->ally)
                return false;

        return true;
}


// GRIM
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
        if (!max_teams)
                return false;

        if (targ->client && attacker->client && targ != attacker)
        {
                // Dirty
                if (coop->value)
                {
                        meansOfDeath |= MOD_FRIENDLY_FIRE;
                        return true;
                }

                if ((!OnSameTeam (targ, attacker)) && (!AllyCheck (targ, attacker)))
                        return false;

                if (targ->is_a_lame_mofo) // No penalty for killing a lame mofo
                        return false;

                meansOfDeath |= MOD_FRIENDLY_FIRE;

                if (!((int)dmflags->value & DF_NO_FRIENDLY_FIRE) || ((int)teamkill->value > 0))
                        return false;

                return true;
                // Dirty
        }
	return false;
}

// Dirty - New
void ImpactSound (edict_t *targ)
{
        int     i;

        if (TypeOfDamage & TOD_PELLET)
                return;

        if (HitLocation & HIT_THROAT)
        {
                gi.sound (targ, CHAN_AUTO, gi.soundindex("misc/hit_throat.wav"), 0.6, ATTN_NORM, 0);
                return;
        }

        if (!(TypeOfDamage & TOD_LEAD))
                return;

        if (HitLocation & HIT_HEAD)
        {
                gi.sound (targ, CHAN_AUTO, gi.soundindex(va("misc/bhit_head%i.wav", (rand()%3)+1)), 1, ATTN_NORM, 0);
                return;
        }

        i = (rand()%5)+1;

        if (i > 3) // don't always play an impact sound??
                return;

        if (targ->deadflag)
                gi.sound (targ, CHAN_AUTO, gi.soundindex(va("misc/bhit%i.wav", i)), 1, ATTN_NORM, 0);
        else if (HitLocation & HIT_SOAKED)
                gi.sound (targ, CHAN_AUTO, gi.soundindex(va("misc/bullet_soak%i.wav", i)), 1, ATTN_NORM, 0);
        else if (HitLocation & (HIT_CHEST|HIT_MID))
                gi.sound (targ, CHAN_AUTO, gi.soundindex(va("misc/bhit_chest%i.wav", i)), 1, ATTN_NORM, 0);
        else
                gi.sound (targ, CHAN_AUTO, gi.soundindex(va("misc/bhit%i.wav", i)), 1, ATTN_NORM, 0);
}
// Dirty - New

qboolean AddWound (edict_t *targ, float original, int dflags, int mod)
{
        int     i, max;
        float   j;

        i = 0;
        max = 0;

        if (!targ->client)
                return false;

        // Armor soaked it...
        if (HitLocation & HIT_SOAKED)
        {
                ImpactSound (targ);
                return false;
        }

        if (dflags & TOD_NO_WOUND)
                return false;

        //gi.dprintf ("addwound\n");

        //targ->client->heal_time += 1;

        // Stomach wounds = extra messy :)
        if (HitLocation & HIT_MID)
                original *= 1.25;

        if (original > 10)
        {
                j = (original / 10) + 0.5;
                i = (j + 1);
        }
        else if (original > 1)
                i = 1;

        if (mod == MOD_FALLING)
                i -= 2;

        if (i < 1)
                return false;

        // Can't heal if you have just been wounded!
        if (i > 1)
                targ->client->pers.heal_rate = 0;

        if (HitLocation & HIT_HEAD)
        {
                targ->client->pers.wounds[W_HEAD] += i;

                // Head loss death hack... so? =P
                if (((targ->health - (original * 4)) < 1) && (targ->client->pers.wounds[W_HEAD] < 7))
                        targ->client->pers.wounds[W_HEAD] = 7;
                        
                if (targ->client->concus_time < level.time)
                        targ->client->concus_time = level.time + ((rand()%5) * 0.5) + 0.5;
                else
                        targ->client->concus_time += (rand()%3) + 1;
        }

        if (HitLocation & (HIT_CHEST|HIT_MID))
        {
                targ->client->pers.wounds[W_CHEST] += i;
                //gi.dprintf ("chest wound\n");
        }

        if (HitLocation & HIT_RIGHT_LEG)
                targ->client->pers.wounds[W_RLEG] += i;

        if (HitLocation & HIT_LEFT_LEG)
                targ->client->pers.wounds[W_LLEG] += i;

        if (HitLocation & HIT_LEFT_ARM)
        {
                // Dirty - holding on? not any more
                if (targ->client->hanging)
                        targ->client->hanging = false;
                // Dirty

                targ->client->pers.wounds[W_LARM] += i;
                // GRIM
                if (((i >= 6) && (HitLocation & HIT_UPPER_ARMS))
                 || ((i >= 4) && !(HitLocation & HIT_UPPER_ARMS)))
                {
                        if (random() < 0.75)
                        {
                                targ->client->weaponstate = WEAPON_READY;
                                if (targ->client->pers.hand == LEFT_HANDED)
                                {
                                        if ((targ->client->pers.weapon_handling) && (targ->client->pers.weapon2))
                                                SwitchGuns(targ);
                                        else if (targ->client->pers.weapon)
                                                Drop_CurrentWeapon (targ, 1);
                                }
                                else if ((targ->client->pers.weapon_handling) && (targ->client->pers.weapon2))
                                        Drop_CurrentWeapon (targ, 2);
                        }
                }
                // GRIM
        }

        if (HitLocation & HIT_RIGHT_ARM)
        {
                // Dirty - holding on? not any more
                if (targ->client->hanging)
                        targ->client->hanging = false;
                // Dirty

                targ->client->pers.wounds[W_RARM] += i;

                // GRIM
                if (((i >= 6) && (HitLocation & HIT_UPPER_ARMS))
                 || ((i >= 4) && !(HitLocation & HIT_UPPER_ARMS)))
                {
                        if (random() < 0.75)
                        {
                                targ->client->weaponstate = WEAPON_READY;
                                if (targ->client->pers.hand != LEFT_HANDED)
                                {
                                        if ((targ->client->pers.weapon_handling) && (targ->client->pers.weapon2))
                                                SwitchGuns(targ);
                                        else if (targ->client->pers.weapon)
                                                Drop_CurrentWeapon (targ, 1);
                                }
                                else if ((targ->client->pers.weapon_handling) && (targ->client->pers.weapon2))
                                        Drop_CurrentWeapon (targ, 2);
                        }
                }
                // GRIM
        }

        targ->client->wound_flash_time = level.time + 2;
        SetWoundsHud (targ);
        ImpactSound (targ);

        return true;
}

int GetHitLocation (edict_t *targ, vec3_t dir, vec3_t point, int dflags, int mod)
{
        vec3_t  forward, right, up, org, vec;
        float   dot_f, dot_r, dot_u;
        int     HL = 0;

        if ((targ->health <= 0) || (targ->deadflag == DEAD_DEAD))
                return 0;
        else if (mod == MOD_LAVA || mod == MOD_SLIME)
        {
                if (targ->waterlevel > 0)
                        HL |= (HIT_RIGHT_LEG|HIT_LEFT_LEG|HIT_UPPER_LEGS);
                if (targ->waterlevel > 1)
                        HL |= (HIT_CHEST|HIT_MID|HIT_UPPER_ARMS|HIT_LEFT_ARM|HIT_LEFT_LEG);
                if (targ->waterlevel > 2)
                        HL |= HIT_HEAD;
                return HL;
        }

        AngleVectors (targ->s.angles, forward, right, up);
        forward[2] = 0;
        right[2] = 0;

        // Dirty - origin if ducking
        if (targ->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
                VectorCopy (targ->s.origin, org);
                org[2] -= 9; //12;
                VectorSubtract (point, org, vec);
        }
        else
                VectorSubtract (point, targ->s.origin, vec);
        // Dirty

        VectorNormalize (vec);
        dot_f = DotProduct (vec, forward);
        dot_r = DotProduct(vec, right);
        dot_u = DotProduct(vec, up);

        //gi.dprintf ("dot_f = %f, dot_r = %f, dot_u = %f \n", dot_f, dot_r, dot_u);

        // Added throat. RaVeN request :)
        if (dot_u > 0.66)
        {
                HL |= HIT_HEAD;
                if (dot_u < 0.70)
                {
                        HL |= HIT_THROAT;
                        //gi.dprintf ("HIT_THROAT\n");
                }
                else if (dot_u > 0.86)
                {
                        HL |= HIT_UPPER_HEAD; // For helmet
                        //gi.dprintf ("HIT_UPPER_HEAD\n");
                }
                //else
                //        gi.dprintf ("HIT_HEAD\n");
        }
        else if (dot_u >= 0)
        {
                if (dot_r > 0.3)
                {
                        HL |= HIT_RIGHT_ARM;
                        if (dot_u > 0.45)
                        {
                                HL |= HIT_UPPER_ARMS;
                                //gi.dprintf ("HIT_UPPER_ARMS HIT_RIGHT_ARM\n");
                        }
                        //else
                                //gi.dprintf ("HIT_RIGHT_ARM\n");
                }
                else if (dot_r < -0.3)
                {
                        HL |= HIT_LEFT_ARM;
                        if (dot_u > 0.45)
                        {
                                HL |= HIT_UPPER_ARMS;
                                //gi.dprintf ("HIT_UPPER_ARMS HIT_LEFT_ARM\n");
                        }
                        //else
                                //gi.dprintf ("HIT_LEFT_ARM\n");
                }
                else
                {
                        if (dot_u > 0.32)
                        {
                                HL |= HIT_CHEST;
                                //gi.dprintf ("HIT_CHEST\n");
                        }
                        else
                        {
                                HL |= HIT_MID;
                                //gi.dprintf ("HIT_MID\n");
                        }
                }
        }
        else
        {
                if (dot_u > -0.5)
                {
                        HL |= HIT_UPPER_LEGS;
                        //gi.dprintf ("HIT_UPPER ");
                }

                if (dot_r < 0)
                {
                        HL |= HIT_LEFT_LEG;
                        //gi.dprintf ("HIT_LEFT_LEG\n");
                }
                else if (dot_r > 0)
                {
                        HL |= HIT_RIGHT_LEG;
                        //gi.dprintf ("HIT_RIGHT_LEG\n");
                }
        }

        if (dot_f < -0.3)
                HL |= HIT_BACK;
        else if (dot_f > 0.3)
                HL |= HIT_FRONT;

        return HL;
}

void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int loc, int type, int mod)
{
        gclient_t       *client;
        int             save, asave, psave, te_sparks;
        float           take, orig_take;
        qboolean        same_team = false;
        qboolean        addwound = false;
        edict_t         *temp;
                
	if (!targ->takedamage)
		return;

	client = targ->client;


        // Paranoid
        if (attacker->master && attacker->master->client)
        {
                temp = attacker->master;
                attacker = temp;
        }
        // Paranoid

        //gi.dprintf ("T_Damage - damage %f\n", damage);
        // Assign hit location;
        if (loc == 0)
        {
                if (targ->client)
                        HitLocation = GetHitLocation (targ, dir, point, type, mod);
                else
                        HitLocation = HIT_CHEST;
        }
        else
                HitLocation = loc;

        // Assign meansOfDeath for Death Messages...
	meansOfDeath = mod;

        // Assign type of damage...
        TypeOfDamage = type;

        if ((type & TOD_LEAD) && !(type & TOD_EXPLOSIVE))
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
        if (CheckTeamDamage(targ, attacker))
                same_team = true;

        VectorNormalize (dir);

        //gi.dprintf ("T_Damage - base %f, ", damage);

        // bonus damage for suprising a monster
        if (!(type & TOD_RADIUS) && (targ->svflags & SVF_MONSTER)
         && attacker->client && (!targ->enemy) && (targ->health > 0))
		damage *= 2;

        if (targ->flags & FL_NO_KNOCKBACK)
		knockback = 0;

        // Figure momentum add
        if (knockback && !(type & TOD_NO_KNOCKBACK))
	{
                if (knockback && (targ->movetype != MOVETYPE_NONE)
                 && (targ->movetype != MOVETYPE_BOUNCE)
                  && (targ->movetype != MOVETYPE_PUSH)
                   && (targ->movetype != MOVETYPE_STOP))
		{
			vec3_t	kvel;
			float	mass;

			if (targ->mass < 50)
				mass = 50;
			else
				mass = targ->mass;

                        VectorScale (dir, 750.0 * knockback / mass, kvel);

                        if (kvel[2] < 5)
                                kvel[2] = 5;
                        //gi.dprintf ("kvel[2] %f, ", kvel[2]);

                        VectorAdd (targ->velocity, kvel, targ->velocity);
		}
	}

        // Dirty
        if (client && (level.match_state == MATCH_START))
                return; // no damage until match begins
        // Dirty

        take = (float)damage;
	save = 0;

        // Check for godmode. Still damage,
        // just no take from health. See below
        if ((targ->flags & FL_GODMODE) && !(type & TOD_NO_PROTECTION))
		SpawnDamage (te_sparks, point, normal, save);

	// check for invincibility
        if ((client && client->invincible_framenum > level.framenum ) && !(type & TOD_NO_PROTECTION))
	{
		if (targ->pain_debounce_time < level.time)
		{
			gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}
		take = 0;
                save = damage;
	}

        if (same_team && ((int)dmflags->value & DF_ARMOR_PROTECT))
		psave = asave = 0;
        else
        {       // Is no power armor for player...
                // GRIM - 10/01/99
                //psave = CheckPowerArmor (targ, point, normal, take, type);
                //take -= psave;
                // GRIM - 10/01/99

                asave = CheckArmor (targ, point, normal, take, te_sparks, type);
                take -= (float)asave;
        }

	//treat cheat/powerup savings the same as armor
	asave += save;

	// team damage avoidance
        if (same_team)
        {
                HitLocation = HIT_SOAKED;

                // Spawn team coloured sparks
                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_SPLASH);
                gi.WriteByte (8);
                gi.WritePosition (point);
                gi.WriteDir (normal);
                gi.WriteByte (SPLASH_LAVA);
                gi.multicast (point, MULTICAST_PVS);
                return;
        }

        //ZOID
        CTFCheckHurtCarrier(targ, attacker);
        //ZOID

        // do the damage
        if (take > 0)
	{
                orig_take = take;

                // Client SpawnDamage handled differently...
                if ((targ->svflags & SVF_MONSTER) || targ->deadflag)
                        SpawnDamage (TE_BLOOD, point, normal, take);
                else if (!client)
                        SpawnDamage (te_sparks, point, normal, take);
                //gi.dprintf ("pre-hitlocation %f, ", take);

                // Paranoid - entity has a minimum amount of damage
                if (targ->mindmg && (take < targ->mindmg))
                        return;
                // Paranoid

                // GRIM
                if ((type & TOD_NO_WOUND) || (type & TOD_NO_ARMOR))
                        take = take;
                else if ((HitLocation & HIT_HEAD) && (!(type & TOD_EXPLOSIVE)))
                        take *= 3; // 4
                else if (HitLocation & HIT_MID)
                        take *= 0.65; // Didn't exist
                else if ((HitLocation & HIT_RIGHT_LEG) || (HitLocation & HIT_LEFT_LEG))
                {
                        if (HitLocation & HIT_UPPER_LEGS)
                                take *= 0.45; // 0.7
                        else
                                take *= 0.4; // 0.55
                }
                else if ((HitLocation & HIT_RIGHT_ARM) || (HitLocation & HIT_LEFT_ARM))
                {
                        if (HitLocation & HIT_UPPER_ARMS)
                                take *= 0.55; // 0.6
                        else
                                take *= 0.45; // 0.5
                }
                // GRIM

                if (take < 1)
                        take = 1;

                //gi.dprintf ("final %f\n", take);

                // Dirty
                if (targ->deadflag)
                {
                        ImpactSound (targ);
                        targ->health = targ->health - (int)take;
                }
                else if (!(targ->flags & FL_GODMODE))
                {
                        if (client)
                        {
                                if (AddWound (targ, orig_take, type, mod))
                                        addwound = true;
                        }
                        targ->health = targ->health - (int)take;
                }
                // Dirty

                if (client && (attacker != world))
                {
                        targ->client->last_means = meansOfDeath;
                        targ->client->last_hitloc = HitLocation;
                }

                if (targ->health <= 0)
		{
                        //if ((targ->svflags & SVF_MONSTER) || (client))
                        //        targ->flags |= FL_NO_KNOCKBACK;
                        Killed (targ, inflictor, attacker, take, point);
			return;
		}
	}

        if (targ->svflags & SVF_MONSTER)
	{
                if (attacker->client)
                        targ->blown_away = attacker;

		M_ReactToDamage (targ, attacker);
                if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take > 0))
		{
			targ->pain (targ, attacker, knockback, take);

			// nightmare mode monsters don't go into pain frames often
			if (skill->value == 3)
				targ->pain_debounce_time = level.time + 5;
		}
        }
	else if (client)
	{
                if (!same_team && OnSameTeam (targ, attacker))
                        same_team = true;

                if ((!(targ->flags & FL_GODMODE)) && addwound)
                        SpawnDamage (TE_BLOOD, point, normal, take);

                targ->last_damage = level.time; // GRIM - 17/01/99

                if (targ != attacker)
                {
                        if ((!same_team) && (attacker->client || (attacker->svflags & SVF_MONSTER)))
                        {
                                targ->last_combat = level.time;

                                if (addwound)
                                        NewEvent (targ, HURT);
                                else
                                        NewEvent (targ, TAKING_FIRE);

                                if (attacker->client)
                                {
                                        attacker->last_combat = level.time;
                                
                                        if (targ->blown_away && (attacker != targ->blown_away))
                                                targ->blown_away = NULL;
                                        else
                                                targ->blown_away = attacker;
                                        
                                        if (attacker != targ->enemy)
                                                targ->oldenemy = targ->enemy;
                                        targ->enemy = attacker;
                                }
                                else
                                        targ->blown_away = NULL;
                        }
                        else if (addwound)
                                NewEvent (targ, HURT);
                }

                // GRIM
                //client->damage_parmor += psave;
                client->damage_parmor = 0;
                // GRIM

		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy (point, client->damage_from);
        }
        else if (take > 0)
        {
                if (targ->pain)
                        targ->pain (targ, attacker, knockback, take);
        }

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
        /* GRIM - Moved up...
	if (client)
	{
	}
        */
}

/*
============
T_RadiusDamage
============
*/
// GRIM
qboolean OutaWaterCheck (edict_t *inflictor, edict_t *ent, float origrad)
{
	vec3_t	v;
        int length;

        if (inflictor->waterlevel && (ent->waterlevel == 0))
        {
                VectorSubtract (ent->s.origin, inflictor->s.origin, v);
                length = VectorLength (v);

                if (length > origrad)
                        return true;
        }

        return false;
}

// This checks to see what individual places of a player are hit...
// FIX ME - Need to figure out a better way!
void DamageAreas (edict_t *targ, edict_t *inflictor, edict_t *attacker, float damage, vec3_t dir, int mod)
{
        vec3_t  forward, right, up, check, leg_ht;
	trace_t	trace;
        int     areas, hit_here, chest;
        float   dmg, cdmg;
        
        chest = 0;
        AngleVectors (targ->s.angles, forward, right, up);

        hit_here = 0;
        areas = 0;

        VectorMA (targ->s.origin, targ->maxs[2], up, check);
        trace = gi.trace (inflictor->s.origin, NULL, NULL, check, inflictor, MASK_SOLID);
        if (trace.fraction == 1.0)
        {
                hit_here |= HIT_HEAD;
                areas++;
        }
        //gi.dprintf("chest %i\n", chest);

        VectorMA (targ->s.origin, targ->mins[0], right, check);
        trace = gi.trace (inflictor->s.origin, NULL, NULL, check, inflictor, MASK_SOLID);
        if (trace.fraction == 1.0)
        {
                hit_here |= HIT_LEFT_ARM;
                areas++;
                chest++;
        }
        //gi.dprintf("chest %i\n", chest);

        VectorMA (targ->s.origin, targ->maxs[0], right, check);
        trace = gi.trace (inflictor->s.origin, NULL, NULL, check, inflictor, MASK_SOLID);
        if (trace.fraction == 1.0)
        {
                hit_here |= HIT_RIGHT_ARM;
                areas++;
                chest++;
        }
        //gi.dprintf("chest %i\n", chest);

        dmg = (targ->mins[2] / 2);
        VectorMA (targ->s.origin, dmg, up, leg_ht);

        VectorMA (leg_ht, targ->mins[0], right, check);
        trace = gi.trace (inflictor->s.origin, NULL, NULL, check, inflictor, MASK_SOLID);
        if (trace.fraction == 1.0)
        {
                hit_here |= HIT_LEFT_LEG;
                areas++;
        }

        VectorMA (leg_ht, targ->maxs[0], right, check);
        trace = gi.trace (inflictor->s.origin, NULL, NULL, check, inflictor, MASK_SOLID);
        if (trace.fraction == 1.0)
        {
                hit_here |= HIT_RIGHT_LEG;
                areas++;
        }

        if ((HitLocation & (HIT_RIGHT_LEG | HIT_LEFT_LEG)) && (HitLocation & HIT_HEAD))
                chest = 2;

        if (chest > 1)
        {
                hit_here |= HIT_CHEST;
                areas += 2;
        }
        //gi.dprintf("chest %i\n", chest);

        dmg = (damage / areas);

        // Chest takes has more area therefore takes more of the explosion
        if (hit_here & HIT_CHEST)
        {
                cdmg = dmg * 2;
                //gi.dprintf("cdmg %f\n", cdmg);
                T_Damage (targ, inflictor, attacker, dir, targ->s.origin, vec3_origin, cdmg, cdmg, HIT_CHEST|HIT_MID, (TOD_RADIUS | TOD_EXPLOSIVE), mod);
        }

        if (hit_here & HIT_HEAD)
                T_Damage (targ, inflictor, attacker, dir, targ->s.origin, vec3_origin, dmg, dmg, HIT_HEAD, (TOD_RADIUS | TOD_EXPLOSIVE), mod);

        if (hit_here & HIT_LEFT_ARM)
                T_Damage (targ, inflictor, attacker, dir, targ->s.origin, vec3_origin, dmg, dmg, HIT_LEFT_ARM, (TOD_RADIUS | TOD_EXPLOSIVE), mod);

        if (hit_here & HIT_RIGHT_ARM)
                T_Damage (targ, inflictor, attacker, dir, targ->s.origin, vec3_origin, dmg, dmg, HIT_RIGHT_ARM, (TOD_RADIUS | TOD_EXPLOSIVE), mod);

        if (hit_here & HIT_LEFT_LEG)
                T_Damage (targ, inflictor, attacker, dir, targ->s.origin, vec3_origin, dmg, dmg, HIT_LEFT_LEG, (TOD_RADIUS | TOD_EXPLOSIVE), mod);

        if (hit_here & HIT_RIGHT_LEG)
                T_Damage (targ, inflictor, attacker, dir, targ->s.origin, vec3_origin, dmg, dmg, HIT_RIGHT_LEG, (TOD_RADIUS | TOD_EXPLOSIVE), mod);
}

void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	edict_t	*ent = NULL;
        vec3_t  v, dir;
        float   points;
        float   origrad, radius_use; // GRIM

        // Exploded in water.
        if (inflictor->waterlevel)
        {
                origrad = radius;
                radius_use = radius * 2;
        }
        else
                radius_use = radius;

        while ((ent = findradius(ent, inflictor->s.origin, radius_use)) != NULL)
	{
                if (ent == ignore)
			continue;

		if (!ent->takedamage)
			continue;

                if (OutaWaterCheck (inflictor, ent, origrad))
			continue;

                VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		points = damage - 0.5 * VectorLength (v);

                // Why Half damage for the fucker!
                // MUZZA - Right on!
                // if (ent == attacker)
                //        points = points * 0.5;

		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
                                VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
                                if (ent->client && (ent->deadflag == DEAD_NO))
                                        DamageAreas (ent, inflictor, attacker, points, dir, mod);
                                else
                                        T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, points, points, 0, (TOD_RADIUS | TOD_EXPLOSIVE), mod);
			}
		}
	}
}
