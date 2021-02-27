#include "g_local.h"

//3.0 matrix jump
void cmd_mjump(edict_t *ent)
{

	if (ent->holdtime > level.time)
		return; // can't use abilities

	if (HasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't use this while carrying the flag!\n");
		return;
	}

	if ((!(ent->v_flags & SFLG_MATRIXJUMP)) && (ent->velocity[2] == 0) && (!(ent->v_flags & SFLG_UNDERWATER)))
	{
		item_t *slot;
		int i;
		qboolean found = false;

		//Find item in inventory
		for (i = 3; i < MAX_VRXITEMS; ++i)
		{
			if (ent->myskills.items[i].itemtype & ITEM_GRAVBOOTS)
			{
				slot = &ent->myskills.items[i];
				found = true;
				break;
			}
		}

		//no boots? no jump
		if (!found) return;

		ent->v_flags ^= SFLG_MATRIXJUMP;
		ent->velocity[2] += MJUMP_VELOCITY;

		//Consume a charge
		if (!(ent->myskills.items[i].itemtype & ITEM_UNIQUE))
			ent->myskills.items[i].quantity -= 1;

		//if out of charges, erase the item
		if (ent->myskills.items[i].quantity == 0)
		{
			int count = 0;
			V_ItemClear(&ent->myskills.items[i]);
			//Alert the player
            safe_cprintf(ent, PRINT_HIGH, "Your anti-gravity boots have broken!\n");
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/itembreak.wav"), 1, ATTN_NORM, 0);

			//Check for more boots (backup items)
			for (i = 3; i < MAX_VRXITEMS; ++i)
				if (ent->myskills.items[i].itemtype & ITEM_GRAVBOOTS)
					++count;
			safe_cprintf(ent, PRINT_HIGH, "Boots left: %d.\n", count);

		}
		else gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/gravjump.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);

		// calling entity made a sound, used to alert monsters
		ent->lastsound = level.framenum;
	}
}
//end matrix jump

qboolean CheckAuraOwner (edict_t *self, int aura_cost);

/*
void Cmd_AmmoStealer_f(edict_t *ent)
{
	edict_t		*other = NULL;
	int			shells=0, bullets=0, rockets=0, slugs=0, cells=0;
	float		steal_base;	//Base multiplier for ammo steal
	int			skill_level;
	qboolean	foundtarget = false;
	vec3_t		start;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_AmmoStealer_f()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[AMMO_STEAL].disable)
		return;

	//3.0 new ammo steal algorithm, more efficient less buggy (no ammo lost during steal)
	if (!G_CanUseAbilities(ent, ent->myskills.abilities[AMMO_STEAL].current_level, COST_FOR_STEALER))
		return;

	G_GetSpawnLocation(ent, 512, vec3_origin, vec3_origin, start);

	//Search for targets
	while ((other = findradius(other, start, 128)) != NULL)
	{
		if (other == ent)
			continue;
		if (!other->inuse)
			continue;
		if (!other->takedamage)
			continue;
		if (other->solid == SOLID_NOT)
			continue;
		if (!other->client)
			continue;
		if (OnSameTeam(ent, other))
			continue;
		if (!visible(ent, other))
			continue;

		//A target has been found
		foundtarget = true;

		//Calculate the steal multiplier (random amount for each target)
		skill_level = ent->myskills.abilities[AMMO_STEAL].current_level;
		//min = 6.25%/lvl, max = 7.14%/lvl
		steal_base = GetRandom((int)(skill_level / 16.0 * 100.0), (int)(skill_level / 14.0 * 100.0)) / 100.0;
		if (steal_base > 0.9)	steal_base = 0.9; //max at 90%

		//Start stealing ammo
		shells	+= other->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))]	* steal_base;
		bullets += other->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]	* steal_base;
		rockets += other->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]	* steal_base;
		slugs	+= other->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))]		* steal_base;
		cells	+= other->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]		* steal_base;
		
		//This next part does the same thing as subtracting the amount stolen from the target
		other->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))]	*= (1 - steal_base);
		other->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]	*= (1 - steal_base);
		other->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]	*= (1 - steal_base);
		other->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))]	*= (1 - steal_base);
		other->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]	*= (1 - steal_base);

		//Play the spell sound!
		gi.sound(ent, CHAN_ITEM, gi.soundindex("spells/telekinesis.wav"), 1, ATTN_NORM, 0);

		//Notify the two clients
		safe_cprintf(other, PRINT_HIGH, "%s just stole some of your ammo!\n", ent->client->pers.netname);
		safe_cprintf(ent, PRINT_HIGH, "You just stole %d%% of %s's ammo!\n", (int)(steal_base * 100), other->client->pers.netname);

		// calling entity made a sound, used to alert monsters
		ent->lastsound = level.framenum;
	}
	//Add what we stole from the other people. :)
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] += shells;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] += bullets;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] += rockets;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] += slugs;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] += cells;
	Check_full(ent);//Make sure we are not over our limits

	if(foundtarget)
	{
		ent->client->ability_delay = level.time + DELAY_AMMOSTEAL;
		ent->client->pers.inventory[power_cube_index] -= COST_FOR_STEALER;
	}
}
*/

void Cmd_BoostPlayer(edict_t *ent)
{
	edict_t		*other = NULL;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	float		boost_delay = DELAY_BOOST;
	int			talentLevel;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_BoostPlayer()\n", ent->client->pers.netname);

	if (ent->myskills.abilities[BOOST_SPELL].disable)
		return;

	//4.07 can't boost while being hurt
	if (ent->lasthurt+DAMAGE_ESCAPE_DELAY > level.time)
		return;

	if (!V_CanUseAbilities (ent, BOOST_SPELL, COST_FOR_BOOST, true))
		return;

	if (HasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't use this ability while carrying the flag!\n");
		return;
	}

	if (ent->client->snipertime >= level.time)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't use boost while trying to snipe!\n");
		return;
	}

	//Talent: Mobility
	talentLevel = getTalentLevel(ent, TALENT_MOBILITY);
	switch(talentLevel)
	{
	case 1:		boost_delay -= 0.15;	break;
	case 2:		boost_delay -= 0.3;		break;
	case 3:		boost_delay -= 0.5;		break;
	default:	//Do nothing
		break;
	}

	ent->client->pers.inventory[power_cube_index] -= COST_FOR_BOOST;
	ent->client->ability_delay = level.time + boost_delay;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	//4.2 player-monster support
	if (PM_PlayerHasMonster(ent))
	{
		other = ent->owner;
		other->monsterinfo.air_frames = 1;// used to turn off velocity restriction
	}
	else
		other = ent;

	other->velocity[0] += forward[0] * 1000;
	other->velocity[1] += forward[1] * 1000;
	other->velocity[2] += forward[2] * 1000;

	if (other->groundentity)
		other->velocity[2] += 300;

	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;

	//Talent: Leap Attack
	if (getTalentLevel(ent, TALENT_LEAP_ATTACK) > 0)
	{
		ent->client->boosted = true; 
		gi.sound (ent, CHAN_VOICE, gi.soundindex (va("spells/leapattack%d.wav", GetRandom(1, 3))), 1, ATTN_NORM, 0);
	}
	else
		gi.sound (ent, CHAN_BODY, gi.soundindex("weapons/rockfly.wav"), 1, ATTN_NORM, 0);
}

void LeapAttack (edict_t *ent)
{
	float	duration;
	vec3_t	point, forward;
	edict_t *e=NULL;

	if (!ent->client->boosted)
		return;

	duration = 0.2 * getTalentLevel(ent, TALENT_LEAP_ATTACK);

	while ((e = findradius (e, ent->s.origin, 128)) != NULL)
	{
		if (!G_ValidTarget(ent, e, true))
			continue;
		if (!infront(ent, e))
			continue;

		G_EntMidPoint(e, point);
		VectorSubtract(point, ent->s.origin, forward);
		T_Damage(e, e, ent, forward, point, vec3_origin, 0, 1000, 0, 0);

		gi.sound (e, CHAN_VOICE, gi.soundindex (va("misc/stun%d.wav", GetRandom(1, 3))), 1, ATTN_NORM, 0);
		
		// stun them
		if (e->client)
		{
			e->holdtime = level.time + duration;
			e->client->ability_delay = level.time + duration;
		}
		else
			e->nextthink = level.time + duration;
	}

	ent->client->boosted = false;
}

void BecomeExplosion2 (edict_t *self);
void Cmd_CorpseExplode(edict_t *ent)
{
	int		damage, min_dmg, max_dmg, slvl;
	float	fraction, radius;
	vec3_t	start, end, forward, right, offset;
	trace_t	tr;
	edict_t *e=NULL;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_CorpseExplode()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[CORPSE_EXPLODE].disable)
		return;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[CORPSE_EXPLODE].current_level, COST_FOR_CORPSEEXPLODE))
		return;

	slvl = ent->myskills.abilities[CORPSE_EXPLODE].current_level;
	radius = CORPSE_EXPLOSION_INITIAL_RADIUS + CORPSE_EXPLOSION_ADDON_RADIUS * slvl;

	// calculate starting position
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, CORPSE_EXPLOSION_MAX_RANGE, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);

	while ((e = findclosestradius (e, tr.endpos, CORPSE_EXPLOSION_SEARCH_RADIUS)) != NULL)
	{
		if (!G_EntExists(e))
			continue;
		if (e->health > 0)
			continue;
		if (e->max_health < 1)
			continue;

		// kill the corpse
		T_Damage(e, e, ent, vec3_origin, e->s.origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_CORPSEEXPLODE);

		// inflict damage
		fraction = 0.1 * GetRandom(5, 10);
		damage = fraction * e->max_health;
		
		// calculate min/max damage range
		min_dmg = CORPSE_EXPLOSION_INITIAL_DAMAGE + CORPSE_EXPLOSION_ADDON_DAMAGE * slvl;
		max_dmg = 5 * min_dmg;

		if (damage < min_dmg)
			damage = min_dmg;
		else if (damage > max_dmg)
			damage = max_dmg;

		T_RadiusDamage (e, ent, damage, NULL, radius, MOD_CORPSEEXPLODE);

		//gi.dprintf("fraction %.1f, damage %d, max_health: %d\n", fraction, damage, e->max_health);

		//Spells like corpse explode shouldn't display 10000 damage, so show the corpse damage instead
		ent->client->ps.stats[STAT_ID_DAMAGE] = damage;

		gi.sound(e, CHAN_ITEM, gi.soundindex("spells/corpseexplodecast.wav"), 1, ATTN_NORM, 0);
		ent->client->pers.inventory[power_cube_index] -= COST_FOR_CORPSEEXPLODE;
		ent->client->ability_delay = level.time + DELAY_CORPSEEXPLODE;
		safe_cprintf(ent, PRINT_HIGH, "Corpse exploded for %d damage!\n", damage);

		//decino: explosion effect
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GRENADE_EXPLOSION);
		gi.WritePosition (e->s.origin);
		gi.multicast (e->s.origin, MULTICAST_PVS);

		//decino: shoot 6 gibs that deal damage
		//az: todo, more copypaste.
		/*for (i = 0; i < 10; i++)
		{
			e->s.angles[YAW] += 36;
			AngleCheck(&e->s.angles[YAW]);

			AngleVectors(e->s.angles, forward, NULL, up);
			fire_gib(ent, e->s.origin, forward, dmg, 0, 1000);
		}*/

		// calling entity made a sound, used to alert monsters
		ent->lastsound = level.framenum;

		break;
	}
}

void Vampire_Think(edict_t *self)
{
	if (self->myskills.abilities[VAMPIRE].current_level > 5)
	{
		if (!level.daytime && self->deadflag != DEAD_DEAD && self->solid != SOLID_NOT)
		{
			if (!self->client->lowlight){
				self->client->ps.rdflags |= RDF_IRGOGGLES;
				self->client->lowlight = true;
			}
		}
		else
		{
			if (self->client->lowlight){
				self->client->ps.rdflags &= ~RDF_IRGOGGLES;
				self->client->lowlight = false;
			}
		}
	}
}

void TeleportPlayer (edict_t *player)
{
	int dist = 512;
	vec3_t forward, start, end;
	trace_t tr;

	//GHz: Calculate end point and trace
	VectorCopy(player->s.origin, start);
	start[2]++;
	AngleVectors (player->s.angles, forward, NULL, NULL);
	//GHz: Keep trying until we get a valid spot
	while (dist > 0){
		VectorMA(start, dist, forward, end);
		tr = gi.trace(start, player->mins, player->maxs, end, player, MASK_SOLID);
		if (!(tr.contents & MASK_SOLID) && tr.fraction == 1.0)
		{
			player->s.event = EV_PLAYER_TELEPORT;
			VectorCopy(tr.endpos, player->s.origin);

			//GHz: Hold in place briefly
			player->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
			player->client->ps.pmove.pm_time = 14;
			//player->client->respawn_time = level.time;

			dist = 0;//GHz: We found a spot, so we're done
			VectorClear(player->velocity);
			gi.linkentity(player);
		}
		else
			dist -= 10;//GHz: No luck, so try further back
	}

}

qboolean ValidTeleportSpot (edict_t *ent, vec3_t spot)
{
	vec3_t	start, forward, right;
	trace_t	tr;

	AngleVectors(ent->s.angles, forward, right, NULL);

	// check above
	VectorCopy(spot, start);
	start[2] += 128;
	tr = gi.trace(spot, NULL, NULL, start, ent, MASK_SOLID);
	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return false;

	// check left
	VectorMA(spot, -128, right, start);
	tr = gi.trace(spot, NULL, NULL, start, ent, MASK_SOLID);
	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return false;

	// check right
	VectorMA(spot, 128, right, start);
	tr = gi.trace(spot, NULL, NULL, start, ent, MASK_SOLID);
	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return false;

	// check forward
	VectorMA(spot, 128, forward, start);
	tr = gi.trace(spot, NULL, NULL, start, ent, MASK_SOLID);
	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return false;

	// check behind
	VectorMA(spot, -128, forward, start);
	tr = gi.trace(spot, NULL, NULL, start, ent, MASK_SOLID);
	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return false;

	return true;
}

void TeleportForward (edict_t *ent)
{
	int		dist = 512;
	vec3_t	angles, offset, forward, right, start, end;
	trace_t	tr;

	if (!G_EntIsAlive(ent))
		return;
	//4.07 can't teleport while being hurt
	if (ent->lasthurt+DAMAGE_ESCAPE_DELAY > level.time)
		return;
	if (HasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't use this while carrying the flag!\n");
		return;
	}

	if (!V_CanUseAbilities(ent, TELEPORT, TELEPORT_COST, true))
		return;
	
	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;

	RemoveAllDrones(ent, false);
	RemoveHellspawn(ent);
	RemoveMiniSentries(ent);
//	VortexRemovePlayerSummonables(ent);

	// get forward vector
	if (!ent->client)
	{
		VectorCopy(ent->s.angles, angles);
		angles[PITCH] = 0;
		AngleVectors(angles, forward, NULL, NULL);
	}
	else
	{
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	}
	// get starting position
	VectorCopy(ent->s.origin, start);
	start[2]++;

	// in CTF, you can't teleport thru solid objects
	/*
	if (ctf->value)
	{
		VectorMA(start, dist, forward, end);
		tr = gi.trace(start, ent->mins, ent->maxs, end, ent, MASK_SHOT);

		ent->s.event = EV_PLAYER_TELEPORT;
		VectorCopy(tr.endpos, ent->s.origin);
		VectorCopy(tr.endpos, ent->s.old_origin);
		VectorClear(ent->velocity);
		gi.linkentity(ent);
		ent->client->pers.inventory[power_cube_index]-=TELEPORT_COST;
		ent->client->ability_delay = level.time + 1;
		return;
	}
	*/

	// keep trying to teleport until there is no room left
	while (dist > 0)
	{
		edict_t *teleport_target;

		//4.2 support player-monsters
		if (PM_PlayerHasMonster(ent))
			teleport_target = ent->owner;
		else
			teleport_target = ent;

		VectorMA(start, dist, forward, end);

		// if there is a nearby sky brush, we shouldn't teleport there
		if (!ValidTeleportSpot(teleport_target, end))
		{
			// try further back
			dist -= 8;
			continue;
		}

		tr = gi.trace(end, teleport_target->mins, teleport_target->maxs, end, teleport_target, MASK_SHOT);

		// is this a valid position?
		if (!(tr.contents & MASK_SHOT) && !(gi.pointcontents(end) & MASK_SHOT) && !tr.allsolid)
		{
			teleport_target->s.event = EV_PLAYER_TELEPORT;
			VectorCopy(end, teleport_target->s.origin);
			VectorCopy(end, teleport_target->s.old_origin);
			VectorClear(teleport_target->velocity);
			gi.linkentity(teleport_target);
			ent->client->pers.inventory[power_cube_index]-=TELEPORT_COST;

			// 3.14 prevent teleport+nova spam
			ent->client->ability_delay = level.time + (ctf->value?2.0:1.0);
			return;
		}
		else
		{
			// try further back
			dist-=8;
		}
	}
}

void Weapon_Blaster (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_BFG (edict_t *ent);

#define AUTOAIM_TARGET_RADIUS		1024

qboolean autoaim_findtarget (edict_t *ent)
{
	edict_t *target = NULL;

	while ((target = findclosestreticle(target, ent, AUTOAIM_TARGET_RADIUS)) != NULL)
	{
		if (!G_ValidTarget(ent, target, true))
			continue;
		if (!nearfov(ent, target, 0, 20))
			continue;
		ent->enemy = target;
		return true;
	}
	return false;
}

void autoaim_getfiringparameters (edict_t *ent, int *speed, qboolean *rocket)
{
	if (ent->mtype)
	{
		switch(ent->mtype)
		{
		case MORPH_MUTANT: *speed = 500; break;
		case MORPH_MEDIC: *speed = 1500; break;
		case MORPH_FLYER: ent->client->weapon_mode?*speed=0:2000; break;
		case MORPH_CACODEMON: *speed = 950; *rocket = true; break;
		}
	}
	else if (PM_PlayerHasMonster(ent))
	{
		if ((ent->owner->mtype == P_TANK) && !ent->client->weapon_mode)
		{
			*speed = 650+30*ent->owner->monsterinfo.level;
			*rocket = true;
		}
	}
	else if (ent->client->pers.weapon)
	{
		if (ent->client->pers.weapon->weaponthink == Weapon_Blaster)
		{
			*speed = BLASTER_INITIAL_SPEED + BLASTER_ADDON_SPEED * ent->myskills.weapons[WEAPON_BLASTER].mods[2].current_level;
		}
		else if (ent->client->pers.weapon->weaponthink == Weapon_GrenadeLauncher)
		{
			*speed = GRENADELAUNCHER_INITIAL_SPEED + GRENADELAUNCHER_ADDON_SPEED * ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[2].current_level;
			if (ent->enemy->groundentity)
				*rocket = true;
		}
		else if (ent->client->pers.weapon->weaponthink == Weapon_RocketLauncher)
		{
			*speed = ROCKETLAUNCHER_INITIAL_SPEED + ROCKETLAUNCHER_ADDON_SPEED * ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[2].current_level;
			*rocket = true;
		}
		else if (ent->client->pers.weapon->weaponthink == Weapon_HyperBlaster)
		{
			*speed = HYPERBLASTER_INITIAL_SPEED + HYPERBLASTER_ADDON_SPEED * ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[2].current_level;
		}
		else if (ent->client->pers.weapon->weaponthink == Weapon_BFG)
		{
			*speed = BFG10K_INITIAL_SPEED + BFG10K_ADDON_SPEED * ent->myskills.weapons[WEAPON_BFG10K].mods[2].current_level;
			*rocket = true;
		}
	}
}

void autoaim_lockontarget (edict_t *ent)
{
	int			i, speed = 0;
	qboolean	rocket = false;
	vec3_t		forward, start;

	autoaim_getfiringparameters(ent, &speed, &rocket);

	//gi.dprintf("%d %s\n", speed, rocket?"true":"false");

	// calculate angle to enemy
	MonsterAim(ent, -1, speed, rocket, 0, forward, start);
	VectorNormalize(forward);
	vectoangles(forward, forward);
	if (forward[PITCH] < -180)
		forward[PITCH] += 360;

	// set view angles to target
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	for (i = 0 ; i < 3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(forward[i] - ent->client->resp.cmd_angles[i]);
	VectorCopy(forward, ent->client->ps.viewangles);
	VectorCopy(forward, ent->client->v_angle);
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
}

void V_AutoAim (edict_t *player)
{
	if (!player->client)
		return;
	if (player->myskills.administrator < 999)
		return;
	if (G_IsSpectator(player))
		return;

	//gi.dprintf("V_AutoAim()\n");

	// we have no previous target
	if (!player->enemy)
	{
		if (autoaim_findtarget(player))
		{
			//gi.dprintf("found target (%s)\n", player->enemy->classname?player->enemy->classname:"null");
			autoaim_lockontarget(player);
		}
	}
	// we've got an invalid target
	else if (!G_ValidTarget(player, player->enemy, true) || !infront(player, player->enemy))
	{
		//gi.dprintf("have invalid target");

		// try to find another target
		if (autoaim_findtarget(player))
			autoaim_lockontarget(player);
		else
		// couldn't find one, so we're done
			player->enemy = NULL;
	}
	// we have a valid target
	else
	{
		//gi.dprintf("have valid target\n");
		autoaim_lockontarget(player);
	}
}
/*
void LockOnTarget (edict_t *player)
{
	int			speed = 0;
	qboolean	rocket = false;
	int			i;
	vec3_t		forward, start;

	if (player->deadflag == DEAD_DEAD)
		return;
	if (que_typeexists(player->curses, CURSE_FROZEN))
		return;
	if (!G_ValidTarget(player, player->enemy, true))
		return;
	if (!infront(player, player->enemy))
		return;

	if (player->mtype)
	{
		switch(player->mtype)
		{
		case MORPH_MUTANT: speed = 500; break;
		case MORPH_MEDIC: speed = 1500; break;
		case MORPH_FLYER: player->client->weapon_mode?speed=0:2000; break;
		case MORPH_CACODEMON: speed = 950; rocket = true; break;
		}
	}
	else if (PM_PlayerHasMonster(player))
	{
		if ((player->owner->mtype = P_TANK) && !player->client->weapon_mode)
		{
			speed = 650+30*player->owner->monsterinfo.level;
			rocket = true;
		}
	}
	else if (player->client->pers.weapon)
	{
		if (player->client->pers.weapon->weaponthink == Weapon_Blaster)
		{
			speed = BLASTER_INITIAL_SPEED + BLASTER_ADDON_SPEED * player->myskills.weapons[WEAPON_BLASTER].mods[2].current_level;
		}
		else if (player->client->pers.weapon->weaponthink == Weapon_GrenadeLauncher)
		{
			speed = GRENADELAUNCHER_INITIAL_SPEED + GRENADELAUNCHER_ADDON_SPEED * player->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[2].current_level;
			if (player->enemy->groundentity)
				rocket = true;
		}
		else if (player->client->pers.weapon->weaponthink == Weapon_RocketLauncher)
		{
			speed = ROCKETLAUNCHER_INITIAL_SPEED + ROCKETLAUNCHER_ADDON_SPEED * player->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[2].current_level;
			rocket = true;
		}
		else if (player->client->pers.weapon->weaponthink == Weapon_HyperBlaster)
		{
			speed = HYPERBLASTER_INITIAL_SPEED + HYPERBLASTER_ADDON_SPEED * player->myskills.weapons[WEAPON_HYPERBLASTER].mods[2].current_level;
		}
		else if (player->client->pers.weapon->weaponthink == Weapon_BFG)
		{
			speed = BFG10K_INITIAL_SPEED + BFG10K_ADDON_SPEED * player->myskills.weapons[WEAPON_BFG10K].mods[2].current_level;
			rocket = true;
		}
	}

	//GHz: Calculate angle to enemy
	MonsterAim(player, 1, speed, rocket, 0, forward, start);
	VectorNormalize(forward);
	vectoangles(forward, forward);
	if (forward[PITCH] < -180)
		forward[PITCH] += 360;

	//GHz: Set view angles to target
	player->client->ps.pmove.pm_type = PM_FREEZE;
	for (i = 0 ; i < 3 ; i++)
		player->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(forward[i] - player->client->resp.cmd_angles[i]);
	VectorCopy(forward, player->client->ps.viewangles);
	VectorCopy(forward, player->client->v_angle);
	player->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
}
*/

void CrippleAttack (edict_t *ent)
{
	int		damage;
	vec3_t	end;
	trace_t	tr;
	edict_t	*e=NULL;

	while ((e = findclosestreticle(e, ent, CRIPPLE_RANGE)) != NULL)
	{
		if (!G_ValidTarget(ent, e, true))
			continue;
		if (!visible(ent, e))
			continue;
		if (!infront(ent, e))
			continue;

		damage = e->health * (1-(1/(1+0.2*ent->myskills.abilities[CRIPPLE].current_level)));
		if (damage > CRIPPLE_MAX_DAMAGE)
			damage = CRIPPLE_MAX_DAMAGE;
		T_Damage(e, ent, ent, vec3_origin, e->s.origin, vec3_origin, damage, 0, DAMAGE_NO_ABILITIES, MOD_CRIPPLE);

		VectorCopy(e->s.origin, end);
		end[2] += 8192;
		tr = gi.trace(e->s.origin, NULL, NULL, end, e, MASK_SHOT);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_MONSTER_HEATBEAM);
		gi.WriteShort (e-g_edicts);
		gi.WritePosition (e->s.origin);
		gi.WritePosition (tr.endpos);
		gi.multicast (e->s.origin, MULTICAST_PVS);
		break;
	}

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/eleccast.wav"), 1, ATTN_NORM, 0);
	ent->client->ability_delay = level.time + CRIPPLE_DELAY;
	ent->client->pers.inventory[power_cube_index]-=CRIPPLE_COST;

	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}

void Cmd_Cripple_f (edict_t *ent)
{
	int	ability_level=ent->myskills.abilities[CRIPPLE].current_level;

	if (!G_CanUseAbilities(ent, ability_level, CRIPPLE_COST))
		return;
	if (ent->myskills.abilities[CRIPPLE].disable)
		return;

	//3.0 amnesia disables cripple
	if (que_findtype(ent->curses, NULL, AMNESIA) != NULL)
		return;
	CrippleAttack(ent);
}

#define BOLT_SPEED				1000
#define BOLT_DURATION			3
#define BOLT_INITIAL_DAMAGE		50
#define BOLT_ADDON_DAMAGE		10
#define BOLT_DELAY				0.2
#define IMP_BOLT_DELAY			0.3
#define IMP_BOLT_RADIUS			100

void MagicBoltExplode(edict_t *self, edict_t *other)
{
	//Do the damage
	T_RadiusDamage(self, self->owner, self->radius_dmg,other, self->dmg_radius, MOD_MAGICBOLT);
}

void magicbolt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//Talent: Improved Magic Bolt
	int talentLevel = getTalentLevel(self->owner, TALENT_IMP_MAGICBOLT);

	if (G_EntExists(other))
	{
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, 
			plane->normal, self->dmg, /*self->dmg*/ 0, 0, MOD_MAGICBOLT);

		// scoring a hit refunds power cubes
		if (talentLevel > 0)
			self->owner->client->pers.inventory[power_cube_index] += self->monsterinfo.cost * (0.4 * talentLevel);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	gi.sound (self, CHAN_WEAPON, gi.soundindex("spells/boom2.wav"), 1, ATTN_NORM, 0);

	if(self->dmg_radius > 0)
		MagicBoltExplode(self, other);

	G_FreeEdict(self);
}

void fire_magicbolt (edict_t *ent, int damage, int radius_damage, float damage_radius, float cost_mult)
{
	edict_t	*bolt;
	vec3_t	offset, forward, right, start;
    
	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;

	// set-up firing parameters
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// create bolt
	bolt = G_Spawn();
	VectorSet (bolt->mins, -10, -10, 10);
	VectorSet (bolt->maxs, 10, 10, 10);
	bolt->s.modelindex = gi.modelindex ("models/objects/flball/tris.md2");
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (forward, bolt->s.angles);
	VectorScale (forward, BOLT_SPEED, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_COLOR_SHELL;
	bolt->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = ent;
	bolt->think = G_FreeEdict;
	bolt->touch = magicbolt_touch;
	bolt->nextthink = level.time + BOLT_DURATION;
	bolt->monsterinfo.cost = BOLT_COST * cost_mult;
	bolt->dmg = damage;
	bolt->dmg_radius = damage_radius;
	bolt->radius_dmg = radius_damage;
	bolt->classname = "magicbolt";	
	gi.linkentity (bolt);

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/holybolt2.wav"), 1, ATTN_NORM, 0);
}

void Cmd_Magicbolt_f (edict_t *ent, float skill_mult, float cost_mult)
{
	int damage, radius_damage=0, cost=BOLT_COST*cost_mult;
	float radius=0;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[MAGICBOLT].current_level, cost))
		return;
	if (ent->myskills.abilities[MAGICBOLT].disable)
		return;

	damage = (BOLT_INITIAL_DAMAGE+BOLT_ADDON_DAMAGE*ent->myskills.abilities[MAGICBOLT].current_level)*skill_mult;

	ent->client->ability_delay = level.time + 0.1; // vrc 2.32 remove  magicbolt delay to spam!

	fire_magicbolt(ent, damage, 0, 0, cost_mult);

	ent->client->pers.inventory[power_cube_index] -= cost;
}

void nova_think (edict_t *self)
{
	self->s.frame+=2;
	if (level.time > self->delay)
		G_FreeEdict(self);
	self->nextthink = level.time + FRAMETIME;
}

void NovaExplosionEffect (vec3_t org)
{
	edict_t *tempent;

	tempent = G_Spawn();
	tempent->s.modelindex = gi.modelindex ("models/objects/nova/tris.md2");
	tempent->think = nova_think;
	tempent->nextthink = level.time + FRAMETIME;
	tempent->s.effects |= EF_PLASMA;
	tempent->delay = level.time + 0.7;
	VectorCopy(org, tempent->s.origin);
	gi.linkentity(tempent);
}

#define NOVA_RADIUS				150
#define NOVA_DEFAULT_DAMAGE		100
#define NOVA_ADDON_DAMAGE		20
#define NOVA_DELAY				0.5
#define FROSTNOVA_RADIUS		150
	
void Cmd_Nova_f (edict_t *ent, int frostLevel, float skill_mult, float cost_mult)
{
	int		damage, cost=NOVA_COST*cost_mult;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[NOVA].current_level, cost))
		return;
	if (ent->myskills.abilities[NOVA].disable)
		return;

	damage = (NOVA_DEFAULT_DAMAGE+NOVA_ADDON_DAMAGE*ent->myskills.abilities[NOVA].current_level)*skill_mult;

	T_RadiusDamage(ent, ent, damage, ent, NOVA_RADIUS, MOD_NOVA);

	//Talent: Frost Nova
	if(frostLevel > 0)
	{
		edict_t	*target = NULL;

		while ((target = findradius(target, ent->s.origin, FROSTNOVA_RADIUS)) != NULL)
		{
			if (target == ent)
				continue;
			if (!target->takedamage)
				continue;
			if (!visible1(ent, target))
				continue;

            //curse with holyfreeze
			target->chill_level = 2 * frostLevel;
			target->chill_time = level.time + 3.0;	//3 second duration

			if (random() > 0.5)
				gi.sound(target, CHAN_ITEM, gi.soundindex("spells/blue1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(target, CHAN_ITEM, gi.soundindex("spells/blue3.wav"), 1, ATTN_NORM, 0);
		}
	}		

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	NovaExplosionEffect(ent->s.origin);
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/novaelec.wav"), 1, ATTN_NORM, 0);

	ent->client->ability_delay = level.time + NOVA_DELAY/* * cost_mult*/;
	ent->client->pers.inventory[power_cube_index] -= cost;

	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}

//Talent: Frost Nova
void Cmd_FrostNova_f (edict_t *ent, float skill_mult, float cost_mult)
{
	int slot = getTalentSlot(ent, TALENT_FROST_NOVA);
	talent_t *talent;
            
	if (slot == -1)
		return;

	talent = &ent->myskills.talents.talent[slot];

	if(talent->upgradeLevel > 0)
	{
		if(talent->delay < level.time)
		{
			Cmd_Nova_f(ent, talent->upgradeLevel, skill_mult, cost_mult);
			talent->delay = level.time + 2.2;	// 2 second recharge.
		}
		else safe_cprintf(ent, PRINT_HIGH, va("You can't cast another frost nova for %0.1f seconds.\n", talent->delay - level.time));
	}
	else 
	{
		safe_cprintf(ent, PRINT_HIGH, "You must upgrade frost nova before you can use it.\n");
	}	
}

void RemoveExplodingArmor (edict_t *ent)
{
	edict_t *e=NULL;

	while((e = G_Find(e, FOFS(classname), "exploding_armor")) != NULL)
	{
		if (e && e->inuse && (e->owner == ent))
		{
			e->think = G_FreeEdict;
			e->nextthink = level.time + FRAMETIME;
		}
	}

	// reset armor counter
	ent->num_armor = 0;
}

void DetonateArmor (edict_t *self)
{
	int	i;

	if (self->solid == SOLID_NOT)
		return; // already flagged for removal

	if (!G_EntExists(self->owner))
	{
		// reduce armor count
		if (self->owner && self->owner->inuse)
			self->owner->num_armor--;

		G_FreeEdict(self);
		return;
	}

	// reduce armor count
	self->owner->num_armor--;

	safe_cprintf(self->owner, PRINT_HIGH, "Exploding armor did %d damage! (%d/%d)\n", 
		self->dmg, self->owner->num_armor, EXPLODING_ARMOR_MAX_COUNT);

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_EXPLODINGARMOR);
	// throw debris around
	if (gi.pointcontents(self->s.origin) == 0) 
	{
		for(i=0;i<GetRandom(2, 6);i++)
			ThrowDebris (self, "models/objects/debris2/tris.md2", 4, self->s.origin);
		for(i=0;i<GetRandom(1, 3);i++)
			ThrowDebris (self, "models/objects/debris1/tris.md2", 2, self->s.origin);
	}
	// create explosion effect
	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	//G_FreeEdict (self);
	// don't remove immediately

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->solid = SOLID_NOT;
}

