// g_weapon.c

#include "g_local.h"
#include "m_player.h"
#include "w_fbomb.h"


qboolean	is_quad;
static byte		is_silenced;

void fire_explosive_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_asha  (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_blooddrainer (edict_t *self, vec3_t start, vec3_t dir, int damage);

void weapon_grenade_fire (edict_t *ent, qboolean held);


//Wyrm: made dinamic
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
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
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t		*noise;

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;


	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
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

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value) 
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
                //Wyrm: check if ammo
                if (ent->item->ammo)
                {
                        ammo = FindItem (ent->item->ammo);
                        if ( (int)dmflags->value & DF_INFINITE_AMMO )
                                Add_Ammo (other, ammo, 1000);
                        else
                                Add_Ammo (other, ammo, ammo->quantity);
                }

                //Wyrm: second ammo
		// give them some ammo with it
                if (ent->item->ammo2)
                {
                        ammo = FindItem (ent->item->ammo2);
                        if ( (int)dmflags->value & DF_INFINITE_AMMO )
                                Add_Ammo (other, ammo, 1000);
                        else
                                Add_Ammo (other, ammo, ammo->quantity);
                }

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
		other->client->newweapon = ent->item;

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

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

        //Weapon Hud
        if (ent->client->pers.weapon)
                ent->client->ps.stats[STAT_WEAPON_STRING] = CS_ITEMS+ITEM_INDEX(ent->client->pers.weapon);

	// set visible model
	if (ent->s.modelindex == 255) {
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

        if (ent->client->pers.weapon != GetInitialWeapon(ent))
        {
                if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
                        ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
                else
                        ent->client->ammo_index = 0;

                //Wyrm: second ammo
                if (ent->client->pers.weapon && ent->client->pers.weapon->ammo2)
                        ent->client->ammo2_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo2));
                else
                        ent->client->ammo2_index = 0;
        }
        else
        {
                ent->client->ammo_index = 0;
                ent->client->ammo2_index = 0;
        }

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
        //Wyrm: chasecam & turret
        if ((!ent->client->chasetoggle)&&(!ent->client->onturret))
                ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	ent->client->anim_priority = ANIM_PAIN;
	if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
			ent->s.frame = FRAME_crpain1;
			ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
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
//        ent->client->newweapon = FindItem ("blaster");
//Use initial weapon
        ent->client->newweapon = GetInitialWeapon(ent);
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
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	}

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots)
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
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

        if (item != GetInitialWeapon(ent))
        {
                if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
                {
                        ammo_item = FindItem(item->ammo);
                        ammo_index = ITEM_INDEX(ammo_item);
                
                        if (!ent->client->pers.inventory[ammo_index])
                        {
                                gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
                                return;
                        }

                        if (ent->client->pers.inventory[ammo_index] < item->quantity)
                        {
                                gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
                                return;
                        }
                }

                //Wyrm: second ammo
                if (item->ammo2 && !g_select_empty->value && !(item->flags & IT_AMMO))
                {
                        ammo_item = FindItem(item->ammo2);
                        ammo_index = ITEM_INDEX(ammo_item);
                
                        if (!ent->client->pers.inventory[ammo_index])
                        {
                                gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
                                return;
                        }

                        if (ent->client->pers.inventory[ammo_index] < item->quantity)
                        {
                                gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
                                return;
                        }
                }
	}

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

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
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

static void Weapon_Generic2 (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
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

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
                //Wyrm: change frames
                if ((ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)||(!changeframes->value))
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
                //Wyrm: changing frames
                if (changeframes->value)
                {
                        ent->client->weaponstate = WEAPON_DROPPING;
                        ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
                }
                else
                {
                        ChangeWeapon (ent);
                        if (ent->client->weaponstate = WEAPON_ACTIVATING)
                                ent->client->pers.weapon->weaponthink(ent);
                }

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
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
                     if (!ent->client->onturret)   
                     {   
                        //Wyrm: second ammo
                        if ((!ent->client->ammo_index) ||
                            (   ( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity) &&
                                ((!ent->client->ammo2_index) || ( ent->client->pers.inventory[ent->client->ammo2_index] >= ent->client->pers.weapon->quantity2))
                            )
                           )
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
                     }
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
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
//ZOID
				if (!CTFApplyStrengthSound(ent))
//ZOID
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

//ZOID
				CTFApplyHasteSound(ent);
//ZOID

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

//ZOID
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
		FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, 
		fire_frames, fire);

	// run the weapon frame again if hasted
	if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate == WEAPON_FIRING)
		return;

	if ((CTFApplyHaste(ent) ||
		(Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate != WEAPON_FIRING))
		&& oldstate == ent->client->weaponstate) {
		Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
			FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, 
			fire_frames, fire);
	}
}
//ZOID

