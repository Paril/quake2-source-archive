// g_weapon.c

#include "g_local.h"
#include "m_player.h"


qboolean	is_quad;
static byte		is_silenced;

void fire_concussiongrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void weapon_grenade_fire (edict_t *ent, qboolean held);


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

	if (CVAR_DEATHMATCH)
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
	else
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
	int			i;
	char *inf_illegal[6]=
	{
		{"weapon_machinegun"},
    	{"weapon_chaingun"},
 		{"weapon_grenadelauncher"},
    	{"weapon_hyperblaster"},
    	{"weapon_railgun"},
		{"weapon_bfg"}
	};
	char *recon_illegal[8]=
	{
		{"weapon_supershotgun"},
		{"weapon_machinegun"},
    	{"weapon_chaingun"},
 		{"weapon_grenadelauncher"},
		{"weapon_rocketlauncher"},
    	{"weapon_hyperblaster"},
    	{"weapon_railgun"},
		{"weapon_bfg"}
	};
	char *lassault_illegal[6]=
	{
		{"weapon_supershotgun"},
    	{"weapon_chaingun"},
 		{"weapon_grenadelauncher"},
		{"weapon_rocketlauncher"},
    	{"weapon_railgun"},
		{"weapon_bfg"}
	};
	char *hdefense_illegal[6]=
	{
		{"weapon_shotgun"},
		{"weapon_machinegun"},
 		{"weapon_grenadelauncher"},
    	{"weapon_hyperblaster"},
    	{"weapon_railgun"},
		{"weapon_bfg"}
	};
	char *seal_illegal[7]=
	{
		{"weapon_supershotgun"},
    	{"weapon_chaingun"},
 		{"weapon_grenadelauncher"},
		{"weapon_rocketlauncher"},
    	{"weapon_hyperblaster"},
    	{"weapon_railgun"},
		{"weapon_bfg"}
	};
	char *engineer_illegal[6]=
	{
    	{"weapon_chaingun"},
 		{"weapon_grenadelauncher"},
		{"weapon_rocketlauncher"},
    	{"weapon_hyperblaster"},
    	{"weapon_railgun"},
		{"weapon_bfg"}
	};
	char *sniper_illegal[7]=
	{
		{"weapon_shotgun"},
		{"weapon_supershotgun"},
    	{"weapon_chaingun"},
 		{"weapon_grenadelauncher"},
		{"weapon_rocketlauncher"},
    	{"weapon_hyperblaster"},
		{"weapon_bfg"}
	};
	char *demo_illegal[6]=
	{
		{"weapon_shotgun"},
		{"weapon_supershotgun"},
    	{"weapon_chaingun"},
    	{"weapon_hyperblaster"},
    	{"weapon_railgun"},
		{"weapon_bfg"}
	};
	char *medic_illegal[7]=
	{
		{"weapon_supershotgun"},
    	{"weapon_chaingun"},
 		{"weapon_grenadelauncher"},
		{"weapon_rocketlauncher"},
    	{"weapon_hyperblaster"},
    	{"weapon_railgun"},
		{"weapon_bfg"}
	};

	index = ITEM_INDEX(ent->item);

	if ( (ENABLE_WEAPONS_STAY) && other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	if (other->client->resp.class == INFANTRY)
	{
		for (i = 0 ; i < 7 ; i++)
		{
			if (!inf_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, inf_illegal[i]) == 0)
				return false;
		}
	}

	if (other->client->resp.class == RECON)
	{
		for (i = 0 ; i < 9 ; i++)
		{
			if (!recon_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, recon_illegal[i]) == 0)
				return false;
		}
	}

	if (other->client->resp.class == LIGHT_ASSAULT)
	{
		for (i = 0 ; i < 7 ; i++)
		{
			if (!lassault_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, lassault_illegal[i]) == 0)
				return false;
		}
	}

	if (other->client->resp.class == HEAVY_DEFENSE)
	{
		for (i = 0 ; i < 7 ; i++)
		{
			if (!hdefense_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, hdefense_illegal[i]) == 0)
				return false;
		}
	}

	if (other->client->resp.class == SEAL)
	{
		for (i = 0 ; i < 8 ; i++)
		{
			if (!seal_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, seal_illegal[i]) == 0)
				return false;
		}
	}

	if (other->client->resp.class == ENGINEER)
	{
		for (i = 0 ; i < 7 ; i++)
		{
			if (!engineer_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, engineer_illegal[i]) == 0)
				return false;
		}
	}

	if (other->client->resp.class == SNIPER)
	{
		for (i = 0 ; i < 8 ; i++)
		{
			if (!sniper_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, sniper_illegal[i]) == 0)
				return false;
		}
	}

	if (other->client->resp.class == DEMOLITIONS)
	{
		for (i = 0 ; i < 7 ; i++)
		{
			if (!demo_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, demo_illegal[i]) == 0)
				return false;
		}
	}

	if (other->client->resp.class == MEDIC)
	{
		for (i = 0 ; i < 8 ; i++)
		{
			if (!medic_illegal[i])
				continue;
			if (Q_strcasecmp(ent->classname, medic_illegal[i]) == 0)
				return false;
		}
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		ammo = FindItem (ENTS_ITEM_AMMO);
		if (ALLOW_INFINITE_AMMO)
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (CVAR_DEATHMATCH)
			{
				if (ENABLE_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
		}
	}

	if (other->client->pers.weapon != ent->item &&
		(other->client->pers.inventory[index] == 1) &&
		( !CVAR_DEATHMATCH || other->client->pers.weapon == item_blaster ) )
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

	// reload their clips - i know its cheap, but it prevents a bug
	ent->client->pistol_rds = 12;
	ent->client->flareg_rds = 4;
	ent->client->shotgun_rds = 6;
	ent->client->sshot_rds = 10;
	ent->client->mg_rds = 30;
	ent->client->dualmg_rds = 60;
	ent->client->chaingun_rds = 75;
	ent->client->gl_rds = 6;
	ent->client->rl_rds = 6;
	ent->client->hb_rds = 40;
	ent->client->rail_rds = 3;

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

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;

	if (!ent->client->chasetoggle && !ent->client->rocketview)
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	ent->client->anim_priority = ANIM_PAIN;
	if ENT_CURRENTLY_IS_CROUCHED
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
	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

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

	if (ENABLE_WEAPONS_STAY)
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

    if( ent->client->weaponstate == WEAPON_RELOADING)
    {
        if(ent->client->reload_time < 0 || ent->client->reload_time > 10)
 	       ent->client->reload_time = 10;
        else if(ent->client->reload_time > 0)
        {
			ent->client->ps.gunindex = 0;
        	ent->client->reload_time--;
        }
        else
        {
			ent->client->ps.gunframe = 0;
			ent->client->reload_time = 0;
            ent->client->weaponstate = WEAPON_END_RELOADING;
            if(strcmp(ent->client->pers.weapon->pickup_name, "Pistol") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  12)
	                ent->client->pistol_rds = 12;
                else
                    ent->client->pistol_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Flare Gun") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  4)
	                ent->client->flareg_rds = 4;
                else
                    ent->client->flareg_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Shotgun") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  6)
	                ent->client->shotgun_rds = 6;
                else
                    ent->client->shotgun_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Super Shotgun") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  10)
	                ent->client->sshot_rds = 10;
                else
                    ent->client->sshot_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Machinegun") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  30)
	                ent->client->mg_rds = 30;
                else
                    ent->client->mg_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Dual Machineguns") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  60)
	                ent->client->dualmg_rds = 60;
                else
                    ent->client->dualmg_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Chaingun") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  75)
	                ent->client->chaingun_rds = 75;
                else
                    ent->client->chaingun_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Grenade Launcher") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  6)
	                ent->client->gl_rds = 6;
                else
                    ent->client->gl_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Rocket Launcher") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  6)
	                ent->client->rl_rds = 6;
                else
                    ent->client->rl_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "HyperBlaster") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  40)
	                ent->client->hb_rds = 40;
                else
                    ent->client->hb_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }
            else if(strcmp(ent->client->pers.weapon->pickup_name, "Railgun") == 0)
            {
	            if(ent->client->pers.inventory[ent->client->ammo_index] >=  3)
	                ent->client->rail_rds = 3;
                else
                    ent->client->rail_rds = ent->client->pers.inventory[ent->client->ammo_index];
            }

			if (!ent->client->chasetoggle && !ent->client->rocketview)
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		}
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
			if ENT_CURRENTLY_IS_CROUCHED
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

	if (ent->client->weaponstate == WEAPON_START_RELOADING)
	{
		if (ent->client->ps.gunframe < FRAME_DEACTIVATE_FIRST)
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		else if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_RELOADING;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING || ent->client->weaponstate == WEAPON_END_RELOADING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
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
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if ENT_CURRENTLY_IS_CROUCHED
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
			if ((!ent->client->ammo_index) ||
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if ENT_CURRENTLY_IS_CROUCHED
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
					gi.sound(ent, CHAN_VOICE, NO_AMMO_SOUND, 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
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
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, DAMAGE3_SOUND, 1, ATTN_NORM, 0);
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}

    if( ent->client->weaponstate == WEAPON_END_MAG)
    {
		if (ent->client->ps.gunframe < FRAME_IDLE_FIRST || ent->client->ps.gunframe > FRAME_IDLE_LAST)
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
		else if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
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

	if (ent->client)
	{
		if (ent->client->concussion_toggle)
		{
			fire_concussiongrenade (ent, start, forward, damage, speed, timer, radius);
		}
		else
			fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);
	}
	else
		fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (!ALLOW_INFINITE_AMMO)
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if ENT_CURRENTLY_IS_CROUCHED
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
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, NO_AMMO_SOUND, 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
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
			gi.sound(ent, CHAN_WEAPON, HANDGRENADE1B_SOUND, 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = HANDGRENADE1C_SOUND;
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

    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->gl_rds == 1))
    {
        ent->client->weaponstate = WEAPON_END_MAG;
        fire_grenade (ent, start, forward, damage, 600, 2.5, radius);
        ent->client->gl_rds--;
    }
    else
    {
        //If no reload, fire normally.
        fire_grenade (ent, start, forward, damage, 600, 2.5, radius);

        ent->client->gl_rds--;
    }

	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_GRENADE|is_silenced);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!ALLOW_INFINITE_AMMO)
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

    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->rl_rds == 1))
    {
        ent->client->weaponstate = WEAPON_END_MAG;
        fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
        ent->client->rl_rds--;
    }
    else
    {
        //If no reload, fire normally.
        fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);

        ent->client->rl_rds--;
    }

	// send muzzle flash
	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_ROCKET|is_silenced);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!ALLOW_INFINITE_AMMO)
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
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

    if ((ent->client->pers.inventory[ent->client->ammo_index] == 1) || (ent->client->hb_rds == 1))
    {
        ent->client->weaponstate = WEAPON_END_MAG;
		gi.sound(ent, CHAN_AUTO, HYPERBLASTER2_SOUND, 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
        fire_blaster (ent, start, forward, damage, 1000, effect, hyper);
        ent->client->hb_rds--;
    }
    else
    {
        //If no reload, fire normally.
        fire_blaster (ent, start, forward, damage, 1000, effect, hyper);

        ent->client->hb_rds--;
    }

	// send muzzle flash
	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_HYPERBLASTER);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void pistol_fire (edict_t *ent)
{
	vec3_t start;
	vec3_t forward, right;
	vec3_t offset;
	int damage;
	int kick = 20;
	float run = IS_RUNNING;

	if (deathmatch->value)
		damage = 15;
	else
		damage = 10;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->pistol_rds == 1))
    {
        ent->client->weaponstate = WEAPON_END_MAG;
		if (CVAR_DEATHMATCH)
		{
			if ENT_CURRENTLY_IS_CROUCHED
				fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_BLASTER);
			else if (run)
				fire_bullet (ent, start, forward, damage, kick, 10, 10, MOD_BLASTER);
			else
				fire_bullet (ent, start, forward, damage, kick, 5, 5, MOD_BLASTER);
		}
		else
		{
			if ENT_CURRENTLY_IS_CROUCHED
				fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_BLASTER);
			else if (run)
				fire_bullet (ent, start, forward, damage, kick, 15, 15, MOD_BLASTER);
			else
				fire_bullet (ent, start, forward, damage, kick, 10, 10, MOD_BLASTER);
		}
        ent->client->pistol_rds--;
    }
    else
    {
        //If no reload, fire normally.
		if (CVAR_DEATHMATCH)
		{
			if ENT_CURRENTLY_IS_CROUCHED
				fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_BLASTER);
			else if (run)
				fire_bullet (ent, start, forward, damage, kick, 10, 10, MOD_BLASTER);
			else
				fire_bullet (ent, start, forward, damage, kick, 5, 5, MOD_BLASTER);
		}
		else
		{
			if ENT_CURRENTLY_IS_CROUCHED
				fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_BLASTER);
			else if (run)
				fire_bullet (ent, start, forward, damage, kick, 15, 15, MOD_BLASTER);
			else
				fire_bullet (ent, start, forward, damage, kick, 10, 10, MOD_BLASTER);
		}

        ent->client->pistol_rds--;
    }

	// send muzzle flash
	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_MACHINEGUN|is_silenced);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!ALLOW_INFINITE_AMMO)
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Pistol_Fire (edict_t *ent)
{
	pistol_fire (ent);
}

