// g_weapon.c

#include "g_local.h"
#include "m_player.h"

static qboolean	is_quad;
static byte		is_silenced;

void weapon_grenade_fire (edict_t *ent, qboolean held);

int applyWeaponBonuses(edict_t *ent, int damage, int count) {
	//int i;
	int lvl;
	iteminfo_t *iteminfo;
	float bonus = 1.0;
	if (!ent->client)
		return damage;
	if (count == 0)
		count = 1;

	iteminfo = getWornItemInfo(ent, 0);

	if (is_quad)
		damage *= 3;
	if (iteminfo->type & GIEX_ITEM_SOLDIER) {
		bonus += 0.022 * pow(ent->client->pers.skills.classLevel[0], 1.1);
	}
	if (iteminfo->type & GIEX_ITEM_TANK) {
		bonus += 0.018 * pow(ent->client->pers.skills.classLevel[1], 1.1);
	}
	if (iteminfo->type & GIEX_ITEM_CLERIC) {
		bonus += 0.014 * pow(ent->client->pers.skills.classLevel[2], 1.1);
	}
	if (iteminfo->type & GIEX_ITEM_MAGE) {
		bonus += 0.014 * pow(ent->client->pers.skills.classLevel[3], 1.1);
	}
	if (iteminfo->type & GIEX_ITEM_VAMPIRE) {
		bonus += 0.020 * pow(ent->client->pers.skills.classLevel[4], 1.1);
	}

	bonus += 0.05 * ent->client->pers.skill[24];

	//Might
	lvl = getAuraLevel(ent, 77);
	if (lvl > 0) {
		int slot = getAuraSlot(ent, 77);
		if (ent->client->aura_caster[slot]->client->magic > 0) {
			float mightbonus = 0.01 * ent->client->aura_level[slot];
			if (ent->client->aura_level[slot] > 40)
				mightbonus -= 0.005 * (ent->client->aura_level[slot] - 40); // Level 40+ gives half normal bonus
			bonus *= 1 + mightbonus;
			ent->client->aura_caster[slot]->client->magic -= 0.5 * damage;
			ent->client->aura_caster[slot]->client->magregentime = level.time + 1.0;
			//gi.dprintf("%d %f %f", damage, mightbonus, bonus);
		}
	}

	//Anti-resist
	lvl = getAuraLevel(ent, 84);
	if (lvl > 0) {
		int slot = getAuraSlot(ent, 84);
		if (ent->client->aura_caster[slot]->client->magic > 0) {
			float arpenalty = 0.9;// * ent->client->aura_level[slot];
			bonus *= arpenalty;
			//Magic is deducted in T_Damage..
			//ent->client->aura_caster[slot]->client->magic -= 0.5 * damage;
			//ent->client->aura_caster[slot]->client->magregentime = level.time + 1.0;
			//gi.dprintf("%d %f %f", damage, mightbonus, bonus);
		}
	}


	damage *= bonus;
/*	if (lvl > 0) {
		gi.dprintf(" %d\n", damage);
	}*/
	if (ent->pain_debounce_time < level.time) {
		if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 1)) {
			ent->client->silencer_shots -= 5;
		} else {
			if (bonus >= 2.0)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("giex/eshot2.wav"), 1, ATTN_NORM, 0);
			else if (bonus >= 1.5)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("giex/eshot1.wav"), 1, ATTN_NORM, 0);
		}
		ent->pain_debounce_time = level.time + 0.4;
	}

	return damage;
}