/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Grenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
                if (!ent->client->onturret)
                {
                        if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
                        {
                                ent->client->latched_buttons &= ~BUTTON_ATTACK;
                                if ((ent->client->ammo_index) || (ent->client->pers.inventory[ent->client->ammo_index]))
                                {
                                        ent->client->ps.gunframe = 1;
                                        ent->client->weaponstate = WEAPON_FIRING;
                                        ent->client->grenade_time = 0;
                                }
                                else
                                {
                                        if (level.time >= ent->pain_debounce_time)
                                        {
                                                gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                                                ent->pain_debounce_time = level.time + 1;
                                        }
                                        NoAmmoWeaponChange (ent);
                                }
                                return;
                        }
                }

                if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
                {
                        if (rand()&15)
                                return;
                }
                
                if (++ent->client->ps.gunframe > 48)
                        ent->client->ps.gunframe = 16;
                return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
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
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

        fire_grenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
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

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right, up, dir;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, up);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	VectorCopy(forward, dir);
	//Wyrm: new optional blaster

	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, dir, damage, 1000, effect, hyper);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	if (hyper)
		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
	else
		gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage;

	if (deathmatch->value)
		damage = 15;
	else
		damage = 10;
	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
				ent->client->pers.inventory[ent->client->ammo_index]--;

        ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
                if ((ent->client->ammo_index)&&
                   (! ent->client->pers.inventory[ent->client->ammo_index] ))
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;
			Blaster_Fire (ent, offset, damage, true, effect);
        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
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
                if (ent->client->ps.gunframe == 12 &&
                ((ent->client->pers.inventory[ent->client->ammo_index]) ||
                (!ent->client->ammo_index))
                )
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

        if ((ent->client->ammo_index)&&
           (ent->client->pers.inventory[ent->client->ammo_index] < 1))
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

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
/*
	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}
*/
	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

        gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
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
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&&
                ((ent->client->pers.inventory[ent->client->ammo_index])||
                (!ent->client->ammo_index))
                )
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

        if ((ent->client->ammo_index)&&
            (ent->client->pers.inventory[ent->client->ammo_index] < shots))
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
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
        vec3_t          forward, right, up;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

        AngleVectors (ent->client->v_angle, forward, right, up);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

        if (ent->client->ps.gunframe == 13)
	{
                vec3_t dir;

                VectorSet(offset, 0, 4,  ent->viewheight-6);
                P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

                //Wyrm! Shell ejecting...
                VectorScale(right, 100, dir);
                VectorMA(dir, 100 + (crandom() * 10.0), up, dir);
                VectorMA(start, 20, forward, start);
                eject_shell(ent, start, dir);
		ent->client->ps.gunframe++;
		return;
	}

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	if (deathmatch->value)
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	else
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
        static int      fire_frames[]   = {8, 9, 13, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
        vec3_t          forward, right, up;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;

        AngleVectors (ent->client->v_angle, forward, right, up);

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (ent->client->ps.gunframe == 15)
        {
                vec3_t dir, dir2, shellstart, shellstart2;

                ent->client->ps.gunframe++;
                //Wyrm! Shell ejecting...
                VectorScale(up, 150 + (crandom() * 10.0), dir);
                VectorScale(up, 150 + (crandom() * 10.0), dir2);
                VectorMA(dir, 100, right, dir);
                VectorMA(dir2, -100, right, dir2);
                VectorMA(start, 22, forward, shellstart);
                VectorMA(shellstart, -5, up, shellstart);
                VectorMA(shellstart, 7, right, shellstart2);
                VectorMA(shellstart, -2, right, shellstart);
                eject_shell(ent, shellstart, dir);
                eject_shell(ent, shellstart2, dir2);
                return;
        }

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
        static int      fire_frames[]   = {7, 15, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
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

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 100;
		kick = 200;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
        if ((ent->client->ammo_index)&&
           (ent->client->pers.inventory[ent->client->ammo_index] < 50))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}


//======================================================================
//Wyrm new weapons!!

//
// Disintegrator
//

void weapon_tracker_fire (edict_t *self)
{
	vec3_t		forward, right;
	vec3_t		start;
	vec3_t		end;
	vec3_t		offset;
	edict_t		*enemy;
	trace_t		tr;
	int			damage;
	vec3_t		mins, maxs;

        if (self->client->ps.gunframe == 12)
	{
                gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
                self->client->weapon_sound = 0;
                self->client->ps.gunframe++;
                return;
	}

        self->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	// PMM - felt a little high at 25
/*
	if(deathmatch->value)
		damage = 30;
	else
*/
        damage = 100;

	if (is_quad)
                damage *= 4;            //pgm

	VectorSet(mins, -16, -16, -16);
	VectorSet(maxs, 16, 16, 16);
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, self->viewheight-8);
	P_ProjectSource (self->client, self->s.origin, offset, forward, right, start);

	// FIXME - can we shorten this? do we need to?
	VectorMA (start, 8192, forward, end);
	enemy = NULL;
	//PMM - doing two traces .. one point and one box.  
	tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_SHOT);
	if(tr.ent != world)
	{
                if(tr.ent->svflags & SVF_MONSTER || tr.ent->client || tr.ent->takedamage)
		{
			if(tr.ent->health > 0)
				enemy = tr.ent;
		}
	}
	else
	{
		tr = gi.trace (start, mins, maxs, end, self, MASK_SHOT);
		if(tr.ent != world)
		{
                        if(tr.ent->svflags & SVF_MONSTER || tr.ent->client || tr.ent->takedamage)
			{
				if(tr.ent->health > 0)
					enemy = tr.ent;
			}
		}
	}

	VectorScale (forward, -2, self->client->kick_origin);
	self->client->kick_angles[0] = -1;

	fire_tracker (self, start, forward, damage, 1000, enemy);