void Weapon_Pistol (edict_t *ent)
{
	static int	pause_frames[]	= {16, 26, 0};
	static int	fire_frames[]	= {3, 0};

	Weapon_Generic (ent, 2, 6, 30, 33, pause_frames, fire_frames, Weapon_Pistol_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = HYPERBLASTER1_SOUND;

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
				gi.sound(ent, CHAN_VOICE, NO_AMMO_SOUND, 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
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
			if (CVAR_DEATHMATCH)
				damage = 15;
			else
				damage = 20;
			Blaster_Fire (ent, offset, damage, true, effect);
			if (!ALLOW_INFINITE_AMMO)
				ent->client->pers.inventory[ent->client->ammo_index]--;

			ent->client->anim_priority = ANIM_ATTACK;
			if ENT_CURRENTLY_IS_CROUCHED
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
		gi.sound(ent, CHAN_AUTO, HYPERBLASTER2_SOUND, 1, ATTN_NORM, 0);
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
	float		run = IS_RUNNING;


    if (!(ent->client->buttons & BUTTON_ATTACK) &&
    ( (ent->client->burstfire_count > 2) ||
       (!ent->client->burstfire_count ) ) )
    {
         ent->client->machinegun_shots=0;
         ent->client->burstfire_count=0;
         ent->client->ps.gunframe++;
         return;
    }

    if (ent->client->burstfire_count < 3)
    {
		if (ent->client->ps.gunframe == 5)
			ent->client->ps.gunframe = 4;
		else
			ent->client->ps.gunframe = 5;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, NO_AMMO_SOUND, 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		ent->client->burstfire_count=0;
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

	// raise the gun as it is firing
	if (!CVAR_DEATHMATCH && !ent->client->pers.fire_mode)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	switch (ent->client->pers.fire_mode)
    {
	    // Fire Burst Fire
        case 1:
	        ent->client->burstfire_count++;
            if (ent->client->burstfire_count < 4)
            {
			    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->mg_rds == 1))
			    {
			        ent->client->weaponstate = WEAPON_END_MAG;
					fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_MACHINEGUN);
			        ent->client->mg_rds--;
			    }
			    else
			    {
			        //If no reload, fire normally.
					fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_MACHINEGUN);

			        ent->client->mg_rds--;
			    }
				G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_MACHINEGUN|is_silenced);

				PlayerNoise(ent, start, PNOISE_WEAPON);

				if (!ALLOW_INFINITE_AMMO)
					ent->client->pers.inventory[ent->client->ammo_index]--;

				ent->client->anim_priority = ANIM_ATTACK;
				if ENT_CURRENTLY_IS_CROUCHED
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
            else if (ent->client->burstfire_count > 6)
			{
                ent->client->burstfire_count=0;
			}
            break;
		// Fire Fully Automatic
        case 0:
        default:
		    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->mg_rds == 1))
		    {
		        ent->client->weaponstate = WEAPON_END_MAG;
				if ENT_CURRENTLY_IS_CROUCHED
					fire_bullet (ent, start, forward, damage, kick, 150, 250, MOD_MACHINEGUN);
				else if (run)
					fire_bullet (ent, start, forward, damage, kick, 400, 600, MOD_MACHINEGUN);
				else
					fire_bullet (ent, start, forward, damage, kick, 300, 500, MOD_MACHINEGUN);
		        ent->client->mg_rds--;
		    }
		    else
		    {
		        //If no reload, fire normally.
				if ENT_CURRENTLY_IS_CROUCHED
					fire_bullet (ent, start, forward, damage, kick, 150, 250, MOD_MACHINEGUN);
				else if (run)
					fire_bullet (ent, start, forward, damage, kick, 400, 600, MOD_MACHINEGUN);
				else
					fire_bullet (ent, start, forward, damage, kick, 300, 500, MOD_MACHINEGUN);

		        ent->client->mg_rds--;
		    }
			G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_MACHINEGUN|is_silenced);

			PlayerNoise(ent, start, PNOISE_WEAPON);

			if (!ALLOW_INFINITE_AMMO)
				ent->client->pers.inventory[ent->client->ammo_index]--;

			ent->client->anim_priority = ANIM_ATTACK;
			if ENT_CURRENTLY_IS_CROUCHED
			{
				ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
				ent->client->anim_end = FRAME_attack8;
			}
            break;
	}
}