void explodingarmor_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (G_EntIsAlive(other) && !OnSameTeam(self, other))
		DetonateArmor(self);
}

qboolean NearbyEnemy (edict_t *self, float radius)
{
	edict_t *e=NULL;

	while ((e = findradius(e, self->s.origin, radius)) != NULL)
	{
		if (!G_ValidTarget(self, e, true))
			continue;
		return true;
	}
	return false;
}

void explodingarmor_effects (edict_t *self)
{
	self->s.effects = self->s.renderfx = 0;

	self->s.effects |= EF_COLOR_SHELL;
	if (self->owner->teamnum == RED_TEAM || ffa->value)
		self->s.renderfx |= RF_SHELL_RED;
	else
		self->s.renderfx |= RF_SHELL_BLUE;

	// flash white prior to explosion
	if (level.time >= self->delay - 10 && level.framenum > self->count)
	{
		self->s.renderfx |= RF_SHELL_RED|RF_SHELL_BLUE|RF_SHELL_YELLOW;
		self->count = level.framenum + (self->delay - level.time) + 1;
		//gi.dprintf("%d %d\n", level.framenum, self->count);
	}
}

void explodingarmor_think (edict_t *self)
{
	float dist=EXPLODING_ARMOR_DETECTION;

	// must have an owner
	if (!G_EntIsAlive(self->owner)) 
	{
		// reduce armor count
		if (self->owner && self->owner->inuse)
			self->owner->num_armor--;

		G_FreeEdict(self);
		return;
	}

	// auto-remove from solid objects
	if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID)
	{
		// reduce armor count
		if (self->owner && self->owner->inuse)
			self->owner->num_armor--;

		gi.dprintf("INFO: Removed exploding armor from solid object.\n");
		safe_cprintf(self->owner, PRINT_HIGH, "Your armor was removed from a solid object.\n");
		G_FreeEdict(self);
		return;
	}

	// blow up if enemy gets too close
	if (NearbyEnemy(self, dist))
	{
		DetonateArmor(self);
		return;
	}

	// blow up if time-out is reached
	if (level.time > self->delay) 
	{
		DetonateArmor(self);
		return;
	}

	explodingarmor_effects(self);

	self->nextthink = level.time + FRAMETIME;
}

void SpawnExplodingArmor (edict_t *ent, int time)
{
	float	value;
	vec3_t	forward, right, start, offset;
	edict_t *armor;

	value = 1+0.4*ent->myskills.abilities[EXPLODING_ARMOR].current_level;

	if (time < 2)
		time = 2;
	if (time > 120)
		time = 120;

	// create basic entity
	armor = G_Spawn();
	armor->owner = ent;
	armor->movetype = MOVETYPE_TOSS;
	armor->solid = SOLID_TRIGGER;
	armor->s.modelindex = gi.modelindex("models/items/armor/body/tris.md2");
	armor->classname = "exploding_armor";
	armor->s.effects |= EF_ROTATE;//(EF_ROTATE|EF_COLOR_SHELL);
	//armor->s.renderfx |= RF_SHELL_RED;	
	armor->s.origin[2] += 32;
	VectorSet(armor->mins,-16,-16,-16);
	VectorSet(armor->maxs, 16, 16, 16);
	armor->touch = explodingarmor_touch;
	armor->think = explodingarmor_think;
	armor->dmg = EXPLODING_ARMOR_DMG_BASE + EXPLODING_ARMOR_DMG_ADDON * ent->myskills.abilities[EXPLODING_ARMOR].current_level;
	armor->dmg_radius = 100 + armor->dmg/3;
	
	if (armor->dmg_radius > EXPLODING_ARMOR_MAX_RADIUS)
		armor->dmg_radius = EXPLODING_ARMOR_MAX_RADIUS;

	armor->delay = level.time + time;
	armor->nextthink = level.time + FRAMETIME;

	// get view origin
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// move armor into position
	VectorMA(start, 48, forward, armor->s.origin);

	// don't spawn in a wall
	if (gi.pointcontents(armor->s.origin) & CONTENTS_SOLID)
	{
		G_FreeEdict(armor);
		return;
	}
	gi.linkentity(armor);

	// toss it forward
	VectorScale(forward, 300, armor->velocity);
	armor->velocity[2] += 200;
	//VectorClear(armor->avelocity);
	VectorCopy(ent->s.angles, armor->s.angles);

	ent->client->pers.inventory[body_armor_index] -= EXPLODING_ARMOR_AMOUNT;
	ent->client->pers.inventory[power_cube_index] -= EXPLODING_ARMOR_COST;
	ent->client->ability_delay = level.time + EXPLODING_ARMOR_DELAY;
	
	ent->num_armor++; // 3.5 keep track of number of armor bombs

	safe_cprintf(ent, PRINT_HIGH, "Your armor will detonate in %d seconds. Move away! (%d/%d)\n", 
		time, ent->num_armor, EXPLODING_ARMOR_MAX_COUNT);

	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}

void Cmd_ExplodingArmor_f (edict_t *ent)
{
	if (debuginfo->value)
		gi.dprintf("DEBUG: Cmd_ExplodingArmor_f()\n");

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		RemoveExplodingArmor(ent);
		safe_cprintf(ent, PRINT_HIGH, "All armor bombs removed.\n");
		return;
	}

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[EXPLODING_ARMOR].current_level, EXPLODING_ARMOR_COST))
		return;
	if (ent->myskills.abilities[EXPLODING_ARMOR].disable)
		return;
	if (ent->client->pers.inventory[body_armor_index] < EXPLODING_ARMOR_AMOUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need at least %d armor to use this ability.\n", EXPLODING_ARMOR_AMOUNT);
		return;
	}
	if (ent->num_armor >= EXPLODING_ARMOR_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "Maximum count of %d reached.\n", EXPLODING_ARMOR_MAX_COUNT);
		return;
	}

	SpawnExplodingArmor(ent, atoi(gi.args()));
}

#define SPIKE_INITIAL_DMG	65
#define SPIKE_ADDON_DMG		25
#define SPIKE_SPEED			1800	// velocity of spike entity
#define SPIKE_COST			25		// cost to use attack
#define SPIKE_DELAY			0.1		// ability delay, in seconds
#define SPIKE_STUN_ADDON	0.05
#define SPIKE_STUN_MIN		0.3
#define SPIKE_STUN_MAX		1.5
#define SPIKE_SHOTS			3		// number of spikes fired per attack; should be an odd number
#define SPIKE_FOV			20		// attack spread, in degrees

void spike_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (G_ValidTarget(self->owner, other, false))
	{
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, 
			plane->normal, self->dmg, self->dmg, 0, MOD_SPIKE);

		gi.sound (other, CHAN_WEAPON, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);
		
		self->owner->health += 1;
		
		// only stun if the entity is alive and they haven't been stunned too recently
		if (G_EntIsAlive(other) && (level.time > (other->holdtime + 1.0)))
		{
			// stun them
			if (other->client)
			{
				other->client->ability_delay = level.time + self->dmg_radius;
				other->holdtime = level.time + self->dmg_radius;
			}
			else
			{
				other->nextthink = level.time + self->dmg_radius;
			}
		}

		
	}

	G_FreeEdict(self);
}

void fire_spike (edict_t *self, vec3_t start, vec3_t dir, int damage, float stun_length, int speed)
{
	edict_t	*bolt;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// create bolt
	bolt = G_Spawn();
//	VectorSet (bolt->mins, -4, -4, 4);
//	VectorSet (bolt->maxs, 4, 4, 4);
	bolt->s.modelindex = gi.modelindex ("models/objects/spike/tris.md2");
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->owner = self;
	bolt->touch = spike_touch;
	bolt->nextthink = level.time + 3;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->dmg_radius = stun_length;
	bolt->classname = "spike";
	gi.linkentity (bolt);

}

void SpikeAttack (edict_t *ent)
{
	int		i, move, damage;
	float	delay;
	vec3_t	angles, v, org;
	vec3_t	offset, forward, right, start;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorCopy(start, org);

	AngleVectors(ent->s.angles, NULL, right, NULL);

	// ready angles
	VectorCopy(forward, v);
	vectoangles(forward, angles);
	move = SPIKE_FOV/SPIKE_SHOTS;

	// calculate damage and stun length
	damage = SPIKE_INITIAL_DMG+SPIKE_ADDON_DMG*ent->myskills.abilities[SPIKE].current_level;

	delay = SPIKE_STUN_ADDON * ent->myskills.abilities[SPIKE].current_level;
	if (delay < SPIKE_STUN_MIN)
		delay = SPIKE_STUN_MIN;
	else if (delay > SPIKE_STUN_MAX)
		delay = SPIKE_STUN_MAX;

	// fire spikes spaced evenly across horizontal plane
	for (i=0; i<SPIKE_SHOTS; i++)
	{
		if (i < SPIKE_SHOTS/2)
		{
			angles[YAW]-=move;
			// move this spike ahead of the last one to avoid collisions
			VectorMA(org, 1, forward, org);
		}
		else if (i > SPIKE_SHOTS/2)
		{
			angles[YAW]+=move;
			VectorMA(org, -1, forward, org);
		}
		else
		{
			vectoangles(forward, angles);
			VectorCopy(start, org);
		}

		ValidateAngles(angles);
		AngleVectors(angles, v, NULL, NULL);
		fire_spike(ent, org, v, damage, delay, SPIKE_SPEED);
	}

	ent->client->pers.inventory[power_cube_index] -= SPIKE_COST;
	ent->client->ability_delay = level.time + SPIKE_DELAY;

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("brain/brnatck2.wav"), 1, ATTN_NORM, 0);
}

void Cmd_Spike_f (edict_t *ent)
{
	if (!G_CanUseAbilities(ent, ent->myskills.abilities[SPIKE].current_level, SPIKE_COST))
		return;
	if (ent->myskills.abilities[SPIKE].disable)
		return;
	SpikeAttack(ent);
}

#define PROXY_COST			25
#define PROXY_MAX_COUNT		6
#define PROXY_BUILD_TIME	1.0
#define PROXY_BASE_DMG		100
#define PROXY_ADDON_DMG		25
#define PROXY_BASE_RADIUS	100
#define PROXY_ADDON_RADIUS	5
#define PROXY_BASE_HEALTH	200
#define PROXY_ADDON_HEALTH	30

void T_RadiusDamage_Nonplayers (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod); //only affects players
void T_RadiusDamage_Players (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod); //only affects players

void proxy_remove (edict_t *self, qboolean print)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	// prepare for removal
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->think = BecomeExplosion1;
	self->nextthink = level.time + FRAMETIME;

	if (self->creator && self->creator->inuse)
	{
		self->creator->num_proxy--;

		if (print)
			safe_cprintf(self->creator, PRINT_HIGH, "%d/%d proxy grenades remaining.\n",
				self->creator->num_proxy, PROXY_MAX_COUNT);
	}
}

void proxy_explode (edict_t *self)
{
	if (self->style)//4.4 proxy needs to be rearmed
		return;

	safe_cprintf(self->creator, PRINT_HIGH, "Proxy detonated.\n");

	T_RadiusDamage_Players(self, self->creator, self->dmg, self, self->dmg_radius, MOD_PROXY);
	T_RadiusDamage_Nonplayers(self, self->creator, self->dmg * 1.5, self, self->dmg_radius * 1.5, MOD_PROXY);

	// create explosion effect
	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	self->style = 1; //4.4 change status to disarmed

	if (getTalentLevel(self->creator, TALENT_INSTANTPROXYS)) // Remove proxys when you have this talent.
		proxy_remove(self, true);
}

qboolean proxy_disabled_think (edict_t *self)
{
	// not disabled
	if (!self->style)
		return false;
	
	// set effects
	if (level.framenum & 8)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= RF_SHELL_YELLOW;
	}
	else
		self->s.effects = self->s.renderfx = 0;

	self->nextthink = level.time + FRAMETIME;
	return true;
}

void proxy_effects (edict_t *self)
{
	// flash to indicate activity
	if (level.time > self->monsterinfo.pausetime)
	{
		if (self->monsterinfo.lefty)
		{
			// flash on
			self->s.effects |= EF_COLOR_SHELL;
			if (self->creator->teamnum == RED_TEAM || ffa->value)
				self->s.renderfx |= RF_SHELL_RED;
			else
				self->s.renderfx |= RF_SHELL_BLUE;

			// toggle
			self->monsterinfo.lefty = 0;

			gi.sound(self, CHAN_VOICE, gi.soundindex("misc/beep.wav"), 1, ATTN_IDLE, 0);
		}
		else
		{
			self->s.effects = self->s.renderfx = 0; // clear all effects
			self->monsterinfo.lefty = 1;// toggle
		}

		self->monsterinfo.pausetime = level.time + 3.0;
	}
}

void proxy_think (edict_t *self)
{
	edict_t *e=NULL;

	// remove proxy if owner is invalid
	if (!G_EntIsAlive(self->creator))
	{
		proxy_remove(self, false);
		return;
	}

	if (proxy_disabled_think(self))//4.4
		return;
	
	// search for nearby targets
	while ((e = findclosestradius(e, self->s.origin, self->dmg_radius)) != NULL)
	{
		if (!G_ValidTarget(self, e, true))
			continue;
		proxy_explode(self);
		break;//4.4
	}

	proxy_effects(self);

	self->nextthink = level.time + FRAMETIME;
}

void proxy_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (G_EntIsAlive(other) && other->client && OnSameTeam(ent, other) // a player on our team
		&& other->client->pers.inventory[power_cube_index] >= 5 // has power cubes
		&& ent->style // we need rearming
		&& level.framenum > ent->monsterinfo.regen_delay1) // check delay
	{
		ent->style = 0;// change status to armed
		ent->health = ent->max_health;// restore health
		other->client->pers.inventory[power_cube_index] -= 5;
		safe_cprintf(other, PRINT_HIGH, "Proxy repaired and re-armed.\n");
		gi.sound(other, CHAN_VOICE, gi.soundindex("weapons/repair.wav"), 1, ATTN_NORM, 0);
		ent->monsterinfo.regen_delay1 = level.framenum + 20;// delay before we can rearm
		ent->nextthink = level.time + 2.0;// delay before arming again
	}
}

void proxy_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	safe_cprintf(self->creator, PRINT_HIGH, "Proxy grenade destroyed.\n");
	proxy_remove(self, true);
}

qboolean NearbyLasers (edict_t *ent, vec3_t org);
void SpawnProxyGrenade (edict_t *self, int cost, float skill_mult, float delay_mult)
{
	//int		cost = PROXY_COST*cost_mult;
	edict_t	*grenade;
	vec3_t	forward, right, offset, start, end;
	vec3_t	bmin, bmax;
	trace_t	tr;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// get starting position and forward vector
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  self->viewheight-8);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);
	// get end position
	VectorMA(start, 64, forward, end);

	// set bbox size
	VectorSet (bmin, -4, -4, -4);
	VectorSet (bmax, 4, 4, 4);

	//tr = gi.trace (start, bmin, bmax, end, self, MASK_SOLID);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SOLID);

	// can't build a laser on sky
	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return;

	if (tr.fraction == 1)
	{
		safe_cprintf(self, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	if (NearbyLasers(self, tr.endpos))
	{
		safe_cprintf(self, PRINT_HIGH, "Too close to a laser.\n");
		return;
	}

	// create bolt
	grenade = G_Spawn();

	grenade->monsterinfo.level = self->myskills.abilities[PROXY].current_level * skill_mult;

	//VectorSet (grenade->mins, -4, -4, -4);
	//VectorSet (grenade->maxs, 4, 4, 4);
	VectorCopy(bmin, grenade->mins);
	VectorCopy(bmax, grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->health = PROXY_BASE_HEALTH + PROXY_ADDON_HEALTH*self->monsterinfo.level;
	grenade->max_health = grenade->health;
	grenade->takedamage = DAMAGE_AIM;
	grenade->die = proxy_die;
	grenade->movetype = MOVETYPE_NONE;
	grenade->clipmask = MASK_SHOT;
	grenade->touch = proxy_touch;//4.4
	grenade->solid = SOLID_BBOX;
	grenade->creator = self;
	grenade->mtype = M_PROXY;//4.5
	grenade->nextthink = level.time + PROXY_BUILD_TIME * delay_mult;
	grenade->think = proxy_think;
	grenade->dmg = PROXY_BASE_DMG + PROXY_ADDON_DMG*grenade->monsterinfo.level;
	grenade->dmg_radius = PROXY_BASE_RADIUS + PROXY_ADDON_RADIUS*grenade->monsterinfo.level;
	grenade->classname = "proxygrenade";

	VectorCopy(tr.endpos, grenade->s.origin);
	VectorCopy(tr.endpos, grenade->s.old_origin);
	VectorCopy(tr.plane.normal, grenade->s.angles);

	gi.linkentity (grenade);

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((self->mtype == MORPH_FLYER && self->myskills.abilities[FLYER].current_level < 5) 
		|| (self->mtype == MORPH_CACODEMON && self->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	self->num_proxy++;
	self->client->pers.inventory[power_cube_index] -= cost;

	if (getTalentLevel(self, TALENT_INSTANTPROXYS) < 2) // No hold time for instantproxys talent.
		self->holdtime = level.time + PROXY_BUILD_TIME * delay_mult;

	self->client->ability_delay = level.time + PROXY_BUILD_TIME * delay_mult;

	safe_cprintf(self, PRINT_HIGH, "Proxy grenade built (%d/%d).\n", 
		self->num_proxy, PROXY_MAX_COUNT);
}

void RemoveProxyGrenades (edict_t *ent)
{
	edict_t *e=NULL;

	while((e = G_Find(e, FOFS(classname), "proxygrenade")) != NULL)
	{
		if (e && (e->creator == ent))
			proxy_remove(e, false);
	}

	// reset proxy counter
	ent->num_proxy = 0;
}

void Cmd_BuildProxyGrenade (edict_t *ent)
{
	int talentLevel, cost = PROXY_COST;
	float skill_mult=1.0, cost_mult=1.0, delay_mult=1.0;//Talent: Rapid Assembly & Precision Tuning

	if(ent->myskills.abilities[PROXY].disable)
		return;

	if (Q_strcasecmp (gi.args(), "count") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have %d/%d proxy grenades.\n",
			ent->num_proxy, PROXY_MAX_COUNT);
		return;
	}

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		RemoveProxyGrenades(ent);
		safe_cprintf(ent, PRINT_HIGH, "All proxy grenades removed.\n");
		return;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	//Talent: Rapid Assembly
	talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);
	if (talentLevel > 0)
		delay_mult -= 0.1 * talentLevel;
	//Talent: Precision Tuning
	else if ((talentLevel = getTalentLevel(ent, TALENT_PRECISION_TUNING)) > 0)
	{
		cost_mult -= PRECISION_TUNING_COST_FACTOR * talentLevel;
		delay_mult -= PRECISION_TUNING_DELAY_FACTOR * talentLevel;
		skill_mult += PRECISION_TUNING_SKILL_FACTOR * talentLevel;
	}
	cost *= cost_mult;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[PROXY].current_level, cost))
		return;

	if (ent->num_proxy >= PROXY_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build any more proxy grenades.\n");
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	SpawnProxyGrenade(ent, cost, skill_mult, delay_mult);
}

void bfire_think (edict_t *self)
{
	if (self->s.frame > 3)
		G_RunFrames(self, 4, 15, false); // burning frames
	else
		self->s.frame++; // ignite frames

	if (!G_EntIsAlive(self->owner) || (self->delay < level.time) || self->waterlevel) 
	{
		G_FreeEdict(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}

void bfire_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!G_EntExists(other) || OnSameTeam(self->owner, other))
		return;
	// set them on fire
	burn_person(other, self->owner, self->dmg);
	// hurt them
	if (level.framenum >= self->monsterinfo.nextattack)
	{
		T_Damage(other, self, self->owner, vec3_origin, self->s.origin, 
				plane->normal, self->dmg, 0, DAMAGE_NO_KNOCKBACK, MOD_BURN);
		self->monsterinfo.nextattack = level.framenum + 1;
	}
}

void ThrowFlame (edict_t *ent, vec3_t start, vec3_t forward, float dist, int speed, int damage, int ttl)
{
	edict_t *fire;

	// create the fire ent
	fire = G_Spawn();
	VectorCopy(start, fire->s.origin);
	VectorCopy(start, fire->s.old_origin);
	vectoangles(forward, fire->s.angles);
	fire->s.angles[PITCH] = 0; // always face up

	// toss it
	VectorScale (forward, speed, fire->velocity);
	if (ent->mtype != TOTEM_FIRE)
		fire->velocity[2] += GetRandom(0, 200);//;//GetRandom(200, 400);
	//else
	//	fire->velocity[2] += 200;

	//gi.dprintf(" velocity[2] = %.0f\n", fire->velocity[2]);
	fire->movetype = MOVETYPE_TOSS;
	fire->owner = ent;
	fire->dmg = damage;
	fire->classname = "fire";
	fire->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
	fire->delay = level.time + ttl;
	fire->think = bfire_think;
	fire->touch = bfire_touch;
	fire->nextthink = level.time+FRAMETIME;
	fire->solid = SOLID_TRIGGER;
	fire->clipmask = MASK_SHOT;
	fire->s.modelindex = gi.modelindex ("models/fire/tris.md2");
	gi.linkentity(fire);
}

void SpawnFlames (edict_t *self, vec3_t start, int num_flames, int damage, int toss_speed)
{
	int		i;
	edict_t *fire;
	vec3_t	forward;

	for (i=0; i<num_flames; i++) {
		// create the fire ent
		fire = G_Spawn();
		VectorCopy(start, fire->s.origin);
		fire->movetype = MOVETYPE_TOSS;
		fire->owner = self;
		fire->dmg = damage;
		fire->classname = "fire";
		fire->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
		fire->delay = level.time + GetRandom(3, 6);
		fire->think = bfire_think;
		fire->touch = bfire_touch;
		fire->nextthink = level.time+FRAMETIME;
		fire->solid = SOLID_TRIGGER;
		fire->clipmask = MASK_SHOT;
		fire->s.modelindex = gi.modelindex ("models/fire/tris.md2");
		gi.linkentity(fire);
		// toss it
		fire->s.angles[YAW] = GetRandom(0, 360);
		AngleVectors (fire->s.angles, forward, NULL, NULL);
		VectorScale (forward, GetRandom((int)(toss_speed*0.5), (int)(toss_speed*1.5)), fire->velocity);
		fire->velocity[2] = GetRandom(200, 400);
	}
}

#define NAPALM_MAX_COUNT		3
#define NAPALM_ATTACK_DELAY		1.0
#define NAPALM_DELAY			1.0
#define NAPALM_COST				25
#define NAPALM_DURATION			10.0
#define NAPALM_INITIAL_DMG		100
#define NAPALM_ADDON_DMG		20
#define NAPALM_INITIAL_RADIUS	100.0
#define NAPALM_ADDON_RADIUS		5.0
#define NAPALM_INITIAL_BURN		0
#define NAPALM_ADDON_BURN		1

void napalm_remove (edict_t *self, qboolean print)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	// prepare for removal
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;

	if (self->owner && self->owner->inuse)
	{
		self->owner->num_napalm--;

		if (print)
			safe_cprintf(self->owner, PRINT_HIGH, "%d/%d napalm grenades remaining.\n",
				self->owner->num_napalm, NAPALM_MAX_COUNT);
	}
}

void napalm_explode (edict_t *self)
{
	vec3_t start;

	VectorCopy(self->s.origin, start);
	start[2] += 16;

	if (!self->waterlevel)
		SpawnFlames(self->owner, start, 3, self->radius_dmg, 80);

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_NAPALM);

	// create explosion effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (start);
	gi.multicast (start, MULTICAST_PHS);
}

void napalm_effects (edict_t *self)
{
	self->s.effects = self->s.renderfx = 0;

	self->s.effects |= EF_COLOR_SHELL;
	if (self->owner->teamnum == RED_TEAM || ffa->value)
		self->s.renderfx |= RF_SHELL_RED;
	else
		self->s.renderfx |= RF_SHELL_BLUE;
}

void napalm_think (edict_t *self)
{
	if (!G_EntIsAlive(self->owner) || (level.time > self->delay))
	{
		napalm_remove(self, true);
		return;
	}

	// assign team colors
	napalm_effects(self);

	if (level.time > self->monsterinfo.attack_finished)
	{
		napalm_explode(self);
		self->monsterinfo.attack_finished = level.time + NAPALM_ATTACK_DELAY;
	}

	self->nextthink = level.time + FRAMETIME;
}

void fire_napalm (edict_t *self, vec3_t start, vec3_t aimdir,
	int explosive_damage, int burn_damage, int speed, float duration, float damage_radius)
{
	edict_t	*grenade;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// create bolt
	grenade = G_Spawn();
	VectorClear(grenade->mins);
	VectorClear(grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	VectorCopy (start, grenade->s.origin);
	VectorCopy (start, grenade->s.old_origin);
	vectoangles (aimdir, grenade->s.angles);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->s.effects |= EF_GRENADE;
	grenade->solid = SOLID_BBOX;
	grenade->owner = self;
	//grenade->touch = spike_touch;
	grenade->nextthink = level.time + 1;
	grenade->delay = level.time+duration;
	grenade->think = napalm_think;
	grenade->dmg = explosive_damage;
	grenade->dmg_radius = damage_radius;
	grenade->radius_dmg = burn_damage;

	grenade->classname = "napalm";
	gi.linkentity (grenade);

	VectorScale (aimdir, speed, grenade->velocity);
	grenade->velocity[2] += 250.0;

	self->num_napalm++;
	//gi.sound (self, CHAN_WEAPON, gi.soundindex("brain/brnatck2.wav"), 1, ATTN_NORM, 0);
}

void RemoveNapalmGrenades (edict_t *ent)
{
	edict_t *e=NULL;

	while((e = G_Find(e, FOFS(classname), "napalm")) != NULL)
	{
		if (e && (e->owner == ent))
			napalm_remove(e, false);
	}

	// reset napalm counter
	ent->num_napalm = 0;
}

void Cmd_Napalm_f (edict_t *ent)
{
	int		damage, burn_dmg, cost;
	float	radius;
	vec3_t	offset, forward, right, start;

	if(ent->myskills.abilities[NAPALM].disable)
		return;

	if (Q_strcasecmp (gi.args(), "count") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have %d/%d napalm grenades.\n",
			ent->num_napalm, NAPALM_MAX_COUNT);
		return;
	}

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		RemoveNapalmGrenades(ent);
		safe_cprintf(ent, PRINT_HIGH, "All napalm grenades removed.\n");
		return;
	}

	//Talent: Bombardier - reduces grenade cost
	cost = NAPALM_COST - getTalentLevel(ent, TALENT_BOMBARDIER);

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[NAPALM].current_level, cost))
		return;

	if (ent->num_napalm >= NAPALM_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't throw any more napalm grenades.\n");
		return;
	}

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	damage = NAPALM_INITIAL_DMG + NAPALM_ADDON_DMG*ent->myskills.abilities[NAPALM].current_level;
	burn_dmg = NAPALM_INITIAL_BURN + NAPALM_ADDON_BURN*ent->myskills.abilities[NAPALM].current_level;
	radius = NAPALM_INITIAL_RADIUS + NAPALM_ADDON_RADIUS*ent->myskills.abilities[NAPALM].current_level;

	fire_napalm(ent, start, forward, damage, burn_dmg, 400, NAPALM_DURATION, radius);

	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + NAPALM_DELAY;
}

void lightningbolt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (G_EntExists(other))
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, 
			plane->normal, self->dmg, self->dmg, 0, MOD_LIGHTNING);
	G_FreeEdict(self);
}

void lightningbolt_think (edict_t *self)
{
	// free self if our time is up
	if (level.time >= self->delay)
	{
		G_FreeEdict(self);
		return;
	}

	// rotate animation frames
	self->s.frame++;
	if (self->s.frame > 2)
		self->s.frame = 0;

	//if (!(level.framenum%2))
	//	gi.dprintf("%f\n", VectorLength(self->velocity));

	self->nextthink = level.time + FRAMETIME;
}

void fire_lightningbolt (edict_t *ent, vec3_t start, vec3_t dir, float speed, int damage)
{
	edict_t *bolt;

	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;

	// create bolt
	bolt = G_Spawn();
	bolt->s.modelindex = gi.modelindex ("models/proj/beam/tris.md2");
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_TAGTRAIL;
	//bolt->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = ent;
	bolt->think = lightningbolt_think;
	bolt->touch = lightningbolt_touch;
	bolt->nextthink = level.time + FRAMETIME;
	bolt->delay = level.time + 10;
	bolt->dmg = damage;
	bolt->classname = "lightning bolt";	
	gi.linkentity (bolt);

	/*
	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	*/
	
	gi.sound(ent, CHAN_ITEM, gi.soundindex("spells/zap2.wav"), 1, ATTN_NORM, 0);
	//ent->client->ability_delay = level.time+BOLT_DELAY;
}

#define METEOR_INITIAL_DMG		100
#define METEOR_ADDON_DMG		40
#define METEOR_INITIAL_RADIUS	200
#define METEOR_ADDON_RADIUS		0
#define METEOR_INITIAL_SPEED	1000
#define METEOR_ADDON_SPEED		0
#define METEOR_CEILING_HEIGHT	1024
#define METEOR_RANGE			8192
#define METEOR_DELAY			1.0

void meteor_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *e=NULL;

	// burn targets within explosion radius
	while ((e = findradius(e, self->s.origin, self->dmg_radius)) != NULL)
	{
		if (!G_ValidTarget(self, e, true))
			continue;
		burn_person(e, self->owner, (int)(self->dmg*0.1));
	}

	// deal radius damage
	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_METEOR);

	// create explosion effect
	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	// create flames near point of detonation
	SpawnFlames(self->owner, self->s.origin, 10, (int)(self->dmg*0.1), 100);

	// remove meteor entity
	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;

	//gi.sound(self, CHAN_VOICE, gi.soundindex("spells/meteorimpact.wav"), 1, ATTN_NORM, 0);
}

void meteor_think (edict_t *self)
{
	if (!G_EntIsAlive(self->owner) || (gi.pointcontents(self->s.origin) & MASK_SOLID))
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + FRAMETIME;
		self->touch = NULL;
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void meteor_ready (edict_t *self)
{
	self->s.effects = EF_FLAG1|EF_COLOR_SHELL;
	self->s.renderfx = RF_SHELL_RED;
	// set meteor model
	self->s.modelindex = gi.modelindex ("models/flames/lavaball/tris.md2");
	// start moving
	VectorScale(self->movedir, self->speed, self->velocity);
	// set touch
	self->touch = meteor_touch;
	// switch to final think
	self->think = meteor_think;
	self->nextthink = level.time + FRAMETIME;
}

void fire_meteor (edict_t *self, vec3_t end, int damage, int radius, int speed)
{
	vec3_t start, forward;
	trace_t tr;
	edict_t *meteor;

	// calculate starting position at ceiling height
	VectorCopy(end, start);
	start[2] += METEOR_CEILING_HEIGHT;

	tr = gi.trace (end, NULL, NULL, start, NULL, MASK_SOLID);

	// abort if we get stuck or we don't have enough room
	if (tr.startsolid || fabs(start[2]-end[2]) < 64)
		return;

	// create meteor entity
	meteor = G_Spawn();
	meteor->speed = speed;
	//VectorScale (dir, speed, meteor->velocity);
	meteor->movetype = MOVETYPE_FLYMISSILE;
	meteor->clipmask = MASK_SHOT;
	meteor->solid = SOLID_BBOX;
	meteor->owner = self;
	meteor->think = meteor_ready;

	meteor->nextthink = level.time + 1.5;
	meteor->dmg = damage;
	meteor->dmg_radius = radius;
	meteor->classname = "meteor";

	// copy meteor starting origin
	VectorCopy (tr.endpos, meteor->s.origin);
	VectorCopy (tr.endpos, meteor->s.old_origin);
	gi.linkentity (meteor);

	// calculate vector to target
	VectorSubtract(end, start, forward);
	VectorNormalize(forward);

	vectoangles (forward, meteor->s.angles);
	VectorCopy(forward, meteor->movedir);

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	gi.sound(meteor, CHAN_WEAPON, gi.soundindex("spells/meteorlaunch_short.wav"), 1, ATTN_NORM, 0);
}

void MeteorAttack (edict_t *ent, int damage, int radius, int speed, float skill_mult, float cost_mult)
{
	//edict_t *meteor;
	vec3_t	start, end, offset, forward, right;
	trace_t	tr;

	damage *= skill_mult;

	// get start position for trace
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	// get end position for trace
	VectorMA(start, METEOR_RANGE, forward, end);

	tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

	// make sure we're starting at the floor
	VectorCopy(tr.endpos, start);
	VectorCopy(start, end);
	end[2] -= 8192;
	tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

	if (tr.fraction == 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "Too far from target.\n");
		return;
	}

	VectorCopy(tr.endpos, end);

	if (distance(end, ent->s.origin) < radius+32)
		safe_cprintf(ent, PRINT_HIGH, "***** WARNING: METEOR TARGET TOO CLOSE! *****\n");

	/*
	
	// create meteor entity
	meteor = G_Spawn();
	meteor->speed = speed;
	//VectorScale (dir, speed, meteor->velocity);
	meteor->movetype = MOVETYPE_FLYMISSILE;
	meteor->clipmask = MASK_SHOT;
	meteor->solid = SOLID_BBOX;
	meteor->owner = ent;
	meteor->think = meteor_ready;
	meteor->nextthink = level.time + 2.0;
	meteor->dmg = damage;
	meteor->dmg_radius = radius;
	meteor->classname = "meteor";	

	// calculate starting position for meteor above target
	VectorCopy(end, start);
	start[2] += METEOR_CEILING_HEIGHT;
	tr = gi.trace (end, NULL, NULL, start, meteor, MASK_SOLID);
	VectorCopy(tr.endpos, start);

	if (tr.startsolid)
	{
		G_FreeEdict(meteor);
		return;
	}

	if (fabs(start[2]-end[2]) < 64)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough room to spawn meteor.\n");
		G_FreeEdict(meteor);
		return;
	}

	// copy meteor starting origin
	VectorCopy (tr.endpos, meteor->s.origin);
	VectorCopy (tr.endpos, meteor->s.old_origin);
	gi.linkentity (meteor);

	// calculate vector to target
	VectorSubtract(end, start, forward);
	VectorNormalize(forward);

	vectoangles (forward, meteor->s.angles);
	VectorCopy(forward, meteor->movedir);
	*/

	fire_meteor(ent, end, damage, radius, speed);

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	//gi.sound(meteor, CHAN_WEAPON, gi.soundindex("spells/meteorlaunch_short.wav"), 1, ATTN_NORM, 0);
	ent->client->ability_delay = level.time + METEOR_DELAY/* * cost_mult*/;
	ent->client->pers.inventory[power_cube_index] -= METEOR_COST * cost_mult;

	// calling entity made a sound, used to alert monsters
	//ent->lastsound = level.framenum;
}