/*
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_TRACKER);
	gi.multicast (self->s.origin, MULTICAST_PVS);
*/
	PlayerNoise(self, start, PNOISE_WEAPON);

	self->client->ps.gunframe++;
        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(self->client->ammo_index))
	self->client->pers.inventory[self->client->ammo_index] -= self->client->pers.weapon->quantity;
}

void Weapon_Disintegrator (edict_t *ent)
{
	static int	pause_frames[]	= {0};
        static int      fire_frames[]   = {6, 12, 0};

        Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, weapon_tracker_fire);
}

/*
        Pulse Rifle
*/

void weapon_pulserifle_fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	offset;
	vec3_t	tempang;
	int		damage;
	int		kick;

        if (ent->client->ps.gunframe == 5)
        {
                if (ent->client->machinegun_shots)
                {
                        ent->client->kick_origin[0]= -0.2;
                        ent->client->kick_angles[0] = ent->client->machinegun_shots * -2.5;
                        ent->client->machinegun_shots--;
                }
                else
                        ent->client->ps.gunframe++;

                return;
        }

	if (deathmatch->value)
	{
		// tone down for deathmatch
		damage = 30;
		kick = 40;
	}
	else
	{
		damage = 50;
		kick = 60;
	}
	
	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
/*
	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
*/
        ent->client->kick_origin[0]= -0.2;
        ent->client->machinegun_shots = 3;
        ent->client->kick_angles[0] = ent->client->machinegun_shots * -2.5;

	VectorCopy (ent->client->v_angle, tempang);
	tempang[YAW] += crandom();

	AngleVectors (tempang, forward, right, NULL);
	
	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	// VectorSet (offset, 0, 7, ent->viewheight - 8);
	VectorSet (offset, 16, 7, ent->viewheight - 8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        fire_pulserifle (ent, start, forward, damage, 500, EF_GRENADE);
/*
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent - g_edicts);
	gi.WriteByte (MZ_IONRIPPER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
*/
        if (!ent->client->silencer_shots)
                gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pulse1.wav"), 1, ATTN_NORM, 0);
        else
                gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pulse1.wav"), 0.2, ATTN_NORM, 0);

	ent->client->ps.gunframe++;
	PlayerNoise (ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
	
                if (ent->client->pers.inventory[ent->client->ammo_index] < 0)
                        ent->client->pers.inventory[ent->client->ammo_index] = 0;
        }
}


void Weapon_PulseRifle (edict_t *ent)
{
        static int pause_frames[] = {23, 45, 0};
        static int fire_frames[] = {4, 5, 0};

        Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, weapon_pulserifle_fire);
}


/*
======================================================================

NAPALM ROCKETS

======================================================================
*/

void Weapon_NapalmRockets_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        fire_flamerocket (ent, start, forward, damage, 650, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_NapalmRockets (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

        Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_NapalmRockets_Fire);
}
/***************************************************/
/**************        pistol         **************/
/***************************************************/

void Weapon_Pistol_Fire (edict_t *ent)
{
	int		damage;

        if (ent->client->ps.gunframe == 4)
        {
                if (ent->client->machinegun_shots)
                        ent->client->machinegun_shots = 0;
                else
                        ent->client->ps.gunframe++;
                return;
        }

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
				ent->client->ps.gunframe = 7;
	}
	else
	{
                vec3_t forward, right, offset, start;

                damage = 14;

                AngleVectors (ent->client->v_angle, forward, right, NULL);
                VectorSet(offset, 0, 8, ent->viewheight-8);
                P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
                fire_bullet (ent, start, forward, damage, 0, 100, 100, MOD_PISTOL);

                gi.WriteByte (svc_muzzleflash);
                gi.WriteShort (ent-g_edicts);
                gi.WriteByte (MZ_MACHINEGUN | is_silenced);
                gi.multicast (ent->s.origin, MULTICAST_PVS);

                PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                        ent->client->pers.inventory[ent->client->ammo_index]--;
        
                ent->client->ps.gunframe = 4;
                ent->client->machinegun_shots = 1;
        }

//        ent->client->ps.gunframe++;
}

void Weapon_Pistol (edict_t *ent)
{
        static int      pause_frames[]  = {19, 32, 0};
        static int      fire_frames[]   = {4, 5, 0};

        Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Pistol_Fire);
}

/***************************************************/
/**************       flaregun        **************/
/***************************************************/

void Weapon_Flaregun_Fire (edict_t *ent)
{
        int damage = 10;
	vec3_t	offset, start;
	vec3_t	forward, right;

	if (is_quad)
		damage *= 4;
        
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 24, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        fire_flaregun (ent, start, forward, damage, 1000);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->ps.gunframe++;
}

void Weapon_Flaregun (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

        Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Flaregun_Fire);
}