void DualMachinegun_Fire_Bullet (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;
	float		run = IS_RUNNING;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 6)
		ent->client->ps.gunframe = 5;
	else
		ent->client->ps.gunframe = 6;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, NO_AMMO_SOUND, 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
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

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->dualmg_rds == 1))
    {
        ent->client->weaponstate = WEAPON_END_MAG;
		if ENT_CURRENTLY_IS_CROUCHED
			fire_bullet (ent, start, forward, damage, kick, 150, 250, MOD_MACHINEGUN);
		else if (run)
			fire_bullet (ent, start, forward, damage, kick, 400, 600, MOD_MACHINEGUN);
		else
			fire_bullet (ent, start, forward, damage, kick, 300, 500, MOD_MACHINEGUN);
        ent->client->dualmg_rds--;
    }
    else
    {
        //If no reload, fire normally.
		if ENT_CURRENTLY_IS_CROUCHED
			fire_bullet (ent, start, forward, damage, kick, 150, 250, MOD_MACHINEGUN);
		else if (run)
			fire_bullet (ent, start, forward, damage, kick, 400, 600, MOD_MACHINEGUN);
		else
			fire_bullet (ent, start, forward, damage, kick, 300, 500, MOD_MACHINEGUN);

        ent->client->dualmg_rds--;
    }

	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_MACHINEGUN|is_silenced);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!ALLOW_INFINITE_AMMO)
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if ENT_CURRENTLY_IS_CROUCHED
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