void Cmd_Meteor_f (edict_t *ent, float skill_mult, float cost_mult)
{
	int damage=METEOR_INITIAL_DMG+METEOR_ADDON_DMG*ent->myskills.abilities[METEOR].current_level;
	int speed=METEOR_INITIAL_SPEED+METEOR_ADDON_SPEED*ent->myskills.abilities[METEOR].current_level;
	int radius=METEOR_INITIAL_RADIUS+METEOR_ADDON_RADIUS*ent->myskills.abilities[METEOR].current_level;
	int	cost=METEOR_COST*cost_mult;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[METEOR].current_level, cost))
		return;
	if (ent->myskills.abilities[METEOR].disable)
		return;

	MeteorAttack(ent, damage, radius, speed, skill_mult, cost_mult);

	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}

void G_SpawnParticleTrail (vec3_t start, vec3_t end, int particles, int color)
{
	int		i, spacing;
	float	dist;
	vec3_t	dir, org;

	VectorCopy(start, org);
	VectorSubtract(end, start, dir);
	dist = VectorLength(dir);
	VectorNormalize(dir);

	spacing = particles+11;

	// particle effects
	for (i=0; i<(int)dist; i+=spacing)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (particles);
		gi.WritePosition (org);
		gi.WriteDir (dir);
		gi.WriteByte (color);
		gi.multicast (org, MULTICAST_PVS);

		// move starting position forward
		VectorMA(org, spacing, dir, org);
	}
}

#define CLIGHTNING_MAX_HOPS			4
#define CLIGHTNING_COLOR			15
#define CLIGHTNING_PARTICLES		3
#define CLIGHTNING_INITIAL_DMG		50
#define CLIGHTNING_ADDON_DMG		15
#define CLIGHTNING_INITIAL_AR		256
#define CLIGHTNING_ADDON_AR			0
#define CLIGHTNING_INITIAL_HR		256
#define CLIGHTNING_ADDON_HR			0
#define CLIGHTNING_DELAY			0.3
#define CLIGHTNING_DMG_MOD			1.25

#define CL_CHECK_AND_ATTACK			1		// check if target is valid, then attack
#define CL_CHECK					2		// check if the target is valid
#define CL_ATTACK					3		// attack only

qboolean ChainLightning_Attack (edict_t *ent, edict_t *target, int damage, int mode)
{
	qboolean result=false;


	if (mode != CL_ATTACK)
	{
		if (G_ValidTargetEnt(target, false) && !OnSameTeam(ent, target))
			result = true;
	}
	else
		result = true;

	if (result && mode != CL_CHECK)
	{
		//gi.dprintf("CL did %d damage at %d\n", damage, level.framenum);
		// deal damage
		T_Damage(target, ent, ent, vec3_origin, target->s.origin, vec3_origin, 
			damage, damage, DAMAGE_ENERGY, MOD_LIGHTNING);
	}

	return result;
}

void ChainLightning (edict_t *ent, vec3_t start, vec3_t aimdir, int damage, int attack_range, int hop_range)
{
	int		i=0, hops=CLIGHTNING_MAX_HOPS;
	vec3_t	end;
	trace_t	tr;
	edict_t	*target=NULL;
	edict_t	*prev_ed[CLIGHTNING_MAX_HOPS]; // list of entities we've previously hit
	qboolean	found=false;

	memset(prev_ed, 0, CLIGHTNING_MAX_HOPS*sizeof(prev_ed[0]));

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;

	// play sound
	gi.sound(ent, CHAN_ITEM, gi.soundindex("spells/thunderbolt.wav"), 1, ATTN_NORM, 0);

	// randomize damage
	//damage = GetRandom((int)(0.5*damage), damage);

	// get ending position
	VectorMA(start, attack_range, aimdir, end);

	// trace from attacker to ending position
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	// is this a non-world entity?
	if (tr.ent && tr.ent != world)
	{
		// try to attack it
		if (ChainLightning_Attack(ent, tr.ent, damage, CL_CHECK_AND_ATTACK))
		{
			// damage is modified with each hop
			damage *= CLIGHTNING_DMG_MOD; 

			prev_ed[0] = tr.ent;
		}
		else
			return; // give up
	}
	
	
	// spawn particle trail
	G_SpawnParticleTrail(start, tr.endpos, CLIGHTNING_PARTICLES, CLIGHTNING_COLOR);

	// we didn't find an entity to jump from
	while (!prev_ed[0] && hops > 0)
	{
		hops--;

		VectorCopy(tr.endpos, start);

		// bounce away from the wall
		VectorMA(aimdir, (-2 * DotProduct(aimdir, tr.plane.normal)), tr.plane.normal, aimdir);
		VectorMA(start, attack_range, aimdir, end);
			
		// trace
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SHOT);

		// spawn particle trail
		G_SpawnParticleTrail(start, tr.endpos, CLIGHTNING_PARTICLES, CLIGHTNING_COLOR);

		// we hit nothing, give up
		if (tr.fraction == 1.0)
			return;

		// we hit an entity
		if (tr.ent && tr.ent != world)
		{
			// try to attack
			if (ChainLightning_Attack(ent, tr.ent, damage, CL_CHECK_AND_ATTACK))
			{
				// damage is modified with each hop
				damage *= CLIGHTNING_DMG_MOD; 

				prev_ed[0] = tr.ent;
				break;
			}
			else
				return;// give up
		}
		//FIXME: if we didn't hit an entity, shouldn't we modify damage again?
	}
	
	// we never hit a valid target, so give up
	if (!prev_ed[0])
		return;

	// find nearby targets and bounce between them
	while ((i<CLIGHTNING_MAX_HOPS-1) && ((target = findradius(target, prev_ed[i]->s.origin, hop_range)) != NULL))
	{
		if (target == prev_ed[0])
			continue;

		// try to attack, if successful then add entity to list
		if (ChainLightning_Attack(ent, target, 0, CL_CHECK) && visible(prev_ed[i], target))
		{
			ChainLightning_Attack(ent, target, damage, CL_ATTACK);

			// damage is modified with each hop
			damage *= CLIGHTNING_DMG_MOD; 

			G_SpawnParticleTrail(prev_ed[i]->s.origin, target->s.origin, 
				CLIGHTNING_PARTICLES, CLIGHTNING_COLOR);

			prev_ed[++i] = target;
		}
	}
}

void Cmd_ChainLightning_f (edict_t *ent, float skill_mult, float cost_mult)
{
	int damage=CLIGHTNING_INITIAL_DMG+CLIGHTNING_ADDON_DMG*ent->myskills.abilities[LIGHTNING].current_level;
	int attack_range=CLIGHTNING_INITIAL_AR+CLIGHTNING_ADDON_AR*ent->myskills.abilities[LIGHTNING].current_level;
	int hop_range=CLIGHTNING_INITIAL_HR+CLIGHTNING_ADDON_HR*ent->myskills.abilities[LIGHTNING].current_level;
	int cost=CLIGHTNING_COST*cost_mult;
	vec3_t start, forward, right, offset;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[LIGHTNING].current_level, cost))
		return;
	if (ent->myskills.abilities[LIGHTNING].disable)
		return;

	damage *= skill_mult;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	ChainLightning(ent, start, forward, damage, attack_range, hop_range);

	ent->client->ability_delay = level.time + CLIGHTNING_DELAY/* * cost_mult*/;
	ent->client->pers.inventory[power_cube_index] -= cost;
}

#define AUTOCANNON_FRAME_ATTACK_START	7
#define AUTOCANNON_FRAME_ATTACK_END		17//20
#define AUTOCANNON_FRAME_SLEEP_START	28
#define AUTOCANNON_FRAME_SLEEP_END		38

#define AUTOCANNON_STATUS_READY			0
#define AUTOCANNON_STATUS_ATTACK		1
#define AUTOCANNON_ATTACK_DELAY			1.0 // refire rate, must be >= 1.0
#define AUTOCANNON_START_DELAY			2.0 // time until the cannon can fire its first shot
#define AUTOCANNON_BUILD_TIME			1.0 // time it takes player to build the autocannon
#define AUTOCANNON_DELAY				1.0	// ability delay
#define AUTOCANNON_RANGE				2048
#define AUTOCANNON_INITIAL_HEALTH		100//250
#define AUTOCANNON_ADDON_HEALTH			40//25
#define AUTOCANNON_INITIAL_DAMAGE		100
#define AUTOCANNON_ADDON_DAMAGE			40//20
#define AUTOCANNON_YAW_SPEED			2
#define AUTOCANNON_COST					50
#define AUTOCANNON_MAX_UNITS			3
//#define AUTOCANNON_REGEN_FRAMES			300
#define AUTOCANNON_START_AMMO			5
#define AUTOCANNON_INITIAL_AMMO			5
#define AUTOCANNON_ADDON_AMMO			0
#define AUTOCANNON_REPAIR_COST			0.1
#define AUTOCANNON_TOUCH_DELAY			3.0

void autocannon_remove (edict_t *self, char *message)
{
	if (self->deadflag == DEAD_DEAD)
		return; // don't call more than once

	if (self->creator)
	{
		self->creator->num_autocannon--; // decrement counter

		if (self->creator->inuse && message)
			safe_cprintf(self->creator, PRINT_HIGH, message);
	}

	// mark for removal
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;
	self->think = BecomeExplosion1;
	self->nextthink = level.time + FRAMETIME;
	gi.unlinkentity(self);
}

void RemoveAutoCannons (edict_t *ent)
{
	edict_t *scan = NULL;

	while((scan = G_Find(scan, FOFS(classname), "autocannon")) != NULL)
	{
		// remove all autocannons owned by this entity
		if (scan && scan->inuse && scan->creator && scan->creator->client && (scan->creator == ent))
			autocannon_remove(scan, NULL);
	}

	ent->num_autocannon = 0; // make sure the counter is zeroed out
}

void autocannon_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	char *s;

	if (self->deadflag == DEAD_DEAD)
		return; // don't call more than once

	if (attacker->client)
		s = va("Your autocannon was destroyed by %s (%d/%d).\n", attacker->client->pers.netname, 
			self->creator->num_autocannon-1, AUTOCANNON_MAX_UNITS);
	else
		s = va("Your autocannon was destroyed (%d/%d).\n", self->creator->num_autocannon-1, AUTOCANNON_MAX_UNITS);
	autocannon_remove(self, s);
}

void mzfire_think (edict_t *self)
{
	int		attack_frame=AUTOCANNON_FRAME_ATTACK_START;
	float	dist=10;
	vec3_t	start, forward;

	if (!self->owner || !self->owner->inuse || (level.framenum > self->count))
	{
		G_FreeEdict(self);
		return;
	}

	if (self->owner->s.frame == attack_frame)
		dist = 2;
	else if (self->owner->s.frame == attack_frame+1)
		dist = 0;
	else if (self->owner->s.frame == attack_frame+2)
		dist = -6;

	// move into position
	VectorCopy(self->owner->s.origin, start);
	start[2] += self->owner->viewheight;
	AngleVectors(self->owner->s.angles, forward, NULL, NULL);
	VectorMA(start, self->owner->maxs[0]+dist, forward, start);
	VectorCopy(start, self->s.origin);
	gi.linkentity(self);

	// modify angles
	vectoangles(forward, forward);
	forward[PITCH] += 90;
	AngleCheck(&forward[PITCH]);
	VectorCopy(forward, self->s.angles);
	

	// play frame sequence
	G_RunFrames(self, 4, 15, false);

	self->nextthink = level.time + FRAMETIME;
}

void MuzzleFire (edict_t *self)
{
	edict_t *fire;

	// create the fire ent
	fire = G_Spawn();
	fire->movetype = MOVETYPE_NONE;
	fire->owner = self;
	fire->count = level.framenum + 2; // duration
	fire->classname = "mzfire";
	fire->think = mzfire_think;
	fire->nextthink = level.time + FRAMETIME;
	fire->s.modelindex = gi.modelindex ("models/fire/tris.md2");
}

void autocannon_attack (edict_t *self)
{
	if (level.time > self->delay)
	{
		vec3_t	start, end, forward, angles;
		trace_t	tr;

		if (!self->light_level)
		{
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->delay = level.time + AUTOCANNON_ATTACK_DELAY;
			return;
		}

		MuzzleFire(self);

		//  entity made a sound, used to alert monsters
		self->lastsound = level.framenum;

		// trace directly ahead of autocannon
		VectorCopy(self->s.origin, start);
		start[2] += self->viewheight;
		VectorCopy(self->s.angles, angles);//4.5
		angles[PITCH] = self->random;//4.5
		AngleVectors(angles, forward, NULL, NULL);
		VectorMA(start, AUTOCANNON_RANGE, forward, end);
		tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

		// fire if an enemy crosses its path
		if (G_EntExists(tr.ent))
			T_Damage (tr.ent, self, self->creator, forward, tr.endpos, tr.plane.normal, 
				self->dmg, self->dmg, DAMAGE_PIERCING, MOD_AUTOCANNON);
		
		// throw debris at impact point
		if (random() < 0.5)
			ThrowDebris (self, "models/objects/debris2/tris.md2", 3, tr.endpos);
		if (random() < 0.5)
			ThrowDebris (self, "models/objects/debris2/tris.md2", 3, tr.endpos);

		gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/sgun1.wav"), 1, ATTN_NORM, 0);

		self->style = AUTOCANNON_STATUS_ATTACK;
		self->delay = level.time + AUTOCANNON_ATTACK_DELAY;
		self->light_level--; // reduce ammo
	}
}

void autocannon_runframes (edict_t *self)
{
	if (self->style == AUTOCANNON_STATUS_ATTACK)
	{
		G_RunFrames(self, AUTOCANNON_FRAME_ATTACK_START, AUTOCANNON_FRAME_ATTACK_END, false);
		if (self->s.frame == AUTOCANNON_FRAME_ATTACK_END)
		{
			self->s.frame = 0; // reset frame
			self->style = AUTOCANNON_STATUS_READY; // reset status
		}
	}
	else
		self->s.frame = 0;
}

void autocannon_aim (edict_t *self)
{
	float	angle;
	vec3_t v;

	if (self->wait > level.time)
	{
		VectorCopy(self->move_origin, v);
		// 0.1=5-6 degrees/frame, 0.2=11-12 degrees/frame, 0.3=17-18 degrees/frame
		VectorScale(v, 0.1, v);
		VectorAdd(v, self->movedir, v);
		VectorNormalize(v);
		VectorCopy(v, self->movedir);
		vectoangles(v, self->s.angles);

		angle = self->s.angles[PITCH];
		AngleCheck(&angle);
		
		if (angle > 270)
		{
			if (angle < 350)
				self->s.angles[PITCH] = 350;
		}
		else if (angle > 10)
			self->s.angles[PITCH] = 30;

		//gi.dprintf("pitch=%d\n", (int)self->s.angles[PITCH]);
	}
	
}

qboolean autocannon_checkstatus (edict_t *self)
{
	vec3_t	end;
	trace_t tr;

	// must have live owner
	if (!G_EntIsAlive(self->creator))
		return false;

	// must be on firm ground
	VectorCopy(self->s.origin, end);
	end[2] -= abs(self->mins[2])+1;
	tr = gi.trace(self->s.origin, self->mins, self->maxs, end, NULL, MASK_SOLID);
	if (tr.fraction == 1.0)
		return false;

	return true; // everything is OK
}

qboolean autocannon_effects (edict_t *self)
{
	self->s.effects = self->s.renderfx = 0;

	if (!(level.framenum % 10) && self->light_level < 1)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= RF_SHELL_YELLOW;
		return true;
	}

	if (!(level.framenum % 20) && self->light_level < 2)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= RF_SHELL_YELLOW;
		return true;
	}

	return false;
}

void autocannon_think (edict_t *self)
{
	vec3_t	angles;//4.5 aiming angles are different than model/gun angles
	vec3_t	start, end, forward;
	trace_t tr;

	if (!autocannon_checkstatus(self))
	{
		autocannon_remove(self, NULL);
		return;
	}

	if (self->holdtime > level.time)
	{
		if (!autocannon_effects(self))
			M_SetEffects(self);

		self->nextthink = level.time + FRAMETIME;
		return;
	}

	// low ammo warning
	if (!(level.framenum%50))
	{
	if (self->light_level < 1)
		safe_cprintf(self->creator, PRINT_HIGH, "***AutoCannon is out of ammo. Re-arm with more shells.***\n");
	else if (self->light_level < 2)
		safe_cprintf(self->creator, PRINT_HIGH, "AutoCannon is low on ammo. Re-arm with more shells.\n");
	}

//	M_Regenerate(self, AUTOCANNON_REGEN_FRAMES, 10, true, false, false);
	M_ChangeYaw(self);

	if (!autocannon_effects(self))
		M_SetEffects(self);

	M_WorldEffects(self);
	//autocannon_aim(self);
	autocannon_runframes(self);

	// trace directly ahead of autocannon
	VectorCopy(self->s.origin, start);
	start[2] += self->viewheight;
	VectorCopy(self->s.angles, angles);//4.5
	angles[PITCH] = self->random;//4.5
	AngleVectors(angles, forward, NULL, NULL);
	VectorMA(start, AUTOCANNON_RANGE, forward, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (self->wait > level.time)
	{
		// draw aiming laser
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (start, MULTICAST_PHS);
	}

	
	// fire if an enemy crosses its path
	if (G_EntIsAlive(tr.ent) && !OnSameTeam(self, tr.ent))
		autocannon_attack(self);

	self->nextthink = level.time + FRAMETIME;
}

void autocannon_buildthink (edict_t *self)
{
	// play sleep frames in reverse, gun is waking up
	G_RunFrames(self, AUTOCANNON_FRAME_SLEEP_START, AUTOCANNON_FRAME_SLEEP_END, true);
	if (self->s.frame == AUTOCANNON_FRAME_SLEEP_START)
	{
		if (!autocannon_checkstatus(self))
		{
			autocannon_remove(self, NULL);
			return;
		}

		self->think = autocannon_think;
		self->movetype = MOVETYPE_NONE; // lock-down

		//self->s.frame = AUTOCANNON_FRAME_ATTACK_START+2;//REMOVE THIS
	}

	self->nextthink = level.time + FRAMETIME;
}

void autocannon_status (edict_t *self, edict_t *other)
{
	safe_cprintf(other, PRINT_HIGH, "AutoCannon Status: Health (%d/%d) Ammo (%d/%d)\n", self->health, 
		self->max_health, self->light_level, self->count);

	self->sentrydelay = level.time + AUTOCANNON_TOUCH_DELAY;
}

void autocannon_reload (edict_t *self, edict_t *other)
{
	int required_ammo = self->count-self->light_level;
	int	*player_ammo = &other->client->pers.inventory[shell_index];

	// full ammo
	if (required_ammo < 1)
		return;
	// player has no ammo
	if (*player_ammo < 1)
		return;
	
	// player has enough ammo
	if (*player_ammo >= required_ammo)
	{
		self->light_level += required_ammo;
		*player_ammo -= required_ammo;
	}
	// player has less ammo than we require
	else
	{
		self->light_level += *player_ammo;
		*player_ammo = 0;
	}

	self->sentrydelay = level.time + AUTOCANNON_TOUCH_DELAY;
	gi.sound(other, CHAN_ITEM, gi.soundindex("plats/pt1_strt.wav"), 1, ATTN_STATIC, 0);
}

void autocannon_repair (edict_t *self, edict_t *other)
{
	int *powercubes = &other->client->pers.inventory[power_cube_index];

	// need to be hurt
	if (self->health >= self->max_health)
		return;

	// need powercubes to perform repairs
	if (*powercubes < 1)
		return;

	// if the player has more than enough pc's to fill up the autocannon
	if (*powercubes * (1/AUTOCANNON_REPAIR_COST) + self->health > self->max_health)
	{
		// spend just enough pc's to repair it
		*powercubes -= (self->max_health - self->health) * AUTOCANNON_REPAIR_COST;
		self->health = self->max_health;
	}
	// use all the player's pc's to repair as much as possible
	else 
	{
		self->health += *powercubes * (1/AUTOCANNON_REPAIR_COST);
		*powercubes = 0;
	}

	self->sentrydelay = level.time + AUTOCANNON_TOUCH_DELAY;
	gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/repair.wav"), 1, ATTN_NORM, 0);
}

void autocannon_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// sanity check, only a teammate can repair/reload
	if (!other->client || !OnSameTeam(ent, other))
		return;

	// activate aiming laser
	ent->wait = level.time + 1.0;

	// time delay before next repair/reload can be performed
	if (ent->sentrydelay > level.time)
		return;

	autocannon_repair(ent, other);
	autocannon_reload(ent, other);
	autocannon_status(ent, other);
}

void CreateAutoCannon (edict_t *ent, int cost, float skill_mult, float delay_mult)
{
	int		talentLevel;
	float	ammo_mult=1.0;
	vec3_t	forward, start, end, angles;
	trace_t	tr;
	edict_t *cannon;

	cannon = G_Spawn();
	cannon->creator = ent;
	cannon->think = autocannon_buildthink;
	cannon->nextthink = level.time + AUTOCANNON_START_DELAY;
	cannon->s.modelindex = gi.modelindex ("models/des/tris.md2");
	//cannon->s.effects |= EF_PLASMA;
	cannon->s.renderfx |= RF_IR_VISIBLE;
	cannon->solid = SOLID_BBOX;
	cannon->movetype = MOVETYPE_TOSS;
	cannon->clipmask = MASK_MONSTERSOLID;
	cannon->deadflag = DEAD_NO;
	cannon->svflags &= ~SVF_DEADMONSTER;
	// NO one liked AI chasing
	//cannon->flags |= FL_CHASEABLE;
	cannon->mass = 300;
	cannon->classname = "autocannon";
	cannon->takedamage = DAMAGE_AIM;
	cannon->monsterinfo.level = ent->myskills.abilities[AUTOCANNON].current_level * skill_mult;
	cannon->light_level = AUTOCANNON_START_AMMO; // current ammo

	//Talent: Storage Upgrade
	talentLevel = getTalentLevel(ent, TALENT_STORAGE_UPGRADE);
	ammo_mult += 0.2 * talentLevel;

	cannon->count = AUTOCANNON_INITIAL_AMMO+AUTOCANNON_ADDON_AMMO*cannon->monsterinfo.level; // max ammo
	cannon->count *= ammo_mult;

	cannon->health = AUTOCANNON_INITIAL_HEALTH+AUTOCANNON_ADDON_HEALTH*cannon->monsterinfo.level;
	cannon->dmg = AUTOCANNON_INITIAL_DAMAGE+AUTOCANNON_ADDON_DAMAGE*cannon->monsterinfo.level;
	cannon->gib_health = -100;
	cannon->viewheight = 10;
	cannon->max_health = cannon->health;
	cannon->die = autocannon_die;
	cannon->touch = autocannon_touch;
	VectorSet(cannon->mins, -28, -28, -24);
	VectorSet(cannon->maxs, 28, 28, 24);
	cannon->s.skinnum = 4;
	cannon->mtype = M_AUTOCANNON;
	cannon->yaw_speed = AUTOCANNON_YAW_SPEED;
	cannon->s.angles[YAW] = ent->s.angles[YAW];
	cannon->ideal_yaw = ent->s.angles[YAW];
	cannon->s.frame = AUTOCANNON_FRAME_SLEEP_END; // cannon starts out retracted

	// starting position for autocannon
	VectorCopy(ent->s.angles, angles);
	angles[PITCH] = 0;
	AngleVectors(angles, forward, NULL, NULL);
	VectorMA(ent->s.origin, ent->maxs[1]+cannon->maxs[1]+18, forward, start);

	// check for ground
	VectorCopy(start, end);
	end[2] += cannon->mins[2]+1;
	tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SHOT);
	if (tr.fraction < 1)
	{
		// move up from ground
		VectorCopy(tr.endpos, start);
		start[2] += fabs(cannon->mins[2]) + 1;
	}

	// make sure station doesn't spawn in a solid
	tr = gi.trace(start, cannon->mins, cannon->maxs, start, NULL, MASK_SHOT);
	if (tr.contents & MASK_SHOT)
	{
		safe_cprintf (ent, PRINT_HIGH, "Can't build autocannon there.\n");
		G_FreeEdict(cannon);
		return;
	}
	
	// move into position
	VectorCopy(tr.endpos, cannon->s.origin);
	VectorCopy(cannon->s.origin, cannon->s.old_origin);
	gi.linkentity(cannon);

	ent->client->ability_delay = level.time + AUTOCANNON_DELAY * delay_mult;
	ent->holdtime = level.time + AUTOCANNON_BUILD_TIME * delay_mult;
	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->num_autocannon++; // increment counter

	safe_cprintf(ent, PRINT_HIGH, "Built %d/%d autocannons.\n", ent->num_autocannon, AUTOCANNON_MAX_UNITS);
}

#define AUTOCANNON_AIM_NEAREST	0
#define AUTOCANNON_AIM_ALL		1

void autocannon_setaim (edict_t *ent, edict_t *cannon)
{
	vec3_t	forward, right, start, offset, end;
	trace_t	tr;

	// get start position for trace
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	// get end position for trace
	VectorMA(start, 8192, forward, end);

	// get vector to the point client is aiming at and convert to angles
	tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);
	VectorSubtract(tr.endpos, cannon->s.origin, forward);

	//VectorNormalize(forward);
	//VectorCopy(forward, cannon->move_origin); // vector to target

	vectoangles(forward, forward);

	// modify pitch angle, but don't go too far
	AngleCheck(&forward[PITCH]);
	if (forward[PITCH] > 270)
	{
		if (forward[PITCH] < 330)
			forward[PITCH] = 330;
	}
	else if (forward[PITCH] > 30)
		forward[PITCH] = 30;
	cannon->random = forward[PITCH];//4.5 this is our aiming pitch

	// modify pitch angle, but don't go too far
	//AngleCheck(&forward[PITCH]);
	if (forward[PITCH] > 270)
	{
		if (forward[PITCH] < 350)
			forward[PITCH] = 350;
	}
	else if (forward[PITCH] > 10)
		forward[PITCH] = 10;
	cannon->s.angles[PITCH] = forward[PITCH];// this is our gun/model pitch

	// copy angles to autocannon
	cannon->move_angles[YAW] = forward[YAW];
	cannon->ideal_yaw = forward[YAW];
	AngleCheck(&cannon->move_angles[YAW]);
	AngleCheck(&cannon->ideal_yaw);
	
	cannon->wait = level.time + 5.0;
}

void Cmd_AutoCannonAim_f (edict_t *ent, int option)
{
	edict_t *e=NULL;

	if (option == AUTOCANNON_AIM_NEAREST)
	{
		while ((e = findclosestradius (e, ent->s.origin, 512)) != NULL)
		{
			if (!e->inuse)
				continue;
			if (e->mtype != M_AUTOCANNON)
				continue;
			if (e->creator && e->creator->inuse && e->creator == ent)
			{
				autocannon_setaim(ent, e);
				break;
			}
		}
	}
	else if (option == AUTOCANNON_AIM_ALL)
	{
		while((e = G_Find(e, FOFS(classname), "autocannon")) != NULL)
		{
			if (e && e->inuse && e->creator && e->creator->client && (e->creator==ent))
				autocannon_setaim(ent, e);
		}
	}

	safe_cprintf(ent, PRINT_HIGH, "Aiming autocannon...\n");
}

void Cmd_AutoCannon_f (edict_t *ent)
{
	int talentLevel, cost=AUTOCANNON_COST;
	float skill_mult=1.0, cost_mult=1.0, delay_mult=1.0;//Talent: Rapid Assembly & Precision Tuning
	char	*arg;

	arg = gi.args();

	if (!Q_strcasecmp(arg, "remove"))
	{
		RemoveAutoCannons(ent);
		return;
	}

	if (!Q_strcasecmp(arg, "aim"))
	{
		Cmd_AutoCannonAim_f(ent,0);
		return;
	}

	if (!Q_strcasecmp(arg, "aimall"))
	{
		Cmd_AutoCannonAim_f(ent,1);
		return;
	}

	//Talent: Rapid Assembly
	talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);
	if (talentLevel > 0)
		delay_mult -= 0.1 * talentLevel;
	//Talent: Precision Tuning
	else if ((talentLevel = getTalentLevel(ent, TALENT_PRECISION_TUNING)) > 0)
	{
		cost_mult -= PRECISION_TUNING_COST_FACTOR * talentLevel;
		delay_mult -= PRECISION_TUNING_DELAY_FACTOR * talentLevel;
		skill_mult += PRECISION_TUNING_SKILL_FACTOR * talentLevel;
	}
	cost *= cost_mult;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[AUTOCANNON].current_level, AUTOCANNON_COST))
		return;
	if (ent->myskills.abilities[AUTOCANNON].disable)
		return;

	if (ent->num_autocannon >= AUTOCANNON_MAX_UNITS)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build any more autocannons.\n");
		return;
	}

	CreateAutoCannon(ent, cost, skill_mult, delay_mult);
}

#define HAMMER_INITIAL_SPEED	400
#define HAMMER_ADDON_SPEED		5
//#define HAMMER_SPIN_RATE		108
#define HAMMER_TURN_RATE		54
#define HAMMER_INITIAL_DAMAGE	100
#define HAMMER_ADDON_DAMAGE		30
#define HAMMER_COST				10
#define HAMMER_DELAY			0.1

qboolean hammer_return (edict_t *self)
{
	// only reverse course once
	if (self->style)
		return false;

	VectorNegate(self->movedir, self->movedir);
	VectorNegate(self->velocity, self->velocity);
	self->style = 1;
	return true;
}

void hammer_think1 (edict_t *self)
{
	vec3_t v;

	// remove hammer if activator dies
	if (!G_EntIsAlive(self->activator))
	{
		if (self->activator && self->activator->inuse)
			self->activator->num_hammers--;

		BecomeTE(self);
		return;
	}

	// make hammer spin
	self->s.angles[YAW] += GetRandom(36, 108);
	AngleCheck(&self->s.angles[YAW]);

	// swing back
	if (level.framenum >= self->monsterinfo.nextattack)
		self->style = 1;
	if (self->style)
	{
		VectorSubtract(self->activator->s.origin, self->s.origin, v);
		VectorNormalize(v);
		//VectorMA(self->velocity, 100.0, v, v);
		//VectorNormalize(v);
		VectorScale (v, 400, self->velocity);
	}

	self->nextthink = level.time + FRAMETIME;
}

void hammer_think (edict_t *self)
{
	vec3_t forward;

	// remove hammer if owner dies
	if (!G_EntIsAlive(self->owner))
	{
		BecomeTE(self);
		return;
	}

	// make hammer spin
	self->s.angles[YAW] += GetRandom(36, 108);
	AngleCheck(&self->s.angles[YAW]);

	// increase hammer velocity
	if (self->speed < 2000)
		self->speed += HAMMER_ADDON_SPEED;

	// reduce turning rate to make spiral bigger
	if (self->count > 9)
	{
		if (self->count <= 0.33*HAMMER_TURN_RATE)
		{
			if (!(level.framenum%3))
				self->count--;
		}
		else if (self->count <= 0.5*HAMMER_TURN_RATE)
		{
			if (!(level.framenum%2))
				self->count--;
		}
		else
			self->count--;
	}

	// modify trajectory and velocity
	self->move_angles[YAW] += self->count;
	AngleCheck(&self->move_angles[YAW]);
	AngleVectors(self->move_angles, forward, NULL, NULL);
	VectorScale (forward, self->speed , self->velocity);

	self->nextthink = level.time + FRAMETIME;
}

void hammer_touch1 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// touched a sky brush
	if (surf && (surf->flags & SURF_SKY))
	{
		// decrement counter
		if (self->activator && self->activator->inuse)
			self->activator->num_hammers--;

		G_FreeEdict(self);
		return;
	}

	// touched a non-brush entity
	if (G_EntExists(other))
	{
		// touched owner/activator
		if (self->activator && self->activator->inuse && other == self->activator)
		{
			// decrement counter
			if (self->activator && self->activator->inuse)
				self->activator->num_hammers--;

			// refund power cubes
			self->activator->client->pers.inventory[power_cube_index] += HAMMER_COST;

			G_FreeEdict(self);
			return;
		}
		// try to hurt anyone else
		else
			T_Damage(other, self, self->activator, vec3_origin, self->s.origin, 
				plane->normal, self->dmg, self->dmg, 0, MOD_HAMMER);
	}

	// explosion effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	gi.sound (self, CHAN_WEAPON, gi.soundindex("spells/boom2.wav"), 1, ATTN_NORM, 0);

	// only bounce once
	if (self->style)
	{
		// decrement counter
		if (self->activator && self->activator->inuse)
			self->activator->num_hammers--;

		G_FreeEdict(self);
	}
	else
		self->style = 1;
}