/***************************************************/
/**************       airfist         **************/
/***************************************************/

void weapon_airfist_fire (edict_t *ent)
{
	vec3_t		start;
        vec3_t          up, forward, right;
	vec3_t		offset;
        int                     damage = 20;
	int			kick = 8;
//        int                     num_traces;

        if (ent->client->ps.gunframe == 12)
	{
                if (ent->client->buttons & BUTTON_ATTACK)
                        ent->client->ps.gunframe = 6;
                else
                {
                        gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                        ent->client->machinegun_shots = 0;
                        ent->client->ps.gunframe++;
                }
		return;
	}

        AngleVectors (ent->client->v_angle, forward, right, up);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}


        if (ent->client->machinegun_shots >= 5)
        {
               gi.sound (ent, CHAN_VOICE, gi.soundindex ("gunner/gunatck3.wav"), 1, ATTN_NORM, 0);
        }
        else
        {
                if (deathmatch->value)
                        fire_airfist(ent, start, ent->client->v_angle, damage, 150, 400, 300);
                else
                        fire_airfist(ent, start, ent->client->v_angle, damage/2, 150, 400, 300);
        }

        ent->client->machinegun_shots++;
        
	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

}

void Weapon_Airfist (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
        static int      fire_frames[]   = {8, 12, 0};

        Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_airfist_fire);
}

/***************************************************/
/**************    double impact      **************/
/***************************************************/
void weapon_dgsupershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
        int                     damage = 100;

        int radius = damage + 40;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
		damage *= 4;

	v[PITCH] = ent->client->v_angle[PITCH];
        v[YAW]   = ent->client->v_angle[YAW] - 6;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);

        fire_grenade3(ent, start, forward, damage, 600, 2 + 0.5*random(), radius);

        v[YAW]   = ent->client->v_angle[YAW] + 6;
	AngleVectors (v, forward, NULL, NULL);
        fire_grenade3(ent, start, forward, damage, 600, 2 + 0.5*random(), radius);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_DGSuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

        Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_dgsupershotgun_fire);
}


/***************************************************/
/**************  Explosive Machinegun **************/
/***************************************************/
void ExplosiveMachinegun_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
        int                     damage = 29;
	int			kick = 2;
	vec3_t		offset;
        int             raising;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
                ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

        if ((ent->client->ammo_index && ent->client->ammo2_index)&&
           ((ent->client->pers.inventory[ent->client->ammo_index] < 1) ||
            ((ent->client->pers.inventory[ent->client->ammo2_index] < 1)&&(!(ent->client->machinegun_shots % 10))))
           )
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

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

        raising = ent->client->machinegun_shots;
        if (raising > 9) raising = 9;

        ent->client->kick_origin[0] = crandom() * 0.35;
        ent->client->kick_angles[0] = raising * -1.5;


	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        fire_explosive_bullet (ent, start, forward, damage, kick, DEFAULT_EXBULLET_HSPREAD, DEFAULT_EXBULLET_VSPREAD, MOD_EXPLOSIVEMACHINEGUN);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
                ent->client->pers.inventory[ent->client->ammo_index] -= 1;
                if (!(ent->client->machinegun_shots % 10))
                        ent->client->pers.inventory[ent->client->ammo2_index] -= 1;
        }
	// raise the gun as it is firing
        ent->client->machinegun_shots++;

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

void Weapon_ExplosiveMachinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

        Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, ExplosiveMachinegun_Fire);
}

/***************************************************/
/**************    FlameThrower       **************/
/***************************************************/
void Flamethrower_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
        {
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);
        }

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&&
                ((ent->client->pers.inventory[ent->client->ammo_index])||
                (!ent->client->ammo_index))
                )
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}


        if ((ent->client->ammo_index)&&
           (!ent->client->pers.inventory[ent->client->ammo_index]))
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
        // get start / end positions
        AngleVectors (ent->client->v_angle, forward, right, up);
        VectorSet(offset, 4, 4, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        
        VectorMA(start, 10, forward, start);

        fire_flame (ent, start, forward, damage, /*speed*/ 200);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
                if (!ent->client->machinegun_shots)
                {
                        ent->client->pers.inventory[ent->client->ammo_index]--;
                        ent->client->machinegun_shots = 10;
                }
                ent->client->machinegun_shots--;
        }
}


void Weapon_Flamethrower (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

        Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Flamethrower_Fire);
}

/***************************************************/
/**************  Street Sweeper       **************/
/***************************************************/
void StreetSweeper_Fire (edict_t *ent)
{
	int			i;
        vec3_t          start, dir, shellstart;
	vec3_t		forward, right, up;
	vec3_t		offset;
        int                     damage = 4;
	int			kick = 2;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&&
                ((ent->client->pers.inventory[ent->client->ammo_index])||
                (!ent->client->ammo_index))
                )
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

        if ((ent->client->ammo_index)&&
          (!ent->client->pers.inventory[ent->client->ammo_index]))
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}
        
        if (!ent->client->machinegun_shots)
        {
                ent->client->machinegun_shots = 1;
                return;
        }
        else
                ent->client->machinegun_shots = 0;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

        AngleVectors (ent->client->v_angle, forward, right, up);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        //Wyrm! Shell ejecting...
        VectorScale(right, -100, dir);
        VectorMA(dir, 150 + (crandom()*20), up, dir);
        VectorMA(start, 25, forward, shellstart);
        VectorMA(shellstart, -5, up, shellstart);
        VectorMA(shellstart, -5, right, shellstart);
        eject_shell(ent, shellstart, dir);

        fire_shotgun (ent, start, forward, damage, kick, 700, 700, 10, MOD_STREETSWEEPER);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_StreetSweeper (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
        static int      fire_frames[]   = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

        Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, StreetSweeper_Fire);
}