void DualMachinegun_Fire (edict_t *ent)
{
	DualMachinegun_Fire_Bullet(ent);
	DualMachinegun_Fire_Bullet(ent);
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Weapon_DualMachinegun (edict_t *ent)
{
	static int	pause_frames[]	= {16, 0};
	static int	fire_frames[]	= {5, 6, 0};

	Weapon_Generic (ent, 4, 6, 16, 19, pause_frames, fire_frames, DualMachinegun_Fire);
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
	float		run = IS_RUNNING;

	if (CVAR_DEATHMATCH)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, CHAINGUN1_SOUND, 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
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
		gi.sound(ent, CHAN_AUTO, CHAINGUN2_SOUND, 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = CHAINGUN3_SOUND;
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if ENT_CURRENTLY_IS_CROUCHED
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

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, NO_AMMO_SOUND, 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
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

	    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->chaingun_rds == 1))
	    {
	        ent->client->weaponstate = WEAPON_END_MAG;
			ent->client->weapon_sound = 0;
			gi.sound(ent, CHAN_AUTO, CHAINGUN2_SOUND, 1, ATTN_IDLE, 0);
			if ENT_CURRENTLY_IS_CROUCHED
				fire_bullet (ent, start, forward, damage, kick, 150, 250, MOD_CHAINGUN);
			else if (run)
				fire_bullet (ent, start, forward, damage, kick, 400, 600, MOD_CHAINGUN);
			else
				fire_bullet (ent, start, forward, damage, kick, 300, 500, MOD_CHAINGUN);
	        ent->client->chaingun_rds--;
	    }
	    else
	    {
	        //If no reload, fire normally.
			if ENT_CURRENTLY_IS_CROUCHED
				fire_bullet (ent, start, forward, damage, kick, 150, 250, MOD_CHAINGUN);
			else if (run)
				fire_bullet (ent, start, forward, damage, kick, 400, 600, MOD_CHAINGUN);
			else
				fire_bullet (ent, start, forward, damage, kick, 300, 500, MOD_CHAINGUN);

	        ent->client->chaingun_rds--;
	    }
	}

	// send muzzle flash
	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, (MZ_CHAINGUN1+shots-1)|is_silenced);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!ALLOW_INFINITE_AMMO)
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
	int			damage = 4;
	int			kick = 8;
	float		run = IS_RUNNING;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->shotgun_rds == 1))
    {
        ent->client->weaponstate = WEAPON_END_MAG;
		if (CVAR_DEATHMATCH)
		{
			if ENT_CURRENTLY_IS_CROUCHED
				fire_shotgun (ent, start, forward, damage, kick, 250, 250, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
			else if (run)
				fire_shotgun (ent, start, forward, damage, kick, 750, 750, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
			else
				fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
		}
		else
		{
			if ENT_CURRENTLY_IS_CROUCHED
				fire_shotgun (ent, start, forward, damage, kick, 250, 250, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
			else if (run)
				fire_shotgun (ent, start, forward, damage, kick, 750, 750, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
			else
				fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
		}
	    ent->client->shotgun_rds--;
    }
    else
    {
        //If no reload, fire normally.
		if (CVAR_DEATHMATCH)
		{
			if ENT_CURRENTLY_IS_CROUCHED
				fire_shotgun (ent, start, forward, damage, kick, 250, 250, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
			else if (run)
				fire_shotgun (ent, start, forward, damage, kick, 750, 750, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
			else
				fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
		}
		else
		{
			if ENT_CURRENTLY_IS_CROUCHED
				fire_shotgun (ent, start, forward, damage, kick, 250, 250, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
			else if (run)
				fire_shotgun (ent, start, forward, damage, kick, 750, 750, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
			else
				fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
		}

        ent->client->shotgun_rds--;
    }

	// send muzzle flash
	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_SHOTGUN|is_silenced);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!ALLOW_INFINITE_AMMO)
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;
	float		run = IS_RUNNING;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}


    if (ent->client->pers.inventory[ent->client->ammo_index] <= 2 || (ent->client->sshot_rds <= 2))
    {
        ent->client->weaponstate = WEAPON_END_MAG;

		if ENT_CURRENTLY_IS_CROUCHED
		{
			v[PITCH] = ent->client->v_angle[PITCH];
			v[YAW]   = ent->client->v_angle[YAW] - 2.5; // tighten up the bursts
			v[ROLL]  = ent->client->v_angle[ROLL];
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 500, 250, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
			v[YAW]   = ent->client->v_angle[YAW] + 2.5; // same
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 500, 250, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
		}
		else if (run) // HAHA! you cant hit SHIT while running... hehe
		{
			v[PITCH] = ent->client->v_angle[PITCH];
			v[YAW]   = ent->client->v_angle[YAW] - 5;
			v[ROLL]  = ent->client->v_angle[ROLL];
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 1400, 750, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
			v[YAW]   = ent->client->v_angle[YAW] + 5;
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 1400, 750, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
		}
		else
		{
			v[PITCH] = ent->client->v_angle[PITCH];
			v[YAW]   = ent->client->v_angle[YAW] - 5;
			v[ROLL]  = ent->client->v_angle[ROLL];
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 1000, 500, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
			v[YAW]   = ent->client->v_angle[YAW] + 5;
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 1000, 500, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
		}

	    ent->client->sshot_rds -= 2;
    }
    else
    {
        //If no reload, fire normally.
		if ENT_CURRENTLY_IS_CROUCHED
		{
			v[PITCH] = ent->client->v_angle[PITCH];
			v[YAW]   = ent->client->v_angle[YAW] - 2.5; // tighten up the bursts
			v[ROLL]  = ent->client->v_angle[ROLL];
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 500, 250, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
			v[YAW]   = ent->client->v_angle[YAW] + 2.5; // same
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 500, 250, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
		}
		else if (run) // HAHA! you cant hit SHIT while running... hehe
		{
			v[PITCH] = ent->client->v_angle[PITCH];
			v[YAW]   = ent->client->v_angle[YAW] - 5;
			v[ROLL]  = ent->client->v_angle[ROLL];
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 1400, 750, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
			v[YAW]   = ent->client->v_angle[YAW] + 5;
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 1400, 750, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
		}
		else
		{
			v[PITCH] = ent->client->v_angle[PITCH];
			v[YAW]   = ent->client->v_angle[YAW] - 5;
			v[ROLL]  = ent->client->v_angle[ROLL];
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 1000, 500, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
			v[YAW]   = ent->client->v_angle[YAW] + 5;
			AngleVectors (v, forward, NULL, NULL);
			fire_shotgun (ent, start, forward, damage, kick, 1000, 500, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
		}

        ent->client->sshot_rds -= 2;
    }

	// send muzzle flash
	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_SSHOTGUN|is_silenced);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!ALLOW_INFINITE_AMMO)
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

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

	if (CVAR_DEATHMATCH)
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

    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->rail_rds == 1))
    {
        ent->client->weaponstate = WEAPON_END_MAG;
		fire_rail (ent, start, forward, damage, kick);
	    ent->client->rail_rds--;
    }
    else
    {
		fire_rail (ent, start, forward, damage, kick);
        ent->client->rail_rds--;
    }

	// send muzzle flash
	G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_RAILGUN|is_silenced);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!ALLOW_INFINITE_AMMO)
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

	if (CVAR_DEATHMATCH)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, MZ_BFG|is_silenced);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
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

	if (!ALLOW_INFINITE_AMMO)
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}