void hammer_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

	if (G_EntExists(other))
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, 
			plane->normal, self->dmg, self->dmg, 0, MOD_HAMMER);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	gi.sound (self, CHAN_WEAPON, gi.soundindex("spells/boom2.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict(self);
}

void SpawnBlessedHammer (edict_t *ent, int boomerang_level)
{
	edict_t *hammer;
	vec3_t	forward, right, offset, start;

	//Talent: Boomerang
	if (boomerang_level > 0)
	{
		// too many hammers out
		if (ent->num_hammers + 1 > boomerang_level)
			return;

		// set-up firing parameters
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -3, ent->client->kick_origin);
		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		VectorMA(start, 40, forward, start);
	}
	else
	{
		// determine starting position
		AngleVectors(ent->s.angles, forward, right, NULL);
		VectorCopy(ent->s.origin, start);

		// adjust so hammer spirals around the owner
		VectorMA(start, 24, forward, start);
		VectorMA(start, 40, right, start);
	}

	// create hammer entity
	hammer = G_Spawn();
	VectorSet (hammer->mins, -8, -8, 0);
	VectorSet (hammer->maxs, 8, 8, 0);
	hammer->s.modelindex = gi.modelindex ("models/objects/masher/tris.md2");
	VectorCopy (start, hammer->s.origin);
	VectorCopy (start, hammer->s.old_origin);

	// set angles
	vectoangles (forward, hammer->s.angles);
	hammer->s.angles[ROLL] = crandom()*GetRandom(0, 20);
	VectorCopy(hammer->s.angles, hammer->move_angles); // controls spiral
	hammer->move_angles[PITCH] = 0;
	
	// set velocity
	VectorScale (forward, HAMMER_INITIAL_SPEED, hammer->velocity);

	hammer->clipmask = MASK_SHOT;
	hammer->solid = SOLID_BBOX;
	hammer->s.effects |= EF_COLOR_SHELL;
	hammer->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	hammer->s.sound = gi.soundindex ("spells/hammerspin.wav");

	//Talent: Boomerang
	if (boomerang_level > 0)
	{
		hammer->activator = ent;
		hammer->touch = hammer_touch1;
		hammer->think = hammer_think1;
		ent->num_hammers++;
		hammer->monsterinfo.nextattack = level.framenum + 10; // framenum when the projectile reverses course
		hammer->movetype = MOVETYPE_WALLBOUNCE;
	}
	else
	{
		hammer->owner = ent;
		hammer->touch = hammer_touch;
		hammer->think = hammer_think;
		hammer->count = HAMMER_TURN_RATE; // adjust this to make spiral bigger or smaller
		hammer->movetype = MOVETYPE_FLYMISSILE;
	}

	hammer->speed = HAMMER_INITIAL_SPEED;
	hammer->nextthink = level.time + FRAMETIME;
	hammer->dmg = HAMMER_INITIAL_DAMAGE+HAMMER_ADDON_DAMAGE*ent->myskills.abilities[HAMMER].current_level;
	hammer->classname = "hammer";	
	gi.linkentity (hammer);

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/hammerlaunch.wav"), 1, ATTN_NORM, 0);

	ent->client->ability_delay = level.time + HAMMER_DELAY;
	ent->client->pers.inventory[power_cube_index] -= HAMMER_COST;

	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}

void Cmd_BlessedHammer_f (edict_t *ent)
{
	if (!G_CanUseAbilities(ent, ent->myskills.abilities[HAMMER].current_level, HAMMER_COST))
		return;
	if (ent->myskills.abilities[HAMMER].disable)
		return;

	SpawnBlessedHammer(ent, 0);
}

//Talent: Boomerang
void Cmd_Boomerang_f (edict_t *ent)
{
	int talentLevel;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[HAMMER].current_level, HAMMER_COST))
		return;
	if (ent->myskills.abilities[HAMMER].disable)
		return;

	talentLevel = getTalentLevel(ent, TALENT_BOOMERANG);
	if (talentLevel < 1)
		return;

	SpawnBlessedHammer(ent, talentLevel);
}

#define BLACKHOLE_COST			50
#define BLACKHOLE_DELAY			10.0
#define BLACKHOLE_EXIT_TIME		30.0

void wormhole_in (edict_t *self, edict_t *other)
{
	
}

void wormhole_out (edict_t *self, edict_t *other)
{

}

void wormhole_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other || !other->inuse)
		return;

	// point to monster's pilot
	if (PM_MonsterHasPilot(other))
		other = other->activator;

	if ((other == self->creator) || IsAlly(self->creator, other))
	{
		float time;
		
		// can't enter wormhole with the flag
		if (HasFlag(other))
			return;
		
		// can't enter wormhole while being hurt
		if (other->lasthurt + DAMAGE_ESCAPE_DELAY > level.time)
			return;
		
		// can't enter wormhole while cursed (this includes healing, bless)
		if (que_typeexists(other->curses, -1))
			return;

		// can't stay in wormhole long if we're warring
		if (SPREE_WAR == true && SPREE_DUDE == other)
			time = 10.0;
		else
			time = BLACKHOLE_EXIT_TIME;

		// reset railgun sniper frames
		other->client->refire_frames = 0;

		VortexRemovePlayerSummonables(other);
		V_RestoreMorphed(other, 50); // un-morph

		other->flags |= FL_WORMHOLE;
		other->movetype = MOVETYPE_NOCLIP;
		other->svflags |= SVF_NOCLIENT;
		other->client->wormhole_time = level.time + BLACKHOLE_EXIT_TIME; // must exit wormhole by this time

		self->nextthink = level.time + FRAMETIME; // close immediately
	}
}

void wormhole_close (edict_t *self)
{
	if (self->s.frame > 0)
	{
		self->s.frame--;
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	G_FreeEdict(self);
}

void wormhole_think (edict_t *self)
{

}

void wormhole_out_ready (edict_t *self)
{
	vec3_t	mins, maxs;
	trace_t tr;

	self->nextthink = level.time + FRAMETIME;

	// wormhole begins to open up
	if (self->s.frame == 0)
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex("spells/portalcast.wav"), 1, ATTN_NORM, 0);

		// delay before we can exit the wormhole
		self->monsterinfo.lefty = level.framenum + 10;
	}

	// wormhole not fully opened
	if (level.framenum < self->monsterinfo.lefty)
	{
		// hold creator in-place until wormhole is fully opened
		self->creator->holdtime = level.time + FRAMETIME;

		if (self->s.frame < 4)
			self->s.frame++;
		return;
	}

	// hold creator in-place
	self->creator->holdtime = level.time + 0.5;

	// don't get stuck in a solid
	VectorSet (mins, -16, -16, -24);
	VectorSet (maxs, 16, 16, 32);
	tr = gi.trace(self->s.origin, mins, maxs, self->s.origin, self->creator, MASK_SHOT);
	if (tr.fraction < 1 || gi.pointcontents(self->s.origin) & CONTENTS_SOLID 
		|| !ValidTeleportSpot(self, self->s.origin))
	{
		self->think = wormhole_close;
		return;
	}

	gi.sound (self, CHAN_WEAPON, gi.soundindex("spells/portalenter.wav"), 1, ATTN_NORM, 0);

	//  entity made a sound, used to alert monsters
	self->creator->lastsound = level.framenum;

	// spawn player at wormhole location
	VectorCopy(self->s.origin, self->creator->s.origin);
	gi.linkentity(self->creator);
	VectorClear(self->creator->velocity);
	self->creator->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	self->creator->client->ps.pmove.pm_time = 14;

	self->creator->movetype = MOVETYPE_WALK;
	self->creator->flags &= ~FL_WORMHOLE;
	self->creator->svflags &= ~SVF_NOCLIENT;

	self->think = wormhole_close;

	self->creator->client->ability_delay = level.time + (ctf->value?2.0:1.0);
	self->creator->client->pers.inventory[power_cube_index] -= BLACKHOLE_COST;
}

void wormhole_in_ready (edict_t *self)
{
	if (self->s.frame < 4)
	{
		if (self->s.frame == 0)
			gi.sound (self, CHAN_WEAPON, gi.soundindex("spells/portalcast.wav"), 1, ATTN_NORM, 0);

		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	// make wormhole semi-solid, ready for entry
	VectorSet(self->mins, -16, -16, -16);
	VectorSet(self->maxs, 16, 16, 16);
	self->solid = SOLID_TRIGGER;
	self->touch = wormhole_touch;
	self->think = wormhole_close;
	self->nextthink = level.time + 1.0;
	gi.linkentity(self);
}

void SpawnWormhole (edict_t *ent, int type)
{
	edict_t	*hole;
	vec3_t	offset, forward, right, start;
    
//	if (!ent->myskills.administrator)
//		return;

	if (ent->flags & FL_WORMHOLE)
	{
		type = 0;
		ent->holdtime = level.time + 1.0;
	}

	// set-up firing parameters
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// create wormhole
	hole = G_Spawn();
	hole->s.modelindex = gi.modelindex ("models/mad/hole/tris.md2");
	VectorCopy (start, hole->s.origin);
	VectorCopy (start, hole->s.old_origin);
	hole->movetype = MOVETYPE_NONE;
	//hole->s.sound = gi.soundindex ("misc/lasfly.wav");
	hole->creator = ent;

	if (type)	// wormhole is an entrance
		hole->think = wormhole_in_ready;
	else		// wormhole is an exit
		hole->think = wormhole_out_ready;

	hole->nextthink = level.time + 0.6;
	hole->classname = "wormhole";
	gi.linkentity (hole);

	// write a nice effect so everyone knows we've cast a spell
	if (type)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_TELEPORT_EFFECT);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		//  entity made a sound, used to alert monsters
		ent->lastsound = level.framenum;
	}
	
	//gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/portalcast.wav"), 1, ATTN_NORM, 0);
//	ent->client->pers.inventory[power_cube_index] -= BLACKHOLE_COST;
//	ent->client->ability_delay = level.time + BLACKHOLE_DELAY;
}

void Cmd_WormHole_f (edict_t *ent)
{
	// allow wormhole exit
	if (ent->flags & FL_WORMHOLE)
		SpawnWormhole(ent, 0);

	if (!V_CanUseAbilities(ent, BLACKHOLE, BLACKHOLE_COST, true))
		return;
	if (ent->myskills.abilities[BLACKHOLE].disable)
		return;

	SpawnWormhole(ent, 1);
}

#define CALTROPS_INITIAL_DAMAGE				50
#define CALTROPS_ADDON_DAMAGE				25
#define CALTROPS_INITIAL_SLOW				0
#define CALTROPS_ADDON_SLOW					0.1
#define CALTROPS_INITIAL_SLOWED_TIME		0
#define CALTROPS_ADDON_SLOWED_TIME			1.0
#define CALTROPS_DURATION					120.0
#define CALTROPS_COST						10
#define CALTROPS_DELAY						0.2
#define CALTROPS_MAX_COUNT					10

void caltrops_remove (edict_t *self)
{
	if (self->owner && self->owner->inuse)
		self->owner->num_caltrops--;

	G_FreeEdict(self);
}

void caltrops_removeall (edict_t *ent)
{
	edict_t *e = NULL;

	while((e = G_Find(e, FOFS(classname), "caltrops")) != NULL) 
	{
		if (e && e->owner && (e->owner == ent))
			caltrops_remove(e);
	}

	ent->num_caltrops = 0;
}

void caltrops_think (edict_t *self)
{
	if ((level.time > self->delay) || !G_EntIsAlive(self->owner))
	{
		caltrops_remove(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void caltrops_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (G_EntExists(other) && !OnSameTeam(self->owner, other))
	{
		// 50% slowed at level 10 for 5 seconds
		other->slowed_factor = 1 / (1 + CALTROPS_INITIAL_SLOW + (CALTROPS_ADDON_SLOW * self->monsterinfo.level));
		other->slowed_time = level.time + (CALTROPS_INITIAL_SLOWED_TIME + (CALTROPS_ADDON_SLOWED_TIME * self->monsterinfo.level));

		T_Damage(other, self, self->owner, self->velocity, self->s.origin, 
			plane->normal, self->dmg, self->dmg, DAMAGE_NO_KNOCKBACK, MOD_CALTROPS);

		gi.sound (other, CHAN_BODY, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);
		
		caltrops_remove(self);
	}
}

void ThrowCaltrops (edict_t *self, vec3_t start, vec3_t forward, int slevel, float duration)
{
	edict_t *caltrops;

	caltrops = G_Spawn();
	VectorCopy(start, caltrops->s.origin);
	caltrops->movetype = MOVETYPE_TOSS;
	caltrops->owner = self;
	caltrops->monsterinfo.level = slevel;
	caltrops->dmg = CALTROPS_INITIAL_DAMAGE + CALTROPS_ADDON_DAMAGE * slevel;
	caltrops->classname = "caltrops";
	caltrops->think = caltrops_think;
	caltrops->touch = caltrops_touch;
	caltrops->nextthink = level.time + FRAMETIME;
	caltrops->delay = level.time + duration;
	VectorSet(caltrops->maxs, 8, 8, 8);
	caltrops->s.angles[PITCH] = -90;
	caltrops->solid = SOLID_TRIGGER;
	caltrops->clipmask = MASK_SHOT;
	caltrops->s.modelindex = gi.modelindex ("models/spike/tris.md2");
	gi.linkentity(caltrops);

	VectorScale(forward, 200, caltrops->velocity);

	self->client->pers.inventory[power_cube_index] -= CALTROPS_COST;
	self->client->ability_delay = level.time + CALTROPS_DELAY;
	self->num_caltrops++;

	safe_cprintf(self, PRINT_HIGH, "Caltrops deployed: %d/%d\n", self->num_caltrops, CALTROPS_MAX_COUNT);

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;
}

void Cmd_Caltrops_f (edict_t *ent)
{
	vec3_t forward, start;

	if (!Q_strcasecmp(gi.args(), "remove"))
	{
		caltrops_removeall(ent);
		safe_cprintf(ent, PRINT_HIGH, "All caltrops removed.\n");
		return;
	}

	if (!V_CanUseAbilities(ent, CALTROPS, CALTROPS_COST, true))
		return;

	if (ent->num_caltrops >= CALTROPS_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You've reached the maximum number of caltrops (%d).\n", CALTROPS_MAX_COUNT);
		return;
	}

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight - 8;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 24, forward, start);
	ThrowCaltrops(ent, start, forward, ent->myskills.abilities[CALTROPS].current_level, CALTROPS_DURATION);
}



/////////////////////////////////////////////////////////////////// FMEDICPACK /////////v_cmd.c  g_misc.c///////////////////////////////////
#define FMEDICPACK_INITIAL_DAMAGE				 50
#define FMEDICPACK_ADDON_DAMAGE					 25
#define FMEDICPACK_INITIAL_SLOW					  0
#define FMEDICPACK_ADDON_SLOW					0.1
#define FMEDICPACK_INITIAL_SLOWED_TIME			  0
#define FMEDICPACK_ADDON_SLOWED_TIME			0.1
#define FMEDICPACK_DURATION						 60
#define FMEDICPACK_COST						     15
#define FMEDICPACK_DELAY						0.1
#define FMEDICPACK_MAX_COUNT					 8

// editado
void fmedi_remove(edict_t *self)
{
	if (self->owner && self->owner->inuse)
		self->owner->num_fmedi--;

	G_FreeEdict(self);
}

void fmedi_removeall(edict_t *ent)
{
	edict_t *e = NULL;

	while ((e = G_Find(e, FOFS(classname), "medic pack")) != NULL)
	{
		if (e && e->owner && (e->owner == ent))
			fmedi_remove(e);
	}

	ent->num_fmedi = 0;
}

void fmedi_think(edict_t *self)
{
	if ((level.time > self->delay) || !G_EntIsAlive(self->owner))
	{
		fmedi_remove(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void fmedi_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (G_EntExists(other) && !OnSameTeam(self->owner, other))
	{
		// 50% slowed at level 10 for 5 seconds
		other->slowed_factor = 1 / (1 + FMEDICPACK_INITIAL_SLOW + (FMEDICPACK_ADDON_SLOW * self->monsterinfo.level));
		other->slowed_time = level.time + (FMEDICPACK_INITIAL_SLOWED_TIME + (FMEDICPACK_ADDON_SLOWED_TIME * self->monsterinfo.level));

		T_Damage(other, self, self->owner, self->velocity, self->s.origin,
			plane->normal, self->dmg, self->dmg, DAMAGE_NO_KNOCKBACK, MOD_FMEDICPACK);

		gi.sound(other, CHAN_BODY, gi.soundindex("misc/n_health.wav"), 1, ATTN_NORM, 0);

		fmedi_remove(self);
	}
}

void Throwfmedi(edict_t *self, vec3_t start, vec3_t forward, int slevel, float duration)
{
	edict_t *fmedi;

	fmedi = G_Spawn();
	VectorCopy(start, fmedi->s.origin);
	fmedi->movetype = MOVETYPE_TOSS;
	fmedi->owner = self;
	fmedi->monsterinfo.level = slevel;
	fmedi->dmg = FMEDICPACK_INITIAL_DAMAGE + FMEDICPACK_ADDON_DAMAGE * slevel; // editar despues
	fmedi->classname = "medic_Pack"; // medic con miniscula no se por que
	fmedi->think = fmedi_think;
	fmedi->touch = fmedi_touch;
	fmedi->nextthink = level.time + FRAMETIME;
	fmedi->delay = level.time + duration;
	VectorSet(fmedi->mins, -15, -15, -15);
	VectorSet(fmedi->maxs, 15, 15, 15);
	fmedi->s.angles[PITCH] = 0;
	fmedi->solid = SOLID_TRIGGER;
	fmedi->s.modelindex = gi.modelindex("models/fmedi/tris.md2"); // modificar modelo  models/spike/tris.md2
	gi.linkentity(fmedi);
	fmedi->clipmask = MASK_SHOT;











	VectorScale(forward, 200, fmedi->velocity);

	self->client->pers.inventory[power_cube_index] -= FMEDICPACK_COST;
	self->client->ability_delay = level.time + FMEDICPACK_DELAY;
	self->num_fmedi++;

	safe_cprintf(self, PRINT_HIGH, "Medic Pack's used: %d/%d\n", self->num_fmedi, FMEDICPACK_MAX_COUNT);

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;
}

void Cmd_fmedi_f(edict_t *ent)
{
	vec3_t forward, start;

	if (!Q_strcasecmp(gi.args(), "remove"))
	{
		fmedi_removeall(ent);
		safe_cprintf(ent, PRINT_HIGH, "All Medic Pack's removed.\n");
		return;
	}

	if (!V_CanUseAbilities(ent, FMEDICPACK, FMEDICPACK_COST, true))
		return;

	if (ent->num_fmedi >= FMEDICPACK_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You've reached the maximum number of Medic Pack's (%d).\n", FMEDICPACK_MAX_COUNT);
		return;
	}

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8, forward, start);
	Throwfmedi(ent, start, forward, ent->myskills.abilities[FMEDICPACK].current_level, FMEDICPACK_DURATION);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define SPIKEGRENADE_COST				25
#define SPIKEGRENADE_DELAY				1.0
#define SPIKEGRENADE_DURATION			10.0
#define SPIKEGRENADE_INITIAL_DAMAGE		50
#define SPIKEGRENADE_ADDON_DAMAGE		15
#define SPIKEGRENADE_INITIAL_SPEED		600
#define SPIKEGRENADE_ADDON_SPEED		0
#define SPIKEGRENADE_TURN_DEGREES		3
#define SPIKEGRENADE_TURN_DELAY			1.0
#define SPIKEGRENADE_TURNS				8
#define SPIKEGRENADE_MAX_COUNT			3

void spikegren_remove (edict_t *self)
{
	if (self->owner && self->owner->inuse)
	{
		self->owner->num_spikegrenades--;
		safe_cprintf(self->owner, PRINT_HIGH, "%d/%d spike grenades remaining\n", self->owner->num_spikegrenades, SPIKEGRENADE_MAX_COUNT);
	}

	G_FreeEdict(self);
}

void spikegren_removeall (edict_t *ent)
{
	edict_t *e = NULL;

	while((e = G_Find(e, FOFS(classname), "spikegren")) != NULL) 
	{
		if (e && e->owner && (e->owner == ent))
			spikegren_remove(e);
	}

	ent->num_spikegrenades = 0;
}

void spikey_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if ((surf && (surf->flags & SURF_SKY)) || !self->creator || !self->creator->inuse)
	{
		G_FreeEdict(self);
		return;
	}

	if (other->takedamage)
	{
		T_Damage(other, self, self->creator, self->velocity, self->s.origin, 
			plane->normal, self->dmg, self->dmg, 0, MOD_SPIKEGRENADE);

		gi.sound (other, CHAN_WEAPON, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);
	}

	G_FreeEdict(self);
}

void fire_spikey (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;

	// create bolt
	bolt = G_Spawn();
	bolt->s.modelindex = gi.modelindex ("models/objects/spike/tris.md2");
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	//bolt->owner = self;
	bolt->creator = self;
	bolt->touch = spikey_touch;
	bolt->nextthink = level.time + 5;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "spikey";
	gi.linkentity (bolt);
}

void spikegren_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		spikegren_remove(self);
		return;
	}
}

void spikegren_explode (edict_t *self)
{
	int		speed, i;
	vec3_t	forward, start;

	if ((level.time > self->delay) || !G_EntIsAlive(self->owner))
	{
		spikegren_remove(self);
		return;
	}

	if (level.time > self->monsterinfo.attack_finished)
	{
		speed = SPIKEGRENADE_INITIAL_SPEED + SPIKEGRENADE_ADDON_SPEED * self->monsterinfo.level;

		G_EntMidPoint(self, start);

		for (i = 0; i < SPIKEGRENADE_TURNS; i++)
		{
			self->s.angles[YAW] += 45;
			AngleCheck(&self->s.angles[YAW]);

			AngleVectors(self->s.angles, forward, NULL, NULL);
			fire_spikey(self->owner, start, forward, self->dmg, speed);
		}

		self->monsterinfo.attack_finished = level.time + SPIKEGRENADE_TURN_DELAY;
	}

	self->s.angles[YAW] += SPIKEGRENADE_TURN_DEGREES;
	AngleCheck(&self->s.angles[YAW]);

	self->nextthink = level.time + FRAMETIME;
}

void spikegren_think (edict_t *self)
{
	if (!G_EntIsAlive(self->owner) || gi.pointcontents(self->s.origin) & MASK_SOLID)
	{
		spikegren_remove(self);
		return;
	}

	if (self->velocity[0] == 0 && self->velocity[1] == 0 && self->velocity[2] == 0)
	{
		trace_t	tr;
		vec3_t	end;

	//	gi.dprintf("%.1f %.1f %.1f\n", self->mins[0], self->mins[1], self->mins[2]);
	//	gi.dprintf("%.1f %.1f %.1f\n", self->maxs[0], self->maxs[1], self->maxs[2]);

		self->delay = level.time + self->delay;	// start the timer
		self->think = spikegren_explode;

		// move into position
		VectorCopy(self->s.origin, end);
		end[2] += 32;
		tr = gi.trace(self->s.origin, NULL, NULL, end, self, MASK_SHOT);
		VectorCopy(tr.endpos, self->s.origin);
		gi.linkentity(self);
		
		// hold in-place
		self->movetype = MOVETYPE_NONE;
	}

	M_SetEffects(self);//4.5

	self->nextthink = level.time + FRAMETIME;
}

void spikegren_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	// explosion effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	safe_cprintf(self->owner, PRINT_HIGH, "Spike grenade destroyed.\n");

	spikegren_remove(self);
}

void ThrowSpikeGrenade (edict_t *self, vec3_t start, vec3_t forward, int slevel, float duration)
{
	edict_t *grenade;

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	grenade = G_Spawn();
	VectorSet (grenade->mins, -10, -10, -16);
	VectorSet (grenade->maxs, 10, 10, 4);

	// don't get stuck in a floor/wall
	if (!G_IsValidLocation(self, start, grenade->mins, grenade->maxs))
	{
		G_FreeEdict(self);
		return;
	}

	VectorCopy(start, grenade->s.origin);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->health = grenade->max_health = 100;//4.4
	grenade->takedamage = DAMAGE_AIM;//4.4
	grenade->die = spikegren_die;//4.4
	grenade->owner = self;
	grenade->monsterinfo.level = slevel;
	grenade->mtype = M_SPIKE_GRENADE;
	grenade->dmg = SPIKEGRENADE_INITIAL_DAMAGE + SPIKEGRENADE_ADDON_DAMAGE * slevel;

	grenade->classname = "spikegren";
	grenade->think = spikegren_think;
	grenade->touch = spikegren_touch;
	grenade->nextthink = level.time + FRAMETIME;
	grenade->delay = duration;
	grenade->solid = SOLID_BBOX;//SOLID_TRIGGER;
	grenade->clipmask = MASK_SHOT;
	grenade->s.modelindex = gi.modelindex ("models/items/ammo/grenades/medium/tris.md2");
	gi.linkentity(grenade);

	VectorScale(forward, 400, grenade->velocity);
	self->num_spikegrenades++;
}

void Cmd_SpikeGrenade_f (edict_t *ent)
{
	int cost;
	vec3_t forward, start;

	if (ent->num_spikegrenades >= SPIKEGRENADE_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You've reached the maximum number of spike grenades (%d)\n", SPIKEGRENADE_MAX_COUNT);
		return;
	}

	//Talent: Bombardier - reduces grenade cost
	cost = SPIKEGRENADE_COST - getTalentLevel(ent, TALENT_BOMBARDIER);

	if (!V_CanUseAbilities(ent, SPIKE_GRENADE, cost, true))
		return;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight - 8;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 24, forward, start);

	// don't allow them to throw through thin walls
	if (!G_IsClearPath(ent, MASK_SHOT, ent->s.origin, start))
		return;

	ThrowSpikeGrenade(ent, start, forward, ent->myskills.abilities[SPIKE_GRENADE].current_level, SPIKEGRENADE_DURATION);

	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + SPIKEGRENADE_DELAY;
}

#define DETECTOR_COST				25
#define DETECTOR_DELAY				2.0
#define DETECTOR_MAX_COUNT			3
#define DETECTOR_INITIAL_HEALTH		50
#define DETECTOR_ADDON_HEALTH		10
#define DETECTOR_INITIAL_RANGE		96
#define DETECTOR_ADDON_RANGE		16
#define DETECTOR_DURATION			120.0		// how long the detector will last
#define DETECTOR_FLAG_DURATION		1.0			// how long a player has the FL_DETECTED flag
#define DETECTOR_GLOW_TIME			1.0			// seconds detector will glow after finding a target

//Talent: Alarm (5 levels)
#define ALARM_INITIAL_HEALTH		0
#define ALARM_ADDON_HEALTH			200
#define ALARM_INITIAL_RANGE			0
#define ALARM_ADDON_RANGE			76.8

void detector_remove (edict_t *self)
{
	if (self->owner && self->owner->inuse)
	{
		self->owner->num_detectors--;
		safe_cprintf(self->owner, PRINT_HIGH, "%d/%d detectors remaining\n", self->owner->num_detectors, DETECTOR_MAX_COUNT);
	}

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_NOCLIENT;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;
}

void detector_removeall (edict_t *ent)
{
	edict_t *e = NULL;

	while((e = G_Find(e, FOFS(classname), "detector")) != NULL) 
	{
		if (e && e->owner && (e->owner == ent))
			detector_remove(e);
	}

	ent->num_detectors = 0;
}

edict_t *detector_findprojtarget (edict_t *self, edict_t *projectile)
{
	edict_t *target=NULL;

	// find enemy that is closest to the projectile
	while ((target = findclosestradius(target, projectile->s.origin, self->dmg_radius)) != NULL)
	{
		// valid target must be within range of the detector
		if (G_ValidTarget(self, target, true) && entdist(self, target) < self->dmg_radius)
			return target;
	}
	return NULL;
}

void ProjectileLockon (edict_t *proj)
{
	vec3_t forward, start;

	G_EntMidPoint(proj->enemy, start);
	VectorSubtract(start, proj->s.origin, forward);
	VectorNormalize(forward);
	VectorCopy (forward, proj->movedir);
	vectoangles (forward, proj->s.angles);
	VectorScale (forward, VectorLength(proj->velocity), proj->velocity);
}

void detector_findprojectile (edict_t *self, char *className)
{
	edict_t *e=NULL, *proj_target;

	while((e = G_Find(e, FOFS(classname), className)) != NULL)
	{
		// only use friendly projectiles
		if (e->owner && e->owner->inuse && !OnSameTeam(self->owner, e->owner))
			continue;

		// find a projectile that is within range of the detector
		if (entdist(self, e) > self->dmg_radius)
			continue;
	
		// this projectile already has a target that is within range
		if (e->enemy && e->enemy->inuse && entdist(self, e->enemy) < self->dmg_radius)
		{
			// lock-on to enemy
			ProjectileLockon(e);
			continue;
		}
		
		// find a new target for the projectile
		if ((proj_target = detector_findprojtarget(self, e)) != NULL)
		{
			e->enemy = proj_target;
			ProjectileLockon(e);
		}
	}
}

qboolean detector_findtarget (edict_t *self)
{
	qboolean	foundTarget=false;
	edict_t		*target=NULL;

	while ((target = findradius(target, self->s.origin, self->dmg_radius)) != NULL)
	{
		// sanity check
		if (!target || !target->inuse || !target->takedamage || target->solid == SOLID_NOT)
			continue;
		// don't target anything dead
		if (target->deadflag == DEAD_DEAD || target->health < 1)
			continue;
		// don't target spawning players
		if (target->client && (target->client->respawn_time > level.time))
			continue;
		// don't target players in chat-protect
		if (!ptr->value && target->client && (target->flags & FL_CHATPROTECT))
			continue;
		if (target->flags & FL_GODMODE)
			continue;
		// don't target spawning world monsters
		if (target->activator && !target->activator->client && (target->svflags & SVF_MONSTER) 
			&& (target->deadflag != DEAD_DEAD) && (target->nextthink-level.time > 2*FRAMETIME))
			continue;
		// don't target teammates
		if (OnSameTeam(self->owner, target))
			continue;
		// visiblity check
		if (!visible(self, target))
			continue;

		// flag them as detected
		target->flags |= FL_DETECTED;
		target->detected_time = level.time + DETECTOR_FLAG_DURATION;
		foundTarget = true;

		// decloak them
		if (target->client)
		{
			target->client->idle_frames = 0;
			target->client->cloaking = false;
		}

		target->svflags &= ~SVF_NOCLIENT;
	}

	return foundTarget;
}

void detector_effects (edict_t *self)
{
	// team colors
	self->s.effects |= EF_COLOR_SHELL;
	if (self->owner->teamnum == BLUE_TEAM)
		self->s.renderfx |= RF_SHELL_BLUE;
	else if (self->owner->teamnum == RED_TEAM)
		self->s.renderfx |= RF_SHELL_RED;
	else if (self->monsterinfo.attack_finished > level.time)
		self->s.renderfx |= RF_SHELL_YELLOW;
	else if (!(level.framenum & 8))
		self->s.renderfx |= RF_SHELL_RED;
	else
		self->s.effects = self->s.renderfx = 0;
}

void detector_think (edict_t *self)
{
	qboolean expired=false;

	if (level.time > self->delay)
		expired = true;

	if (expired || !G_EntIsAlive(self->owner))
	{
		if (expired && self->owner && self->owner->inuse)
			safe_cprintf(self->owner, PRINT_HIGH, "A detector timed-out. (%d/%d)\n", self->owner->num_detectors, DETECTOR_MAX_COUNT);

		detector_remove(self);
		return;
	}

	if (detector_findtarget(self))
	{
		// play a sound to alert players
		if (level.time > self->sentrydelay)
		{
			gi.sound(self->owner, CHAN_VOICE, gi.soundindex("detector/alarm3.wav"), 1, ATTN_NORM, 0);
			self->sentrydelay = level.time + 1.0;
		}

		// glow for awhile
		self->monsterinfo.attack_finished = level.time + DETECTOR_GLOW_TIME;
	}

	detector_effects(self);

	// find projectiles that are within range and redirect them towards nearest enemy
	if (self->mtype == M_DETECTOR)
	{
		detector_findprojectile(self, "rocket");
		detector_findprojectile(self, "bolt");
		detector_findprojectile(self, "bfg blast");
		detector_findprojectile(self, "magicbolt");
		detector_findprojectile(self, "grenade");
		detector_findprojectile(self, "hgrenade");
		detector_findprojectile(self, "skull");
		detector_findprojectile(self, "spike");
		detector_findprojectile(self, "spikey");
		detector_findprojectile(self, "fireball");
		detector_findprojectile(self, "plasma bolt");
		detector_findprojectile(self, "acid");
	}

	self->nextthink = level.time + FRAMETIME;
}

void detector_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	safe_cprintf(self->owner, PRINT_HIGH, "A detector was destroyed. (%d/%d)\n", self->owner->num_detectors, DETECTOR_MAX_COUNT);
	detector_remove(self);
}