/***************************************************/
/**************  Cluster Grenades     **************/
/***************************************************/
void weapon_clustergrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
        int             damage = 90;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

        fire_clustergrenade (ent, start, forward, damage, 600, 1, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index]-=3;
}

void Weapon_ClusterGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

        Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_clustergrenadelauncher_fire);
}

/***************************************************/
/**************  Guided Misiles       **************/
/***************************************************/
void Weapon_GuidedMisiles_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        fire_guidedrocket (ent, start, forward, damage, 650, 200, 200);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
                ent->client->pers.inventory[ent->client->ammo_index]-=1;
                ent->client->pers.inventory[ent->client->ammo2_index]-=3;
        }
}

void Weapon_GuidedMisiles (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

        Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_GuidedMisiles_Fire);
}

/***************************************************/
/**************      Freeze gun       **************/
/***************************************************/
void ThrowIce (edict_t *self);
void IcePosFromPlayer (edict_t *ent, float size, vec3_t output);
void IceThink (edict_t *self);
void CreateIce (edict_t *self);

void Weapon_Freezegun_Fire (edict_t *ent)
{
        int             j;
        float frame;
        int damage = 10;

        ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
                if (ent->client->iceball)
                        ThrowIce(ent);
                
                ent->client->machinegun_shots = 0;

		ent->client->ps.gunframe++;
	}
	else
	{
                if ((ent->client->ammo_index)&&
                   (! ent->client->pers.inventory[ent->client->ammo_index] ))
		{
                        if (ent->client->iceball)
                                ThrowIce(ent);

			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
                        if (ent->client->iceball != NULL)
                        {
                                if (ent->client->iceball->s.frame < 19)
                                {
                                        if (ent->client->machinegun_shots >= 1)
                                        {
                                                        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
                                                           &&(ent->client->ammo_index))
                                                        ent->client->pers.inventory[ent->client->ammo_index]--;
                                                ent->client->machinegun_shots = 0;
                                        }
                                        else
                                                ent->client->machinegun_shots++;

                                        //Wyrm: ctf not added yet
                                        //damage = CTFApplyStrength(ent, damage);
                                        ent->client->iceball->dmg += damage;

                                        ent->client->iceball->s.frame++;
                                        
                                        frame = floor(0.6*ent->client->iceball->s.frame);
                                        for (j=0 ; j <=2 ; j++)
                                        {
                                                ent->client->iceball->maxs[j] = 4 + frame;
                                                ent->client->iceball->mins[j] = -4 - frame;
                                        }
                                        
                                }
                        }
                        else
                        {
                                if (ent->client->machinegun_shots)
                                {
                                        if (ent->client->machinegun_shots >= 10)
                                                ent->client->machinegun_shots = 0;
                                        else
                                                ent->client->machinegun_shots++;
                                }
                                else
                                        CreateIce(ent);
                        }
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 &&
                ((!ent->client->ammo_index)||
                (ent->client->pers.inventory[ent->client->ammo_index]))
                )
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_Freezegun (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

        Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_Freezegun_Fire);
}

/***************************************************/
/******************  Plasma Gun  *******************/
/***************************************************/
void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, vec3_t up, vec3_t right, int damage, int speed);

void Weapon_PlasmaGun_Fire (edict_t *ent)
{
        vec3_t  offset, forward, right, up, start;
	int		damage;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
                if ((ent->client->ammo_index)&&
                   (! ent->client->pers.inventory[ent->client->ammo_index] ))
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
                        AngleVectors (ent->client->v_angle, forward, right, up);

                        VectorScale (forward, -2, ent->client->kick_origin);
                        ent->client->kick_angles[0] = -1;

                        VectorSet(offset, 8, 8, ent->viewheight-8);
                        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

                        if (!deathmatch->value)
                                damage = 25;
                        else
                                damage = 20;

                        fire_plasma (ent, start, forward, right, up, damage, 1000);
                        gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/plasma.wav"), 1, ATTN_NORM, 0);
                        
        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
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
		if (ent->client->ps.gunframe == 12 &&
                ((ent->client->pers.inventory[ent->client->ammo_index])||
                (!ent->client->ammo_index))
                )
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_PlasmaGun (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

        Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_PlasmaGun_Fire);
}