static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type) {
	edict_t		*noise;
	edict_t		*blip = NULL;
return;
	if (deathmatch->value && !dmmonsters->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;

	if (type == PNOISE_WEAPON) {
		if (who->client->silencer_shots > 0) {
			who->client->silencer_shots--;
			return;
		} else {
			who->client->silencer_shots = 0;
		}
	}

	if (!who->mynoise) {
		noise = G_Spawn();
		noise->classid = CI_P_NOISE;
		noise->classname = "player noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classid = CI_P_NOISE;
		noise->classname = "player noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	// Zheele
	while ((blip = findradius(blip, where, 750)) != NULL) {
		if (!blip->inuse)
			continue;
		if (!blip->takedamage)
			continue;
		if (!(blip->svflags & SVF_MONSTER) || (blip->monsterinfo.aiflags & AI_GOOD_GUY))
			continue;
		if ((!blip->monsterinfo.aggressive) && (deathmatch->value) && (game.craze != 10) && (game.monsterhunt != 10))
			continue;
		if (blip->enemy)
			continue;
		if (((float) pow(who->radius_dmg, GIEX_MONSTER_PLAYERLEVEL_MULT_POW) * GIEX_MONSTER_PLAYERLEVEL_MULT) < ((float) blip->monsterinfo.skill * 0.75) && (game.monsterhunt != 10)) {
			continue;
		}
		blip->enemy = who;
		FoundTarget(blip);
		if (!(blip->monsterinfo.aiflags & AI_SOUND_TARGET) && (blip->monsterinfo.sight))
			blip->monsterinfo.sight (blip, blip->enemy);
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy (where, noise->s.origin);
	VectorSubtract (where, noise->maxs, noise->absmin);
	VectorAdd (where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity (noise);
}


qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;

	index = ITEM_INDEX(ent->item);

	if ( /*( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value)
		&&*/ other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
/*		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000, true);
		else*/
			Add_Ammo (other, ammo, ammo->quantity, true);

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
//				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
/*				else
					SetRespawn (ent, 30);*/
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}

//	if (other->client->pers.weapon != ent->item &&
//		(other->client->pers.inventory[index] == 1) &&
//		( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
//		other->client->newweapon = ent->item;

	return true;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon (edict_t *ent)
{
	int i;

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.skills.wornItem[0];
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
//	ent->client->pers.skills.wornItem[0] = ent->client->pers.skills.activatingItem[0];
//	ent->client->pers.skills.activatingItem[0] = 0;

	// set visible model
	if (ent->s.modelindex == 255) {
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	if (!ent->client->pers.weapon) {	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	ent->client->anim_priority = ANIM_PAIN;
	if(ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
			ent->s.frame = FRAME_crpain1;
			ent->client->anim_end = FRAME_crpain4;
	} else {
			ent->s.frame = FRAME_pain301;
			ent->client->anim_end = FRAME_pain304;

	}
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	return;

	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] )
	{
		ent->client->newweapon = FindItem ("railgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] )
	{
		ent->client->newweapon = FindItem ("hyperblaster");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] )
	{
		ent->client->newweapon = FindItem ("chaingun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] )
	{
		ent->client->newweapon = FindItem ("machinegun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] )
	{
		ent->client->newweapon = FindItem ("super shotgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))] )
	{
		ent->client->newweapon = FindItem ("shotgun");
		return;
	}
	ent->client->newweapon = FindItem ("blaster");
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{
	// if just died, put the weapon away
	if (ent->health < 1)
	{
		return;
		//ent->client->newweapon = NULL;
		//ChangeWeapon (ent);
	}

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots > 0)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent);
	}
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon (edict_t *ent, gitem_t *item)
{
	//int			ammo_index;
//	gitem_t		*ammo_item;

	return;
	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	//if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	//{
		//ammo_item = FindItem(item->ammo);
		//ammo_index = ITEM_INDEX(ammo_item);

/*		if (!ent->client->pers.inventory[ammo_index])
		{
			gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		} */
	//}

	// change to this weapon when down
	ent->client->newweapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	return;
/*	if ((int)(dmflags->value) & DF_WEAPONS_STAY)*/
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING) {
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST) {
			if (ent->client->pers.itemchangetime < level.time) {
				ChangeWeapon (ent);
			}
			return;
		} else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4) {
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			} else {
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;

			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING) {
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST) {
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING)) {
		if (ent->client->pers.itemchanging == 0) {
			ent->client->weaponstate = WEAPON_DROPPING;
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

			if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
			{
				ent->client->anim_priority = ANIM_REVERSE;
				if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crpain4+1;
					ent->client->anim_end = FRAME_crpain1;
				}
				else
				{
					ent->s.frame = FRAME_pain304+1;
					ent->client->anim_end = FRAME_pain301;

				}
			}
		}
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY) {
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)/* && (ent->client->spelltime < level.time)*/) {
			iteminfo_t *iteminfo;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.skills.wornItem[0] == -1) {
				gi.cprintf(ent, PRINT_HIGH, "No weapon in hand\n");
				return;
			}
			iteminfo = getWornItemInfo(ent, 0);
			if ((iteminfo->name[0] == '\0') || (iteminfo->item == NULL) || (iteminfo->item != ent->client->pers.weapon)) {
				gi.cprintf(ent, PRINT_HIGH, "Weapon mismatch, please report this to Chamooze and try to remember events around the last time you switched weapons.\nWorn weapon: %s, Actual weapon: %s\nTry equipping another weapon before continuing to play.\n", iteminfo->name, ent->client->pers.weapon->classname);
				return;
			}
			if ((!ent->client->ammo_index) ||
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity)) {
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				} else {
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			} else {
				if (level.time >= ent->pain_debounce_time) {
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		} else {
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST) {
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames) {
				for (n = 0; pause_frames[n]; n++) {
					if (ent->client->ps.gunframe == pause_frames[n]) {
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}


/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent, qboolean held) {
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;
	iteminfo_t *info = getWornItemInfo(ent, 0);

	damage = info->arg1 + info->arg2 * ent->client->pers.skill[1];
	radius = info->arg6 + info->arg7 * ent->client->pers.skill[1];
	damage = applyWeaponBonuses(ent, damage, 1);

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + ((GRENADE_TIMER + info->arg10) - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	speed *= info->arg8 + info->arg9 * ent->client->pers.skill[8];
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	} else {
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Grenade (edict_t *ent) {
	iteminfo_t *info = getWornItemInfo(ent, 0);
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY)) {
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING) {
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY) {
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) ) {
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index]) {
				ent->client->ps.gunframe = 1 + info->arg3;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			} else {
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48)) {
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING) {
		if (ent->client->ps.gunframe == 5) {
			ent->client->ps.gunframe += info->arg4;
			if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 0)) {
				ent->client->silencer_shots -= 3;
			} else {
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
			}
		}

		if (ent->client->ps.gunframe == 11) {
			if (!ent->client->grenade_time) {
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2 + info->arg10;
				if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 4)) {
					ent->client->silencer_shots -= 1;
				} else {
					ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
				}
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time) {
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up) {
				if (level.time >= ent->client->grenade_time) {
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				} else {
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12) {
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
			ent->client->ps.gunframe += info->arg5;
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16) {
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage;
	float	radius;
	iteminfo_t *info = getWornItemInfo(ent, 0);

	damage = info->arg1 + info->arg2 * ent->client->pers.skill[1];
	radius = info->arg4 + 0.2 * ent->client->pers.skill[1];
	damage = applyWeaponBonuses(ent, damage, 1);

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_grenade (ent, start, forward, damage, info->arg6, info->arg5, info->arg7, radius);

	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
		ent->client->silencer_shots -= 1;
	} else {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_GRENADE | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
	ent->client->silencer_shots-=7;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};
	iteminfo_t *info = getWornItemInfo(ent, 0);

	Weapon_Generic (ent, 5, 16 + info->arg3, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	int		dmg_lvl = ent->client->pers.skill[1];
	iteminfo_t *info = getWornItemInfo(ent, 0);

	damage = info->arg1 + info->arg2 * dmg_lvl;
	radius_damage = damage;
	damage_radius = info->arg4 + info->arg5 * dmg_lvl;
	damage = applyWeaponBonuses(ent, damage, 1);
	if (is_quad)
		radius_damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rocket (ent, start, forward, damage, info->arg6 + info->arg7 * ent->client->pers.skill[7], damage_radius, radius_damage);
	ent->client->silencer_shots-=6;

	// send muzzle flash
	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
		ent->client->silencer_shots -= 1;
	} else {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_ROCKET | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};
	iteminfo_t *info = getWornItemInfo(ent, 0);

	Weapon_Generic (ent, 4, 12 + info->arg3, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, int speed, qboolean hyper, int effect) {
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	damage = applyWeaponBonuses(ent, damage, 1);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (speed > 0) { // No flight speed => Hitscan
		if (hyper) {
			fire_blaster (ent, start, forward, damage, speed, 0, hyper);
			ent->client->silencer_shots-=1;
		} else {
			fire_blaster (ent, start, forward, damage, speed, effect, hyper);
			ent->client->silencer_shots-=3;
		}
	} else {
		fire_lead(ent, start, forward, damage, 0, TE_BLASTER, 0, 0, MOD_BLASTER);
	}

	// send muzzle flash
	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58]> 5)) {
		ent->client->silencer_shots -= 1;
	} else {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		if (hyper)
			gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
		else
			gi.WriteByte (MZ_BLASTER | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Blaster_Fire (edict_t *ent) {
	int index;
	int item = getWornItem(ent, GIEX_ITEM_WEAPON);
	iteminfo_t *info = getWornItemInfo(ent, 0);
	int c = getBlasterConsumption(ent, item, info);

	ent->client->ps.gunframe++;

	index = ITEM_INDEX(FindItem("Blaster cells"));
	if (ent->client->pers.inventory[index] >= c) {
		ent->client->pers.inventory[index] -= c;
	} else {
		return;
	}

	Blaster_Fire (ent, vec3_origin, getBlasterDamage(ent, item, info), getBlasterSpeed(ent, item, info), false, EF_BLASTER);
}

void Weapon_Blaster (edict_t *ent) {
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	iteminfo_t *info = getWornItemInfo(ent, 0);
	Weapon_Generic (ent, 4, 4 + getBlasterFirerate(ent, getWornItem(ent, GIEX_ITEM_WEAPON), info), 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent) {
	float	rotation;
	vec3_t	offset;
	int		damage;
	iteminfo_t *info = getWornItemInfo(ent, 0);

	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58]> 6)) {
		ent->client->silencer_shots -= 5;
	} else {
		ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		} else if ((ent->client->ps.gunframe != 11) || (info->arg3 < 6)) {
			int amount = (int) ceil(info->arg8 + info->arg9 * ent->client->pers.skill[15]);
			rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			damage = info->arg1 + info->arg2 * ent->client->pers.skill[1];

			Blaster_Fire (ent, offset, damage, info->arg6 + info->arg7 * ent->client->pers.skill[7], true, 0);
			ent->client->pers.inventory[ent->client->ammo_index]--;

			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_HyperBlaster (edict_t *ent) {
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};
	iteminfo_t	*info = getWornItemInfo(ent, 0);

	if (info->arg3 == 2) {
		fire_frames[0] = 7;
		fire_frames[1] = 9;
		fire_frames[2] = 11;
		fire_frames[3] = 0;
		fire_frames[4] = 0;
		fire_frames[5] = 0;
		fire_frames[6] = 0;
	} else if (info->arg3 == 3) {
		fire_frames[0] = 8;
		fire_frames[1] = 11;
		fire_frames[2] = 0;
		fire_frames[3] = 0;
		fire_frames[4] = 0;
		fire_frames[5] = 0;
		fire_frames[6] = 0;
	} else if (info->arg3 == 6) {
		fire_frames[0] = 7;
		fire_frames[1] = 11;
		fire_frames[2] = 0;
		fire_frames[3] = 0;
		fire_frames[4] = 0;
		fire_frames[5] = 0;
		fire_frames[6] = 0;
	} else {
		fire_frames[0] = 6;
		fire_frames[1] = 7;
		fire_frames[2] = 8;
		fire_frames[3] = 9;
		fire_frames[4] = 10;
		fire_frames[5] = 11;
		fire_frames[6] = 0;
	}

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage;
	int			kick = 0;
	vec3_t		offset;
	int item = getWornItem(ent, GIEX_ITEM_WEAPON);
	iteminfo_t	*info = getWornItemInfo(ent, GIEX_ITEM_WEAPON);
	int	i, spread = getMachinegunSpread(ent, item, info);

//	damage = info->arg1 + info->arg2 * ent->client->pers.skill[1];
	damage = getMachinegunDamage(ent, item, info);

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	damage = applyWeaponBonuses(ent, damage, 1);
//	if (is_quad)
//		kick *= 4;

	for (i=1 ; i<3 ; i++) {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, spread, spread, MOD_MACHINEGUN);

	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
		ent->client->silencer_shots -= 1;
	} else {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]		= {23, 45, 0};
	static int	fire_frames[]		= {4, 5, 0};
	int firerate;
	iteminfo_t	*info = getWornItemInfo(ent, 0);
	firerate = (int) info->arg3;

	if (firerate > 0) {
		fire_frames[1] = 0;
		firerate--;
	} else {
		firerate = 0; // In case some machinegun has negative firerate :)
		fire_frames[1] = 5;
	}

	Weapon_Generic (ent, 3, 5 + firerate, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire (edict_t *ent)
{
	int			shots, flash_shots = 1;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 0;
	int item = getWornItem(ent, GIEX_ITEM_WEAPON);
	iteminfo_t	*info = getWornItemInfo(ent, 0);
	int	i, spread = getMachinegunSpread(ent, item, info);

//	damage = info->arg1 + info->arg2 * ent->client->pers.skill[1];
	damage = getChaingunDamage(ent, item, info);

	if (ent->client->ps.gunframe == 5) {
		if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 6)) {
			ent->client->silencer_shots -= 1;
		} else {
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);
		}
		ent->client->ps.gunframe += ent->client->pers.skill[5];
	}

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK)) {
		ent->client->ps.gunframe = 32; // End firing
		ent->client->weapon_sound = 0;
		return;
	} else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index]) {
		ent->client->ps.gunframe = 15; // Loop back
	} else {
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22) {
		ent->client->weapon_sound = 0;
		if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 6)) {
			ent->client->silencer_shots -= 1;
		} else {
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
		}
		ent->client->ps.gunframe += ent->client->pers.skill[5];
	} else {
		if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 6)) {
			ent->client->silencer_shots -= 1;
		} else {
			ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
		}
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	} else {
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9) {
		shots = info->arg4;
	} else if (ent->client->ps.gunframe <= 14) {
		if (ent->client->buttons & BUTTON_ATTACK) {
			shots = info->arg4 + 1; flash_shots = 2;
		} else {
			shots = info->arg4; flash_shots = 1;
		}
	}
	else { // 15 - 20
		if ((ent->client->ps.gunframe == 18) || (ent->client->ps.gunframe == 20)) {
			shots = info->arg4 + 2; flash_shots = 2;
		} else {
			shots = info->arg4 + 1; flash_shots = 1;
		}
	}

	if (shots < 1)
		shots = 1;
	if (ent->client->pers.inventory[ent->client->ammo_index] < shots) {
		shots = ent->client->pers.inventory[ent->client->ammo_index];
	}

	if (!shots) {
		if (level.time >= ent->pain_debounce_time) {
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	damage = applyWeaponBonuses(ent, damage, 1);
//	if (is_quad)
//		kick *= 4;

	for (i=0 ; i<3 ; i++) {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++) {
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, spread, spread, MOD_CHAINGUN);
	}

	// send muzzle flash
	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
		ent->client->silencer_shots -= shots;
	} else {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte ((MZ_CHAINGUN1 + flash_shots) | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->silencer_shots-=3;
	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick = 8;
	int item = getWornItem(ent, GIEX_ITEM_WEAPON);
	iteminfo_t	*info = getWornItemInfo(ent, GIEX_ITEM_WEAPON);
	int			count = getShotgunCount(ent, item, info); //info->arg4 + (int) (info->arg5 * ent->client->pers.skill[3]);
	int spread = getShotgunSpread(ent, item, info);

//	damage = getShotgunTotalDamage(ent, item, info);// (int) ceil((info->arg1 + info->arg2 * (float) ent->client->pers.skill[1]) * info->arg4);
	damage = (int) ceil(info->arg1 + info->arg2 * (float) ent->client->pers.skill[1]); //(info->arg4 / 2));
	damage /= count;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	damage = applyWeaponBonuses(ent, damage, count);
	if (is_quad)
		kick *= 4;

	fire_shotgun (ent, start, forward, damage, kick, spread, spread, count, MOD_SHOTGUN);
	ent->client->silencer_shots-=4;

	// send muzzle flash
	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
		ent->client->silencer_shots -= 1;
	} else {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SHOTGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 0};

	iteminfo_t *info = getWornItemInfo(ent, 0);
	Weapon_Generic (ent, 7, 7 + getShotgunFirerate(ent, getWornItem(ent, GIEX_ITEM_WEAPON), info), 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	iteminfo_t *info = getWornItemInfo(ent, 0);
	int			damage;
	int			ammo;
	int			kick = 9;
	int			count = (info->arg4 + (int)(info->arg5 * ent->client->pers.skill[3])) / 2;

	damage = (int) ceil(info->arg1 + info->arg2 * (float) ent->client->pers.skill[1]) * 0.5; //(info->arg4 / 2));
	damage /= count;
//	damage = info->arg1 + (int) ceil(info->arg2 * ent->client->pers.skill[1]);

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	damage = applyWeaponBonuses(ent, damage, count);
	if (is_quad)
		kick *= 4;

	v[PITCH] = ent->client->v_angle[PITCH];
	v[ROLL]  = ent->client->v_angle[ROLL];
	if (ent->client->ps.gunframe == 7) {
		v[YAW]   = ent->client->v_angle[YAW] - 5;
		AngleVectors (v, forward, NULL, NULL);
		fire_shotgun (ent, start, forward, damage, kick, info->arg6, info->arg6, count, MOD_SSHOTGUN);
		if (info->arg7 == 0) {
			v[YAW]   = ent->client->v_angle[YAW] + 5;
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, info->arg6, info->arg6, count, MOD_SSHOTGUN);
		}
	} else {
		v[YAW]   = ent->client->v_angle[YAW] + 5;
		AngleVectors (v, forward, NULL, NULL);
		fire_shotgun (ent, start, forward, damage, kick, info->arg6, info->arg6, count, MOD_SSHOTGUN);
	}
	ent->client->silencer_shots-=3;

	// send muzzle flash
	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
		ent->client->silencer_shots -= 1;
	} else {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SSHOTGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		ammo = 1;
		if (info->arg7 == 0) {
			ammo = 2;
		}
		ent->client->pers.inventory[ent->client->ammo_index] -= ammo;
//	}
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 9, 0};

	iteminfo_t *info = getWornItemInfo(ent, 0);
	if (info->arg7 == 0) {
		fire_frames[1] = 0;
	} else {
		fire_frames[1] = fire_frames[0] + info->arg7;
	}
	Weapon_Generic (ent, 6, 17 + info->arg3, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	iteminfo_t *info = getWornItemInfo(ent, 0);

	damage = info->arg1 + info->arg2 * ent->client->pers.skill[1];
	if (ent->client->pers.skill[1] > info->arg4) {
		damage += info->arg5 * (ent->client->pers.skill[1] - info->arg4);
	}
	kick = damage / 2 + 150;

	damage = applyWeaponBonuses(ent, damage, 1);
	if (is_quad)
		kick *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick);
	ent->client->silencer_shots-=8;

	// send muzzle flash
	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
		ent->client->silencer_shots -= 1;
	} else {
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_RAILGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};
	iteminfo_t *info = getWornItemInfo(ent, 0);

	Weapon_Generic (ent, 3, 18 + info->arg3, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent) {
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage, ammo, speed;
	float		damage_radius;
	iteminfo_t *info = getWornItemInfo(ent, 0);

	damage = (info->arg1 + info->arg3 * (ent->client->ps.gunframe - 8)) * (1.0 + ent->client->pers.skill[1] * info->arg2);
	ammo = (int) ceil((ent->client->ps.gunframe - 8) * info->arg7);
	speed = (info->arg4 + info->arg5 * (float) ent->client->pers.skill[7]) - (ent->client->ps.gunframe - 8) * info->arg6;
	damage_radius = 120 + damage / 10.0;
	if (damage_radius > 140)
		damage_radius = 140;

	if (ent->client->ps.gunframe == 9) {
		// send muzzle flash
		if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
			ent->client->silencer_shots -= 1;
		} else {
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BFG | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < ammo) {
		ent->client->ps.gunframe = 32 - (ent->client->ps.gunframe - 8);
		return;
	}
	if ( (ent->client->ps.gunframe < (17 + info->arg9)) && ((ent->client->buttons & BUTTON_ATTACK) || (ent->client->ps.gunframe < (9 + info->arg8)))) {
		ent->client->ps.gunframe++;
		return;
	}
	damage = applyWeaponBonuses(ent, damage, 1);

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -(ent->client->ps.gunframe - 8) * 2;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, damage, speed, damage_radius);
	ent->client->silencer_shots-=9;

	ent->client->ps.gunframe = 32 - (ent->client->ps.gunframe - 8);

	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ammo;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 10, 11, 12, 13, 14, 15, 16, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}