void BuildDetector (edict_t *self, vec3_t start, vec3_t forward, int slvl, float duration, int cost, float delay_mult)
{
	edict_t *detector;
	trace_t	tr;
	vec3_t	end;

	detector = G_Spawn();
	VectorSet (detector->mins, -4, -4, -4);
	VectorSet (detector->maxs, 4, 4, 4);
	VectorCopy(start, detector->s.origin);
	detector->movetype = MOVETYPE_NONE;
	detector->owner = self;
	detector->monsterinfo.level = slvl;

	detector->classname = "detector";
	detector->think = detector_think;
	detector->nextthink = level.time + FRAMETIME;
	detector->solid = SOLID_BBOX;
	detector->takedamage = DAMAGE_AIM;
	
	detector->die = detector_die;
	detector->clipmask = MASK_SHOT;

	//Talent: Alarm
	if (duration)
	{
		detector->mtype = M_DETECTOR;
		detector->health = detector->max_health = DETECTOR_INITIAL_HEALTH + DETECTOR_ADDON_HEALTH * slvl;
		detector->delay = level.time + duration;
		detector->dmg_radius = DETECTOR_INITIAL_RANGE + DETECTOR_ADDON_RANGE * slvl;
	}
	else
	{
		detector->mtype = M_ALARM;
		detector->flags |= FL_NOTARGET;
		detector->health = ALARM_INITIAL_HEALTH + ALARM_ADDON_HEALTH * slvl;
		detector->delay = level.time + 9999.0;
		detector->dmg_radius = ALARM_INITIAL_RANGE + ALARM_ADDON_RANGE * slvl;
	}

	detector->s.modelindex = gi.modelindex ("models/objects/detector/tris.md2");

	// get end position
	VectorMA(start, 64, forward, end);

	tr = gi.trace (start, detector->mins, detector->maxs, end, self, MASK_SOLID);

	// can't build on a sky brush
	if (tr.surface && (tr.surface->flags & SURF_SKY))
	{
		G_FreeEdict(detector);
		return;
	}

	if (tr.fraction == 1)
	{
		G_FreeEdict(detector);
		safe_cprintf(self, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	VectorCopy(tr.endpos, detector->s.origin);
	VectorCopy(tr.endpos, detector->s.old_origin);
	vectoangles(tr.plane.normal, detector->s.angles);
	detector->s.angles[PITCH] += 90;
	AngleCheck(&detector->s.angles[PITCH]);
	gi.linkentity(detector);

	self->num_detectors++;
	self->client->ability_delay = self->holdtime = level.time + DETECTOR_DELAY * delay_mult;
	self->client->pers.inventory[power_cube_index] -= cost;

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;
}

#define LASERTRAP_INITIAL_HEALTH	0
#define LASERTRAP_ADDON_HEALTH		100
#define LASERTRAP_INITIAL_DAMAGE	0
#define LASERTRAP_ADDON_DAMAGE		100
#define LASERTRAP_DELAY				1.0
#define LASERTRAP_COST				50
#define LASERTRAP_RANGE				64.0
#define LASERTRAP_MINIMUM_RANGE		64.0
#define LASERTRAP_YAW_SPEED			15

void lasertrap_remove (edict_t *self)
{
	// reset counter
	if (self->activator && self->activator->inuse)
		self->activator->num_detectors--;

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_NOCLIENT;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;
}

void lasertrap_removeall (edict_t *ent, qboolean effect)
{
	edict_t *e;

	if (!ent || !ent->inuse)
		return;

	// remove all traps
	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		// find all laser traps that we own
		if (e && e->inuse && (e->mtype == M_ALARM) && e->activator 
			&& e->activator->inuse && e->activator == ent)
		{
			if (effect && e->solid != SOLID_NOT)
			{
				// explosion effect
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_EXPLOSION1);
				gi.WritePosition (e->s.origin);
				gi.multicast (e->s.origin, MULTICAST_PVS);
			}

			lasertrap_remove(e);
		}
	}

	ent->num_detectors = 0; // reset counter
}

void lasertrap_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	// explosion effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	lasertrap_remove(self);

	safe_cprintf(self->activator, PRINT_HIGH, "A laser trap was destroyed (%d remain).\n", self->activator->num_detectors);

}

qboolean lasertrap_validtarget (edict_t *self, edict_t *target, float dist, qboolean vis)
{
	vec3_t v;

	// basic check
	if (!G_ValidTarget(self, target, vis))
		return false;

	// check distance on a 2-D plane
	VectorSubtract(target->s.origin, self->s.origin, v);
	v[2] = 0;
	if (VectorLength(v) > dist)
		return false; // do not exceed maximum laser distance

	return true;
}

void lasertrap_findtarget (edict_t *self)
{
	edict_t *e=NULL;

	while ((e = findradius(e, self->s.origin, 256.0)) != NULL)//FIXME: should do a better search
	{
		if (!lasertrap_validtarget(self, e, LASERTRAP_RANGE, true))
			continue;
		self->enemy = e;
		break;
	}

	// if we are idle with no enemy, then become invisible to AI
	if (!self->enemy)
		self->flags |= FL_NOTARGET;
}

void lasertrap_firelaser (edict_t *self, vec3_t dir)
{
	vec3_t	start, end, forward;
	trace_t	tr;

	// trace ahead, stopping at any wall
	VectorCopy(self->s.origin, start);
	VectorMA(start, self->random, dir, end);
	tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
	if (tr.fraction < 1)
	{
		// touched a wall, reduce maximum distance
		self->random = distance(tr.endpos, start);
		// too short, abort
		if (self->random < LASERTRAP_MINIMUM_RANGE)
		{
			lasertrap_remove(self);
			if (self->activator && self->activator->inuse)
				safe_cprintf(self->activator, PRINT_HIGH, "Lasertrap self-destructed. Too close to wall.\n");
			return;
		}
	}

	// trace down 32 units to the floor
	VectorCopy(tr.endpos, start);
	VectorCopy(tr.endpos, end);
	end[2] -= 32;
	tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
	if (tr.fraction == 1.0) // too far down, abort
		return;

	// copy floor position as starting point, then trace up
	//tr.endpos[2]+=24;
	VectorCopy(tr.endpos, start);
	VectorCopy(tr.endpos, end);
	end[2] += 128;
	tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SHOT);

	// bfg laser effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	// throw sparks
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_LASER_SPARKS);
	gi.WriteByte(1); // number of sparks
	gi.WritePosition(start);
	gi.WriteDir(vec3_origin);
	gi.WriteByte(209); // color
	gi.multicast(start, MULTICAST_PVS);

	// hurt any valid target that touches the laser
	if (G_ValidTarget(self, tr.ent, false))
	{
		// calculate attack vector
		VectorSubtract(tr.ent->s.origin, start, forward);// up
		VectorNormalize(forward);

		// hurt them
		T_Damage(tr.ent, self, self, forward, tr.endpos, vec3_origin, self->dmg, 0, DAMAGE_ENERGY, MOD_LASER_DEFENSE);//FIXME: change mod
	}
}

void lasertrap_attack (edict_t *self)
{
	vec3_t angles, forward, right;
	
	VectorCopy(self->s.angles, angles);
	AngleVectors(angles, forward, right, NULL);

	// activate first laser directly ahead
	lasertrap_firelaser(self, forward);

	// activate second laser to the right
	lasertrap_firelaser(self, right);

	// rotate angles 180 degrees
	angles[YAW] += 180;
	AngleCheck(&angles[YAW]);
	AngleVectors(angles, forward, right, NULL);

	// activate second laser behind
	lasertrap_firelaser(self, forward);

	// activate second laser to the left
	lasertrap_firelaser(self, right);
	
	self->flags &= ~FL_NOTARGET; // become visible to AI
	self->s.angles[YAW] += self->yaw_speed; // rotate
	self->random--;// slowly move closer to emitter

	// play a sound to alert players
	if (level.time > self->sentrydelay)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("detector/alarm3.wav"), 1, ATTN_NORM, 0);
		self->sentrydelay = level.time + 1.0;
	}
}

void lasertrap_effects (edict_t *self)
{
	edict_t *cl_ent;
	self->s.effects = self->s.renderfx = 0;

	// display shell if we have an enemy or after a delay
	if (self->enemy || !(level.framenum % 20))
	{
		// make sure
		if ((cl_ent = G_GetClient(self)) != NULL)
		{
			self->s.effects |= EF_COLOR_SHELL;

			// glow red if we are hostile against players or our activator is on the red team
			if (cl_ent->teamnum == RED_TEAM || ffa->value)
				self->s.renderfx = RF_SHELL_RED;
			else
				self->s.renderfx = RF_SHELL_BLUE;
		}
	}
}

void lasertrap_think (edict_t *self)
{
	// remove if owner dies or becomes invalid
	if (!G_EntIsAlive(self->activator))
	{
		lasertrap_remove(self);
		return;
	}

	// if enemy is valid and in-range, then attack
	if (lasertrap_validtarget(self, self->enemy, LASERTRAP_RANGE+8, false))
		lasertrap_attack(self);
	// if we had an enemy, reset
	else if (self->enemy)
	{
		self->enemy = NULL;
		self->random = 64;// reset maximum laser range
	}
	// find a new target
	else
		lasertrap_findtarget(self);

	lasertrap_effects(self);

	self->nextthink = level.time + FRAMETIME;
}

void lasertrap_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(ent, other, plane, surf);

	if (G_EntIsAlive(other) && other->client && OnSameTeam(ent, other) // a player on our team
		&& other->client->pers.inventory[power_cube_index] >= 5 // has power cubes
		&& ent->health < ent->max_health // need repair
		&& level.framenum > ent->monsterinfo.regen_delay1) // check delay
	{
		ent->health = ent->max_health;// restore health
		other->client->pers.inventory[power_cube_index] -= 5;
		safe_cprintf(other, PRINT_HIGH, "Laser trap repaired (%d/%dh).\n", ent->health, ent->max_health);
		gi.sound(other, CHAN_VOICE, gi.soundindex("weapons/repair.wav"), 1, ATTN_NORM, 0);
		ent->monsterinfo.regen_delay1 = level.framenum + 20;// delay before we can rearm
	}
}

void ThrowLaserTrap (edict_t *self, vec3_t start, vec3_t aimdir, int skill_level)
{
	edict_t *trap;
	vec3_t	forward, right, up, dir;

	// get aiming angles
	vectoangles(aimdir, dir);
	// get directional vectors
	AngleVectors(dir, forward, right, up);

	trap = G_Spawn();
	VectorSet (trap->mins, -4, -4, -2);
	VectorSet (trap->maxs, 4, 4, 2);
	trap->movetype = MOVETYPE_TOSS;
	trap->activator = self;
	trap->monsterinfo.level = skill_level;

	trap->classname = "lasertrap";
	trap->think = lasertrap_think;
	trap->nextthink = level.time + FRAMETIME;
	trap->solid = SOLID_BBOX;
	trap->takedamage = DAMAGE_AIM;
	trap->touch = lasertrap_touch;
	
	trap->die = lasertrap_die;
	trap->clipmask = MASK_SHOT;
	trap->mtype = M_ALARM;//FIXME: change this?
	trap->random = LASERTRAP_RANGE;// maximum laser range
	trap->yaw_speed = LASERTRAP_YAW_SPEED;// rotation speed
	trap->health = trap->max_health = LASERTRAP_INITIAL_HEALTH + LASERTRAP_ADDON_HEALTH * skill_level;
	trap->dmg = LASERTRAP_INITIAL_DAMAGE + LASERTRAP_ADDON_DAMAGE * skill_level;
	trap->delay = level.time + 1.0; // activation time

	trap->s.modelindex = gi.modelindex ("models/objects/detector/tris.md2");

	VectorCopy (start, trap->s.origin);
	gi.linkentity (trap);

	// increment number of detectors/traps, set delay, and use power cubes
	self->num_detectors++;
	self->client->ability_delay = level.time + LASERTRAP_DELAY;
	self->client->pers.inventory[power_cube_index] -= LASERTRAP_COST;

	// throw
	VectorScale (aimdir, 400, trap->velocity);
	VectorMA (trap->velocity, 200 + crandom() * 10.0, up, trap->velocity);
	VectorMA (trap->velocity, crandom() * 10.0, right, trap->velocity);

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;
}

void Cmd_Detector_f (edict_t *ent)
{
	float skill_mult=1.0, cost_mult=1.0, delay_mult=1.0;
	int cost=DETECTOR_COST, talentLevel;
	vec3_t forward, start;

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "All detectors removed.\n");
		detector_removeall(ent);
		lasertrap_removeall(ent, true);//4.4 Talent: Alarm
		return;
	}

	//Talent: Rapid Assembly
	talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);
	if (talentLevel > 0)
		delay_mult -= 0.1 * talentLevel;
	//Talent: Precision Tuning
	else if ((talentLevel = getTalentLevel(ent, TALENT_PRECISION_TUNING)) > 0)
	{
		cost_mult -= PRECISION_TUNING_COST_FACTOR * talentLevel;
		delay_mult -= PRECISION_TUNING_DELAY_FACTOR * talentLevel;
		skill_mult += PRECISION_TUNING_SKILL_FACTOR * talentLevel;
	}
	cost *= cost_mult;

	if (!V_CanUseAbilities(ent, DETECTOR, DETECTOR_COST, true))
		return;

	if (ent->num_detectors >= DETECTOR_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You've reached the maximum number of detectors (%d)\n", DETECTOR_MAX_COUNT);
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight - 8;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	//VectorMA(start, 24, forward, start);
	BuildDetector(ent, start, forward, (int)(ent->myskills.abilities[DETECTOR].current_level * skill_mult), DETECTOR_DURATION, cost, delay_mult);
}

void Cmd_LaserTrap_f (edict_t *ent)
{
	int		talentLevel=getTalentLevel(ent, TALENT_ALARM);
	vec3_t	forward, start;

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "All detectors and laser traps removed.\n");
		detector_removeall(ent);
		lasertrap_removeall(ent, false);
		return;
	}

	if (!V_CanUseAbilities(ent, DETECTOR, LASERTRAP_COST, true))
		return;

	if (talentLevel < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need to upgrade laser trap before you can use it.\n");
		return;
	}

	if (ent->num_detectors >= DETECTOR_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You've reached the maximum number of laser traps.\n");
		return;
	}
	
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight - 8;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	ThrowLaserTrap(ent, start, forward, talentLevel);
}

#define CONVERSION_INITIAL_RANGE		196.0
#define CONVERSION_ADDON_RANGE			0
#define CONVERSION_INITIAL_DURATION		0
#define CONVERSION_ADDON_DURATION		6.0
#define CONVERSION_INITIAL_CHANCE		0.25
#define CONVERSION_ADDON_CHANCE			0.025
#define CONVERSION_MAX_CHANCE			0.5
#define CONVERSION_COST					25
#define CONVERSION_DELAY				2.0

void drone_cleargoal(edict_t *self);

qboolean RestorePreviousOwner (edict_t *ent)
{
	if (!(ent->flags & FL_CONVERTED))
		return false;
	if (!ent->prev_owner || (ent->prev_owner && !ent->prev_owner->inuse))
		return false;
	return ConvertOwner(ent->prev_owner, ent, 0, false);
}

qboolean ConvertOwner (edict_t *ent, edict_t *other, float duration, qboolean print)
{
	int		current_num, max_num;
	edict_t *old_owner, **new_owner;

	// don't convert to a player if they are not a valid target
	//FIXME: this fails on players with godmode :(
	if (ent->client && !G_ValidTarget(NULL, ent, false))
	{
	//	gi.dprintf("%s is not a valid target\n", ent->client->pers.netname);
		return false;
	}

	// convert monster
	if (!strcmp(other->classname, "drone") && (other->mtype != M_COMMANDER))
	{
		if (ent->client)
			max_num = MAX_MONSTERS;
		else
			max_num = 999; // no maximum count for world monsters

		if (other->monsterinfo.bonus_flags)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "Can't convert champion and unique monsters\n");
			return false;
		}

		if (ent->num_monsters + other->monsterinfo.control_cost > max_num)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "Insufficient monster slots for conversion\n");
			return false;
		}

		// set-up pointers
		old_owner = other->activator;
		new_owner = &other->activator;

		// monsters converted in invasion mode should try hunting/following navi when they switch back
		if (!ent->client && invasion->value)
		{
			other->monsterinfo.aiflags &= ~AI_STAND_GROUND;
			other->monsterinfo.aiflags |= AI_FIND_NAVI;
		}

		// decrement summonable counter for previous owner
		old_owner->num_monsters -= other->monsterinfo.control_cost;
		old_owner->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", other, old_owner->num_monsters_real);
		
		// increment summonable counter for new owner
		ent->num_monsters += other->monsterinfo.control_cost;
		ent->num_monsters_real++;
		// gi.bprintf(PRINT_HIGH, "adding %p (%d)\n", other, ent->num_monsters_real);

		// number of summonable slots in-use
		current_num = old_owner->num_monsters;
		
		// make this monster blink
		//FIXME: we should probably have another effect to indicate change of ownership
		other->monsterinfo.selected_time = level.time + 3.0;	
	}
	else if (!strcmp(other->classname, "hellspawn"))
	{
		if (ent->skull && ent->skull->inuse)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "You already have a hellspawn\n");
			return false;
		}

		current_num = max_num = 1;

		// set-up pointers
		old_owner = other->activator;
		new_owner = &other->activator;

		// previous owner no longer controls this entity
		old_owner->skull = NULL;

		// new owner should have a link to hellspawn
		ent->skull = other;
	}
	else if (!strcmp(other->classname, "Sentry_Gun"))
	{
		max_num = SENTRY_MAXIMUM;

		if (ent->num_sentries + 2 > max_num + 1)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "You have reached the sentry limit\n");
			return false;
		}

		// set-up pointers
		old_owner = other->creator;
		new_owner = &other->creator;

		// previous owner no longer controls this entity
		old_owner->num_sentries -= 2;
		ent->num_sentries += 2;
		current_num = old_owner->num_sentries;

		// update stand owner too
		other->sentry->creator = ent;
	}
	else if (!strcmp(other->classname, "msentrygun"))
	{
		max_num = MAX_MINISENTRIES;

		if (ent->num_sentries + 1 > max_num)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "You have reached the sentry limit\n");
			return false;
		}

		// set-up pointers
		old_owner = other->creator;
		new_owner = &other->creator;

		// previous owner no longer controls this entity
		old_owner->num_sentries--;
		ent->num_sentries++;
		current_num = old_owner->num_sentries;

		// update base owner too
		other->owner->creator = ent;
	}
	else if (!strcmp(other->classname, "spikeball"))
	{
		max_num = SPIKEBALL_MAX_COUNT;

		if (ent->num_spikeball + 1 > max_num)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum amount of spores\n");
			return false;
		}

		// set-up pointers
		old_owner = other->activator;
		new_owner = &other->activator;
		other->owner = ent;

		// previous owner no longer controls this entity
		old_owner->num_spikeball--;
		ent->num_spikeball++;
		current_num = old_owner->num_spikeball;
	}
	else if (!strcmp(other->classname, "spiker"))
	{
		max_num = SPIKER_MAX_COUNT;

		if (ent->num_spikers + 1 > max_num)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum amount of spikers\n");
			return false;
		}

		// set-up pointers
		old_owner = other->activator;
		new_owner = &other->activator;

		// previous owner no longer controls this entity
		old_owner->num_spikers--;
		ent->num_spikers++;
		current_num = old_owner->num_spikers;
	}
	else if (!strcmp(other->classname, "gasser"))
	{
		max_num = GASSER_MAX_COUNT;

		if (ent->num_gasser + 1 > max_num)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum amount of gassers\n");
			return false;
		}

		// set-up pointers
		old_owner = other->activator;
		new_owner = &other->activator;

		// previous owner no longer controls this entity
		old_owner->num_gasser--;
		ent->num_gasser++;
		current_num = old_owner->num_gasser;
	}
	else if (!strcmp(other->classname, "obstacle"))
	{
		max_num = OBSTACLE_MAX_COUNT;

		if (ent->num_obstacle + 1 > max_num)
		{
			if (print && ent->client)
				safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum amount of obstacles\n");
			return false;
		}

		// set-up pointers
		old_owner = other->activator;
		new_owner = &other->activator;

		// previous owner no longer controls this entity
		old_owner->num_obstacle--;
		ent->num_obstacle++;
		current_num = old_owner->num_obstacle;
	}
	else
		return false; // unsupported entity

	*new_owner = ent;

	// if the new owner is the same as the old one, then remove the converted flag
	if (other->prev_owner && (other->prev_owner == ent))
	{
		other->prev_owner = NULL;
		other->flags &= ~FL_CONVERTED;
		other->removetime = 0; // do not expire
	}
	else
	{
		other->prev_owner = old_owner;
		other->flags |= FL_CONVERTED;
	}

	if (print)
	{
		if (old_owner->client)
			safe_cprintf(old_owner, PRINT_HIGH, "Your %s was converted by %s (%d/%d)\n", 
				V_GetMonsterName(other), ent->client->pers.netname, current_num, max_num);

		if (ent->client)
			safe_cprintf(ent, PRINT_HIGH, "A level %d %s was converted to your side for %.0f seconds\n", other->monsterinfo.level, V_GetMonsterName(other), duration);
	}

	if (duration > 0)
		other->removetime = level.time + duration;

	// If they had a goal, clear it, now.
	drone_cleargoal(other);

	return true;
}

qboolean CanConvert (edict_t *ent, edict_t *other)
{
	if (!other)
		return false;

	if (!other->inuse || !other->takedamage || other->health<1 || other->deadflag==DEAD_DEAD)
		return false;

	if (OnSameTeam(ent, other))
	{
		// only allow conversion of allied summonables in PvP mode
		if (V_IsPVP() && IsAlly(ent, G_GetSummoner(other)))
			return true;
		else
			return false;
	}

	return true;
}

void Cmd_Conversion_f (edict_t *ent)
{
	int		duration = CONVERSION_INITIAL_DURATION + CONVERSION_ADDON_DURATION * ent->myskills.abilities[CONVERSION].current_level;
	float	chance = CONVERSION_INITIAL_CHANCE + CONVERSION_ADDON_CHANCE * ent->myskills.abilities[CONVERSION].current_level;
	float	range = CONVERSION_INITIAL_RANGE + CONVERSION_ADDON_RANGE * ent->myskills.abilities[CONVERSION].current_level;
	edict_t *e=NULL;

	if (!V_CanUseAbilities(ent, CONVERSION, CONVERSION_COST, true))
		return;

	if (chance > CONVERSION_MAX_CHANCE)
		chance = CONVERSION_MAX_CHANCE;

	while ((e = findclosestreticle(e, ent, range)) != NULL)
	{
		float r = random();

		if (!CanConvert(ent, e))
			continue;
		if (!infront(ent, e))
			continue;
		
	//	gi.dprintf("random = %f chance = %f\n", r, chance);

		if ((chance > r) && ConvertOwner(ent, e, duration, true))
			gi.sound(e, CHAN_ITEM, gi.soundindex("spells/conversion.wav"), 1, ATTN_NORM, 0);
		else
			safe_cprintf(ent, PRINT_HIGH, "Conversion failed.\n");

		ent->client->ability_delay = level.time + CONVERSION_DELAY;
		ent->client->pers.inventory[power_cube_index] -= CONVERSION_COST;

		// write a nice effect so everyone knows we've cast a spell
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_TELEPORT_EFFECT);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		//  entity made a sound, used to alert monsters
		ent->lastsound = level.framenum;
		return;
	}
}

void ProjectileReverseCourse (edict_t *proj)
{
	VectorNegate(proj->movedir, proj->movedir);
	VectorNegate(proj->velocity, proj->velocity);
	vectoangles(proj->velocity, proj->s.angles);
	ValidateAngles(proj->s.angles);
}

void DeflectProjectile (edict_t *self, char *className, float chance, qboolean in_front)
{
	edict_t *e=NULL;

	while((e = G_Find(e, FOFS(classname), className)) != NULL)
	{
		// only use non-friendly projectiles
		if (e->owner && e->owner->inuse && OnSameTeam(self, e->owner))
			continue;

		//Talent: Repel
		// only deflect projectiles away from our front
		if (in_front && !infront(self, e))
			continue;

		// this projectiles has already been redirected back at its owner
		if (e->enemy && e->owner && (e->enemy == e->owner))
			continue;

		// calculate projectile speed and compare distance
		// if the speed is greater than the distance remaining, then we are going
		// to hit next frame, so we need to deflect this shot
		if (VectorLength(e->velocity)*FRAMETIME < entdist(self, e)-(G_GetHypotenuse(self->maxs)+1))
			continue;
		
		if (random() > chance)
			continue;

		// redirect projectile towards its owner
		if (e->owner && e->owner->inuse && e->owner->takedamage && visible(e, e->owner))
		{
			e->enemy = e->owner;
			e->owner = self; // take ownership
			ProjectileReverseCourse(e);
		}
	}	
}

void DeflectProjectiles (edict_t *self, float chance, qboolean in_front)
{
	DeflectProjectile(self, "rocket", chance, in_front);
	DeflectProjectile(self, "bolt", chance, in_front);
	DeflectProjectile(self, "bfg blast", chance, in_front);
	DeflectProjectile(self, "magicbolt", chance, in_front);
	DeflectProjectile(self, "grenade", chance, in_front);
	DeflectProjectile(self, "hgrenade", chance, in_front);
	DeflectProjectile(self, "skull", chance, in_front);
	DeflectProjectile(self, "spike", chance, in_front);
	DeflectProjectile(self, "spikey", chance, in_front);
	DeflectProjectile(self, "fireball", chance, in_front);
	DeflectProjectile(self, "plasma bolt", chance, in_front);
	DeflectProjectile(self, "acid", chance, in_front);
}

#define MIRROR_INITIAL_HEALTH		0	
#define MIRROR_ADDON_HEALTH			100
#define MIRROR_COST					50
#define MIRROR_DELAY				1.0

void mirrored_remove (edict_t *self)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	if (self->activator && self->activator->inuse)
	{
		if (self->activator->mirror1 == self)
			self->activator->mirror1 = NULL;
			
		else if (self->activator->mirror2 == self)
			self->activator->mirror2 = NULL;
	}

	self->think = BecomeTE;
	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_NOCLIENT;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;
}

void mirrored_removeall (edict_t *ent)
{
	if (ent->mirror1)
		mirrored_remove(ent->mirror1);
	if (ent->mirror2)
		mirrored_remove(ent->mirror2);
}

void UnGhostMirror (edict_t *self)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_YES;
}

void GhostMirror (edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_NOT;
	self->takedamage = DAMAGE_NO;
}

qboolean UpdateMirroredEntity (edict_t *self, float dist)
{
	vec3_t	angles;
	vec3_t	forward, right, start, end;
	trace_t	tr;
	edict_t *target;
	
	if (!self->activator || !self->activator->inuse || !self->activator->client)
		return false;

	if (PM_PlayerHasMonster(self->activator))
	{
		target = self->activator->owner;
		self->owner = target;
	}
	else
	{
		target = self->activator;
		self->owner = self->activator;
	}

	// copy current animation frame
	self->s.frame = target->s.frame;

	if (level.framenum >= self->count)
	{
		// copy owner's bounding box
		VectorCopy(target->mins, self->mins);
		VectorCopy(target->maxs, self->maxs);

		// copy yaw for position calculation
		VectorCopy(target->s.angles, angles);
		angles[YAW] = target->s.angles[YAW];
		angles[PITCH] = 0;
		angles[ROLL] = 0;

		AngleVectors(angles, NULL, right, NULL);

		// calculate starting position
		if (self->activator->mtype == MORPH_FLYER || self->activator->mtype == MORPH_CACODEMON)
		{
			// trace sideways
			VectorMA(self->owner->s.origin, dist, right, end);
			tr = gi.trace(self->owner->s.origin, NULL, NULL, end, target, MASK_SOLID);
		}
		else
		{
			VectorCopy(target->s.origin, start);
			VectorCopy(start, end);
			end[2] += self->maxs[2];
			// trace up
			tr = gi.trace(start, NULL, NULL, end, target, MASK_SOLID);
			VectorMA(tr.endpos, dist, right, end);
			// trace sideways
			tr = gi.trace(start, NULL, NULL, end, target, MASK_SOLID);
			VectorCopy(tr.endpos, start);
			VectorCopy(start, end);
			end[2] = target->absmin[2];
			// trace down
			tr = gi.trace(start, self->mins, self->maxs, end, target, MASK_SHOT);
			// no floor
			if ((tr.fraction == 1.0) && target->groundentity)
				return false;
			
			
		}

		VectorCopy(tr.endpos, end);
		// check final position
		tr = gi.trace(end, self->mins, self->maxs, end, target, MASK_SHOT);
		if (tr.fraction < 1 && (tr.ent != self->owner->mirror1) && (tr.ent != self->owner->mirror2))
			return false;
		
		// update position
		VectorCopy(end, self->s.origin);
		gi.linkentity(self);

		// copy everything from our owner
		self->model = target->model;
		self->s.skinnum = target->s.skinnum;
		self->s.modelindex = target->s.modelindex;
		self->s.modelindex2 = target->s.modelindex2;
		self->s.effects = target->s.effects;
		self->s.renderfx = target->s.renderfx;

		// set angles to point where our owner is pointing
		VectorCopy(target->s.origin, start);
		start[2] += self->activator->viewheight - 8;
		AngleVectors(self->activator->client->v_angle, forward, NULL, NULL);
		VectorMA(start, 8192, forward, end);
		tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
		VectorSubtract(tr.endpos, start, forward);
		vectoangles(forward, self->s.angles);
		self->s.angles[PITCH] = target->s.angles[PITCH];

		self->count = level.framenum + 1;
	}

	// we have a valid position for this entity, so make it visible
	UnGhostMirror(self);
	return true;
}

#define		MIRROR_POSITION_MIDDLE		1
#define		MIRROR_POSITION_LEFT		2
#define		MIRROR_POSITION_RIGHT		3

qboolean MirroredEntitiesExist (edict_t *ent)
{
	//return (G_EntExists(ent->mirror1) && G_EntExists(ent->mirror2));
	return ent->mirror1 && ent->mirror1->inuse && ent->mirror2 && ent->mirror2->inuse;
}

void UpdateMirroredEntities (edict_t *ent)
{
	int		i, pos;
	float	dist;

	if (!MirroredEntitiesExist(ent))
		return;

	// randomize desired position to fool opponents
	if (!(level.framenum%50))
		ent->mirroredPosition = GetRandom(1, 3);

	pos = ent->mirroredPosition;

	// calculate distance from owner
	if (PM_PlayerHasMonster(ent))
		dist = (2*ent->owner->maxs[1])+8;
	else
		dist = (2*ent->maxs[1])+8;

	for (i=0; i<3; i++)
	{
		// try middle position
		if ((pos == MIRROR_POSITION_MIDDLE) && UpdateMirroredEntity(ent->mirror1, dist)
			&& UpdateMirroredEntity(ent->mirror2, -dist))
			return;
		// try left position
		else if ((pos == MIRROR_POSITION_LEFT) && UpdateMirroredEntity(ent->mirror1, -dist)
			&& UpdateMirroredEntity(ent->mirror2, -2*dist))
			return;
		// try right
		else if ((pos == MIRROR_POSITION_RIGHT) && UpdateMirroredEntity(ent->mirror1, dist)
			&& UpdateMirroredEntity(ent->mirror2, 2*dist))
			return;
		pos++;
		if (pos > MIRROR_POSITION_RIGHT)
			pos = MIRROR_POSITION_MIDDLE;
		ent->mirroredPosition = pos;
	}

	// we couldn't find a valid position, so hide them
	GhostMirror(ent->mirror1);
	GhostMirror(ent->mirror2);
}

void mirrored_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->activator && self->activator->inuse && self->deadflag != DEAD_DEAD)
	{
		if (self->activator->client)
			safe_cprintf(self->activator, PRINT_HIGH, "Your decoy was killed.\n");
		mirrored_removeall(self->activator);
	}
}

void mirrored_think (edict_t *self)
{
	if (!G_EntIsAlive(self->activator))
	{
		mirrored_remove(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;

}

char *V_GetClassSkin (edict_t *ent);

edict_t *MirrorEntity (edict_t *ent)
{
	edict_t *e;

	e = G_Spawn();
	e->svflags |= SVF_MONSTER;
	e->classname = "mirrored";
	e->activator = e->owner = ent;
	e->takedamage = DAMAGE_YES;
	e->health = e->max_health = MIRROR_INITIAL_HEALTH + MIRROR_ADDON_HEALTH * ent->myskills.level;
	e->mass = 200;
	e->clipmask = MASK_MONSTERSOLID;
	e->movetype = MOVETYPE_NONE;
	e->s.renderfx |= RF_IR_VISIBLE;
//	e->flags |= FL_CHASEABLE;
	e->solid = SOLID_BBOX;
	e->think = mirrored_think;
	e->die = mirrored_die;
	e->mtype = M_MIRROR;
	e->touch = V_Touch;
	VectorCopy(ent->mins, e->mins);
	VectorCopy(ent->maxs, e->maxs);
	e->nextthink = level.time + FRAMETIME;

	return e;
}

void Cmd_Antigrav_f (edict_t *ent)
{
	if ((!ent->inuse) || (!ent->client))
		return;
	if(ent->myskills.abilities[ANTIGRAV].disable)
		return;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[ANTIGRAV].current_level, 0))
		return;

	if (ent->antigrav == true)
	{
		safe_cprintf(ent, PRINT_HIGH, "Antigrav disabled.\n");
		ent->antigrav = false;
		return;
	}

	if(ent->myskills.abilities[ANTIGRAV].disable)
		return;

	if (HasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't use this ability while carrying the flag!\n");
		return;
	}

	//3.0 amnesia disables super speed
	if (que_findtype(ent->curses, NULL, AMNESIA) != NULL)
		return;

	safe_cprintf(ent, PRINT_HIGH, "Antigrav enabled.\n");
	ent->antigrav= true;
}

#define FIREBALL_INITIAL_DAMAGE		50
#define FIREBALL_ADDON_DAMAGE		15
#define FIREBALL_INITIAL_RADIUS		100
#define FIREBALL_ADDON_RADIUS		2.5
#define FIREBALL_INITIAL_SPEED		650
#define FIREBALL_ADDON_SPEED		35
#define FIREBALL_INITIAL_FLAMES		5
#define FIREBALL_ADDON_FLAMES		0
#define FIREBALL_INITIAL_FLAMEDMG	0
#define FIREBALL_ADDON_FLAMEDMG		2
#define FIREBALL_DELAY				0.3

void fireball_explode (edict_t *self, cplane_t *plane)
{
	int		i;
	vec3_t	forward;
	edict_t *e=NULL;

	// burn targets within explosion radius
	while ((e = findradius(e, self->s.origin, self->dmg_radius)) != NULL)
	{
		if (!G_ValidTarget(self, e, true))
			continue;
		burn_person(e, self->owner, self->radius_dmg);
	}
		
	for (i=0; i<self->count; i++)
	{
		if (plane)
		{
			VectorCopy(plane->normal, forward);
		}
		else
		{
			VectorNegate(self->velocity, forward);
			VectorNormalize(forward);
		}

		// randomize aiming vector
		forward[YAW] += 0.5 * crandom();
		forward[PITCH] += 0.5 * crandom();

		// create the flame entities
		ThrowFlame(self->owner, self->s.origin, forward, 0, GetRandom(50, 150), self->radius_dmg, GetRandom(3, 5));
	}

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_FIREBALL);
	BecomeExplosion1(self);
}