/***************************************************/
/**************  Anti-Matter Cannon   **************/
/***************************************************/
void weapon_asha_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
        int                     count;

        damage = 80;
        kick = 200;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

        if (ent->groundentity)
        {
                ent->groundentity = NULL;
                ent->s.origin[2]++;
        }

        VectorMA (ent->velocity, -300, forward, ent->velocity);

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        for (count = 1 ; count <= 6 ; count++)
                fire_asha (ent, start, forward, damage, kick, 1310, 655, MOD_ANTIMATTER);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index] -= 3;
}


void Weapon_AntimatterCannon (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

        Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_asha_fire);
}

/***************************************************/
/**************     Positron Beam     **************/
/***************************************************/
void PositronBeamFire(edict_t *ent, vec3_t start, vec3_t dir);

void weapon_positron_fire (edict_t *ent)
{
        vec3_t          start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

        damage = 80;
        kick = 200;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        PositronBeamFire(ent, start, forward);

        if (!ent->client->silencer_shots)
                gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pt_fire.wav"), 1, ATTN_NORM, 0);
        else
                gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pt_fire.wav"), 0.2, ATTN_NORM, 0);
        
	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
                ent->client->pers.inventory[ent->client->ammo_index] -= 2;
                ent->client->pers.inventory[ent->client->ammo2_index] -= 10;
        }
}


void Weapon_PositronBeam (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

        Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_positron_fire);
}


void weapon_bfgrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
        int             damage = 200;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

        fire_bfgrenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
		ent->client->pers.inventory[ent->client->ammo_index]--;
                ent->client->pers.inventory[ent->client->ammo2_index]-=20;
        }
}

void Weapon_BFGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

        Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_bfgrenadelauncher_fire);
}

void weapon_proxgrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
        int damage_multiplier = 1;

	if (is_quad)
                damage_multiplier = 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

        fire_proxgrenade (ent, start, forward, damage_multiplier, 600);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
		ent->client->pers.inventory[ent->client->ammo_index]--;
                ent->client->pers.inventory[ent->client->ammo2_index]-=2;
        }
}

void Weapon_ProxGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

        Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_proxgrenadelauncher_fire);
}

void weapon_railgrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
        int             damage = 60;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

        fire_railgrenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
		ent->client->pers.inventory[ent->client->ammo_index]--;
                ent->client->pers.inventory[ent->client->ammo2_index]-=2;
        }
}

void Weapon_RailGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

        Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_railgrenadelauncher_fire);
}

void weapon_stickinggrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

        fire_stickinggrenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_StickingGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

        Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_stickinggrenadelauncher_fire);
}

void weapon_napalmgrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

        fire_flamegrenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_NapalmGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

        Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_napalmgrenadelauncher_fire);
}

/*
======================================================================

TRIPBOMB

======================================================================
*/

#define TBOMB_DELAY	1.0

qboolean fire_tripbomb(edict_t *self, vec3_t start, vec3_t dir, float timer, float damage, float damage_radius, qboolean quad);

void weapon_tripbomb_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (fire_tripbomb(ent, start, forward, TBOMB_DELAY, damage, radius, is_quad))
        {
                if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
                   &&(ent->client->ammo_index))
                {
                        ent->client->pers.inventory[ent->client->ammo_index] -= 2;
                        ent->client->pers.inventory[ent->client->ammo2_index] -= 2;
                }

                // play quad sound
                //playQuadSound(ent);
        }

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Tripbomb (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if ((ent->client->ammo_index)&&
                           (ent->client->ammo2_index)&&
                           (ent->client->pers.inventory[ent->client->ammo_index] > 1)&&
                           (ent->client->pers.inventory[ent->client->ammo2_index] > 1))
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

//                if (ent->client->ps.gunframe == 11)
//                        ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
                        weapon_tripbomb_fire (ent);
		}

#if 0
		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;
#endif

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
			ent->client->weaponstate = WEAPON_READY;
	}
}


/*
======================================================================

LASER TRIPBOMB

======================================================================
*/

qboolean fire_lasertripbomb(edict_t *self, vec3_t start, vec3_t dir, float timer, float damage, float damage_radius, qboolean quad);

void weapon_lasertripbomb_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
        int             damage = 65;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (fire_lasertripbomb(ent, start, forward, TBOMB_DELAY, damage, radius, is_quad))
        {
        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                {
                        ent->client->pers.inventory[ent->client->ammo_index] -= 1;
                        ent->client->pers.inventory[ent->client->ammo2_index] -= 5;
                } 

                // play quad sound
                //playQuadSound(ent);
        }

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_LaserTripbomb (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if ((ent->client->ammo_index)&&
                           (ent->client->ammo2_index)&&
                           (ent->client->pers.inventory[ent->client->ammo_index])&&
                           (ent->client->pers.inventory[ent->client->ammo2_index] > 4))
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

//                if (ent->client->ps.gunframe == 11)
//                        ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
                        weapon_lasertripbomb_fire (ent);
		}

#if 0
		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;
#endif

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
			ent->client->weaponstate = WEAPON_READY;
	}
}



/*
======================================================================

Energy Vortex

======================================================================
*/
void fire_energyvortex (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);