//======================================================================

/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_arm_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	iteminfo_t	*info = getWornItemInfo(ent, 0);
	int			kick = info->arg9;
	int			refire = info->arg3 + 2;
	int			recycle = info->arg4 + 1;
	int			range = info->arg5;
	int			count = info->arg6;// + (int) (info->arg6 * ent->client->pers.skill[3]);
	int			cost = info->arg10;

	if (refire < 2)
		refire = 2;
	if (recycle < 1)
		recycle = 1;

	cost *= ((float) (ent->client->max_magic + 1200.0)) / 1200.0;

	damage = (int) ceil(info->arg1 + info->arg2 * (float) ent->client->pers.skill[1]);

	if ((!(ent->client->buttons & BUTTON_ATTACK)) || (ent->client->magic < cost)) {
		ent->client->ps.gunframe = 9 + recycle;
		return;
	}
	ent->client->magic -= cost;
	if (ent->client->magregentime < level.time + 0.5)
		ent->client->magregentime = level.time + 0.5;

/*	if (ent->client->ps.gunframe == 9) {
		ent->client->ps.gunframe++;
		return;
	}*/

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	damage = applyWeaponBonuses(ent, damage, count);
	if (is_quad)
		kick *= 4;

	fire_armblast(ent, start, forward, damage, kick, info->arg7, info->arg8, count, range, MOD_ARM);
	ent->client->silencer_shots-=4;

	// send muzzle flash
	if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 7)) {
		ent->client->silencer_shots -= 1;
	} else {
		gi.sound(ent, CHAN_AUTO, gi.soundindex("parasite/paratck2.wav"), 1, ATTN_NORM, 0);
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_NUKE1);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