void fireball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// remove fireball if owner dies or becomes invalid or if we touch a sky brush
	if (!G_EntIsAlive(ent->owner) || (surf && (surf->flags & SURF_SKY)))
	{
		G_FreeEdict(ent);
		return;
	}

	//FIXME: this sound is overridden by BecomeExplosion1()'s sound
	gi.sound (ent, CHAN_VOICE, gi.soundindex (va("spells/largefireimpact%d.wav", GetRandom(1, 3))), 1, ATTN_NORM, 0);
	fireball_explode(ent, plane);
}

void fireball_think (edict_t *self)
{
	int i;
	vec3_t start, forward;

	if (level.time > self->delay || self->waterlevel)
	{
		G_FreeEdict(self);
		return;
	}

	// 0 = black, 8 = grey, 15 = white, 16 = light brown, 20 = brown, 57 = light orange, 66 = orange/red, 73 = maroon
	// 106 = pink, 113 = light blue, 119 = blue, 123 = dark blue, 200 = pale green, 205 = dark green, 209 = bright green
	// 217 = white, 220 = yellow, 226 = orange, 231 = red/orange, 240 = red, 243 = dark blue

	VectorCopy(self->s.origin, start);
	VectorCopy(self->velocity, forward);
	VectorNormalize(forward);

	// create a trail behind the fireball
	for (i=0; i<6; i++)
	{

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(1); // number of sparks
		gi.WritePosition(start);
		gi.WriteDir(forward);
		gi.WriteByte(223); // color
		gi.multicast(start, MULTICAST_PVS);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(1); // number of sparks
		gi.WritePosition(start);
		gi.WriteDir(forward);
		gi.WriteByte(231); // color
		gi.multicast(start, MULTICAST_PVS);

		VectorMA(start, -16, forward, start);
	}

	self->nextthink = level.time + FRAMETIME;
}

void fire_fireball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, float damage_radius, int speed, int flames, int flame_damage)
{
	edict_t	*fireball;
	vec3_t	dir;
	vec3_t	forward;

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// get aiming angles
	vectoangles(aimdir, dir);
	// get directional vectors
	AngleVectors(dir, forward, NULL, NULL);

	// spawn grenade entity
	fireball = G_Spawn();
	VectorCopy (start, fireball->s.origin);
	fireball->movetype = MOVETYPE_TOSS;//MOVETYPE_FLYMISSILE;
	fireball->clipmask = MASK_SHOT;
	fireball->solid = SOLID_BBOX;
	fireball->s.modelindex = gi.modelindex ("models/objects/flball/tris.md2");
	fireball->owner = self;
	fireball->touch = fireball_touch;
	fireball->think = fireball_think;
	fireball->dmg_radius = damage_radius;
	fireball->dmg = damage;
	fireball->radius_dmg = flame_damage;
	fireball->count = flames;
	fireball->classname = "fireball";
	fireball->delay = level.time + 10.0;
	gi.linkentity (fireball);
	fireball->nextthink = level.time + FRAMETIME;
	VectorCopy(dir, fireball->s.angles);

	// adjust velocity
	VectorScale (aimdir, speed, fireball->velocity);
	fireball->velocity[2] += 150;
}

void icebolt_explode (edict_t *self, cplane_t *plane)
{
	edict_t *e=NULL;

	// chill targets within explosion radius
	while ((e = findradius(e, self->s.origin, self->dmg_radius)) != NULL)
	{
		if (!G_ValidTarget(self, e, true))
			continue;
		e->chill_level = self->chill_level;
		e->chill_time = level.time + self->chill_time;
	}

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_ICEBOLT);//CHANGEME--fix MOD
	BecomeTE(self);//CHANGEME--need ice explosion effect
}

void icebolt_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// remove icebolt if owner dies or becomes invalid or if we touch a sky brush
	if (!G_EntIsAlive(ent->owner) || (surf && (surf->flags & SURF_SKY)))
	{
		G_FreeEdict(ent);
		return;
	}

	gi.sound (ent, CHAN_VOICE, gi.soundindex (va("spells/blastimpact%d.wav", GetRandom(1, 3))), 1, ATTN_NORM, 0);
	icebolt_explode(ent, plane);
}

void icebolt_think (edict_t *self)
{
	int i;
	vec3_t start, forward;

	if (level.time > self->delay)
	{
		G_FreeEdict(self);
		return;
	}

	// 0 = black, 8 = grey, 15 = white, 16 = light brown, 20 = brown, 57 = light orange, 66 = orange/red, 73 = maroon
	// 106 = pink, 113 = light blue, 119 = blue, 123 = dark blue, 200 = pale green, 205 = dark green, 209 = bright green
	// 217 = white, 220 = yellow, 226 = orange, 231 = red/orange, 240 = red, 243 = dark blue

	VectorCopy(self->s.origin, start);
	VectorCopy(self->velocity, forward);
	VectorNormalize(forward);

	// create a trail behind the fireball
	for (i=0; i<6; i++)
	{

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(1); // number of sparks
		gi.WritePosition(start);
		gi.WriteDir(forward);
		gi.WriteByte(113); // color
		gi.multicast(start, MULTICAST_PVS);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(1); // number of sparks
		gi.WritePosition(start);
		gi.WriteDir(forward);
		gi.WriteByte(119); // color
		gi.multicast(start, MULTICAST_PVS);

		VectorMA(start, -16, forward, start);
	}

	self->nextthink = level.time + FRAMETIME;
}

void fire_icebolt (edict_t *self, vec3_t start, vec3_t aimdir, int damage, float damage_radius, int speed, int chillLevel, float chillDuration)
{
	edict_t	*icebolt;
	vec3_t	dir;
	vec3_t	forward;

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// get aiming angles
	vectoangles(aimdir, dir);
	// get directional vectors
	AngleVectors(dir, forward, NULL, NULL);

	// spawn grenade entity
	icebolt = G_Spawn();
	VectorCopy (start, icebolt->s.origin);
	icebolt->s.effects |= EF_COLOR_SHELL;
	icebolt->s.renderfx |= RF_SHELL_BLUE;
	icebolt->movetype = MOVETYPE_FLYMISSILE;
	icebolt->clipmask = MASK_SHOT;
	icebolt->solid = SOLID_BBOX;
	icebolt->s.modelindex = gi.modelindex ("models/objects/flball/tris.md2");
	icebolt->owner = self;
	icebolt->touch = icebolt_touch;
	icebolt->think = icebolt_think;
	icebolt->dmg_radius = damage_radius;
	icebolt->dmg = damage;
	icebolt->chill_level = chillLevel;
	icebolt->chill_time = chillDuration;
	icebolt->classname = "icebolt";
	icebolt->delay = level.time + 10.0;
	gi.linkentity (icebolt);
	icebolt->nextthink = level.time + FRAMETIME;
	VectorCopy(dir, icebolt->s.angles);

	// adjust velocity
	VectorScale (aimdir, speed, icebolt->velocity);
}

// note: there are only 5 talent levels, so addon values will be higher
#define ICEBOLT_INITIAL_DAMAGE				100
#define ICEBOLT_ADDON_DAMAGE				20
#define ICEBOLT_INITIAL_RADIUS				100
#define ICEBOLT_ADDON_RADIUS				0
#define ICEBOLT_INITIAL_SPEED				650
#define ICEBOLT_ADDON_SPEED					0
#define ICEBOLT_INITIAL_CHILL_DURATION		0
#define ICEBOLT_ADDON_CHILL_DURATION		0.4
#define ICEBOLT_DELAY						0.3

//Talent: Ice Bolt
void Cmd_IceBolt_f (edict_t *ent, float skill_mult, float cost_mult)
{
	int		slvl = getTalentLevel(ent, TALENT_ICE_BOLT);
	int		damage, fblvl, speed, cost=ICEBOLT_COST*cost_mult;
	float	radius, chill_duration;
	vec3_t	forward, right, start, offset;

	// you need to have fireball upgraded to use icebolt
	if (!V_CanUseAbilities(ent, FIREBALL, cost, true))
		return;

	// current fireball level
	fblvl = ent->myskills.abilities[FIREBALL].current_level;

	// talent isn't upgraded
	if (slvl < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You must upgrade ice bolt before you can use it.\n");
		return;
	}

	chill_duration = (ICEBOLT_INITIAL_CHILL_DURATION + ICEBOLT_ADDON_CHILL_DURATION * slvl) * skill_mult;
	damage = (FIREBALL_INITIAL_DAMAGE + FIREBALL_ADDON_DAMAGE * fblvl) * skill_mult;
	radius = FIREBALL_INITIAL_RADIUS + FIREBALL_ADDON_RADIUS * fblvl;
	speed = FIREBALL_INITIAL_SPEED + FIREBALL_ADDON_SPEED * fblvl;

	
	//gi.dprintf("dmg:%d slvl:%d skill_mult:%f chill_duration:%f\n",damage,slvl,skill_mult,chill_duration);

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	fire_icebolt(ent, start, forward, damage, radius, speed, 2*slvl, chill_duration);

	ent->client->ability_delay = level.time + ICEBOLT_DELAY/* * cost_mult*/;
	ent->client->pers.inventory[power_cube_index] -= cost;

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound (ent, CHAN_ITEM, gi.soundindex("spells/coldcast.wav"), 1, ATTN_NORM, 0);
}

void Cmd_Fireball_f (edict_t *ent, float skill_mult, float cost_mult)
{
	int		slvl = ent->myskills.abilities[FIREBALL].current_level;
	int		damage, speed, flames, flamedmg, cost=FIREBALL_COST*cost_mult;
	float	radius;
	vec3_t	forward, right, start, offset;

	if (!V_CanUseAbilities(ent, FIREBALL, cost, true))
		return;

	if (ent->waterlevel > 1)
		return;

	damage = (FIREBALL_INITIAL_DAMAGE + FIREBALL_ADDON_DAMAGE * slvl) * skill_mult;
	radius = FIREBALL_INITIAL_RADIUS + FIREBALL_ADDON_RADIUS * slvl;
	speed = FIREBALL_INITIAL_SPEED + FIREBALL_ADDON_SPEED * slvl;
	flames = FIREBALL_INITIAL_FLAMES + FIREBALL_ADDON_FLAMES * slvl;
	flamedmg = (FIREBALL_INITIAL_FLAMEDMG + FIREBALL_ADDON_FLAMEDMG * slvl) * skill_mult;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	fire_fireball(ent, start, forward, damage, radius, speed, flames, flamedmg);

	ent->client->ability_delay = level.time + FIREBALL_DELAY/* * cost_mult*/;
	ent->client->pers.inventory[power_cube_index] -= cost;

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound (ent, CHAN_ITEM, gi.soundindex("spells/firecast.wav"), 1, ATTN_NORM, 0);
}

#define PLASMABOLT_INITIAL_DAMAGE		50
#define PLASMABOLT_ADDON_DAMAGE			15
#define PLASMABOLT_INITIAL_RADIUS		100
#define PLASMABOLT_ADDON_RADIUS			5
#define PLASMABOLT_INITIAL_SPEED		750
#define PLASMABOLT_ADDON_SPEED			0
#define PLASMABOLT_INITIAL_DURATION		2.0
#define PLASMABOLT_ADDON_DURATION		0
#define PLASMABOLT_COST					20
#define PLASMABOLT_DELAY				0.3
#define PLASMABOLT_DELAY_PVP			0.7

void plasmabolt_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// remove plasmabolt if owner dies or becomes invalid or if we touch a sky brush or if we bounced too much
	if (!G_EntIsAlive(ent->owner) || (surf && (surf->flags & SURF_SKY)) || (ent->style >= 10))
	{
		G_FreeEdict(ent);
		return;
	}

	// if this is the first impact, set expiration timer
	if (!ent->style)
	{
		ent->style++; // increment the number of bounces
		ent->delay = level.time + ent->random;
	}
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound (ent, CHAN_VOICE, gi.soundindex (va("spells/largefireimpact%d.wav", GetRandom(1, 3))), 1, ATTN_NORM, 0);

	if (G_EntExists(other))
	{
		T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, ent->dmg, 0, MOD_PLASMABOLT);
		T_RadiusDamage(ent, ent->owner, ent->dmg, other, ent->dmg_radius, MOD_PLASMABOLT);
		G_FreeEdict(ent);
		return;
	}

	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_PLASMABOLT);
}

void plasmabolt_think (edict_t *self)
{
	// owner must be alive
	if (!G_EntIsAlive(self->owner) || (self->delay < level.time)) 
	{
		BecomeTE(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void fire_plasmabolt (edict_t *self, vec3_t start, vec3_t aimdir, int damage, float damage_radius, int speed, float duration)
{
	edict_t	*bolt;
	vec3_t	dir;
	vec3_t	forward;

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// get aiming angles
	vectoangles(aimdir, dir);
	// get directional vectors
	AngleVectors(dir, forward, NULL, NULL);

	// spawn plasmabolt entity
	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	bolt->movetype = MOVETYPE_WALLBOUNCE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	bolt->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
	bolt->owner = self;
	bolt->touch = plasmabolt_touch;
	bolt->think = plasmabolt_think;
	bolt->dmg_radius = damage_radius;
	bolt->dmg = damage;
	bolt->classname = "plasma bolt";
	bolt->random = duration;
	bolt->delay = level.time + 10.0;
	gi.linkentity(bolt);
	bolt->nextthink = level.time + FRAMETIME;
	VectorCopy(dir, bolt->s.angles);

	// adjust velocity
	VectorScale (aimdir, speed, bolt->velocity);
}

void Cmd_Plasmabolt_f (edict_t *ent)
{
	int		slvl = ent->myskills.abilities[PLASMA_BOLT].current_level;
	int		damage, speed, duration;
	float	radius;
	vec3_t	forward, right, start, offset;

	if (!V_CanUseAbilities(ent, PLASMA_BOLT, PLASMABOLT_COST, true))
		return;

	damage = PLASMABOLT_INITIAL_DAMAGE + PLASMABOLT_ADDON_DAMAGE * slvl;
	radius = PLASMABOLT_INITIAL_RADIUS + PLASMABOLT_ADDON_RADIUS * slvl;
	speed = PLASMABOLT_INITIAL_SPEED + PLASMABOLT_ADDON_SPEED * slvl;
	duration = PLASMABOLT_INITIAL_DURATION + PLASMABOLT_ADDON_DURATION * slvl;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	fire_plasmabolt(ent, start, forward, damage, radius, speed, duration);

	if (pvm->value)
		ent->client->ability_delay = level.time + PLASMABOLT_DELAY;
	else
		ent->client->ability_delay = level.time + PLASMABOLT_DELAY_PVP;

	ent->client->pers.inventory[power_cube_index] -= PLASMABOLT_COST;

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound (ent, CHAN_ITEM, gi.soundindex("spells/holybolt2.wav"), 1, ATTN_NORM, 0);
}

#define LIGHTNING_STRIKE_RADIUS		64
#define LIGHTNING_MIN_DELAY			1
#define LIGHTNING_MAX_DELAY			3
#define LIGHTNING_INITIAL_DAMAGE	50
#define LIGHTNING_ADDON_DAMAGE		15
#define LIGHTNING_INITIAL_DURATION	5.0
#define LIGHTNING_ADDON_DURATION	0
#define LIGHTNING_INITIAL_RADIUS	128
#define LIGHTNING_ADDON_RADIUS		0
#define LIGHTNING_ABILITY_DELAY		1.0

void lightningstorm_think (edict_t *self)
{
	edict_t *e=NULL;
	vec3_t	start, end, dir;
	trace_t tr;

	// owner must be alive
	if (!G_EntIsAlive(self->owner) || HasFlag(self->owner) || (self->delay < level.time)) 
	{
		G_FreeEdict(self);
		return;
	}

	// calculate randomized starting position
	VectorCopy(self->s.origin, start);
	start[0] += GetRandom(0, (int)self->dmg_radius) * crandom();
	start[1] += GetRandom(0, (int)self->dmg_radius) * crandom();
	tr = gi.trace(self->s.origin, NULL, NULL, start, NULL, MASK_SOLID);
	VectorCopy(tr.endpos, start);
	VectorCopy(start, end);
	end[2] += 8192;
	tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
	VectorCopy(tr.endpos, start);

	// calculate ending position
	VectorCopy(start, end);
	end[2] -= 8192;
	tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SHOT);
	VectorCopy(tr.endpos, end);

	while ((e = findradius(e, tr.endpos, LIGHTNING_STRIKE_RADIUS)) != NULL)
	{
		//FIXME: make a noise when we hit something?
		if (e && e->inuse && e->takedamage)
		{
			tr = gi.trace(end, NULL, NULL, e->s.origin, NULL, MASK_SHOT);
			VectorSubtract(tr.endpos, end, dir);
			VectorNormalize(dir);
			T_Damage(e, self, self->owner, dir, tr.endpos, tr.plane.normal, self->dmg, 0, DAMAGE_ENERGY, MOD_LIGHTNING_STORM);
		}
	}
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_HEATBEAM);
	gi.WriteShort (self-g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (end);
	gi.multicast (end, MULTICAST_PVS);

//	gi.sound (self, CHAN_WEAPON, gi.soundindex("spells/chargedbolt1.wav"), 1, ATTN_NORM, 0);

	self->nextthink = level.time + GetRandom(LIGHTNING_MIN_DELAY, LIGHTNING_MAX_DELAY) * FRAMETIME;
}

void SpawnLightningStorm (edict_t *ent, vec3_t start, float radius, int duration, int damage)
{
	edict_t *storm;

	storm = G_Spawn();
	storm->solid = SOLID_NOT;
	storm->svflags |= SVF_NOCLIENT;
	storm->owner = ent;
	storm->delay = level.time + duration;
	storm->nextthink = level.time + FRAMETIME;
	storm->think = lightningstorm_think;
	VectorCopy(start, storm->s.origin);
	VectorCopy(start, storm->s.old_origin);
	storm->dmg_radius = radius;
	storm->dmg = damage;
	gi.linkentity(storm);
}

void Cmd_LightningStorm_f (edict_t *ent, float skill_mult, float cost_mult)
{
	int		slvl = ent->myskills.abilities[LIGHTNING_STORM].current_level;
	int		damage, duration, cost=LIGHTNING_COST*cost_mult;
	float	radius;
	vec3_t	forward, offset, right, start, end;
	trace_t	tr;

	if (!V_CanUseAbilities(ent, LIGHTNING_STORM, cost, true))
		return;

	damage = LIGHTNING_INITIAL_DAMAGE + LIGHTNING_ADDON_DAMAGE * slvl * skill_mult;
	duration = LIGHTNING_INITIAL_DURATION + LIGHTNING_ADDON_DURATION * slvl;
	radius = LIGHTNING_INITIAL_RADIUS + LIGHTNING_ADDON_DURATION * slvl;

	// randomize damage
	damage = GetRandom((int)(0.5*damage), damage);

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	// get ending position
	VectorCopy(start, end);
	VectorMA(start, 8192, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	SpawnLightningStorm(ent, tr.endpos, radius, duration, damage);

	ent->client->ability_delay = level.time + LIGHTNING_ABILITY_DELAY/* * cost_mult*/;
	ent->client->pers.inventory[power_cube_index] -= cost;

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/eleccast.wav"), 1, ATTN_NORM, 0);
}

void V_Push (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float maxvel = 300;

	// our activator or ally can push us
	if (other && other->inuse && other->client && self->activator && self->activator->inuse 
		&& (other == self->activator || IsAlly(self->activator, other)))
	{
		vec3_t forward, right, offset, start;

		// don't push if we are standing on this entity
		if (other->groundentity && other->groundentity == self)
			return;
		
		self->movetype_prev = self->movetype;
		self->movetype_frame = level.framenum + 5;
		self->movetype = MOVETYPE_STEP;

		AngleVectors (other->client->v_angle, forward, right, NULL);
		VectorScale (forward, -3, other->client->kick_origin);
		VectorSet(offset, 0, 7,  other->viewheight-8);
		P_ProjectSource (other->client, other->s.origin, offset, forward, right, start);

		VectorMA(self->velocity, 50, forward, self->velocity);

		// cap maximum velocity
		if (self->velocity[0] > maxvel)
			self->velocity[0] = maxvel;
		if (self->velocity[0] < -maxvel)
			self->velocity[0] = -maxvel;
		if (self->velocity[1] > maxvel)
			self->velocity[1] = maxvel;
		if (self->velocity[1] < -maxvel)
			self->velocity[1] = -maxvel;
		if (self->velocity[2] > maxvel)
			self->velocity[2] = maxvel;
		if (self->velocity[2] < -maxvel)
			self->velocity[2] = -maxvel;
	}
}

// touch function for all the gloom stuff
void organ_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(ent, other, plane, surf);

	// don't push or heal something that's already dead or invalid
	if (!ent || !ent->inuse || !ent->takedamage || ent->health < 1)
		return;

	V_Push(ent, other, plane, surf);

	if (G_EntIsAlive(other) && other->client && OnSameTeam(ent, other) 
		&& other->client->pers.inventory[power_cube_index] >= 5
		&& level.time > ent->lasthurt + 0.5 && ent->health < ent->max_health
		&& level.framenum > ent->monsterinfo.regen_delay1)
	{
		ent->health_cache += (int)(0.5 * ent->max_health);
		ent->monsterinfo.regen_delay1 = level.framenum + 10;
		other->client->pers.inventory[power_cube_index] -= 5;
	}
}

// generic remove function for all the gloom stuff
void organ_remove (edict_t *self, qboolean refund)
{
	if (!self || !self->inuse || self->deadflag == DEAD_DEAD)
		return;

	if (self->mtype == M_COCOON)
	{
		// restore cocooned entity
		if (self->enemy && self->enemy->inuse)
		{
			self->enemy->movetype = self->count;
			self->enemy->svflags &= ~SVF_NOCLIENT;
			self->enemy->flags &= FL_COCOONED;//4.4
		}
	}

	if (self->activator && self->activator->inuse)
	{
		if (!self->monsterinfo.slots_freed)
		{
			if (self->mtype == M_OBSTACLE)
				self->activator->num_obstacle--;
			if (self->mtype == M_BOX)
				self->activator->num_box--;
			else if (self->mtype == M_SPIKER)
				self->activator->num_spikers--;
			else if (self->mtype == M_HEALER)
				self->activator->healer = NULL;
			else if (self->mtype == M_COCOON)
				self->activator->cocoon = NULL;
			else if (self->mtype == M_GASSER)
				self->activator->num_gasser--;
			else if (self->mtype == M_SPIKEBALL)
				self->activator->num_spikeball--;

			self->monsterinfo.slots_freed = true;
		}

		if (refund)
			self->activator->client->pers.inventory[power_cube_index] += (self->health / self->max_health) * self->monsterinfo.cost;
	}

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_NOCLIENT;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;
}

qboolean organ_checkowner (edict_t *self)
{
	qboolean remove = false;

	// make sure activator exists
	if (!self->activator || !self->activator->inuse)
		remove = true;
	// make sure player activator is alive
	else if (self->activator->client && (self->activator->health < 1 
		|| (self->activator->client->respawn_time > level.time) 
		|| self->activator->deadflag == DEAD_DEAD))
		remove = true;

	if (remove)
	{
		organ_remove(self, false);
		return false;
	}

	return true;
}

void organ_restoreMoveType (edict_t *self)
{
	if (self->movetype_prev && level.framenum >= self->movetype_frame)
	{
		self->movetype = self->movetype_prev;
		self->movetype_prev = 0;
	}
}

void organ_removeall (edict_t *ent, char *classname, qboolean refund)
{
	edict_t *e = NULL;

	while((e = G_Find(e, FOFS(classname), classname)) != NULL) 
	{
		if (e && e->activator && e->activator->inuse && (e->activator == ent) && !RestorePreviousOwner(e))
			organ_remove(e, refund);
	}
}

//Talent: Exploding Bodies
qboolean organ_explode (edict_t *self)
{
	int damage, radius, talentLevel = getTalentLevel(self->activator, TALENT_EXPLODING_BODIES);

	if (talentLevel < 1 || self->style)
		return false;

	// cause the damage
	damage = radius = 100 * talentLevel;
	if (radius > 200)
		radius = 200;
	T_RadiusDamage (self, self, damage, self, radius, MOD_CORPSEEXPLODE);

	gi.sound (self, CHAN_VOICE, gi.soundindex (va("spells/corpse_explode%d.wav", GetRandom(1, 6))), 1, ATTN_NORM, 0);
	return true;
}

#define HEALER_INITIAL_HEALTH		100
#define HEALER_ADDON_HEALTH			40
#define HEALER_FRAMES_GROW_START	0
#define HEALER_FRAMES_GROW_END		15
#define HEALER_FRAMES_START			16
#define HEALER_FRAMES_END			26
#define HEALER_FRAME_DEAD			4
#define HEALER_COST					50
#define HEALER_DELAY				1.0

void healer_heal (edict_t *self, edict_t *other)
{
	float	value;
	//int	talentLevel = getTalentLevel(self->activator, TALENT_SUPER_HEALER);
	qboolean regenerate = false;

	// (apple)
	// In the healer's case, checking for the health 
	// self->health >= 1 should be enough,
	// but used G_EntIsAlive for consistency.
	if (G_EntIsAlive(other) && G_EntIsAlive(self) && OnSameTeam(self, other) && other != self)
	{
		int frames = 5000 / (15 * self->monsterinfo.level); // idk how much would this change tbh

		value = 1.0 + 0.1 * getTalentLevel(self->activator, TALENT_SUPER_HEALER);
		// regenerate health
		if (M_Regenerate(other, frames, 1, value, true, false, false, &other->monsterinfo.regen_delay2))
			regenerate = true;

		// regenerate armor
		/*
		if (talentLevel > 0)
		{
			frames *= (3.0 - (0.4 * talentLevel)); // heals 100% armor in 5 seconds at talent level 5
			if (M_Regenerate(other, frames, 10,  1.0, false, true, false, &other->monsterinfo.regen_delay3))
				regenerate = true;
		}*/
		
		// play healer sound if health or armor was regenerated
		if (regenerate && (level.time > self->msg_time))
		{
			gi.sound(self, CHAN_AUTO, gi.soundindex("organ/healer1.wav"), 1, ATTN_NORM, 0);
			self->msg_time = level.time + 1.5;
		}
	}
}

void healer_attack (edict_t *self)
{
	edict_t *e=NULL;

	while ((e = findradius(e, self->s.origin, 96)) != NULL)
		healer_heal(self, e);
}

void healer_think (edict_t *self)
{
	if (!G_EntIsAlive(self->activator))
	{
		organ_remove(self, false);
		return;
	}

	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// don't slide
	if (self->groundentity)
		VectorClear(self->velocity);
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	if (level.time > self->lasthurt + 1.0)
		M_Regenerate(self, 300, 10, 1.0, true, false, false, &self->monsterinfo.regen_delay1);

	G_RunFrames(self, HEALER_FRAMES_START, HEALER_FRAMES_END, false);

	self->nextthink = level.time + FRAMETIME;
}

void healer_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(ent, other, plane, surf);
	healer_heal(ent, other);	
}

void healer_grow (edict_t *self)
{
	if (!G_EntIsAlive(self->activator))
	{
		organ_remove(self, false);
		return;
	}

	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// don't slide
	if (self->groundentity)
		VectorClear(self->velocity);
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == HEALER_FRAMES_GROW_END)
	{
		self->think = healer_think;
		self->touch = healer_touch;
		self->style = 0;// done growing
		return;
	}

	if (self->s.frame == HEALER_FRAMES_GROW_START)
		gi.sound(self, CHAN_VOICE, gi.soundindex("organ/organe3.wav"), 1, ATTN_STATIC, 0);

	G_RunFrames(self, HEALER_FRAMES_GROW_START, HEALER_FRAMES_GROW_END, false);
}

void healer_dead (edict_t *self)
{
	if (level.time > self->delay)
	{
		organ_remove(self, false);
		return;
	}

	if (level.time == self->delay - 5)
		self->s.effects |= EF_PLASMA;
	else if (level.time == self->delay - 2)
		self->s.effects |= EF_SPHERETRANS;

	self->nextthink = level.time + FRAMETIME;
}

void healer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int n;

	if (!self->monsterinfo.slots_freed && self->activator && self->activator->inuse)
	{
		self->activator->healer = NULL;
		self->monsterinfo.slots_freed = true;
	
		if (PM_MonsterHasPilot(attacker))
			attacker = attacker->owner;

		if (attacker->client)
			safe_cprintf(self->activator, PRINT_HIGH, "Your healer was killed by %s\n", attacker->client->pers.netname);
		else if (attacker->mtype)
			safe_cprintf(self->activator, PRINT_HIGH, "Your healer was killed by a %s\n", V_GetMonsterName(attacker));
		else
			safe_cprintf(self->activator, PRINT_HIGH, "Your healer was killed by a %s\n", attacker->classname);
	}

	if (self->health <= self->gib_health || organ_explode(self))
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		organ_remove(self, false);
		return;
	}

	if (self->deadflag == DEAD_DYING)
		return;

	self->think = healer_dead;
	self->deadflag = DEAD_DYING;
	self->delay = level.time + 20.0;
	self->nextthink = level.time + FRAMETIME;
	self->s.frame = HEALER_FRAME_DEAD;
	self->maxs[2] = 4;
	gi.linkentity(self);
}

edict_t *CreateHealer (edict_t *ent, int skill_level)
{
	edict_t *e;

	e = G_Spawn();
	e->style = 1; //growing
	e->activator = ent;
	e->think = healer_grow;
	e->nextthink = level.time + FRAMETIME;
	e->s.modelindex = gi.modelindex ("models/objects/organ/healer/tris.md2");
	e->s.renderfx |= RF_IR_VISIBLE;
	e->solid = SOLID_BBOX;
	e->movetype = MOVETYPE_TOSS;
	e->clipmask = MASK_MONSTERSOLID;
	e->mass = 500;
	e->classname = "healer";
	e->takedamage = DAMAGE_AIM;
	e->health = e->max_health = HEALER_INITIAL_HEALTH + HEALER_ADDON_HEALTH * skill_level;
	e->monsterinfo.level = skill_level;
	e->gib_health = -200;
	e->die = healer_die;
	e->touch = V_Touch;
	VectorSet(e->mins, -28, -28, 0);
	VectorSet(e->maxs, 28, 28, 32);
	e->mtype = M_HEALER;
	ent->healer = e;

	return e;
}

void Cmd_Healer_f (edict_t *ent)
{
	edict_t *healer;
	vec3_t	start;

	if (ent->healer && ent->healer->inuse)
	{
		organ_remove(ent->healer, true);
		safe_cprintf(ent, PRINT_HIGH, "Healer removed\n");
		return;
	}

	if (!V_CanUseAbilities(ent, HEALER, HEALER_COST, true))
		return;

	healer = CreateHealer(ent, ent->myskills.abilities[HEALER].current_level);
	if (!G_GetSpawnLocation(ent, 100, healer->mins, healer->maxs, start))
	{
		ent->healer = NULL;
		G_FreeEdict(healer);
		return;
	}
	VectorCopy(start, healer->s.origin);
	VectorCopy(ent->s.angles, healer->s.angles);
	healer->s.angles[PITCH] = 0;
	gi.linkentity(healer);
	healer->monsterinfo.cost = HEALER_COST;

	ent->client->ability_delay = level.time + HEALER_DELAY;
	ent->client->pers.inventory[power_cube_index] -= HEALER_COST;

	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}

#define SPIKER_FRAMES_GROW_START		0
#define SPIKER_FRAMES_GROW_END			12
#define SPIKER_FRAMES_NOAMMO_START		14
#define SPIKER_FRAMES_NOAMMO_END		17
#define SPIKER_FRAMES_REARM_START		19
#define SPIKER_FRAMES_REARM_END			23
#define SPIKER_FRAME_READY				13
#define SPIKER_FRAME_DEAD				18

void spiker_dead (edict_t *self)
{
	if (level.time > self->delay)
	{
		organ_remove(self, false);
		return;
	}

	if (level.time == self->delay - 5)
		self->s.effects |= EF_PLASMA;
	else if (level.time == self->delay - 2)
		self->s.effects |= EF_SPHERETRANS;

	self->nextthink = level.time + FRAMETIME;
}

void spiker_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int max = SPIKER_MAX_COUNT;
	int cur;

	if (!self->monsterinfo.slots_freed && self->activator && self->activator->inuse)
	{
		self->activator->num_spikers--;
		cur = self->activator->num_spikers;
		self->monsterinfo.slots_freed = true;
		
		if (PM_MonsterHasPilot(attacker))
			attacker = attacker->owner;

		if (attacker->client)
			safe_cprintf(self->activator, PRINT_HIGH, "Your spiker was killed by %s (%d/%d remain)\n", attacker->client->pers.netname, cur, max);
		else if (attacker->mtype)
			safe_cprintf(self->activator, PRINT_HIGH, "Your spiker was killed by a %s (%d/%d remain)\n", V_GetMonsterName(attacker), cur, max);
		else
			safe_cprintf(self->activator, PRINT_HIGH, "Your spiker was killed by a %s (%d/%d remain)\n", attacker->classname, cur, max);
	}

	if (self->health <= self->gib_health || organ_explode(self))
	{
		int n;

		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		organ_remove(self, false);
		return;
	}

	if (self->deadflag == DEAD_DYING)
		return;

	self->think = spiker_dead;
	self->deadflag = DEAD_DYING;
	self->delay = level.time + 5;
	self->nextthink = level.time + FRAMETIME;
	self->s.frame = SPIKER_FRAME_DEAD;
	self->movetype = MOVETYPE_TOSS;
	self->maxs[2] = 16;
	gi.linkentity(self);
}