void weapon_energyvortex_fire (edict_t *ent)
{
	vec3_t	offset, start;
        vec3_t  forward, right, up, dir;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
                if (!ent->client->silencer_shots)
                        gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/bfg__f1y.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/bfg__f1y.wav"), 0.2, ATTN_NORM, 0);

		ent->client->ps.gunframe++;

                //PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
        if ((ent->client->ammo_index)&&(ent->client->pers.inventory[ent->client->ammo_index] < 100))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

        AngleVectors (ent->client->v_angle, forward, right, up);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
/*
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;
*/

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


        VectorMA(up, 4, forward, dir);
        VectorNormalize(dir);

        fire_energyvortex (ent, start, dir, damage, 500, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index] -= 100;
}

void Weapon_EnergyVortex (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

        Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_energyvortex_fire);
}

/*
======================================================================

Vacuum Maker

======================================================================
*/
void fire_vacuummaker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);

void weapon_vacuummaker_fire (edict_t *ent)
{
	vec3_t	offset, start;
        vec3_t  forward, right, up, dir;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

        if ((ent->client->ps.gunframe == 11)||
        ((!ent->client->pers.inventory[ent->client->ammo_index])&&(ent->client->ammo_index))
        )
        {
                ent->client->ps.gunframe = 32;
                return;
        }

        if (ent->client->ps.gunframe == 9)
        {
                if (!ent->client->silencer_shots)
                        gi.sound(ent, CHAN_VOICE, gi.soundindex("vacuum.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound(ent, CHAN_VOICE, gi.soundindex("vacuum.wav"), 0.2, ATTN_NORM, 0);
        }

	if (is_quad)
		damage *= 4;

        AngleVectors (ent->client->v_angle, forward, right, up);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
/*
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;
*/

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


        VectorMA(up, 4, forward, dir);
        VectorNormalize(dir);

        fire_vacuummaker (ent, start, dir, damage, 500, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if (ent->client->ps.gunframe == 10)
        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index] -= 1;

        if ((ent->client->ps.gunframe == 11)&&
        ((ent->client->pers.inventory[ent->client->ammo_index])||
        (!ent->client->ammo_index))
        &&(ent->client->buttons & BUTTON_ATTACK))
                ent->client->ps.gunframe = 9;
}

void Weapon_VacuumMaker (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
        static int      fire_frames[]   = {9, 10, 11, 0};

        Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_vacuummaker_fire);
}

/*************************************************/
/**************    flak cannon      **************/
/*************************************************/
void fire_flakcannon (edict_t *self, vec3_t start, vec3_t aimdir, int ammount, int speed);

void weapon_flakcannon_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
        int                     damage = 100;

        int radius = damage + 40;

        if (ent->client->ps.gunframe == 12)
        {
                if (!ent->client->machinegun_shots)
                        ent->client->ps.gunframe = 17;
                else
                {
                        if (!ent->client->silencer_shots)
                                gi.sound(ent, CHAN_WEAPON, gi.soundindex("flak2.wav"), 1, ATTN_NORM, 0);
                        else
                                gi.sound(ent, CHAN_WEAPON, gi.soundindex("flak2.wav"), 0.2, ATTN_NORM, 0);
                        ent->client->ps.gunframe++;
                }

                ent->client->machinegun_shots = !ent->client->machinegun_shots;

                return;
        }

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
		damage *= 4;

	v[PITCH] = ent->client->v_angle[PITCH];
        if (!ent->client->machinegun_shots)
                v[YAW]   = ent->client->v_angle[YAW] + 3;
        else
                v[YAW]   = ent->client->v_angle[YAW] - 3;

	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);

        fire_flakcannon (ent, start, forward, 5, 800);
/*
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_SSHOTGUN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
*/
        if (!ent->client->silencer_shots)
                gi.sound(ent, CHAN_WEAPON, gi.soundindex("flak1.wav"), 1, ATTN_NORM, 0);
        else
                gi.sound(ent, CHAN_WEAPON, gi.soundindex("flak1.wav"), 0.2, ATTN_NORM, 0);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
                ent->client->pers.inventory[ent->client->ammo_index] -= 1;
                ent->client->pers.inventory[ent->client->ammo2_index] -= 5;
        }
}

void Weapon_FlakCannon (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
        static int      fire_frames[]   = {7, 12, 0};

        Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_flakcannon_fire);
}

/***************************************************/
/**************    blood drainer      **************/
/***************************************************/

void weapon_blooddrainer_fire (edict_t *ent)
{
	vec3_t		start;
        vec3_t          forward, right, up;
	vec3_t		offset;
        int                     damage = 100;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

        AngleVectors (ent->client->v_angle, forward, right, up);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;


	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

        fire_blooddrainer (ent, start, forward, damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
		ent->client->pers.inventory[ent->client->ammo_index]--;
                ent->client->pers.inventory[ent->client->ammo2_index]--;
        }
}

void Weapon_BloodDrainer (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
        static int      fire_frames[]   = {8, 9, 0};

        Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_blooddrainer_fire);
}