/*		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SHOTGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);*/
	}

	if ((ent->client->ps.gunframe == 9) && (ent->client->buttons & BUTTON_ATTACK)) {
		ent->client->ps.gunframe -= refire;
	}
	ent->client->ps.gunframe++;

//	if (!(ent->client->buttons & BUTTON_ATTACK))
//		ent->client->ps.gunframe += recycle;
	PlayerNoise(ent, start, PNOISE_WEAPON);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Arm(edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_arm_fire);
}

int getBlasterDamage(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg1 + info->arg2 * ent->client->pers.skill[1];
}
int getBlasterSpeed(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg6 + info->arg7 * ent->client->pers.skill[1];
}
int getBlasterConsumption(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg8 + info->arg9 * ent->client->pers.skill[1];
}
int getBlasterFirerate(edict_t *ent, int item, iteminfo_t *info) {
	return 4 + info->arg3;
}
int getBlasterPierce(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg4 + info->arg5 * ent->client->pers.skill[2];;
}
int getWeaponPierce(edict_t *ent, int item) {
	return 0.02 * ent->client->pers.skill[2];
}

int getShotgunTotalDamage(edict_t *ent, int item, iteminfo_t *info) {
	return (int) ceil((info->arg1 + info->arg2 * (float) ent->client->pers.skill[1]) * info->arg4);
}
int getShotgunCount(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg4 + (int) (info->arg5 * ent->client->pers.skill[3]);
}
int getShotgunSpread(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg6 * (1 + 0.04 * ent->client->pers.skill[3]);
}
int getShotgunFirerate(edict_t *ent, int item, iteminfo_t *info) {
	return 11 + info->arg3;
}