//======================================================================

/*
 * Forward declaration for fire_flaregun(), which is defined in
 * g_weapon.c.
 */
void fire_flaregun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);

/*
 * weapon_flaregun_fire (edict_t *ent)
 *
 * Basically used to wrap the call to fire_flaregun(), this function
 * calculates all the parameters needed by fire_flaregun.  Calls
 * fire_flaregun and then subtracts 1 from the firing entity's
 * cell stash.
 */
void weapon_flaregun_fire(edict_t *ent)
{
	vec3_t offset;
    vec3_t forward, right;
    vec3_t start;

    // Setup the parameters used in the call to fire_flaregun()
    VectorSet(offset, 8, 8, ent->viewheight-8);
    AngleVectors (ent->client->v_angle, forward, right, NULL);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("medic/medatck1.wav"), 1, ATTN_NORM, 0);

    if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->flareg_rds == 1))
    {
        ent->client->weaponstate = WEAPON_END_MAG;
    	fire_flaregun (ent, start, forward, 0, 800, 25, 0);
	    ent->client->flareg_rds--;
    }
    else
    {
		fire_flaregun (ent, start, forward, 0, 800, 25, 0);
        ent->client->flareg_rds--;
    }

    // Bump the gunframe
    ent->client->ps.gunframe++;

    PlayerNoise(ent, start, PNOISE_WEAPON);

    // Subtract one cell from our inventory
    ent->client->pers.inventory[ent->client->ammo_index]--;
}

/*
 * Weapon_FlareGun (edict_t *ent)
 */
void Weapon_FlareGun (edict_t *ent)
{
	static int pause_frames[] = {39, 45, 50, 53, 0};
    static int fire_frames[] = {9, 17, 0};

    // Check the top of p_weapon.c for definition of Weapon_Generic
    //
    Weapon_Generic (ent, 8, 13, 49, 53, pause_frames, fire_frames, weapon_flaregun_fire);
}