void spiker_attack (edict_t *self)
{
	float	dist;
	float	range=SPIKER_INITIAL_RANGE+SPIKER_ADDON_RANGE*self->monsterinfo.level;
	int		speed=SPIKER_INITIAL_SPEED+SPIKER_ADDON_SPEED*self->monsterinfo.level;
	vec3_t	forward, start, end;
	edict_t *e=NULL;

	if (self->monsterinfo.attack_finished > level.time)
		return;

	VectorCopy(self->s.origin, start);
	start[2] = self->absmax[2] - 16;

	while ((e = findradius(e, self->s.origin, range)) != NULL)
	{
		if (!G_ValidTarget(self, e, true))
			continue;
		
		// copy target location
		G_EntMidPoint(e, end);

		// calculate distance to target
		dist = distance(e->s.origin, start);

		// move our target point based on projectile and enemy velocity
		VectorMA(end, (float)dist/speed, e->velocity, end);
				
		// calculate direction vector to target
		VectorSubtract(end, start, forward);
		VectorNormalize(forward);

		fire_spike(self, start, forward, self->dmg, 0, speed);
		
		//FIXME: only need to do this once
		self->monsterinfo.attack_finished = level.time + SPIKER_REFIRE_DELAY;
		self->s.frame = SPIKER_FRAMES_NOAMMO_START;
	
		gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/twang.wav"), 1, ATTN_NORM, 0);
		//health_factor = 1;
		

	
		
	}	
}

void NearestNodeLocation (vec3_t start, vec3_t node_loc);
int FindPath(vec3_t start, vec3_t destination);
void spiker_think (edict_t *self)
{
	edict_t *e=NULL;

	if (!organ_checkowner(self))
		return;

	organ_restoreMoveType(self);

	if (self->removetime > 0)
	{
		qboolean converted=false;

		if (self->flags & FL_CONVERTED)
			converted = true;

		if (level.time > self->removetime)
		{
			// if we were converted, try to convert back to previous owner
			if (converted && self->prev_owner && self->prev_owner->inuse)
			{
				if (!ConvertOwner(self->prev_owner, self, 0, false))
				{
					organ_remove(self, false);
					return;
				}
			}
			else
			{
				organ_remove(self, false);
				return;
			}
		}
		// warn the converted monster's current owner
		else if (converted && self->activator && self->activator->inuse && self->activator->client 
			&& (level.time > self->removetime-5) && !(level.framenum%10))
				safe_cprintf(self->activator, PRINT_HIGH, "%s conversion will expire in %.0f seconds\n", 
					V_GetMonsterName(self), self->removetime-level.time);	
	}

	// try to attack something
	spiker_attack(self);

	V_HealthCache(self, (int)(0.2 * self->max_health), 1);

	//if (level.time > self->lasthurt + 1.0) 
	//	M_Regenerate(self, 300, 10, true, false, false, &self->monsterinfo.regen_delay1);

	// run animation frames
	if (self->monsterinfo.attack_finished - 0.5 > level.time)
	{
		if (self->s.frame != SPIKER_FRAME_READY)
			G_RunFrames(self, SPIKER_FRAMES_NOAMMO_START, SPIKER_FRAMES_NOAMMO_END, false);
	}
	else
	{
		if (self->s.frame != SPIKER_FRAME_READY && self->s.frame != SPIKER_FRAMES_REARM_END)
			G_RunFrames(self, SPIKER_FRAMES_REARM_START, SPIKER_FRAMES_REARM_END, false);
		else
			self->s.frame = SPIKER_FRAME_READY;
	}

	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// ground friction to prevent excessive sliding
	if (self->groundentity)
	{
		self->velocity[0] *= 0.2;
		self->velocity[1] *= 0.2;
	}
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	gi.linkentity(self);
	
	self->nextthink = level.time + FRAMETIME;
}

void spiker_grow (edict_t *self) //Crecimiento de Spiker
{
	if (!organ_checkowner(self))
		return;

	organ_restoreMoveType(self);

	V_HealthCache(self, (int)(0.2 * self->max_health), 1);

	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// don't slide
	if (self->groundentity)
		VectorClear(self->velocity);
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	self->s.effects |= EF_PLASMA;

	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == SPIKER_FRAMES_GROW_END && self->health >= self->max_health)
	{
		self->s.frame = SPIKER_FRAME_READY;
		self->think = spiker_think;
		self->style = 0;//done growing
		return;
	}

	if (self->s.frame == SPIKER_FRAMES_GROW_START)
		gi.sound(self, CHAN_VOICE, gi.soundindex("organ/organe3.wav"), 1, ATTN_NORM, 0);

	if (self->s.frame != SPIKER_FRAMES_GROW_END)
		G_RunFrames(self, SPIKER_FRAMES_GROW_START, SPIKER_FRAMES_GROW_END, false);
}

edict_t *CreateSpiker (edict_t *ent, int skill_level)
{
	edict_t *e;

	e = G_Spawn();
	e->style = 1; //growing
	e->activator = ent;
	e->think = spiker_grow;
	e->nextthink = level.time + FRAMETIME;
	e->s.modelindex = gi.modelindex ("models/objects/organ/spiker/tris.md2");
	e->s.renderfx |= RF_IR_VISIBLE;
	e->solid = SOLID_BBOX;
	e->movetype = MOVETYPE_TOSS;
	e->clipmask = MASK_MONSTERSOLID;
	e->svflags |= SVF_MONSTER;//Note/Important/Hint: tells MOVETYPE_STEP physics to clip on any solid object (not just walls)
	e->mass = 500;
	e->classname = "spiker";
	e->takedamage = DAMAGE_AIM;
	e->max_health = SPIKER_INITIAL_HEALTH + SPIKER_ADDON_HEALTH * skill_level;
	e->health = 0.5*e->max_health;
	e->dmg = SPIKER_INITIAL_DAMAGE + SPIKER_ADDON_DAMAGE * skill_level;

	e->monsterinfo.level = skill_level;
	e->gib_health = -250;
	e->die = spiker_die;
	e->touch = organ_touch;
	VectorSet(e->mins, -24, -24, 0);
	VectorSet(e->maxs, 24, 24, 48);
	e->mtype = M_SPIKER;

	ent->num_spikers++;

	return e;
}


void Cmd_Spiker_f (edict_t *ent)
{
	int		cost = SPIKER_COST;
	edict_t *spiker;
	vec3_t	start;

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		organ_removeall(ent, "spiker", true);
		safe_cprintf(ent, PRINT_HIGH, "Spikers removed\n");
		ent->num_spikers = 0;
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	if (!V_CanUseAbilities(ent, SPIKER, cost, true))
		return;

	if (ent->num_spikers >= SPIKER_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum amount of spikers (%d)\n", SPIKER_MAX_COUNT);
		return;
	}

	spiker = CreateSpiker(ent, ent->myskills.abilities[SPIKER].current_level);
	if (!G_GetSpawnLocation(ent, 100, spiker->mins, spiker->maxs, start))
	{
		ent->num_spikers--;
		G_FreeEdict(spiker);
		return;
	}
	VectorCopy(start, spiker->s.origin);
	VectorCopy(ent->s.angles, spiker->s.angles);
	spiker->s.angles[PITCH] = 0;
	gi.linkentity(spiker);
	spiker->monsterinfo.attack_finished = level.time + 1.0;
	spiker->monsterinfo.cost = 35 + 1 * ent->myskills.abilities[SPIKER].current_level;
	T_Damage(ent, ent, ent, ent->velocity, ent->s.origin, ent->s.origin, spiker->monsterinfo.cost, 0, 0, 0);
	ent->client->damage_blood = 1;
	safe_cprintf(ent, PRINT_HIGH, "Spiker created (%d/%d)\n", ent->num_spikers, SPIKER_MAX_COUNT);
	ent->client->ability_delay = level.time + SPIKER_DELAY;
	ent->holdtime = level.time + SPIKER_DELAY;
	
	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}




















#define OBSTACLE_INITIAL_HEALTH			0		
#define OBSTACLE_ADDON_HEALTH			195
#define OBSTACLE_INITIAL_DAMAGE			0
#define OBSTACLE_ADDON_DAMAGE			75	
#define OBSTACLE_COST					25
#define OBSTACLE_DELAY					0.5

#define OBSTACLE_FRAMES_GROW_START		0
#define OBSTACLE_FRAMES_GROW_END		9
#define OBSTACLE_FRAME_READY			12
#define OBSTACLE_FRAME_DEAD				13

void obstacle_dead (edict_t *self)
{
	if (level.time > self->delay)
	{
		organ_remove(self, false);
		return;
	}

	if (level.time == self->delay - 5)
		self->s.effects |= EF_PLASMA;
	else if (level.time == self->delay - 2)
		self->s.effects |= EF_SPHERETRANS;

	self->nextthink = level.time + FRAMETIME;
}

void obstacle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int max = OBSTACLE_MAX_COUNT;
	int cur;

	if (!self->monsterinfo.slots_freed && self->activator && self->activator->inuse)
	{
		self->activator->num_obstacle--;
		cur = self->activator->num_obstacle;
		self->monsterinfo.slots_freed = true;
		
		if (PM_MonsterHasPilot(attacker))
			attacker = attacker->owner;

		if (attacker->client)
			safe_cprintf(self->activator, PRINT_HIGH, "Your obstacle was killed by %s (%d/%d remain)\n", attacker->client->pers.netname, cur, max);
		else if (attacker->mtype)
			safe_cprintf(self->activator, PRINT_HIGH, "Your obstacle was killed by a %s (%d/%d remain)\n", V_GetMonsterName(attacker), cur, max);
		else
			safe_cprintf(self->activator, PRINT_HIGH, "Your obstacle was killed by a %s (%d/%d remain)\n", attacker->classname, cur, max);
	}

	if (self->health <= self->gib_health || organ_explode(self))
	{
		int n;

		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		organ_remove(self, false);
		return;
	}

	if (self->deadflag == DEAD_DYING)
		return;

	self->think = spiker_dead;
	self->deadflag = DEAD_DYING;
	self->delay = level.time + 20.0;
	self->nextthink = level.time + FRAMETIME;
	self->s.frame = OBSTACLE_FRAME_DEAD;
	self->movetype = MOVETYPE_TOSS;
	self->touch = V_Touch;
	self->maxs[2] = 16;
	gi.linkentity(self);
}

void obstacle_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	organ_touch(self, other, plane, surf);

	if (other && other->inuse && other->takedamage && !OnSameTeam(self->activator, other) 
		&& (level.framenum >= self->monsterinfo.jumpup))
	{
		T_Damage(other, self, self, self->velocity, self->s.origin, 
			plane->normal, self->dmg, 0, 0, MOD_OBSTACLE);
		self->monsterinfo.jumpup = level.framenum + 1;
	}
	else if (other && other->inuse && other->takedamage)
	{
		self->svflags &= ~SVF_NOCLIENT;
		self->monsterinfo.idle_frames = 0;
	}
}

void obstacle_cloak (edict_t *self)
{
	// already cloaked
	if (self->svflags & SVF_NOCLIENT)
	{
		// random chance for obstacle to become uncloaked for a frame
		//if (level.time > self->lasthbshot && random() <= 0.05)
		if (!(level.framenum % 50) && random() > 0.5)
		{
			self->svflags &= ~SVF_NOCLIENT;
			//self->lasthbshot = level.time + 1.0;
		}
		return;
	}

	// cloak after idling for awhile
	if (self->monsterinfo.idle_frames >= self->monsterinfo.nextattack)
		self->svflags |= SVF_NOCLIENT;
}

void obstacle_move (edict_t *self)
{
	// check for ground entity
	if (!M_CheckBottom(self) || self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// check for movement/sliding
	if (!(int)self->velocity[0] && !(int)self->velocity[1] && !(int)self->velocity[2])
	{
		// stick to the ground
		if (self->groundentity && self->groundentity == world)
			self->movetype = MOVETYPE_NONE;
		else
			self->movetype = MOVETYPE_STEP;

		// increment idle frames
		self->monsterinfo.idle_frames++;
	}
	else
	{
		// de-cloak
		self->svflags &= ~SVF_NOCLIENT;
		// reset idle frames
		self->monsterinfo.idle_frames = 0;
	}

	// ground friction to prevent excessive sliding
	if (self->groundentity)
	{
		self->velocity[0] *= 0.5;
		self->velocity[1] *= 0.5;
	}
}

void obstacle_think (edict_t *self)
{
	if (!organ_checkowner(self))
		return;

	organ_restoreMoveType(self);
	obstacle_cloak(self);
	obstacle_move(self);

	V_HealthCache(self, (int)(0.2 * self->max_health), 1);

//	if (level.time > self->lasthurt + 1.0)
//		M_Regenerate(self, 300, 10, true, false, false, &self->monsterinfo.regen_delay1);

	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	self->nextthink = level.time + FRAMETIME;
}

void obstacle_grow (edict_t *self)
{
	if (!organ_checkowner(self))
		return;

	organ_restoreMoveType(self);

	V_HealthCache(self, (int)(0.2 * self->max_health), 1);

	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// don't slide
	if (self->groundentity)
		VectorClear(self->velocity);
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	self->s.effects |= EF_PLASMA;

	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == OBSTACLE_FRAMES_GROW_END && self->health >= self->max_health)
	{
		self->style = 0;//done growing
		self->s.frame = OBSTACLE_FRAME_READY;
		self->think = obstacle_think;
		self->touch = obstacle_touch;
		return;
	}

	if (self->s.frame == OBSTACLE_FRAMES_GROW_START)
		gi.sound(self, CHAN_VOICE, gi.soundindex("organ/organe3.wav"), 1, ATTN_NORM, 0);

	if (self->s.frame != OBSTACLE_FRAMES_GROW_END)
		G_RunFrames(self, OBSTACLE_FRAMES_GROW_START, OBSTACLE_FRAMES_GROW_END, false);
}

edict_t *CreateObstacle (edict_t *ent, int skill_level)
{
	edict_t *e;

	e = G_Spawn();
	e->style = 1; //growing
	e->activator = ent;
	e->think = obstacle_grow;
	e->nextthink = level.time + FRAMETIME;
	e->s.modelindex = gi.modelindex ("models/objects/organ/obstacle/tris.md2");
	e->s.renderfx |= RF_IR_VISIBLE;
	e->solid = SOLID_BBOX;
	e->movetype = MOVETYPE_TOSS;
	e->svflags |= SVF_MONSTER;
	e->clipmask = MASK_MONSTERSOLID;
	e->mass = 500;
	e->classname = "obstacle";
	e->takedamage = DAMAGE_AIM;
	e->max_health = OBSTACLE_INITIAL_HEALTH + OBSTACLE_ADDON_HEALTH * skill_level;
	e->health = 0.5*e->max_health;
	e->dmg = OBSTACLE_INITIAL_DAMAGE + OBSTACLE_ADDON_DAMAGE * skill_level;
	e->monsterinfo.nextattack = 100 - 9 * getTalentLevel(ent, TALENT_PHANTOM_OBSTACLE);
	e->monsterinfo.level = skill_level;
	e->gib_health = -250;
	e->die = obstacle_die;
	e->touch = organ_touch;
	VectorSet(e->mins, -16, -16, 0);
	VectorSet(e->maxs, 16, 16, 40);
	e->mtype = M_OBSTACLE;

	ent->num_obstacle++;

	return e;
}

void Cmd_Obstacle_f (edict_t *ent)
{
	int		cost = OBSTACLE_COST;
	edict_t *obstacle;
	vec3_t	start;

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		organ_removeall(ent, "obstacle", true);
		safe_cprintf(ent, PRINT_HIGH, "Obstacles removed\n");
		ent->num_obstacle = 0;
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	if (!V_CanUseAbilities(ent, OBSTACLE, cost, true))
		return;

	if (ent->num_obstacle >= OBSTACLE_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum amount of obstacles (%d)\n", OBSTACLE_MAX_COUNT);
		return;
	}

	obstacle = CreateObstacle(ent, ent->myskills.abilities[OBSTACLE].current_level);
	if (!G_GetSpawnLocation(ent, 100, obstacle->mins, obstacle->maxs, start))
	{
		ent->num_obstacle--;
		G_FreeEdict(obstacle);
		return;
	}
	VectorCopy(start, obstacle->s.origin);
	VectorCopy(ent->s.angles, obstacle->s.angles);
	obstacle->s.angles[PITCH] = 0;
	gi.linkentity(obstacle);
	obstacle->monsterinfo.cost = cost;
	
	// ent->client->pers.inventory[power_cube_index] -= cost; Old Method
	ent->client->ability_delay = level.time + OBSTACLE_DELAY;
	safe_cprintf(ent, PRINT_HIGH, "Obstacle created (%d/%d)\n", ent->num_obstacle,OBSTACLE_MAX_COUNT);
	
	T_Damage(ent, ent, ent, ent->velocity, ent->s.origin, ent->s.origin, obstacle->monsterinfo.cost, 0, 0, 0);
	ent->client->damage_blood = 1;

	//ent->holdtime = level.time + OBSTACLE_DELAY;

	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// DEFENSIVE BOX HABILIDAD ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BOX_INITIAL_HEALTH			150		
#define BOX_ADDON_HEALTH			75
#define BOX_INITIAL_DAMAGE			120
#define BOX_ADDON_DAMAGE			40	
#define BOX_PC_COST					50
#define BOX_DELAY					0.5




void box_dead(edict_t *self){
	int danonepe = BOX_INITIAL_DAMAGE;
	int mejorao = BOX_ADDON_DAMAGE;

	if (level.time > self->delay)
	{
		organ_remove(self, false);
		return;
	}
	if (level.time == self->delay - 5)
		self->s.effects |= EF_PLASMA;
	else if (level.time == self->delay - 2)
		self->s.effects |= EF_SPHERETRANS;
	self->nextthink = 2;
}

void box_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point){
	int max = BOX_MAX_COUNT;
	int cur;

	if (!self->monsterinfo.slots_freed && self->activator && self->activator->inuse)
	{
		self->activator->num_box--;
		cur = self->activator->num_box;
		self->monsterinfo.slots_freed = true;

		if (PM_MonsterHasPilot(attacker))
			attacker = attacker->owner;

		if (attacker->client)
			safe_cprintf(self->activator, PRINT_HIGH, "Your box was killed by %s (%d/%d remain)\n", attacker->client->pers.netname, cur, max);
		else if (attacker->mtype)
			safe_cprintf(self->activator, PRINT_HIGH, "Your box was killed by a %s (%d/%d remain)\n", V_GetMonsterName(attacker), cur, max);
		else
			safe_cprintf(self->activator, PRINT_HIGH, "Your box was killed by a %s (%d/%d remain)\n", attacker->classname, cur, max);
	}

	if (self->health <= self->gib_health || organ_explode(self)){
		int n;

		gi.sound(self, CHAN_VOICE, gi.soundindex("makron/popup.wav"), 1, ATTN_NORM, 0);
		for (n = 0; n < 2; n++)
			ThrowGib(self, "models/crate/crate32.md2", damage, GIB_ORGANIC);
		for (n = 0; n < 4; n++)
			ThrowGib(self, "models/crate/crate32.md2", damage, GIB_ORGANIC);
		organ_remove(self, false);
		return;
	}

	if (self->deadflag == DEAD_DYING)
		return;

	self->think = spiker_dead;
	self->deadflag = DEAD_DYING;
	self->delay = 0;
	self->nextthink = level.time + FRAMETIME;
	self->s.frame = OBSTACLE_FRAME_DEAD;
	self->movetype = MOVETYPE_TOSS;
	self->touch = V_Touch;
	self->maxs[2] = 16;
	gi.linkentity(self);
}

void box_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf){
	organ_touch(self, other, plane, surf);

	if (other && other->inuse  && !OnSameTeam(self->activator, other)
		&& (level.framenum >= self->monsterinfo.jumpup))
	{			
		self->monsterinfo.jumpup = level.framenum + 1;
	}
	else if (other && other->inuse )
	{
		self->svflags &= ~SVF_NOCLIENT;
		self->monsterinfo.idle_frames = 0;
	}
}



void box_move(edict_t *self){
	// check for ground entity
	if (!M_CheckBottom(self) || self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround(self);
	}

	// check for movement/sliding
	if (!(int)self->velocity[0] && !(int)self->velocity[1] && !(int)self->velocity[2])
	{
		// stick to the ground
		if (self->groundentity && self->groundentity == world)
			self->movetype = MOVETYPE_NONE;
		else
			self->movetype = MOVETYPE_STEP;

		// increment idle frames
		self->monsterinfo.idle_frames++;

		// ground friction to prevent excessive sliding
		if (self->groundentity)
		{
			self->velocity[0] *= 0.001;
			self->velocity[1] *= 0.001;
		}
	}
	if (self->groundentity)
		VectorClear(self->velocity);
}
void box_think(edict_t *self){
	if (!organ_checkowner(self))
		return;

	organ_restoreMoveType(self);
	
	box_move(self);

	V_HealthCache(self, (int)(0.1 * self->max_health), 1);

	
	M_CatagorizePosition(self);
	M_WorldEffects(self);
	M_SetEffects(self);

	self->nextthink = level.time + FRAMETIME;
}

edict_t *CreateBox(edict_t *ent, int skill_level)
{
	edict_t *e;

	e = G_Spawn();

	e->activator = ent;
	e->nextthink = level.time + FRAMETIME;
	e->s.modelindex = gi.modelindex("models/crate/crate64.md2");
	e->s.renderfx |= RF_IR_VISIBLE;
	e->solid = SOLID_BBOX;
	e->movetype = MOVETYPE_TOSS;
	e->svflags |= SVF_MONSTER;
	e->clipmask = MASK_MONSTERSOLID;
	e->mass = 5500;
	e->classname = "box";
	e->takedamage = DAMAGE_AIM;
	e->max_health = BOX_INITIAL_HEALTH + 950 * ent->myskills.level;
	e->health = 0.5*e->max_health;
	e->dmg = 0;
	e->monsterinfo.level = skill_level;
	e->gib_health = -250;
	e->die = box_die;
	e->touch = organ_touch;
	VectorSet(e->mins, -32, -32, 0);
	VectorSet(e->maxs, 32, 32,64);
	e->mtype = M_BOX;

	ent->num_box++;

	return e;
}

void Cmd_box_f(edict_t *ent)
{

	int		cost = BOX_PC_COST;
	edict_t *box;
	vec3_t	start;

	if (Q_strcasecmp(gi.args(), "remove") == 0)
	{
		organ_removeall(ent, "box", true);
		safe_cprintf(ent, PRINT_HIGH, "Boxes removed\n");
		ent->num_box = 0;
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}



	if (!V_CanUseAbilities(ent, BOX, cost, true))
		return;

	if (ent->num_box >= 2)
	{
		safe_cprintf(ent, PRINT_HIGH, "Too many boxes (%d)\n", 2);
		return;
	}

	box = CreateBox(ent, ent->myskills.abilities[BOX].current_level);

		

	if (!G_GetSpawnLocation(ent, 100, box->mins, box->maxs, start))
	{
		ent->num_box--;
		G_FreeEdict(box);
		return;
	}
	VectorCopy(start, box->s.origin);
	VectorCopy(ent->s.angles, box->s.angles);
	box->s.angles[PITCH] = 0;
	gi.linkentity(box);
	box->monsterinfo.cost = cost;

	ent->client->pers.inventory[power_cube_index] -= 35;

	safe_cprintf(ent, PRINT_HIGH, "Defensive box created (%d/%d)\n", ent->num_box, 2);
//
//	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define GASSER_FRAMES_ATTACK_START		1	
#define GASSER_FRAMES_ATTACK_END		6
#define GASSER_FRAMES_REARM_START		7
#define GASSER_FRAMES_REARM_END			8
#define GASSER_FRAMES_IDLE_START		9
#define GASSER_FRAMES_IDLE_END			12
#define GASSER_FRAME_DEAD				6

#define GASSER_RANGE					150
#define GASSER_REFIRE					2.5
#define GASSER_INITIAL_DAMAGE			0
#define GASSER_ADDON_DAMAGE				45
#define GASSER_INITIAL_HEALTH			150
#define GASSER_ADDON_HEALTH				80
#define GASSER_INITIAL_ATTACK_RANGE		100
#define GASSER_ADDON_ATTACK_RANGE		0
#define GASSER_COST						25
#define GASSER_DELAY					1.0

#define GASCLOUD_FRAMES_GROW_START		10
#define GASCLOUD_FRAMES_GROW_END		18
#define GASCLOUD_FRAMES_IDLE_START		19
#define GASCLOUD_FRAMES_IDLE_END		23

#define GASCLOUD_POISON_DURATION		10.0
#define GASCLOUD_POISON_FACTOR			0.1

void poison_think (edict_t *self)
{
	if (!G_EntIsAlive(self->enemy) || !G_EntIsAlive(self->activator) || (level.time > self->delay))
	{
		G_FreeEdict(self);
		return;
	}

	if (level.framenum >= self->monsterinfo.nextattack)
	{
		T_Damage(self->enemy, self, self->activator, vec3_origin, self->enemy->s.origin, vec3_origin, self->dmg, 0, 0, self->style);
		self->monsterinfo.nextattack = level.framenum + floattoint(self->random);
		self->random *= 1.25;
	}

	self->nextthink = level.time + FRAMETIME;
}

void CreatePoison (edict_t *ent, edict_t *targ, int damage, float duration, int meansOfDeath)
{
	edict_t *e;

	e = G_Spawn();
	e->activator = ent;
	e->solid = SOLID_NOT;
	e->movetype = MOVETYPE_NOCLIP;
	e->svflags |= SVF_NOCLIENT;
	e->classname = "poison";
	e->delay = level.time + duration;
	e->owner = e->enemy = targ;
	e->random = 1; // starting refire delay (in frames)
	e->dmg = damage;
	e->mtype = POISON;
	e->style = meansOfDeath;
	e->think = poison_think;
	e->nextthink = level.time + FRAMETIME;
	VectorCopy(targ->s.origin, e->s.origin);
	gi.linkentity(e);

	if (!que_addent(targ->curses, e, duration))
		G_FreeEdict(e);
}

void gascloud_sparks (edict_t *self, int num, int radius)
{
	int		i;
	vec3_t	start;

	// 0 = black, 8 = grey, 15 = white, 16 = light brown, 20 = brown, 57 = light orange, 66 = orange/red, 73 = maroon
	// 106 = pink, 113 = light blue, 119 = blue, 123 = dark blue, 200 = pale green, 205 = dark green, 209 = bright green
	// 217 = white, 220 = yellow, 226 = orange, 231 = red/orange, 240 = red, 243 = dark blue

	for (i=0; i<num; i++)
	{
		VectorCopy(self->s.origin, start);
		start[0] += crandom() * GetRandom(0, radius);
		start[1] += crandom() * GetRandom(0, radius);
		start[2] += crandom() * GetRandom(0, radius);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(1); // number of sparks
		gi.WritePosition(start);
		gi.WriteDir(vec3_origin);
		gi.WriteByte(GetRandom(200, 209)); // color
		gi.multicast(start, MULTICAST_PVS);
	}
}

void gascloud_runframes (edict_t *self)
{
	if (level.time > self->delay - 0.8)
	{
		G_RunFrames(self, GASCLOUD_FRAMES_GROW_START, GASCLOUD_FRAMES_GROW_END, true);
		self->s.effects |= EF_SPHERETRANS;
	}
	else if (self->s.frame < GASCLOUD_FRAMES_GROW_END)
		G_RunFrames(self, GASCLOUD_FRAMES_GROW_START, GASCLOUD_FRAMES_GROW_END, false);
}

void gascloud_attack (edict_t *self)
{
	que_t	*slot=NULL;
	edict_t *e=NULL;

	while ((e = findradius(e, self->s.origin, self->dmg_radius)) != NULL)
	{
		if (G_ValidTarget(self, e, true))
		{
			// otherwise, update the attack frequency to once per server frame
			if ((slot = que_findtype(e->curses, NULL, POISON)) != NULL)
			{
				slot->ent->random = 1;
				slot->ent->monsterinfo.nextattack = level.framenum + 1;
				slot->ent->delay = level.time + self->radius_dmg;
				slot->time = level.time + self->radius_dmg;
			}
			// if target is not already poisoned, create poison entity
			else
				CreatePoison(self->activator, e, (int)(GASCLOUD_POISON_FACTOR*self->dmg), self->radius_dmg, MOD_GAS);
			T_Damage(e, self, self->activator, vec3_origin, e->s.origin, vec3_origin, (int)((1.0-GASCLOUD_POISON_FACTOR)*self->dmg), 0, 0, MOD_GAS);
		}
		else if (e && e->inuse && e->takedamage && visible(self, e) && !OnSameTeam(self, e))
			T_Damage(e, self, self->activator, vec3_origin, e->s.origin, vec3_origin, self->dmg, 0, 0, MOD_GAS);
	}
}

void gascloud_move (edict_t *self)
{
	trace_t tr;
	vec3_t	start;

	VectorCopy(self->s.origin, start);
	start[2]++;
	tr = gi.trace(self->s.origin, NULL, NULL, start, NULL, MASK_SOLID);
	if (tr.fraction == 1.0)
	{
		self->s.origin[2]++;
		gi.linkentity(self);
	}
}

void gascloud_think (edict_t *self)
{
	if (!G_EntIsAlive(self->activator) || level.time > self->delay)
	{
		G_FreeEdict(self);
		return;
	}
	
	gascloud_sparks(self, 3, (int)self->dmg_radius);
	gascloud_attack(self);
	gascloud_move(self);
	gascloud_runframes(self);
	
	self->nextthink = level.time + FRAMETIME;	
}

void SpawnGasCloud (edict_t *ent, vec3_t start, int damage, float radius, float duration)
{
	edict_t *e;

	e = G_Spawn();
	e->activator = ent->activator;
	e->solid = SOLID_NOT;
	e->movetype = MOVETYPE_NOCLIP;
	e->classname = "gas cloud";
	e->delay = level.time + duration;
	e->radius_dmg = GASCLOUD_POISON_DURATION; // poison effect duration
	e->dmg = damage;
	e->dmg_radius = radius;
	e->think = gascloud_think;
	e->s.modelindex = gi.modelindex ("models/objects/smokexp/tris.md2");
	e->s.skinnum = 1;
	e->s.effects |= EF_PLASMA;
	e->nextthink = level.time + FRAMETIME;
	VectorCopy(start, e->s.origin);
	VectorCopy(ent->s.angles, e->s.angles);
	gi.linkentity(e);
}

void fire_acid (edict_t *self, vec3_t start, vec3_t aimdir, int projectile_damage, float radius, 
				int speed, int acid_damage, float acid_duration);

void gasser_acidattack (edict_t *self)
{
	float	dist;
	float	range=384;
	int		speed=600;
	vec3_t	forward, start, end;
	edict_t *e=NULL;

	if (self->monsterinfo.attack_finished > level.time)
		return;

	VectorCopy(self->s.origin, start);
	start[2] = self->absmax[2] - 16;

	while ((e = findradius(e, self->s.origin, range)) != NULL)
	{
		if (!G_ValidTarget(self, e, true))
			continue;
		
		// copy target location
		G_EntMidPoint(e, end);

		// calculate distance to target
		dist = distance(e->s.origin, start);

		// move our target point based on projectile and enemy velocity
		VectorMA(end, (float)dist/speed, e->velocity, end);
				
		// calculate direction vector to target
		VectorSubtract(end, start, forward);
		VectorNormalize(forward);

		fire_acid(self, self->s.origin, forward, 200, 64, speed, 20, 10.0);
		
		//FIXME: only need to do this once
		self->monsterinfo.attack_finished = level.time + 2.0;
		self->s.frame = GASSER_FRAMES_ATTACK_END-2;
		//gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/twang.wav"), 1, ATTN_NORM, 0);
	}	
}

void gasser_attack (edict_t *self)
{
	vec3_t start;

	if (self->monsterinfo.attack_finished > level.time)
		return;

	VectorCopy(self->s.origin, start);
	start[2] = self->absmax[2] + 8;
	SpawnGasCloud(self, start, self->dmg, self->dmg_radius, 4.0);

	self->s.frame = GASSER_FRAMES_ATTACK_START+2;
	self->monsterinfo.attack_finished = level.time + GASSER_REFIRE;
}

qboolean gasser_findtarget (edict_t *self)
{
	edict_t *e=NULL;

	while ((e = findradius(e, self->s.origin, GASSER_RANGE)) != NULL)
	{
		if (!G_ValidTarget(self, e, true))
			continue;
		self->enemy = e;
		return true;
	}
	self->enemy = NULL;
	return false;
}

void gasser_think (edict_t *self)
{
	if (!organ_checkowner(self))
		return;

	organ_restoreMoveType(self);

//	if (level.time > self->lasthurt + 1.0)
//		M_Regenerate(self, 300, 10, true, false, false, &self->monsterinfo.regen_delay1);

	V_HealthCache(self, (int)(0.2 * self->max_health), 1);

	if (gasser_findtarget(self))
		gasser_attack(self);
	//gasser_acidattack(self);

	if (self->s.frame < GASSER_FRAMES_ATTACK_END)
		G_RunFrames(self, GASSER_FRAMES_ATTACK_START, GASSER_FRAMES_ATTACK_END, false);
	else if (self->s.frame < GASSER_FRAMES_REARM_END && level.time > self->monsterinfo.attack_finished - 0.2)
		G_RunFrames(self, GASSER_FRAMES_REARM_START, GASSER_FRAMES_REARM_END, false);
	else if (level.time > self->monsterinfo.attack_finished)
	{
		gascloud_sparks(self, 1, 32);
		G_RunFrames(self, GASSER_FRAMES_IDLE_START, GASSER_FRAMES_IDLE_END, false);
	}

	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// ground friction to prevent excessive sliding
	if (self->groundentity)
	{
		self->velocity[0] *= 0.5;
		self->velocity[1] *= 0.5;
	}
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	self->nextthink = level.time + FRAMETIME;
}

void gasser_grow (edict_t *self)
{
	if (!organ_checkowner(self))
		return;

	organ_restoreMoveType(self);

	V_HealthCache(self, (int)(0.2 * self->max_health), 1);

	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// ground friction to prevent excessive sliding
	if (self->groundentity)
	{
		self->velocity[0] *= 0.5;
		self->velocity[1] *= 0.5;
	}
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	self->s.effects |= EF_PLASMA;

	if (self->health >= self->max_health)
	{
		self->style = 0;//done growing
		self->think = gasser_think;
	}

	self->nextthink = level.time + FRAMETIME;
}

void gasser_dead (edict_t *self)
{
	if (level.time > self->delay)
	{
		organ_remove(self, false);
		return;
	}

	if (level.time == self->delay - 5)
		self->s.effects |= EF_PLASMA;
	else if (level.time == self->delay - 2)
		self->s.effects |= EF_SPHERETRANS;

	self->nextthink = level.time + FRAMETIME;
}

void gasser_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int max = GASSER_MAX_COUNT;
	int cur;

	if (!self->monsterinfo.slots_freed && self->activator && self->activator->inuse)
	{
		self->activator->num_gasser--;
		cur = self->activator->num_gasser;
		self->monsterinfo.slots_freed = true;
		
		if (PM_MonsterHasPilot(attacker))
			attacker = attacker->owner;

		if (attacker->client)
			safe_cprintf(self->activator, PRINT_HIGH, "Your gasser was killed by %s (%d/%d remain)\n", attacker->client->pers.netname, cur, max);
		else if (attacker->mtype)
			safe_cprintf(self->activator, PRINT_HIGH, "Your gasser was killed by a %s (%d/%d remain)\n", V_GetMonsterName(attacker), cur, max);
		else
			safe_cprintf(self->activator, PRINT_HIGH, "Your gasser was killed by a %s (%d/%d remain)\n", attacker->classname, cur, max);
	}

	if (self->health <= self->gib_health || organ_explode(self))
	{
		int n;

		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		organ_remove(self, false);
		return;
	}

	if (self->deadflag == DEAD_DYING)
		return;

	self->think = gasser_dead;
	self->deadflag = DEAD_DYING;
	self->delay = level.time + 20.0;
	self->nextthink = level.time + FRAMETIME;
	self->s.frame = GASSER_FRAME_DEAD;
	self->movetype = MOVETYPE_TOSS;
	gi.linkentity(self);
}