/***************************************************/
/**************    lightning gun      **************/
/***************************************************/
void weapon_lightning_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;

        damage = 30;

	if (is_quad)
		damage *= 4;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
                ent->client->ps.gunframe = 18;
                ent->client->machinegun_shots = 0;
                return;
        }

        if ((ent->client->ammo_index)&&
           (ent->client->pers.inventory[ent->client->ammo_index] < 1))
	{
                ent->client->ps.gunframe = 18;
                ent->client->machinegun_shots = 0;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

        if (gi.pointcontents (start) & MASK_WATER)
        {
                int cells;

                if (ent->client->ammo_index)
                {
                        cells = ent->client->pers.inventory[ent->client->ammo_index];
                        ent->client->pers.inventory[ent->client->ammo_index] = 0;
                }
                else
                        cells = 100;

                T_WaterRadiusDamage (ent, ent, 35*cells, NULL, 500, MOD_DISCHARGE);
        }
        else
        {
                fire_lightning (ent, start, forward, damage);
                if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
                   &&(ent->client->ammo_index))
                        ent->client->pers.inventory[ent->client->ammo_index]--;
        }

        if (!ent->client->machinegun_shots)
        {
                if (!ent->client->silencer_shots)
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("light/lstart.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound(ent, CHAN_AUTO, gi.soundindex("light/lstart.wav"), 0.2, ATTN_NORM, 0);
                ent->client->machinegun_shots = 1;
        }

        if (ent->client->machinegun_shots == 1)
        {
                if (!ent->client->silencer_shots)
                        gi.sound(ent, CHAN_VOICE, gi.soundindex("light/lhit.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound(ent, CHAN_VOICE, gi.soundindex("light/lhit.wav"), 0.2, ATTN_NORM, 0);
                ent->client->machinegun_shots = 2;
        }
        else if (ent->client->machinegun_shots < 7)
                ent->client->machinegun_shots++;
        else
                ent->client->machinegun_shots = 1;

//        ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

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


void Weapon_LightningGun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

        Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_lightning_fire);
}

/*******************************
 ******Super Nailgun************
 *******************************/
void SuperNailgun_Fire (edict_t *ent)
{
	int			i;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

                damage = 18;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&&
                ((ent->client->pers.inventory[ent->client->ammo_index] > 1)||
                (!ent->client->ammo_index))
                )
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

        if (ent->client->ammo_index &&
               (ent->client->pers.inventory[ent->client->ammo_index] < 2))
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

		// get start / end positions
        AngleVectors (ent->client->v_angle, forward, right, up);
        r = 7 + crandom()*4;
        u = crandom()*4 - 1;
        VectorSet(offset, 0, r, u + ent->viewheight-8);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        fire_nail (ent, start, forward, damage, 1000, MOD_SUPERNAIL);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
        gi.WriteByte ((MZ_CHAINGUN1 ) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}


void Weapon_SuperNailgun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

        Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, SuperNailgun_Fire);
}

void Nailgun_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
        int                     damage = 18;
	int			kick = 2;
	vec3_t		offset;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
        ent->client->kick_angles[0] = 0;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        fire_nail (ent, start, forward, damage, 1000, MOD_NAIL);

        gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                ent->client->pers.inventory[ent->client->ammo_index] -= 2;

        ent->client->ps.gunframe++;
}

void Weapon_Nailgun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
        static int      fire_frames[]   = {4, 0};

        Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Nailgun_Fire);
}

void Weapon_Bucky_Fire (edict_t *ent)
{
	int		damage;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
        vec3_t          offset;

        damage = 110;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        fire_buckyball (ent, start, forward, damage, 1000, damage + 40);

        if (!ent->client->silencer_shots)
                gi.sound(ent, CHAN_VOICE, gi.soundindex("bucky.wav"), 1, ATTN_NORM, 0);
        else
                gi.sound(ent, CHAN_VOICE, gi.soundindex("bucky.wav"), 0.2, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
                                ent->client->pers.inventory[ent->client->ammo_index]-=3;
	ent->client->ps.gunframe++;
}

void Weapon_Bucky (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

        Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Bucky_Fire);
}
/*
======================================================================

NUKE

======================================================================
*/

void weapon_nuke_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        fire_nuke (ent, start, forward, damage, 800, damage_radius, damage);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
                ent->client->pers.inventory[ent->client->ammo_index] -= 3;
                ent->client->pers.inventory[ent->client->ammo2_index] -= 3;
        }
}

void Weapon_Nuke (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

        Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_nuke_fire);
}


/**********************************************/
/**************  Homing Rockets  **************/
/**********************************************/
void Weapon_Homing_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        fire_homing (ent, start, forward, damage, 650, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
                ent->client->pers.inventory[ent->client->ammo_index]-=1;
                ent->client->pers.inventory[ent->client->ammo2_index]-=3;
        }
}

void Weapon_Homing (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

        Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_Homing_Fire);
}

/**************************************************/
/**************  Perforator Rockets  **************/
/**************************************************/
void Weapon_Perforator_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
        radius_damage = 80;
        damage_radius = 80;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        fire_perforator (ent, start, forward, damage, 650, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

        if ((! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
           &&(ent->client->ammo_index))
        {
                ent->client->pers.inventory[ent->client->ammo_index]--;
                ent->client->pers.inventory[ent->client->ammo2_index]--;
        }
}

void Weapon_Perforator (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

        Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_Perforator_Fire);
}