int getSuperShotgunTotalDamage(edict_t *ent, int item, iteminfo_t *info) {
	return (int) ceil((info->arg1 + info->arg2 * (float) ent->client->pers.skill[1]) * info->arg4);
}
int getSuperShotgunCount(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg4 + (int) (info->arg5 * ent->client->pers.skill[3]);
}
int getSuperShotgunSpread(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg6 * (1 + 0.05 * ent->client->pers.skill[3]);;
}
int getSuperShotgunFirerate(edict_t *ent, int item, iteminfo_t *info) {
	return 11 + info->arg3;
}
int getSuperShotgunBarrelDelay(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg7;
}

int getMachinegunDamage(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg1 + info->arg2 * ent->client->pers.skill[1];
}
int getMachinegunSpread(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg4 - info->arg6 * ent->client->pers.skill[8];
}
int getMachinegunFirerate(edict_t *ent, int item, iteminfo_t *info) {
	return 1 + info->arg3;
}

int getChaingunDamage(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg1 + info->arg2 * ent->client->pers.skill[1];
}
int getChaingunSpread(edict_t *ent, int item, iteminfo_t *info) {
	return info->arg4 - info->arg6 * ent->client->pers.skill[8];
}
int getChaingunTotalDamage(edict_t *ent, int item, iteminfo_t *info) {
	return getChaingunDamage(ent, item, info) * (info->arg5 + 1.5);
}