edict_t *CreateGasser (edict_t *ent, int skill_level)
{
	edict_t *e;

	e = G_Spawn();
	e->style = 1; //growing
	e->activator = ent;
	e->think = gasser_grow;
	e->nextthink = level.time + FRAMETIME;
	e->s.modelindex = gi.modelindex ("models/objects/organ/gas/tris.md2");
	e->s.renderfx |= RF_IR_VISIBLE;
	e->solid = SOLID_BBOX;
	e->movetype = MOVETYPE_TOSS;
	e->clipmask = MASK_MONSTERSOLID;
	e->svflags |= SVF_MONSTER;// tells physics to clip on any solid object (not just walls)
	e->mass = 500;
	e->classname = "gasser";
	e->takedamage = DAMAGE_AIM;
	e->max_health = GASSER_INITIAL_HEALTH + GASSER_ADDON_HEALTH * skill_level;
	e->health = 0.5*e->max_health;
	e->dmg = GASSER_INITIAL_DAMAGE + GASSER_ADDON_DAMAGE * skill_level;
	e->dmg_radius = GASSER_INITIAL_ATTACK_RANGE + GASSER_ADDON_ATTACK_RANGE * skill_level;

	e->monsterinfo.level = skill_level;
	e->gib_health = -250;
	e->s.frame = GASSER_FRAMES_IDLE_START;
	e->die = gasser_die;
	e->touch = organ_touch;
	VectorSet(e->mins, -8, -8, -5);
	VectorSet(e->maxs, 8, 8, 15);
	e->mtype = M_GASSER;

	ent->num_gasser++;

	return e;
}

void Cmd_Gasser_f (edict_t *ent)
{
	int		cost = GASSER_COST;
	edict_t *gasser;
	vec3_t	start;

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		organ_removeall(ent, "gasser", true);
		safe_cprintf(ent, PRINT_HIGH, "Gassers removed\n");
		ent->num_gasser = 0;
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	if (!V_CanUseAbilities(ent, GASSER, cost, true))
		return;

	if (ent->num_gasser >= GASSER_MAX_COUNT)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum amount of gassers (%d)\n", GASSER_MAX_COUNT);
		return;
	}

	gasser = CreateGasser(ent, ent->myskills.abilities[GASSER].current_level);
	if (!G_GetSpawnLocation(ent, 100, gasser->mins, gasser->maxs, start))
	{
		ent->num_gasser--;
		G_FreeEdict(gasser);
		return;
	}
	VectorCopy(start, gasser->s.origin);
	VectorCopy(ent->s.angles, gasser->s.angles);
	gasser->s.angles[PITCH] = 0;
	gi.linkentity(gasser);
	gasser->monsterinfo.attack_finished = level.time + 1.0;
	gasser->monsterinfo.cost = cost;

	safe_cprintf(ent, PRINT_HIGH, "Gasser created (%d/%d)\n", ent->num_gasser, (int)GASSER_MAX_COUNT);

	T_Damage(ent, ent, ent, ent->velocity, ent->s.origin, ent->s.origin, gasser->monsterinfo.cost, 0, 0, 0);
	ent->client->damage_blood = 1;

	
	//ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + GASSER_DELAY;
	//ent->holdtime = level.time + GASSER_DELAY;

	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}

#define COCOON_FRAMES_IDLE_START	73//36
#define COCOON_FRAMES_IDLE_END		87//50
#define COCOON_FRAME_STANDBY		89//52
#define COCOON_FRAMES_GROW_START	90//53
#define COCOON_FRAMES_GROW_END		108//72

#define COCOON_INITIAL_HEALTH		0
#define COCOON_ADDON_HEALTH			100
#define COCOON_INITIAL_DURATION		50		// number of frames enemy will remain in cocoon
#define COCOON_ADDON_DURATION		0
#define COCOON_MINIMUM_DURATION		50
#define COCOON_INITIAL_FACTOR		1.0
#define COCOON_ADDON_FACTOR			0.05
#define COCOON_INITIAL_TIME			30.0
#define COCOON_ADDON_TIME			1.5
#define COCOON_COST					50
#define COCOON_DELAY				1.0

void cocoon_dead (edict_t *self)
{
	if (level.time > self->delay)
	{
		organ_remove(self, false);
		return;
	}

	if (level.time == self->delay - 5)
		self->s.effects |= EF_PLASMA;
	else if (level.time == self->delay - 2)
		self->s.effects |= EF_SPHERETRANS;

	self->nextthink = level.time + FRAMETIME;
}

void cocoon_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (!self->monsterinfo.slots_freed && self->activator && self->activator->inuse)
	{
		self->activator->cocoon = NULL;
		self->monsterinfo.slots_freed = true;
		
		if (PM_MonsterHasPilot(attacker))
			attacker = attacker->owner;

		if (attacker->client)
			safe_cprintf(self->activator, PRINT_HIGH, "Your cocoon was killed by %s\n", attacker->client->pers.netname);
		else if (attacker->mtype)
			safe_cprintf(self->activator, PRINT_HIGH, "Your cocoon was killed by a %s\n", V_GetMonsterName(attacker));
		else
			safe_cprintf(self->activator, PRINT_HIGH, "Your cocoon was killed by a %s\n", attacker->classname);
	}

	// restore cocooned entity
	if (self->enemy && self->enemy->inuse && !self->deadflag)
	{
		self->enemy->movetype = self->count;
		self->enemy->svflags &= ~SVF_NOCLIENT;
		self->enemy->flags &= FL_COCOONED;

		if (self->enemy->client)
		{
			self->enemy->holdtime = level.time + 0.5;
			self->enemy->client->ability_delay = level.time + 0.5;
		}
	}

	if (self->health <= self->gib_health || organ_explode(self))
	{
		int n;

		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		organ_remove(self, false);
		return;
	}

	if (self->deadflag == DEAD_DYING)
		return;

	self->think = cocoon_dead;
	self->deadflag = DEAD_DYING;
	self->delay = level.time + 20.0;
	self->nextthink = level.time + FRAMETIME;
	self->s.frame = COCOON_FRAME_STANDBY;
	self->movetype = MOVETYPE_TOSS;
	self->maxs[2] = 0;
	gi.linkentity(self);
}

void cocoon_cloak (edict_t *self)
{
	// cloak disabled or not upgraded
	if (self->monsterinfo.jumpdn == -1)
		return;

	// don't cloak if we're holding the flag carrier!
	if (self->enemy->client && self->enemy->client->pers.inventory[flag_index])
		return;

	// already cloaked
	if (self->svflags & SVF_NOCLIENT)
		return;
	
	// cloak after jumpdn frames of attacking
	if (self->monsterinfo.jumpup >= self->monsterinfo.jumpdn)
		self->svflags |= SVF_NOCLIENT;
}

void cocoon_attack (edict_t *self)
{
	int		frames;
	float	time;
	vec3_t	start;

	if (!G_EntIsAlive(self->enemy))
	{
		if (self->enemy)
			self->enemy = NULL;
		return;
	}
	
	cocoon_cloak(self);

	if (level.framenum >= self->monsterinfo.nextattack)
	{
		int		heal;
		float	duration = COCOON_INITIAL_TIME + COCOON_ADDON_TIME * self->monsterinfo.level;
		float	factor = COCOON_INITIAL_FACTOR + COCOON_ADDON_FACTOR * self->monsterinfo.level;

		// give them a damage/defense bonus for awhile
		self->enemy->cocoon_time = level.time + duration;
		self->enemy->cocoon_factor = factor;

		if (self->enemy->client)
			safe_cprintf(self->enemy, PRINT_HIGH, "You have gained a damage/defense bonus of +%.0f%c for %.0f seconds\n", 
				(factor * 100) - 100, '%', duration); 
		
		//4.4 give some health
		heal = self->enemy->max_health * (0.25 + (0.075 * self->monsterinfo.level));
		if (self->enemy->health < self->enemy->max_health)
		{
			self->enemy->health += heal;
			if (self->enemy->health > self->enemy->max_health)
				self->enemy->health = self->enemy->max_health;
		}
		
		
		//Talent: Phantom Cocoon - decloak when entity emerges
		self->svflags &= ~SVF_NOCLIENT; 
		self->monsterinfo.jumpup = 0;

		self->enemy->svflags &= ~SVF_NOCLIENT;
		self->enemy->movetype = self->count;
		self->enemy->flags &= ~FL_COCOONED;//4.4
	//	self->owner = self->enemy;
		self->enemy = NULL;
		self->s.frame = COCOON_FRAME_STANDBY;
		self->maxs[2] = 0;//shorten bbox
		self->touch = V_Touch;
		return;
	}

	if (!(level.framenum % 10) && self->enemy->client)
		safe_cprintf(self->enemy, PRINT_HIGH, "You will emerge from the cocoon in %d second(s)\n", 
			(int)((self->monsterinfo.nextattack - level.framenum) / 10));

	time = level.time + FRAMETIME;

	// hold target in-place
	if (!strcmp(self->enemy->classname, "drone"))
	{
		self->enemy->monsterinfo.pausetime = time;
		self->enemy->monsterinfo.stand(self->enemy);
	}
	else
		self->enemy->holdtime = time;

	// keep morphed players from shooting
	if (PM_MonsterHasPilot(self->enemy))
	{
		self->enemy->owner->client->ability_delay = time;
		self->enemy->owner->monsterinfo.attack_finished = time;
	}
	else
	{
		// no using abilities!
		if (self->enemy->client)
			self->enemy->client->ability_delay = time;
		//if(strcmp(self->enemy->classname, "spiker") != 0)
		if (self->enemy->mtype == M_SPIKER)
			self->enemy->monsterinfo.attack_finished = time;
	}
	
	// move position
	VectorCopy(self->s.origin, start);
	start[2] += fabs(self->enemy->mins[2]) + 1;
	VectorCopy(start, self->enemy->s.origin);
	gi.linkentity(self->enemy);
	
	// hide them
	self->enemy->svflags |= SVF_NOCLIENT;

	frames = COCOON_INITIAL_DURATION + COCOON_ADDON_DURATION * self->monsterinfo.level;
	if (frames < COCOON_MINIMUM_DURATION)
		frames = COCOON_MINIMUM_DURATION;

	/*if (M_Regenerate(self->enemy, frames, 1, true, true, false, &self->monsterinfo.regen_delay2) && (level.time > self->msg_time))
	{
		gi.sound(self, CHAN_AUTO, gi.soundindex("organ/healer1.wav"), 1, ATTN_NORM, 0);
		self->msg_time = level.time + 1.5;
	}
	*/

	self->monsterinfo.jumpup++;//Talent: Phantom Cocoon - keep track of attack frames
}

void cocoon_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int frames;

	V_Touch(ent, other, plane, surf);
	
	if (!ent->groundentity || ent->groundentity != world)
		return;
	if (level.framenum < ent->monsterinfo.nextattack)
		return;
	if (!G_ValidTargetEnt(other, true) || !OnSameTeam(ent, other))
		return;
	if (other->mtype == M_SPIKEBALL)//4.4
		return;
	if (other->movetype == MOVETYPE_NONE)
		return;

	ent->enemy = other;

	frames = COCOON_INITIAL_DURATION + COCOON_ADDON_DURATION * ent->monsterinfo.level;
	if (frames < COCOON_MINIMUM_DURATION)
		frames = COCOON_MINIMUM_DURATION;
	ent->monsterinfo.nextattack = level.framenum + frames;

	// don't let them move (or fall out of the map)
	ent->count = other->movetype;
	other->movetype = MOVETYPE_NONE;
	other->flags |= FL_COCOONED;//4.4

	if (other->client)
		safe_cprintf(other, PRINT_HIGH, "You have been cocooned for %d seconds\n", (int)(frames / 10));
}

void cocoon_think (edict_t *self)
{
	trace_t tr;

	if (!G_EntIsAlive(self->activator))
	{
		organ_remove(self, false);
		return;
	}

	cocoon_attack(self);

	if (level.time > self->lasthurt + 1.0)
		M_Regenerate(self, 300, 10,  1.0, true, false, false, &self->monsterinfo.regen_delay1);

	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// don't slide
	if (self->groundentity)
		VectorClear(self->velocity);
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == COCOON_FRAME_STANDBY)
	{
		vec3_t maxs;

		VectorCopy(self->maxs, maxs);
		maxs[2] = 40;

		tr = gi.trace(self->s.origin, self->mins, maxs, self->s.origin, self, MASK_SHOT);
		if (tr.fraction == 1.0)
		{
			//self->touch = cocoon_touch;
			self->maxs[2] = 40;
			self->s.frame++;
		}
		return;
	}
	else if (self->s.frame > COCOON_FRAME_STANDBY && self->s.frame < COCOON_FRAMES_GROW_END)
	{
		G_RunFrames(self, COCOON_FRAMES_GROW_START, COCOON_FRAMES_GROW_END, false);
	}
	else if (self->s.frame == COCOON_FRAMES_GROW_END)
	{
		self->style = 0; //done growing
		self->touch = cocoon_touch;
		self->s.frame = COCOON_FRAMES_IDLE_START;
	}
	else
		G_RunFrames(self, COCOON_FRAMES_IDLE_START, COCOON_FRAMES_IDLE_END, false);
}

edict_t *CreateCocoon (edict_t *ent, int skill_level)
{
	//Talent: Phantom Cocoon
	int talentLevel = getTalentLevel(ent, TALENT_PHANTOM_COCOON);

	edict_t *e;

	e = G_Spawn();
	e->style = 1; //growing
	e->activator = ent;
	e->think = cocoon_think;
	e->nextthink = level.time + FRAMETIME;
	e->s.modelindex = gi.modelindex ("models/objects/cocoon/tris.md2");
	e->s.renderfx |= RF_IR_VISIBLE;
	e->solid = SOLID_BBOX;
	e->movetype = MOVETYPE_TOSS;
	e->svflags |= SVF_MONSTER;
	e->clipmask = MASK_MONSTERSOLID;
	e->mass = 500;
	e->classname = "cocoon";
	e->takedamage = DAMAGE_AIM;
	e->health = e->max_health = COCOON_INITIAL_HEALTH + COCOON_ADDON_HEALTH * skill_level;
	e->monsterinfo.level = skill_level;
	
	//Talent: Phantom Cocoon - frames before cloaking
	if (talentLevel > 0)
		e->monsterinfo.jumpdn = 50 - 8 * talentLevel;
	else
		e->monsterinfo.jumpdn = -1; // cloak disabled

	e->gib_health = -200;
	e->s.frame = COCOON_FRAME_STANDBY;
	e->die = cocoon_die;
	e->touch = V_Touch;
	VectorSet(e->mins, -50, -50, -40);
	VectorSet(e->maxs, 50, 50, 40);
	e->mtype = M_COCOON;

	ent->cocoon = e;

	return e;
}

void Cmd_Cocoon_f (edict_t *ent)
{
	edict_t *cocoon;
	vec3_t	start;

	if (ent->cocoon && ent->cocoon->inuse)
	{
		organ_remove(ent->cocoon, true);
		safe_cprintf(ent, PRINT_HIGH, "Cocoon removed\n");
		return;
	}

	if (!V_CanUseAbilities(ent, COCOON, COCOON_COST, true))
		return;

	cocoon = CreateCocoon(ent, ent->myskills.abilities[COCOON].current_level);
	if (!G_GetSpawnLocation(ent, 100, cocoon->mins, cocoon->maxs, start))
	{
		ent->cocoon = NULL;
		G_FreeEdict(cocoon);
		return;
	}
	VectorCopy(start, cocoon->s.origin);
	VectorCopy(ent->s.angles, cocoon->s.angles);
	cocoon->s.angles[PITCH] = 0;
	gi.linkentity(cocoon);
	cocoon->monsterinfo.cost = COCOON_COST;

	safe_cprintf(ent, PRINT_HIGH, "Cocoon created\n");
	gi.sound(cocoon, CHAN_VOICE, gi.soundindex("organ/organe3.wav"), 1, ATTN_STATIC, 0);

	ent->client->pers.inventory[power_cube_index] -= COCOON_COST;
	ent->client->ability_delay = level.time + COCOON_DELAY;

	//  entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
}

void drop_make_touchable (edict_t *ent);
void meditate_think (edict_t *self)
{
	self->s.effects &= ~EF_PLASMA;
	self->think = drop_make_touchable;
	self->nextthink = level.time + FRAMETIME;
}

void Cmd_Meditate_f (edict_t *ent)
{
	//Talent: Meditation
	int talentLevel = getTalentLevel(ent, TALENT_MEDITATION);

	if (talentLevel < 1)
		return;

	if (ent->client->ability_delay > level.time)
		return;

	ent->manacharging = !ent->manacharging;

	if (ent->manacharging)
	{
		gi.cprintf(ent, PRINT_HIGH, "Charging mana.\n");
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "No longer charging mana.\n");
		ent->client->ability_delay = level.time + 2;
	}

	// old meditation
	/*power_cube = FindItem("Power Cube");

	ent->client->pers.inventory[ITEM_INDEX(power_cube)] += 20*talentLevel;*/

	// ent->holdtime = level.time + 2;

	// ent->client->ability_delay = level.time + 2;
}

#define LASERPLATFORM_INITIAL_HEALTH		0
#define LASERPLATFORM_ADDON_HEALTH			200
#define LASERPLATFORM_INITIAL_DURATION		9999.9
#define LASERPLATFORM_ADDON_DURATION		0
#define LASERPLATFORM_MAX_COUNT				2
#define LASERPLATFORM_COST					10.0

void RemoveLaserPlatform (edict_t *laserplatform)
{
	if (laserplatform->activator && laserplatform->activator->inuse)
		laserplatform->activator->num_laserplatforms--;

	laserplatform->think = BecomeTE;
	laserplatform->deadflag = DEAD_DEAD;
	laserplatform->takedamage = DAMAGE_NO;
	laserplatform->nextthink = level.time + FRAMETIME;
	laserplatform->svflags |= SVF_NOCLIENT;
	laserplatform->solid = SOLID_NOT;
	//gi.linkentity(laserplatform);
}

void laserplatform_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	RemoveLaserPlatform(self);
}

void laserplatform_think (edict_t *self)
{
	if (!G_EntIsAlive(self->activator) || level.time > self->delay)
	{
		RemoveLaserPlatform(self);
		return;
	}

	self->s.renderfx = RF_SHELL_GREEN;
	
	if (level.time + 10.0 > self->delay && level.time >= self->monsterinfo.attack_finished)
	{

		self->s.renderfx |= RF_SHELL_RED|RF_SHELL_BLUE; // flash white
		self->monsterinfo.attack_finished = level.time + 1.0;
	}

	self->nextthink = level.time + FRAMETIME;
}

edict_t *CreateLaserPlatform (edict_t *ent, int skill_level)
{
	edict_t *e;

	e = G_Spawn();
	e->activator = ent;
	e->think = laserplatform_think;
	e->nextthink = level.time + FRAMETIME;
	e->s.modelindex = gi.modelindex ("models/items/armor/effect/tris.md2");
	e->s.angles[PITCH] += 90;
	e->solid = SOLID_BBOX;
	e->movetype = MOVETYPE_NONE;
	//e->svflags |= SVF_MONSTER;
	e->clipmask = MASK_MONSTERSOLID;
	e->mass = 500;
	e->classname = "laser_platform";
	e->delay = level.time + LASERPLATFORM_INITIAL_DURATION + LASERPLATFORM_ADDON_DURATION * skill_level;
	e->takedamage = DAMAGE_AIM;
	e->health = e->max_health = LASERPLATFORM_INITIAL_HEALTH + LASERPLATFORM_ADDON_HEALTH * skill_level;
	e->monsterinfo.level = skill_level;
	e->die = laserplatform_die;
	e->touch = V_Touch;
	e->s.effects |= EF_COLOR_SHELL|EF_SPHERETRANS;
	e->s.renderfx |= RF_SHELL_GREEN;
	VectorSet(e->mins, -28, -28, -18);
	VectorSet(e->maxs, 58, 28, -16);
	e->mtype = M_LASERPLATFORM;

	//ent->cocoon = e;
	ent->num_laserplatforms++;

	return e;
}

void RemoveOldestLaserPlatform (edict_t *ent)
{
	edict_t *e, *oldest=NULL;

	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		// find all laser platforms that we own
		if (e && e->inuse && (e->mtype == M_LASERPLATFORM) && e->activator && e->activator->inuse && e->activator == ent)
		{
			// update pointer to the platform with the earliest expiration time
			if (!oldest || e->delay < oldest->delay)
				oldest = e;
		}
	}

	// remove it
	if (oldest)
		RemoveLaserPlatform(oldest);
}

void RemoveAllLaserPlatforms (edict_t *ent)
{
	edict_t *e;

	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		// find all laser platforms that we own
		if (e && e->inuse && (e->mtype == M_LASERPLATFORM) && e->activator && e->activator->inuse && e->activator == ent)
			RemoveLaserPlatform(e);
	}
}

void Cmd_CreateLaserPlatform_f (edict_t *ent)
{	
	int		*cubes = &ent->client->pers.inventory[power_cube_index];
	int		talentLevel = getTalentLevel(ent, TALENT_LASER_PLATFORM);
	vec3_t	start;
	edict_t *laserplatform;

	if (talentLevel < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need to upgrade laser platform talent before you can use it.\n");
		return;
	}

	if (*cubes < LASERPLATFORM_COST)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need %d more power cubes to use this ability.\n", (int)(LASERPLATFORM_COST - *cubes));
		return;
	}

	// if we have reached our max, then remove the oldest one
	if (ent->num_laserplatforms + 1 > LASERPLATFORM_MAX_COUNT)
		RemoveOldestLaserPlatform(ent);

	laserplatform = CreateLaserPlatform(ent, talentLevel);

	if (!G_GetSpawnLocation(ent, 100, laserplatform->mins, laserplatform->maxs, start))
	{
		RemoveLaserPlatform(laserplatform);
		return;
	}

	VectorCopy(start, laserplatform->s.origin);
	gi.linkentity(laserplatform);

	ent->client->pers.inventory[power_cube_index] -= LASERPLATFORM_COST;
}

#define HOLYGROUND_INITIAL_DURATION		0
#define HOLYGROUND_ADDON_DURATION		12.0
#define HOLYGROUND_COST					25
#define HOLYGROUND_DELAY				1.0

#define HOLYGROUND_HOLY					1
#define HOLYGROUND_UNHOLY				2

void holyground_sparks (vec3_t org, int num, int radius, int color)
{
	int		i;
	vec3_t	start, end;
	trace_t	tr;

	// 0 = black, 8 = grey, 15 = white, 16 = light brown, 20 = brown, 57 = light orange, 66 = orange/red, 73 = maroon
	// 106 = pink, 113 = light blue, 119 = blue, 123 = dark blue, 200 = pale green, 205 = dark green, 209 = bright green
	// 217 = white, 220 = yellow, 226 = orange, 231 = red/orange, 240 = red, 243 = dark blue

	for (i=0; i<num; i++)
	{
		// calculate random position on 2-D plane and trace to it
		VectorCopy(org, start);
		VectorCopy(start, end);
		end[0] += crandom() * GetRandom(0, radius);
		end[1] += crandom() * GetRandom(0, radius);
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);

		// push to the floor
		VectorCopy(tr.endpos, start);
		VectorCopy(start, end);
		end[2] -= 8192;
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
		VectorCopy(tr.endpos, start);
		start[2] += GetRandom(0, 8);

		// throw sparks
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(1); // number of sparks
		gi.WritePosition(start);
		gi.WriteDir(vec3_origin);
		gi.WriteByte(color); // color
		gi.multicast(start, MULTICAST_PVS);
	}
}

qboolean holyground_visible (edict_t *self, edict_t *target)
{
	trace_t	tr;
	vec3_t	start, end;

	// trace from start to end at the same height
	VectorCopy(self->s.origin, start);
	VectorCopy(target->s.origin, end);
	end[2] = start[2];
	tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
	if (tr.fraction < 1)
		return false;

	// check for obstructions
	VectorCopy(end, start);
	tr = gi.trace(start, NULL, NULL, target->s.origin, NULL, MASK_SOLID);
	if (tr.fraction < 1)
		return false;

	// we didn't hit any walls, so we are good
	return true;
}

void holyground_attack (edict_t *self, float radius)
{
	edict_t *target = NULL;

	while ((target = findradius(target, self->s.origin, radius)) != NULL)
	{
		// standard entity checks
		if (!G_EntIsAlive(target))
			continue;
		if (!target->takedamage || (target->solid == SOLID_NOT))
			continue;
		if (target->client && target->client->invincible_framenum > level.framenum)
			continue;
		if (target->flags & FL_CHATPROTECT || target->flags & FL_GODMODE || target->flags & FL_NOTARGET)
			continue;
		if (target->svflags & SVF_NOCLIENT && target->mtype != M_FORCEWALL)
			continue;
		if (que_typeexists(target->curses, CURSE_FROZEN))
			continue;

		// target must be on the ground
		if (!target->groundentity)
			continue;

		if (!holyground_visible(self, target))
			continue;
		
		if (OnSameTeam(self->owner, target))
		{
			if (self->style == HOLYGROUND_HOLY)
			{
				// heal
				if (target->health < target->max_health)
				{
					int health = target->max_health - target->health;
					if (health > self->dmg)
						health = self->dmg;
					target->health += health;
				}
						
				// throw green sparks
				holyground_sparks(target->s.origin, 1, (int)(target->maxs[1]+32), 209);
			}
		}
		else if (self->style == HOLYGROUND_UNHOLY)
		{
			if (level.framenum > self->monsterinfo.nextattack)
			{
				// deal damage
				T_Damage(target, self, self->owner, vec3_origin, target->s.origin, vec3_origin, 50, 0, 0, MOD_UNHOLYGROUND);
				self->monsterinfo.nextattack = level.framenum + 5;
				//gi.dprintf("%.1.f %.1f\n", self->monsterinfo.nextattack, level.time);
			}

			// throw red sparks
			holyground_sparks(target->s.origin, 1, (int)(target->maxs[1]+32), 240);
		}
	}
}

void holyground_remove (edict_t *ent, edict_t *self)
{
	// if ent isn't specified, then always remove; otherwise, make sure this entity is ours
	if (!ent || (self && self->inuse && self->mtype == M_HOLYGROUND 
		&& ent->inuse && self->owner && self->owner->inuse && self->owner == ent))
	{
		// prep entity for removal
		self->think = G_FreeEdict;
		self->nextthink = level.time + FRAMETIME;

		// clear owner's pointer to this entity
		if (self->owner && self->owner->inuse)
			self->owner->holyground = NULL;
	}
	// this isn't ours to remove, but clear the invalid pointer
	else if (ent)
		ent->holyground = NULL;
}


void holyground_think (edict_t *self)
{
	if (!G_EntIsAlive(self->owner) || level.time > self->delay)
	{
		holyground_remove(NULL, self);
		return;
	}

	holyground_sparks(self->s.origin, 1, 256, 15);
	holyground_attack(self, 256);

	self->nextthink = level.time + FRAMETIME;
}

void CreateHolyGround (edict_t *ent, int type, int skill_level)
{
	edict_t *e;

	e = G_Spawn();
	e->owner = ent;
	e->dmg = 1;
	e->mtype = M_HOLYGROUND;
	e->think = holyground_think;
	e->nextthink = level.time + FRAMETIME;
	e->classname = "holy_ground";
	e->svflags |= SVF_NOCLIENT;
	e->monsterinfo.level = skill_level;
	e->delay = level.time + HOLYGROUND_INITIAL_DURATION + HOLYGROUND_ADDON_DURATION * skill_level;
	e->style = type;

	ent->holyground = e;

	VectorCopy(ent->s.origin, e->s.origin);
	gi.linkentity(e);

	ent->client->ability_delay = level.time + HOLYGROUND_DELAY;
	ent->client->pers.inventory[power_cube_index] -= HOLYGROUND_COST;
}

void Cmd_HolyGround_f (edict_t *ent)
{
	//Talent: Holy Ground
	int talentLevel = getTalentLevel(ent, TALENT_HOLY_GROUND);

	if (level.time < pregame_time->value)
		return;

	if (talentLevel < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need to upgrade this talent before you can use it!\n");
		return;
	}
	if (ent->holyground && ent->holyground->inuse)
	{
		holyground_remove(ent, ent->holyground);
		safe_cprintf(ent, PRINT_HIGH, "Holy ground removed.\n");
		return;
	}
	if (ent->client->ability_delay > level.time)
	{
		safe_cprintf(ent, PRINT_HIGH, "You must wait %.1f seconds before you can use this talent.\n", 
			ent->client->ability_delay-level.time);
		return;
	}
	if (ent->client->pers.inventory[power_cube_index] < HOLYGROUND_COST)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need %d cubes before you can use this talent.\n", 
			HOLYGROUND_COST-ent->client->pers.inventory[power_cube_index]);
		return;
	}
	
	CreateHolyGround(ent, HOLYGROUND_HOLY, talentLevel);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("spells/sanctuary.wav"), 1, ATTN_NORM, 0);
}

void Cmd_UnHolyGround_f (edict_t *ent)
{
	//Talent: Unholy Ground
	int talentLevel = getTalentLevel(ent, TALENT_UNHOLY_GROUND);

	if (talentLevel < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need to upgrade this talent before you can use it!\n");
		return;
	}
	if (ent->holyground && ent->holyground->inuse)
	{
		holyground_remove(ent, ent->holyground);
		safe_cprintf(ent, PRINT_HIGH, "Un-holy ground removed.\n");
		return;
	}
	if (ent->client->ability_delay > level.time)
	{
		safe_cprintf(ent, PRINT_HIGH, "You must wait %.1f seconds before you can use this talent.\n", 
			ent->client->ability_delay-level.time);
		return;
	}
	if (ent->client->pers.inventory[power_cube_index] < HOLYGROUND_COST)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need %d cubes before you can use this talent.\n", 
			HOLYGROUND_COST-ent->client->pers.inventory[power_cube_index]);
		return;
	}

	CreateHolyGround(ent, HOLYGROUND_UNHOLY, talentLevel);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("spells/sanctuary.wav"), 1, ATTN_NORM, 0);
}

#define PURGE_COST		50
#define PURGE_DELAY		5.0

void Cmd_Purge_f (edict_t *ent)
{
	//Talent: Purge
	int talentLevel = getTalentLevel(ent, TALENT_PURGE);

	if (talentLevel < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need to upgrade this talent before you can use it!\n");
		return;
	}
	
	if (ent->client->ability_delay > level.time)
	{
		safe_cprintf(ent, PRINT_HIGH, "You must wait %.1f seconds before you can use this talent.\n", 
			ent->client->ability_delay-level.time);
		return;
	}
	if (ent->client->pers.inventory[power_cube_index] < PURGE_COST)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need %d cubes before you can use this talent.\n", 
			PURGE_COST-ent->client->pers.inventory[power_cube_index]);
		return;
	}

	//Give them a short period of total immunity
	ent->client->invincible_framenum = level.framenum + 3*talentLevel; //up to 2 seconds at level 5

	//Give them a short period of curse immunity
	ent->holywaterProtection = level.time + talentLevel; //up to 5 seconds at level 5

	//You can only drink 1/sec
	ent->client->ability_delay = level.time + PURGE_DELAY;

	//Remove all curses
	CurseRemove(ent, 0);

	ent->client->pers.inventory[power_cube_index] -= PURGE_COST;
	ent->client->ability_delay = level.time + PURGE_DELAY;

	gi.sound(ent, CHAN_AUTO, gi.soundindex("spells/purification.wav"), 1, ATTN_NORM, 0);
}


void Cmd_SelfDestruct_f(edict_t *self)
{
	int damage;

	if (!V_CanUseAbilities(self, SELFDESTRUCT, 0, true))
		return;

	damage = self->myskills.abilities[SELFDESTRUCT].current_level * SELFDESTRUCT_BONUS + SELFDESTRUCT_BASE;

	// do the damage
	T_RadiusDamage(self, self, damage, self, SELFDESTRUCT_RADIUS, MOD_SELFDESTRUCT);
	T_Damage(self, self, self, vec3_origin, self->s.origin, vec3_origin, damage * 0.6, 0, 0, MOD_SELFDESTRUCT);

	// GO BOOM!
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->client->ability_delay = level.time + 1;
	return;
}
