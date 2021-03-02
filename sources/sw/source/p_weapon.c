
// g_weapon.c

#include "g_local.h"
#include "m_player.h"

void weapon_wrocket_fire (edict_t *ent);


static byte		is_silenced;

/*========CaRRaC -- Light Saber blade tip points START*/
vec3_t chop[] =		//5 frames FRAME_chop
{
	-29.5,	-1,		18,
	-10.5,	-2,		42,
	34,		-1.5,	28.5,
	28,		0,		-13.5,
	31,		-1.5,	12,
};

vec3_t thrust[] =		//6 frames FRAME_thrust
{
	31.5,	-2,		18,
	42,		-0.5,	14.5,
	42,		-0.5,	14.5,
	42,		-0.5,	14.5,
	35.5,	-0.5,	34.5,
	24.5,	-2,		28,
};

vec3_t upprcutlft[] =		//7 frames FRAME_upprcutlft
{
	24.5,	-21.5,	9.5,
	18,		-20,	-14,
	36,		-7.5,	2.5,
	33,		15,		23,
	-18,	15.5,	30.5,
	-1.5,	13.5,	35.5,
	11,		5,		33,
};

vec3_t upprcutrt[] =		//7 frames FRAME_upprcutrt
{
	13.5,	22,		18,
	-5,		25,		-6.5,
	8,		21.5,	-6,
	5,		12.5,	36,
	-20.5,	-11,	30.5,
	-10.5,	-12,	35,
	-0.5,	-12,	34,
};

vec3_t swinglft[] =		//7 frames FRAME_swinglft
{
	-8,		-18.5,	29.5,
	-25,	-12,	22.5,
	35.5,	-15,	17,
	18.5,	30,		20,
	-13,	31.5,	18,
	3,		25,		29,
	10,		7.5,	33,
};

vec3_t swingrt[] =		//7 frames FRAME_swingrt
{
	-2,		10,		32.5,
	-21,	7.5,	21.5,
	31,		13,		19,
	30.5,	-21.5,	19.5,
	-8.5,	-36,	18,
	10,		-23.5,	28.5,
	12.5,	-13.5,	30,
};

vec3_t sliceleft[] =		//7 frames FRAME_slicelft
{
	-9,		-7.5,	32.5,
	-21,	-4,		18.5,
	32,		-16.5,	23.5,
	37,		-6,		-6.5,
	21.5,	16.5,	-19,
	32,		15.5,	2.5,
	27,		5,		21,
};

vec3_t sliceright[] =		//7 frames FRAME_slicert
{
	-0.5,	3.5,	36,
	-19,	3.5,	25.5,
	2.5,	11.5,	35,
	37,		3.5,	18,
	25.5,	-15,	-20.5,
	32.5,	-13,	-2,
	24.5,	-8,		21.5,
};

//RipVTide
vec3_t crawlslrt[] =		//5 frames FRAME_slicert
{
	5.5,	-3.5,	22,
	-18,	-2,		4.5,
	3.5,	-8,		21,
	34.5,	-5,		-18,
	37,		-8.5,	5.5,
};

vec3_t crawlsllf[] =		//5 frames FRAME_slicert
{
	10,		8,		21,
	-16.5,	4.5,	10,
	-2,		12,		16,
	32.5,	10,		5.5,
	36.5,	3.5,	-5.5,
};

vec3_t runsla[] =
{
	5,		-4,		37,
	-17,	-1,		22,
	6,		-8,		38.5,
	34.5,	1,		-4.4,
	38,		-2.5,	14,
};

vec3_t runslb[] =
{
	8,		6.5,	36.5,
	-16,	1.5,	17.5,
	-2.5,	11,		34.5,
	31.5,	15.5,	19,
	36,		9,		10,
};

vec3_t b_upleft[] =		//7 frames FRAME_b_upleft
{
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
};

vec3_t b_upright[] =		//7 frames FRAME_b_upright
{
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
};

vec3_t b_left[] =		//7 frames FRAME_b_left
{
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
};

vec3_t b_right[] =		//7 frames FRAME_b_right
{
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
};

vec3_t b_lowleft[] =		//7 frames FRAME_b_lowleft
{
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
};

vec3_t b_lowright[] =		//7 frames FRAME_b_lowright
{
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
};

/*========CaRRaC -- Light Saber blade tip points END*/

void Reload_NULL (edict_t *ent)
{
	return;
}

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

	if ((who->flags & FL_NOTARGET) || (Force_constant_active (who, LFORCE_INVISIBILITY) != 255 && who->client->invisi_time < level.time))
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

//	if (!(ent->spawnflags & DROPPED_ITEM) )
//	{
		// give them some ammo with it
		if (strcmp(ent->item->classname, "weapon_saber") != 0)
		{
			ammo = FindItem (ent->item->ammo);
			if ( (int)dmflags->value & DF_INFINITE_AMMO )
			{
				Add_Ammo (other, ammo, 1000);
			}
			else
			{
				Add_Ammo (other, ammo, ammo->quantity);
			}
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
//	}

	if ((other->client->pers.weapon != ent->item) && (other->client->pers.inventory[index] == 1) && (p_weap->value) && !(other->client->pers.weapon == FindItem("Lightsaber")))
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
	gitem_t *weapon;

	weapon = FindItem ("Lightsaber");

	ent->client->in_snipe = 0;
//	ent->client->zoom_factor = 90;
	ent->client->ps.fov = 90;
//	set_fov (ent);

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_thermal_fire (ent, false, 0);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == 255)
	{
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
	if (!ent->client->chasetoggle && !ent->client->in_snipe)
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);


//CaRRaC START reload anims, not hacked pain anims
//NOTE! get priv to add crouch reload frames.
	ent->client->anim_priority = ANIM_PAIN;
	if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
			ent->s.frame = FRAME_crpainA;
			ent->client->anim_end = FRAME_crpainB;
	}
	else
	{
//			ent->s.frame = FRAME_paingunA;
//			ent->client->anim_end = FRAME_paingunB;
		if(ent->holstered == 2)
		{
			ent->s.frame = FRAME_cWEPA;
			ent->client->anim_end = FRAME_cWEPB;
		}
		else
		{
			ent->s.frame = FRAME_unholsterA;
			ent->client->anim_end = FRAME_unholsterB;
		}
	}
//RipVTide
//Note - Code added here for saber skins
	if(saberonly->value)
	{
		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/sabre/on.wav"), 1, ATTN_NORM, 0);
		ChasecamStart (ent);
	}
	else if(ent->client->pers.weapon == weapon)
	{
		if (ent->client->chasetoggle == 1)
		{
			ent->client->waschase = true;
		}
		else
		{
			ent->client->waschase = false;
			ChasecamStart (ent);
		}
		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/sabre/on.wav"), 1, ATTN_NORM, 0);
		ent->client->wassaber = true;
		if(!ent->client->saber_c)
		{
			ent->s.modelindex2 = gi.modelindex ("players/male/w_sblue.md2");
		}
		else
		{
			if(ent->client->saber_c == 2)
			{
				ent->s.modelindex2 = gi.modelindex ("players/male/w_sgren.md2");
			}
			else if(ent->client->saber_c == 1)
			{
				ent->s.modelindex2 = gi.modelindex ("players/male/w_sred.md2");
			}
			else if(ent->client->saber_c == 3)
			{
				ent->s.modelindex2 = gi.modelindex ("players/male/w_syell.md2");
			}
			else
			{
				ent->s.modelindex2 = gi.modelindex ("players/male/w_sblue.md2");
			}
		}
	}
	else
	{
		if (ent->client->waschase == true)
		{
			ent->client->waschase = false;
		}
		else if (ent->client->chasetoggle == 1 && ent->client->wassaber)
		{
			ChasecamRemove (ent, "off");
			ent->client->waschase = false;
		}
		else
		{
			ent->client->waschase = false;
		}
		ent->client->wassaber = false;
	}
//CaRRaC END
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	ent->client->in_snipe = 0;
//	ent->client->zoom_factor = 90;
	ent->client->ps.fov = 90;

	if(ent->client->mouse_s)
		_stuffcmd(ent, "sensitivity \"%f\"\n", ent->client->mouse_s);

	//	set_fov (ent);

	//if(!ent->is_bot)
	//	gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);

	if (ent->client->pers.clipammo[WEAP_BEAMTUBE]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Beam_Tube"))])
	{
		ent->client->newweapon = FindItem ("Beam_Tube");
		return;
	}
	if (ent->client->pers.clipammo[WEAP_NIGHTSTINGER]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Night_Stinger"))] )
	{
		ent->client->newweapon = FindItem ("Night_Stinger");
		return;
	}
	if (ent->client->pers.clipammo[WEAP_DISRUPTOR]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Disruptor"))] )
	{
		ent->client->newweapon = FindItem ("Disruptor");
		return;
	}
	if (ent->client->pers.clipammo[WEAP_REPEATER]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Repeater"))] )
	{
		ent->client->newweapon = FindItem ("Repeater");
		return;
	}
	if ( ent->client->pers.clipammo[WEAP_BOWCASTER]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Bowcaster"))] )
	{
		ent->client->newweapon = FindItem ("Bowcaster");
		return;
	}
	if ( ent->client->pers.clipammo[WEAP_RIFLE]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Trooper_Rifle"))] )
	{
		ent->client->newweapon = FindItem ("Trooper_Rifle");
		return;
	}
	if (ent->client->pers.clipammo[WEAP_PISTOL]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Blaster"))] )
	{
		ent->client->newweapon = FindItem ("Blaster");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Lightsaber"))])
	{
		ent->client->newweapon = FindItem ("Lightsaber");
	}
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent, int secfunc)
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
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent, secfunc);
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

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_index = ITEM_INDEX(item);

		if (!ent->client->pers.clipammo[ammo_index])
		{
			safe_cprintf (ent, PRINT_HIGH, "No ammo for %s.\n", item->pickup_name);
			return;
		}

		if (ent->client->pers.clipammo[ammo_index] < item->quantity)
		{
			safe_cprintf (ent, PRINT_HIGH, "Not enough ammo for %s.\n", item->pickup_name);
			return;
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
		safe_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
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
#define FRAME_RELOAD_FIRST		(FRAME_DEACTIVATE_LAST + 1)

/*
No move key		Center Ready		Crouch					Jump
"+forward"		Chop			Thrust					Jump Forward
"+back"		Jump Back		Lean Back + Center Ready		Jump Back
"+left"		Slice Left		Upper Cut Left			Block Low Left
"+right"		Slice Right		Upper Cut Right			Block Low Right
"+strafe left"	Swing Left		Block Up Left				Jump Left
"+strafe right"	Swing Right		Block Up Right			Jump Right

NEW KEY LAYOUT

No move key			Center Ready		Crouch
"+forward"			Chop			Thrust
"+back"			Block Low		Block High
"+strafe left"		Swing Left		Upper Cut Left
"+strafe right"		Swing Right		Upper Cut Right
"+strafe left +back"	Slice Left		Block Left
"+strafe right +back"	Slice Right		Block Right
*/

void Weapon_Generic_Saber (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;
	int		idle_first;

	if(!(ent->client->vflags & IN_VEHICLE))
	{
		if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
			return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			if(ent->holstered == 2)
			{
				ent->holstered = 1;
				ent->s.modelindex2 = 0;
			}
			else if(ent->holstered == 3)
			{
				ent->holstered = 0;
				ent->s.modelindex2 = 255;
			}
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
//CaRRaC START holster anims, not hack pain anims
			if(ent->holstered == 3)
			{
				ent->s.frame = FRAME_nWEPA;
				ent->client->anim_end = FRAME_nWEPB;
			}
			else
			{
				ent->s.frame = FRAME_holsterA;
				ent->client->anim_end = FRAME_holsterB;
			}
//CaRRaC END
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
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
//CaRRaC START holster anims, not hack pain anims
			if(ent->holstered == 3)
			{
				ent->s.frame = FRAME_nWEPA;
				ent->client->anim_end = FRAME_nWEPB;
			}
			else
			{
				ent->s.frame = FRAME_holsterA;
				ent->client->anim_end = FRAME_holsterB;
			}
//CaRRaC END
		}
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) || ent->client->swing_frame > 0)
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;

//			ent->client->ps.gunframe = FRAME_FIRE_FIRST;
			ent->client->weaponstate = WEAPON_FIRING;

			switch(ent->client->swing_num)
			{
				case SWING_CHOP:
					ent->client->ps.gunframe = SAB_swingltA;
					ent->s.frame = FRAME_chopA;
					ent->client->anim_end = FRAME_chopB;
				break;

				case BLOCK_LOWRIGHT:
					ent->client->ps.gunframe = SAB_blocklltA;
					ent->s.frame = FRAME_b_lowrightA;
					ent->client->anim_end = FRAME_b_lowrightB;
				break;

				case BLOCK_LOWLEFT:
					ent->client->ps.gunframe = SAB_blocklltA;
					ent->s.frame = FRAME_b_lowleftA;
					ent->client->anim_end = FRAME_b_lowleftB;
				break;

				case SWING_LEFT:
					ent->client->ps.gunframe = SAB_swingltA;
					ent->s.frame = FRAME_swinglftA;
					ent->client->anim_end = FRAME_swinglftB;
				break;

				case SWING_RIGHT:
					ent->client->ps.gunframe = SAB_swingrtA;
					ent->s.frame = FRAME_swingrtA;
					ent->client->anim_end = FRAME_swingrtB;
				break;

				case SWING_SLICELEFT:
					ent->client->ps.gunframe = SAB_swingltA;
					ent->s.frame = FRAME_slicelftA;
					ent->client->anim_end = FRAME_slicelftB;
				break;

				case SWING_SLICERIGHT:
					ent->client->ps.gunframe = SAB_slicertA;
					ent->s.frame = FRAME_slicertA;
					ent->client->anim_end = FRAME_slicertB;
				break;

				case SWING_THRUST:
					ent->client->ps.gunframe = SAB_thrustA;
					ent->s.frame = FRAME_thrustA;
					ent->client->anim_end = FRAME_thrustB;
				break;

				case BLOCK_UPRIGHT:
					ent->client->ps.gunframe = SAB_blockultA;
					ent->s.frame = FRAME_b_uprightA;
					ent->client->anim_end = FRAME_b_uprightB;
				break;

				case BLOCK_UPLEFT:
					ent->client->ps.gunframe = SAB_blockultA;
					ent->s.frame = FRAME_b_upleftA;
					ent->client->anim_end = FRAME_b_upleftB;
				break;

				case SWING_UCUTLEFT:
					ent->client->ps.gunframe = SAB_ucutltA;
					ent->s.frame = FRAME_upprcutlftA;
					ent->client->anim_end = FRAME_upprcutlftB;
				break;

				case SWING_UCUTRIGHT:
					ent->client->ps.gunframe = SAB_ucutrtA;
					ent->s.frame = FRAME_upprcutrtA;
					ent->client->anim_end = FRAME_upprcutrtB;
				break;

				case BLOCK_LEFT:
					ent->client->ps.gunframe = SAB_blockltA;
					ent->s.frame = FRAME_b_leftA;
					ent->client->anim_end = FRAME_b_leftB;
				break;

				case BLOCK_RIGHT:
					ent->client->ps.gunframe = SAB_blockrtA;
					ent->s.frame = FRAME_b_rightA;
					ent->client->anim_end = FRAME_b_rightB;
				break;

				case SWING_CROUCH_A:
					ent->client->ps.gunframe = SAB_crouch1A;
					ent->s.frame = FRAME_e_crouch1A;
					ent->client->anim_end = FRAME_e_crouch1B;
				break;

				case SWING_CROUCH_B:
					ent->client->ps.gunframe = SAB_crouch2A;
					ent->s.frame = FRAME_e_crouch2A;
					ent->client->anim_end = FRAME_e_crouch2B;
				break;

				case SWING_RUN_A:
					ent->client->ps.gunframe = SAB_runsl1A;
					ent->s.frame = FRAME_e_runsl1A;
					ent->client->anim_end = FRAME_e_runsl1B;
				break;

				case SWING_RUN_B:
					ent->client->ps.gunframe = SAB_runsl2A;
					ent->s.frame = FRAME_e_runsl2A;
					ent->client->anim_end = FRAME_e_runsl2B;
				break;

				default:
					ent->client->ps.gunframe = SAB_swingltA;
					ent->s.frame = FRAME_chopA;
					ent->client->anim_end = FRAME_chopB;
				break;
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
		//		for (n = 0; fire_frames[n]; n++)
//		{
//			if (ent->client->ps.gunframe == fire_frames[n])
//			{
				fire (ent);
//				break;
//			}
//		}

//		if (!fire_frames[n])
//			ent->client->ps.gunframe++;
		switch(ent->client->swing_num)
		{
			case SWING_CHOP:
				idle_first = SAB_chopB;
			break;

			case BLOCK_LOWRIGHT:
				idle_first = SAB_blocklrtB;
			break;

			case BLOCK_LOWLEFT:
				idle_first = SAB_blocklltB;
			break;

			case SWING_LEFT:
				idle_first = SAB_swingltB;
			break;

			case SWING_RIGHT:
				idle_first = SAB_swingrtB;
			break;

			case SWING_SLICELEFT:
				idle_first = SAB_sliceltB;
			break;

			case SWING_SLICERIGHT:
				idle_first = SAB_slicertB;
			break;

			case SWING_THRUST:
				idle_first = SAB_thrustB;
			break;

			case BLOCK_UPRIGHT:
				idle_first = SAB_blockurtB;
			break;

			case BLOCK_UPLEFT:
				idle_first = SAB_blockultB;
			break;

			case SWING_UCUTLEFT:
				idle_first = SAB_ucutrtB;
			break;

			case SWING_UCUTRIGHT:
				idle_first = SAB_ucutrtB;
			break;

			case BLOCK_LEFT:
				idle_first = SAB_blockltB;
			break;

			case BLOCK_RIGHT:
				idle_first = SAB_blockrtB;
			break;

			case SWING_CROUCH_A:
				idle_first = SAB_crouch1B;
			break;

			case SWING_CROUCH_B:
				idle_first = SAB_crouch2B;
			break;

			default:
				idle_first = SAB_chopB;
			break;
		}

//		if(!ent->is_bot)
//			gi.dprintf("frame %i swingnum %i idle_first %i\n", ent->client->ps.gunframe, ent->client->swing_num, idle_first);


//		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
		if (ent->client->ps.gunframe >= idle_first+1)
		{
			ent->client->swing_frame = 0;
//			if(!ent->is_bot)
//				gi.dprintf("swing ended\n");
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int FRAME_RELOAD_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent), void (*reload)(edict_t *ent))
{
	int		n;
	int		weap;
	char	*weap2;

	if (ent->client->pers.weapon)
		weap2 = ent->client->pers.weapon->classname;
	else
		weap2 = "";

	weap = ITEM_INDEX(ent->client->pers.weapon);

	if(!(ent->client->vflags & IN_VEHICLE))
	{
		if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
			return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST || ent->holstered == 2)
		{
			ChangeWeapon (ent);
			if(ent->holstered == 2)
			{
				ent->s.modelindex2 = 0;
				ent->holstered = 1;
			}
			else if(ent->holstered == 3)
			{
				ent->holstered = 0;
				ent->s.modelindex2 = 255;
			}
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->client->anim_priority = ANIM_REVERSE;
				ent->s.frame = FRAME_crpainA+1;
				ent->client->anim_end = FRAME_crpainB;
			}
			else if(ent->holstered == 3)
			{
				ent->s.frame = FRAME_nWEPA;
				ent->client->anim_end = FRAME_nWEPB;
			}
			else
			{
				ent->s.frame = FRAME_holsterA;
				ent->client->anim_end = FRAME_holsterB;
			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
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

	if (ent->client->newweapon && ent->holstered == 3)
	{
//		ent->s.modelindex2 = 255;
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_LAST;
		return;
	}
	else if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING) && (ent->client->weaponstate != WEAPON_RELOADING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->client->anim_priority = ANIM_REVERSE;
				ent->s.frame = FRAME_crpainA+1;
				ent->client->anim_end = FRAME_crpainB;
			}
			else if(ent->holstered == 3)
			{
				ent->s.frame = FRAME_nWEPA;
				ent->client->anim_end = FRAME_nWEPB;
			}
			else
			{
				ent->s.frame = FRAME_holsterA;
				ent->client->anim_end = FRAME_holsterB;
			}
		}
		return;
	}

	if(ent->holstered == 1)
	{
		ent->s.modelindex2 = 0;
		ent->client->weaponstate = WEAPON_READY;
		return;
	}
	if (ent->client->weaponstate == WEAPON_READY && ((ent->client->menu_time+0.5)<=level.time))
	{
		if (ent->client->reload)
		{
			ent->client->reload--;
			if(ent->client->reload == 11)
			{
//				gi.dprintf("reload\n");
				if (ent->client->pers.inventory[ent->client->ammo_index])
				{
					ent->client->ps.gunframe = FRAME_RELOAD_FIRST;
					ent->client->weaponstate = WEAPON_RELOADING;
	
					// start the animation
					//RipVTide
					ent->client->anim_priority = ANIM_ATTACK;
					if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
					{
						ent->s.frame = FRAME_crattakA-1;
						ent->client->anim_end = FRAME_crattakB;
					}
					else
					{
						ent->s.frame = FRAME_reloadA;
						ent->client->anim_end = FRAME_reloadB;
					}
				}
				else
				{
					if (level.time >= ent->pain_debounce_time)
					{
//						gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
						ent->pain_debounce_time = level.time + 1;
					}
					if(!ent->client->pers.clipammo[weap])
						NoAmmoWeaponChange (ent);
				}
			}
		}
		else if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (FRAME_RELOAD_LAST == 61)
			{
				ent->client->ps.gunframe = 54;
				ent->client->weaponstate = WEAPON_FIRING;
				fire (ent);
			}
			else if (/*(!ent->client->ammo_index) || */(ent->client->pers.clipammo[weap] >= ent->client->pers.weapon->quantity)/* || ent->client->pers.inventory[ent->client->ammo_index]*/)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattakA-1;
					ent->client->anim_end = FRAME_crattakB;
				}
				else if (ent->waterlevel == 3)
				{
					ent->s.frame = FRAME_swattackA;
					ent->client->anim_end = FRAME_swattackB;
				}
				else if ((strcmp(weap2, "weapon_wristrocket") == 0) || (strcmp(weap2, "weapon_missiletube") == 0) || (strcmp(weap2, "weapon_blasterpistol") == 0))
				{
					ent->s.frame = FRAME_firegun2A-1;
					ent->client->anim_end = FRAME_firegun2B;
				}
				else
				{
					ent->s.frame = FRAME_attackgunA-1;
					ent->client->anim_end = FRAME_attackgunB;
				}
			}
			else if(ent->client->pers.inventory[ent->client->ammo_index] && (!ent->client->a_reload))
			{
				fire (ent);
			}
			else if(ent->client->pers.inventory[ent->client->ammo_index])
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
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
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
		{
			ent->client->weaponstate = WEAPON_READY;
		}

		if (ent->client->ps.gunframe > 61 && FRAME_DEACTIVATE_LAST == 53)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST+1;
		}
	}
	else if (ent->client->weaponstate == WEAPON_RELOADING)
	{

		if(ent->client->ps.gunframe == FRAME_RELOAD_FIRST)
			reload (ent);

		if(ent->client->ps.gunframe == FRAME_RELOAD_LAST)
		{
//			reload (ent);
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}
		ent->client->ps.gunframe++;
	}
}

/*
======================================================================

THERMAL

======================================================================
*/

#define THERMAL_TIMER		5.0
#define THERMAL_MINSPEED	350
#define THERMAL_MAXSPEED	500
#define THERMAL_DAMAGE		10000
#define THERMAL_BLASTRADIUS	200

void weapon_thermal_fire (edict_t *ent, qboolean held, int secfunc)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	float	timer;
	int		speed;
	float	radius;

	ent->client->invisi_time = level.time + 2;

	radius = THERMAL_BLASTRADIUS;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	if(timer < 0)
		timer = 0;
	speed = THERMAL_MINSPEED + (THERMAL_TIMER - timer) * ((THERMAL_MAXSPEED - THERMAL_MINSPEED) / THERMAL_TIMER);

	if(secfunc == 0)
	{
		fire_thermal (ent, start, forward, THERMAL_DAMAGE, speed, THERMAL_TIMER, radius, held, secfunc);
	}
	else if (secfunc == 1)
	{
		fire_thermal (ent, start, forward, THERMAL_DAMAGE, speed, THERMAL_TIMER, radius, held, secfunc);
	}

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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
		ent->s.frame = FRAME_crattakA-1;
		ent->client->anim_end = FRAME_crattakB;
	}
	else
	{
		ent->s.frame = FRAME_throwA;
		ent->client->anim_end = FRAME_throwB;
	}
}

void Weapon_Thermal (edict_t *ent, int secfunc)
{
	if (!ent->client->pers.inventory[ent->client->ammo_index])
	{
		if ((secfunc == 1) && (level.time >= ent->pain_debounce_time))
		{
			ent->pain_debounce_time = level.time + 1;
//			ent->client->sec_sec = 1;
		}

		NoAmmoWeaponChange (ent);
	}

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
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
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
		if (ent->client->ps.gunframe == 3)
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 8)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + THERMAL_TIMER + 0.2;
//				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
/*			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_thermal_fire (ent, true, 0);
				ent->client->grenade_blew_up = true;
			}*/

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

		if (ent->client->ps.gunframe == 9)
		{
			ent->client->weapon_sound = 0;
/*			if(ent->client->sec_sec == 1)
			{
				weapon_thermal_fire (ent, false, 1);
				ent->client->sec_sec == 0;
			}
			else*/
				weapon_thermal_fire (ent, false, 0);
		}

		if ((ent->client->ps.gunframe == 12) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 13)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

/*
======================================================================

CaRRaC --

Beam Tube

======================================================================
*/
void weapon_beamtube_fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start, offset;
	int damage;

	ent->client->invisi_time = level.time + 2;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if(!ent->client->pers.clipammo[WEAP_BEAMTUBE])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_BEAMTUBE] = CLIP_BEAMTUBE;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			VectorSet(offset, 26, 6, ent->viewheight);
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/beam/fire.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, start, PNOISE_WEAPON);
			damage = 10;
			fire_beamtube (ent, start, forward, damage);
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				ent->client->pers.clipammo[WEAP_BEAMTUBE]--;
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 9 && ent->client->pers.clipammo[WEAP_BEAMTUBE])
			ent->client->ps.gunframe = 8;
	}

/*	if (ent->client->ps.gunframe == 9)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}
*/

}

void Weapon_BeamTube_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_BEAMTUBE] = CLIP_BEAMTUBE;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/beamclip/tris.md2");
		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/beam/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_BeamTube(edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {12, 14, 0};
	static int	fire_frames[] = {8, 0};

	Weapon_Generic (ent, 7, 9, 15, 23, 35, pause_frames, fire_frames, weapon_beamtube_fire, Weapon_BeamTube_Reload);
}

/*
======================================================================

CaRRaC --

Night Stinger

======================================================================
*/
void Sniper_Fire (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t	forward, right;
	vec3_t	start, end, dir;
	trace_t	tr;

	ent->client->invisi_time = level.time + 2;

//	SWTC_MuzzleFlash (ent, MZ_NIGHTSTINGER);

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, g_offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorMA(ent->s.origin, 8096, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	VectorSubtract (end, start, dir);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (start, MULTICAST_PHS);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BUBBLETRAIL2);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (start, MULTICAST_PHS);

	if (tr.ent->takedamage)
	{
		T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, 300, 0, DAMAGE_ENERGY, MOD_NIGHTSTINGER);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BLOOD);
		gi.WritePosition(tr.ent->s.origin);
		gi.WriteDir(dir);
		gi.multicast(tr.ent->s.origin, MULTICAST_PVS);
	}

	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/sniper/fire.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void fire_sniper_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);

void Sniper_Bullet_Fire (edict_t *ent, int damage)
{
	vec3_t	forward, right;
	vec3_t	org, start, end;
	trace_t	angle_trace;

	ent->client->invisi_time = level.time + 2;

	VectorCopy(ent->s.origin, org);
	org[2] += ent->viewheight;
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, vec3_origin, forward, right, start);

	VectorScale (forward, -7, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorMA(ent->s.origin, 512, forward, end);
	angle_trace = gi.trace(ent->s.origin, NULL, NULL, end, ent, 0);

	VectorSubtract(angle_trace.endpos, ent->s.origin, forward);
	VectorNormalize(forward);

	fire_sniper_bullet (ent, start, forward, damage, 65535);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void weapon_sniper_fire (edict_t *ent)
{
	int damage;
	vec3_t offset;

	if(skill->value < 3)
	{
		damage = 150;
	}
	else
	{
		damage = 200;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (ent->client->ps.gunframe == 10 && ent->client->pers.clipammo[WEAP_NIGHTSTINGER])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if(!ent->client->pers.clipammo[WEAP_NIGHTSTINGER])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_NIGHTSTINGER] = CLIP_SNIPER;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			VectorSet(offset, 14, 3, ent->viewheight-4);
			Sniper_Fire (ent, offset, damage);
//			Sniper_Bullet_Fire (ent, damage);
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				ent->client->pers.clipammo[WEAP_NIGHTSTINGER]--;
		}
		ent->client->ps.gunframe++;
	}
}

void weapon_sniper_fire_sec (edict_t *ent)
{
	/* God I'm lazy ;) */

	sniper_zoom (ent);
	return;

	if(ent->client->in_snipe || !ent->groundentity)
	{
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model); 
		ent->client->in_snipe = 0;
//		ent->client->zoom_factor = 90;
		ent->client->ps.fov = 90;
		set_fov (ent);
	}
	else
	{
//		if(!ent->client->zoom_factor || ent->client->zoom_factor == 90)
//			ent->client->zoom_factor = 45;
//		gi.dprintf("start %i\n", ent->client->zoom_factor);
		ent->client->in_snipe = 1;
		ent->client->ps.gunindex = 0;
		if (ent->client->chasetoggle > 0)
			ChasecamRemove (ent, "off");
	}
}

void Weapon_Sniper_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_NIGHTSTINGER] = CLIP_SNIPER;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/snipclip/tris.md2");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/sniper/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_Sniper(edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {15, 0};
	static int	fire_frames[] = {5, 0};

	if(secfunc == 1)
	{
		weapon_sniper_fire_sec(ent);
	}
	else
	{
		Weapon_Generic (ent, 4, 13, 19, 24, 36, pause_frames, fire_frames, weapon_sniper_fire, Weapon_Sniper_Reload);
	}
}

/*
======================================================================

CaRRaC --

Disruptor

======================================================================
*/
void Disruptor_Fire (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t	forward, right;
	vec3_t	start, end;
	trace_t	angle_trace;

	ent->client->invisi_time = level.time + 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, g_offset, forward, right, start);

	VectorScale (forward, -4, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorMA(ent->s.origin, 512, forward, end);
	angle_trace = gi.trace(ent->s.origin, NULL, NULL, end, ent, 0);

	VectorSubtract(angle_trace.endpos, ent->s.origin, forward);
	VectorNormalize(forward);

	fire_disruptor (ent, start, forward, damage, 1500);
	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte(0);
//	if (hyper)
//		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
//	else
//		gi.WriteByte (MZ_BLASTER | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/disrupt/fire.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void weapon_disruptor_fire (edict_t *ent)
{
	int damage;
	vec3_t offset;

	if((skill->value < 3) && (!deathmatch->value))
	{
		damage = 150;
	}
	else
	{
		damage = 300;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (ent->client->ps.gunframe == 10 && ent->client->pers.clipammo[WEAP_DISRUPTOR])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if(!ent->client->pers.clipammo[WEAP_DISRUPTOR])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_DISRUPTOR] = CLIP_DISRUPTOR;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			VectorSet(offset, 14, 3, ent->viewheight-4);
			Disruptor_Fire (ent, offset, damage);
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				ent->client->pers.clipammo[WEAP_DISRUPTOR]--;
		}
		ent->client->ps.gunframe++;
	}
}

void weapon_disruptor_fire_sec (edict_t *ent)
{
	int damage;
	vec3_t offset;

	damage = 100*ent->client->pers.clipammo[WEAP_DISRUPTOR];

	if(skill->value < 3)
	{
		damage *= 0.75;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (ent->client->ps.gunframe == 10 && ent->client->pers.inventory[ent->client->ammo_index])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if(!ent->client->pers.clipammo[WEAP_DISRUPTOR])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_DISRUPTOR] = CLIP_DISRUPTOR;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			VectorSet(offset, 14, 3, ent->viewheight-4);
			Disruptor_Fire (ent, offset, damage);
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				ent->client->pers.inventory[ent->client->ammo_index] = 0;
		}
		ent->client->ps.gunframe++;
		NoAmmoWeaponChange (ent);
	}
}

void Weapon_Disruptor_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_DISRUPTOR] = CLIP_DISRUPTOR;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/discharg/tris.md2");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/disrupt/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_Disruptor(edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {12, 16, 25, 0};
	static int	fire_frames[] = {4, 0};

	Weapon_Generic (ent, 3, 7, 27, 31, 43, pause_frames, fire_frames, weapon_disruptor_fire, Weapon_Disruptor_Reload);
}

/*
======================================================================

CaRRaC --

Missile Tube

======================================================================
*/
void weapon_mtube_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	ent->client->invisi_time = level.time + 2;

	if(skill->value < 3)
	{
		damage = 150 + (int)(random() * 20.0);
		radius_damage = 120;
		damage_radius = 120;
	}
	else
	{
		damage = 300 + (int)(random() * 50.0);
		radius_damage = 140;
		damage_radius = 140;
	}
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
//RipVTide
	else
	{
		if (ent->client->ps.gunframe == 10 && ent->client->pers.inventory[ent->client->ammo_index])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if(!ent->client->pers.clipammo[WEAP_MISSILETUBE])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_DISRUPTOR] = CLIP_DISRUPTOR;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			AngleVectors (ent->client->v_angle, forward, right, NULL);

			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;
	
			VectorSet(offset, 40, 13, ent->viewheight-3);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/mtube/fire.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, start, PNOISE_WEAPON);
			fire_missile (ent, start, forward, damage, 900, damage_radius, radius_damage);

			// send muzzle flash
//			gi.WriteByte (svc_muzzleflash);
//			gi.WriteShort (ent-g_edicts);
//			gi.WriteByte (MZ_ROCKET | is_silenced);
//			gi.multicast (ent->s.origin, MULTICAST_PVS);

			ent->client->ps.gunframe++;

			PlayerNoise(ent, start, PNOISE_WEAPON);

			if(ent->client->pers.clipammo[WEAP_MISSILETUBE])
				if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.clipammo[WEAP_MISSILETUBE]--;

		}
		ent->client->ps.gunframe++;
	}

//	if(!ent->client->pers.clipammo[WEAP_MISSILETUBE])
//	{
//		if(ent->client->pers.inventory[ent->client->ammo_index])
//		{
//			ent->client->reload = 12;
//			ent->client->pers.inventory[ent->client->ammo_index]--;
//			ent->client->pers.clipammo[WEAP_MISSILETUBE] = CLIP_MISSILETUBE;
//		}
//		else
//		{
//			if (level.time >= ent->pain_debounce_time)
//			{
//				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
//				ent->pain_debounce_time = level.time + 1;
//			}
//			NoAmmoWeaponChange (ent);
//		}
//		return;
//	}

//	AngleVectors (ent->client->v_angle, forward, right, NULL);
//
//	VectorScale (forward, -2, ent->client->kick_origin);
//	ent->client->kick_angles[0] = -1;
//
//	VectorSet(offset, 40, 13, ent->viewheight-3);
//	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//	fire_missile (ent, start, forward, damage, 900, damage_radius, radius_damage);
//
//	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte (MZ_ROCKET | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);
//
//	ent->client->ps.gunframe++;
//
//	PlayerNoise(ent, start, PNOISE_WEAPON);
//
//	if(ent->client->pers.clipammo[WEAP_MISSILETUBE])
//		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//			ent->client->pers.clipammo[WEAP_MISSILETUBE]--;

}

void weapon_mtube_fire_sec (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	ent->client->invisi_time = level.time + 2;

	if(skill->value < 3)
	{
		damage = 150 + (int)(random() * 20.0);
		radius_damage = 120;
		damage_radius = 120;
	}
	else
	{
		damage = 300 + (int)(random() * 50.0);
		radius_damage = 140;
		damage_radius = 140;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (ent->client->ps.gunframe == 10 && ent->client->pers.inventory[ent->client->ammo_index])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if(!ent->client->pers.clipammo[WEAP_MISSILETUBE])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_DISRUPTOR] = CLIP_DISRUPTOR;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			AngleVectors (ent->client->v_angle, forward, right, NULL);

			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;
	
			VectorSet(offset, 40, 13, ent->viewheight-3);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/mtube/fire.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, start, PNOISE_WEAPON);
			fire_sec_missile (ent, start, forward, damage, 900, damage_radius, radius_damage);

			// send muzzle flash
//			gi.WriteByte (svc_muzzleflash);
//			gi.WriteShort (ent-g_edicts);
//			gi.WriteByte (MZ_ROCKET | is_silenced);
//			gi.multicast (ent->s.origin, MULTICAST_PVS);

			ent->client->ps.gunframe++;

			if(ent->client->pers.clipammo[WEAP_MISSILETUBE])
				if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.clipammo[WEAP_MISSILETUBE]--;

		}
		ent->client->ps.gunframe++;
	}

//	if(!ent->client->pers.clipammo[WEAP_MISSILETUBE])
//	{
//		if(ent->client->pers.inventory[ent->client->ammo_index])
//		{
//			ent->client->reload = 12;
//			ent->client->pers.inventory[ent->client->ammo_index]--;
//			ent->client->pers.clipammo[WEAP_MISSILETUBE] = CLIP_MISSILETUBE;
//		}
//		else
//		{
//			if (level.time >= ent->pain_debounce_time)
//			{
//				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
//				ent->pain_debounce_time = level.time + 1;
//			}
//			NoAmmoWeaponChange (ent);
//		}
//		return;
//	}

//	AngleVectors (ent->client->v_angle, forward, right, NULL);
//
//	VectorScale (forward, -2, ent->client->kick_origin);
//	ent->client->kick_angles[0] = -1;
//
//	VectorSet(offset, 40, 13, ent->viewheight-3);
//	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//	fire_missile (ent, start, forward, damage, 900, damage_radius, radius_damage);
//
//	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte (MZ_ROCKET | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);
//
//	ent->client->ps.gunframe++;
//
//	PlayerNoise(ent, start, PNOISE_WEAPON);
//
//	if(ent->client->pers.clipammo[WEAP_MISSILETUBE])
//		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//			ent->client->pers.clipammo[WEAP_MISSILETUBE]--;

}

void Weapon_MissileTube_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_MISSILETUBE] = CLIP_MISSILETUBE;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/rockets/tris.md2");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/mtube/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_MissileTube (edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {20, 30, 0};
	static int	fire_frames[] = {8, 0};

	if(secfunc == 1)
	{
		Weapon_Generic (ent, 7, 16, 32, 40, 52, pause_frames, fire_frames, weapon_mtube_fire_sec, Weapon_MissileTube_Reload);
	}
	else
	{
		Weapon_Generic (ent, 7, 16, 32, 40, 52, pause_frames, fire_frames, weapon_mtube_fire, Weapon_MissileTube_Reload);
	}
}

/*
======================================================================

CaRRaC --

Wrist Rocket

======================================================================
*/
void weapon_wrocket_fire_sec (edict_t *ent)
{
	if(ent->client->togo > 2)
	{
	}
	else if((ent->client->pers.clipammo[WEAP_WRISTROCKET] >= ent->client->pers.weapon->quantity))
	{
		ent->client->togo++;
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		{
			ent->client->pers.clipammo[WEAP_WRISTROCKET]--;
		}
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/reload.wav"), 1, ATTN_NORM, 0);
	}
	else if(ent->client->pers.inventory[ent->client->ammo_index])
	{
//		ent->client->pers.inventory[ent->client->ammo_index]--;
//		ent->client->pers.clipammo[WEAP_DISRUPTOR] = CLIP_DISRUPTOR;
		if(!(ent->client->a_reload == 1))
			ent->client->reload = 12;
//		else
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
		ent->client->togo++;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/reload.wav"), 1, ATTN_NORM, 0);
	}
	else if (level.time >= ent->pain_debounce_time)
	{
//		gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
		ent->pain_debounce_time = level.time + 1;
		NoAmmoWeaponChange (ent);
		weapon_wrocket_fire(ent);
	}
}










/*


		if (!(ent->client->buttons & BUTTON_ATTACK))
		{
			ent->client->ps.gunframe++;
		}
		else
		{
			if (ent->client->ps.gunframe == 10 && ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 8;
				return;
			}
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
				else
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);

				AngleVectors (ent->client->v_angle, forward, right, NULL);
				VectorScale (forward, -2, ent->client->kick_origin);
				ent->client->kick_angles[0] = -1;
		
				VectorSet(offset, 14, 6, ent->viewheight-6);
				P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/fire.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, start, PNOISE_WEAPON);

				fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

				VectorSet (r_offset, 0.1, 0, 0);
	//			AngleVectors (angle, r_offset, NULL, NULL);
				VectorAdd (r_offset, forward, forward);
				VectorSet (r_offset, 0, 0.1, 0);
				VectorAdd (r_offset, forward, forward);
					
				fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

				VectorSet (r_offset, 0.2, 0, 0);
	//			AngleVectors (angle, r_offset, NULL, NULL);
				VectorAdd (-r_offset, forward, forward);
				VectorSet (r_offset, 0, 0.2, 0);
				VectorAdd (-r_offset, forward, forward);
					
				fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

				// send muzzle flash
	//			gi.WriteByte (svc_muzzleflash);
	//			gi.WriteShort (ent-g_edicts);
	//			gi.WriteByte (MZ_ROCKET | is_silenced);
	//			gi.multicast (ent->s.origin, MULTICAST_PVS);
		
				ent->client->ps.gunframe++;

				ent->client->togo = 0;
			}
			else
			{
				AngleVectors (ent->client->v_angle, forward, right, NULL);

				VectorScale (forward, -2, ent->client->kick_origin);
				ent->client->kick_angles[0] = -1;
		
				VectorSet(offset, 14, 6, ent->viewheight-6);
				P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/fire.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, start, PNOISE_WEAPON);

				fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

				VectorSet (r_offset, 0.1, 0, 0);
	//			AngleVectors (angle, r_offset, NULL, NULL);
				VectorAdd (r_offset, forward, forward);
				VectorSet (r_offset, 0, 0.1, 0);
				VectorAdd (r_offset, forward, forward);
					
				fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

				VectorSet (r_offset, 0.2, 0, 0);
	//			AngleVectors (angle, r_offset, NULL, NULL);
				VectorAdd (-r_offset, forward, forward);
				VectorSet (r_offset, 0, 0.2, 0);
				VectorAdd (-r_offset, forward, forward);
					
				fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

				// send muzzle flash
	//			gi.WriteByte (svc_muzzleflash);
	//			gi.WriteShort (ent-g_edicts);
	//			gi.WriteByte (MZ_ROCKET | is_silenced);
	//			gi.multicast (ent->s.origin, MULTICAST_PVS);
		
				ent->client->ps.gunframe++;

				ent->client->togo = 0;

				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
	}
	else if(ent->client->togo > 1)
	{
	}




*/
//	if(!ent->client->pers.clipammo[WEAP_WRISTROCKET])
//	{
//		if(ent->client->pers.inventory[ent->client->ammo_index])
//		{
//			ent->client->reload = 12;
//			ent->client->pers.inventory[ent->client->ammo_index]--;
//			ent->client->pers.clipammo[WEAP_WRISTROCKET] = CLIP_WRISTROCKET;
//		}
//		else
//		{
//			if (level.time >= ent->pain_debounce_time)
//			{
//				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
//				ent->pain_debounce_time = level.time + 1;
//			}
//			NoAmmoWeaponChange (ent);
//		}
//		return;
//	}
//
//	AngleVectors (ent->client->v_angle, forward, right, NULL);
//
//	VectorScale (forward, -2, ent->client->kick_origin);
//	ent->client->kick_angles[0] = -1;
//
//	VectorSet(offset, 14, 6, ent->viewheight-6);
//	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//	fire_rocket (ent, start, forward, damage, 1800, damage_radius, radius_damage);
//
//	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte (MZ_ROCKET | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);
//
//	ent->client->ps.gunframe++;
//
//	PlayerNoise(ent, start, PNOISE_WEAPON);
//
//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.clipammo[WEAP_WRISTROCKET]--;
//}

void weapon_wrocket_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	vec3_t	r_offset;

	ent->client->invisi_time = level.time + 2;

	if(skill->value < 3)
	{
		damage = 70;
		radius_damage = 70;
		damage_radius = 70;
	}
	else
	{
		damage = 90;
		radius_damage = 100;
		damage_radius = 100;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else if (ent->client->ps.gunframe == 10 && ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 8;
		return;
	}
	else if(ent->client->togo)
	{
		if(ent->client->togo == 3)
		{
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;
		
			VectorSet(offset, 14, 6, ent->viewheight-6);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/fire.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, start, PNOISE_WEAPON);

			fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

			VectorSet (r_offset, 0.1, 0, 0);
			VectorAdd (r_offset, forward, forward);
			VectorSet (r_offset, 0, 0.1, 0);
			VectorAdd (r_offset, forward, forward);
					
			fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

			VectorSet (r_offset, 0.2, 0, 0);
			VectorAdd (-r_offset, forward, forward);
			VectorSet (r_offset, 0, 0.2, 0);
			VectorAdd (-r_offset, forward, forward);
					
			fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

			ent->client->ps.gunframe++;

			if(ent->client->pers.clipammo[WEAP_WRISTROCKET])
			{
				ent->client->togo = 1;
				ent->client->pers.clipammo[WEAP_WRISTROCKET]--;
			}
			else
				ent->client->togo = 0;
		}
		else if(ent->client->togo == 2)
		{
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;
		
			VectorSet(offset, 14, 6, ent->viewheight-6);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/fire.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, start, PNOISE_WEAPON);

			VectorSet (r_offset, 0.05, 0, 0);
			VectorAdd (r_offset, forward, forward);
			VectorSet (r_offset, 0, 0.05, 0);
			VectorAdd (r_offset, forward, forward);
					
			fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

			VectorSet (r_offset, 0.1, 0, 0);
			VectorAdd (-r_offset, forward, forward);
			VectorSet (r_offset, 0, 0.1, 0);
			VectorAdd (-r_offset, forward, forward);
					
			fire_rocket (ent, start, forward, damage, 2000, damage_radius, radius_damage);

			ent->client->ps.gunframe++;

			if(ent->client->pers.clipammo[WEAP_WRISTROCKET])
			{
				ent->client->togo = 1;
				ent->client->pers.clipammo[WEAP_WRISTROCKET]--;
			}
			else
				ent->client->togo = 0;		}
		else
		{
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;
		
			VectorSet(offset, 14, 6, ent->viewheight-6);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/fire.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, start, PNOISE_WEAPON);

			fire_homing (ent, start, forward, damage, 2000, damage_radius, radius_damage);

			ent->client->ps.gunframe++;

			if(ent->client->pers.clipammo[WEAP_WRISTROCKET])
			{
				ent->client->togo = 1;
				ent->client->pers.clipammo[WEAP_WRISTROCKET]--;
			}
			else
				ent->client->togo = 0;
		}
	}
	else if(!ent->client->pers.clipammo[WEAP_WRISTROCKET])
	{
		if(ent->client->pers.inventory[ent->client->ammo_index])
		{
			if(!(ent->client->a_reload == 1))
				ent->client->reload = 12;
//			else
//				gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			if (level.time >= ent->pain_debounce_time)
			{
//				gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
	}
	else
	{
		AngleVectors (ent->client->v_angle, forward, right, NULL);

		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;

		VectorSet(offset, 14, 6, ent->viewheight-6);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/fire.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, start, PNOISE_WEAPON);
		fire_homing (ent, start, forward, damage, 1800, damage_radius, radius_damage);

		ent->client->ps.gunframe++;

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		{
			ent->client->pers.clipammo[WEAP_WRISTROCKET]--;
			ent->client->togo = 1;
		}
	}
}

//	if(!ent->client->pers.clipammo[WEAP_WRISTROCKET])
//	{
//		if(ent->client->pers.inventory[ent->client->ammo_index])
//		{
//			ent->client->reload = 12;
//			ent->client->pers.inventory[ent->client->ammo_index]--;
//			ent->client->pers.clipammo[WEAP_WRISTROCKET] = CLIP_WRISTROCKET;
//		}
//		else
//		{
//			if (level.time >= ent->pain_debounce_time)
//			{
//				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
//				ent->pain_debounce_time = level.time + 1;
//			}
//			NoAmmoWeaponChange (ent);
//		}
//		return;
//	}
//
//	AngleVectors (ent->client->v_angle, forward, right, NULL);
//
//	VectorScale (forward, -2, ent->client->kick_origin);
//	ent->client->kick_angles[0] = -1;
//
//	VectorSet(offset, 14, 6, ent->viewheight-6);
//	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//	fire_rocket (ent, start, forward, damage, 1800, damage_radius, radius_damage);
//
//	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte (MZ_ROCKET | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);
//
//	ent->client->ps.gunframe++;
//
//	PlayerNoise(ent, start, PNOISE_WEAPON);
//
//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.clipammo[WEAP_WRISTROCKET]--;
//}

void Weapon_WristRocket_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_WRISTROCKET] = CLIP_WRISTROCKET;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/wrocket/tris.md2");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/wrocket/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_WristRocket(edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {32, 40, 0};
	static int	fire_frames[] = {5, 0};

	if((secfunc == 1) && (ent->client->togo < 3))
	{
		Weapon_Generic (ent, 4, 23, 48, 53, 61, pause_frames, fire_frames, weapon_wrocket_fire_sec, Weapon_WristRocket_Reload);
	}
	else
	{
		Weapon_Generic (ent, 4, 23, 48, 53, 62, pause_frames, fire_frames, weapon_wrocket_fire, Weapon_WristRocket_Reload);
	}
}


/*
======================================================================

CaRRaC --

Bowcaster

======================================================================
*/

void Bowcaster_Fire (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t	forward, right;
	vec3_t	start, end;
	trace_t	angle_trace;

	ent->client->invisi_time = level.time + 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, g_offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorMA(ent->s.origin, 512, forward, end);
	angle_trace = gi.trace(ent->s.origin, NULL, NULL, end, ent, 0);

	VectorSubtract(angle_trace.endpos, ent->s.origin, forward);
	VectorNormalize(forward);

	fire_bowcaster (ent, start, forward, damage, 2048);
	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte(0);
//	if (hyper)
//		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
//	else
//		gi.WriteByte (MZ_BLASTER | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/arrow/fire.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void weapon_bowcaster_fire (edict_t *ent)
{
	vec3_t	offset;
	int		damage;

	if(skill->value < 3)
	{
		damage = 40;
	}
	else
	{
		damage = 70;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (ent->client->ps.gunframe == 15 && ent->client->pers.clipammo[WEAP_BOWCASTER])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if(!ent->client->pers.clipammo[WEAP_BOWCASTER])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_BOWCASTER] = CLIP_BOWCASTER;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			VectorSet(offset, 35, 10, ent->viewheight-10);
			Bowcaster_Fire (ent, offset, damage);
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				ent->client->pers.clipammo[WEAP_BOWCASTER]--;
		}

		ent->client->ps.gunframe++;
	}
}

void Weapon_Bowcaster_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_BOWCASTER] = CLIP_BOWCASTER;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/arrows/tris.md2");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/arrow/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_Bowcaster (edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {20, 26, 0};
	static int	fire_frames[] = {8, 0};

	Weapon_Generic (ent, 7, 15, 30, 38, 50, pause_frames, fire_frames, weapon_bowcaster_fire, Weapon_Bowcaster_Reload);
}


/*
======================================================================

CaRRaC --

Light Repeating Blaster Rifle

======================================================================
*/
void Repeater_Fire (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t	forward, right;
	vec3_t	start, end;
	trace_t	angle_trace;

	ent->client->invisi_time = level.time + 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, g_offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorMA(ent->s.origin, 512, forward, end);
	angle_trace = gi.trace(ent->s.origin, NULL, NULL, end, ent, 0);

	VectorSubtract(angle_trace.endpos, ent->s.origin, forward);
	VectorNormalize(forward);

	fire_repeater (ent, start, forward, damage, 2048);
	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte(0);
//	if (hyper)
//		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
//	else
//		gi.WriteByte (MZ_BLASTER | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/rapid/fire.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void weapon_repeater_fire (edict_t *ent)
{
	int damage;
	vec3_t offset;

	if(skill->value < 3)
	{
		damage = 50;
	}
	else
	{
		damage = 70+(random()*10);
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (ent->client->ps.gunframe == 10 && ent->client->pers.clipammo[WEAP_REPEATER])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if(!ent->client->pers.clipammo[WEAP_REPEATER])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_REPEATER] = CLIP_REPEATER;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			VectorSet(offset, 47, 10, ent->viewheight-13);
			Repeater_Fire (ent, offset, 20);
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				ent->client->pers.clipammo[WEAP_REPEATER]--;
		}

		ent->client->ps.gunframe++;
	}
}

void Weapon_Repeater_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_REPEATER] = CLIP_REPEATER;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/rapclip/tris.md2");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/rapid/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_Repeater (edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {18, 26, 0};
	static int	fire_frames[] = {7, 0};

//	if(secfunc == 1)
//	{
//		Weapon_Generic (ent, 6, 15, 26, 33, 45, pause_frames, fire_frames, weapon_repeater_fire, Weapon_Repeater_Reload);
//	}
//	else
//	{
		Weapon_Generic (ent, 6, 15, 26, 33, 45, pause_frames, fire_frames, weapon_repeater_fire, Weapon_Repeater_Reload);
//	}
}

/*
======================================================================

CaRRaC --

Storm Trooper Rifle

======================================================================
*/
void weapon_rifle_fire (edict_t *ent)
{
	vec3_t offset;
	int damage;

	ent->client->invisi_time = level.time + 2;
	
	if(skill->value < 3)
	{
		damage = 25;
	}
	else
	{
		damage = 30+random()*10;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (ent->client->ps.gunframe == 10 && ent->client->pers.clipammo[WEAP_RIFLE])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if(!ent->client->pers.clipammo[WEAP_RIFLE])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_RIFLE] = CLIP_RIFLE;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			VectorSet(offset, 22, 8, ent->viewheight-10);
			Blaster_Fire (ent, offset, damage, true);
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
//				ent->client->pers.inventory[ent->client->ammo_index]--;
				ent->client->pers.clipammo[WEAP_RIFLE]--;
		}

		ent->client->ps.gunframe++;
	}
}

void Weapon_Rifle_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_RIFLE] = CLIP_RIFLE;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/riflclip/tris.md2");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/rifle/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_Rifle (edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {20, 26, 0};
	static int	fire_frames[] = {8, 10, 0};
	static int	reload_frames[] = {10, 0};

	Weapon_Generic (ent, 7, 11, 28, 31, 43, pause_frames, fire_frames, weapon_rifle_fire, Weapon_Rifle_Reload);
}

/*
======================================================================

CaRRaC --

Holstered Weapon

======================================================================
*/

void Weapon_Hands_Fire (edict_t *ent)
{
	return;
}

void Weapon_Hands (edict_t *ent, int secfunc)
{
	Weapon_Generic (ent, 0, 0, 0, 0, 0, 0, 0, Weapon_Hands_Fire, Weapon_Hands_Fire);
}

/*
======================================================================

CaRRaC --

Saber Throw

======================================================================
*/

void saber_throw_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_FreeEdict(self);
}

void Saber_Throw (edict_t *self)
{
	edict_t	*saber;
	vec3_t		forward;

	self->client->invisi_time = level.time + 2;

	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorNormalize(forward);
	saber = G_Spawn();
	saber->avelocity[1] = 180;
	VectorCopy (self->s.origin, saber->s.origin);
	vectoangles (forward, saber->s.angles);
	VectorScale (forward, 600, saber->velocity);
	saber->movetype = MOVETYPE_FLYMISSILE;
	saber->clipmask = MASK_SHOT;
	saber->solid = SOLID_BBOX;
	saber->s.renderfx |= RF_FULLBRIGHT;
	VectorClear (saber->mins);
	VectorClear (saber->maxs);
	saber->s.modelindex = gi.modelindex ("models/objects/lsaber/tris.md2");
	saber->s.frame = 0;
	saber->owner = self;
	saber->touch = saber_throw_touch;
	saber->nextthink = level.time + 10;
	saber->think = G_FreeEdict;
	saber->dmg = 1000;
	saber->classname = "saber";
}	

void weapon_saber_throw (edict_t *ent)
{
	vec3_t offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (ent->client->ps.gunframe == 10 && ent->client->pers.inventory[ent->client->ammo_index])
		{
			ent->client->ps.gunframe = 8;
			return;
		}
		if (ent->client->pers.inventory[ent->client->ammo_index] <= 1)
		{
			if (level.time >= ent->pain_debounce_time)
			{
//				gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
			VectorSet(offset, 22, 8, ent->viewheight-10);
			Saber_Throw (ent);
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				ent->client->pers.inventory[ent->client->ammo_index] -= 2;
		}
	}
}

void Weapon_SaberThrow (edict_t *ent, int secfunc)
{
	static int	pause_frames[] = {20, 26, 0};
	static int	fire_frames[] = {8, 10, 0};

	Weapon_Generic (ent, 7, 10, 28, 31, 32, pause_frames, fire_frames, weapon_saber_throw, weapon_saber_throw);
}

/*
======================================================================

CaRRaC --

Light Saber

SPLASH EFFECTS:
1 = light brown + spark sound
2 = light blue
3 = Brown;
4 = Yellow
5 = Light brown
6 = Dark Brown + Red

======================================================================
*/

//#define SABER_DEBUG

int Check_Saber_Block(int swing, int block)
{
	int will_block[4];

	switch(block)
	{
		case BLOCK_LOW:
			will_block[0] = SWING_UCUTLEFT;
			will_block[1] = SWING_UCUTRIGHT;
			will_block[2] = SWING_LEFT;
			will_block[3] = SWING_RIGHT;
		break;

		case BLOCK_UP:
			will_block[0] = SWING_CHOP;
			will_block[1] = SWING_SLICERIGHT;
			will_block[2] = SWING_SLICELEFT;
			will_block[3] = 0;
		break;

		case BLOCK_LEFT:
			will_block[0] = SWING_RIGHT;
			will_block[1] = SWING_SLICERIGHT;
			will_block[2] = SWING_UCUTRIGHT;
			will_block[3] = 0;
		break;

		case BLOCK_RIGHT:
			will_block[0] = SWING_LEFT;
			will_block[1] = SWING_SLICELEFT;
			will_block[2] = SWING_UCUTLEFT;
			will_block[3] = 0;
		break;

		default:
			return 0;
	}

	if(swing == SWING_THRUST)
	{
		return 255;
	}
	if(will_block[0] == swing)
	{
		return 255;
	}
	else if(will_block[1] == swing)
	{
		return 255;
	}
	else if(will_block[2] == swing)
	{
		return 255;
	}
	else if(will_block[3] == swing)
	{
		return 255;
	}
	else
	{
		return 0;
	}
}
/*
 Block Low Left
                          Thrust
 Block Low Right
                          Thrust
*/
void Saber_Quick_Attack (edict_t *ent, int block)
{
	int attack[4];
	float n;

	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	int	fire_frames1[] = {21, 22, 23, 0};

	switch(block)
	{
		case BLOCK_LOWLEFT:
			attack[0] = SWING_THRUST;
			attack[1] = SWING_THRUST;
			attack[2] = SWING_THRUST;
			attack[3] = SWING_THRUST;
		break;

		case BLOCK_LOWRIGHT:
			attack[0] = SWING_THRUST;
			attack[1] = SWING_THRUST;
			attack[2] = SWING_THRUST;
			attack[3] = SWING_THRUST;
		break;

		case BLOCK_UPLEFT:
			attack[0] = SWING_SLICERIGHT;
			attack[1] = SWING_RIGHT;
			attack[2] = SWING_UCUTRIGHT;
			attack[3] = SWING_UCUTRIGHT;
		break;

		case BLOCK_UPRIGHT:
			attack[0] = SWING_SLICELEFT;
			attack[1] = SWING_LEFT;
			attack[2] = SWING_UCUTLEFT;
			attack[3] = SWING_UCUTLEFT;
		break;

		case BLOCK_LEFT:
			attack[0] = SWING_RIGHT;
			attack[1] = SWING_SLICERIGHT;
			attack[2] = SWING_UCUTRIGHT;
			attack[3] = SWING_UCUTRIGHT;
		break;

		case BLOCK_RIGHT:
			attack[0] = SWING_LEFT;
			attack[1] = SWING_SLICELEFT;
			attack[2] = SWING_UCUTLEFT;
			attack[3] = SWING_UCUTLEFT;
		break;

		default:
			return;
	}

	n = random();

	if(n > 0.75)
	{
		ent->client->swing_num = attack[0];
	}
	else if(n > 0.5)
	{
		ent->client->swing_num = attack[1];
	}
	else if(n > 0.25)
	{
		ent->client->swing_num = attack[2];
	}
	else
	{
		ent->client->swing_num = attack[3];
	}

	ent->client->swing_frame = 1;
	ent->client->buttons |= BUTTON_ATTACK;
	Weapon_Generic_Saber (ent, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);
}

int Saber_Block (edict_t *self, edict_t *other)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	int	fire_frames1[] = {21, 22, 23, 0};

	if(infront (other, self))
	{
		other->client->buttons |= BUTTON_ATTACK;

		if(self->client->swing_num == SWING_CHOP || self->client->swing_num == SWING_THRUST)
		{
			other->client->swing_num = BLOCK_UP;
		}
		else if(self->client->swing_num == SWING_UCUTLEFT || self->client->swing_num == SWING_UCUTRIGHT)
		{
			other->client->swing_num = BLOCK_LOW;
		}
		else if(self->client->swing_num == SWING_RIGHT || self->client->swing_num == SWING_SLICERIGHT)
		{
			other->client->swing_num = BLOCK_LEFT;
		}
		else if(self->client->swing_num == SWING_LEFT || self->client->swing_num == SWING_SLICELEFT)
		{
			other->client->swing_num = BLOCK_RIGHT;
		}

		other->client->swing_frame = 1;
		Weapon_Generic_Saber (other, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);
		return 1;
	}
	return 0;
}

void check_saber_hit (edict_t *ent, vec3_t point1, vec3_t point2, vec3_t dir, int damage)
{
	trace_t	tr;

	tr = gi.trace (point1, NULL, NULL, point2, ent, MASK_ALL);

	if(tr.ent->takedamage && tr.ent->health > 0)
	{
		if(tr.ent->client && tr.ent->client->pers.weapon == FindItem("Lightsaber"))
		{
			if(tr.ent->client->weaponstate == WEAPON_FIRING && Check_Saber_Block(ent->client->swing_num, tr.ent->client->swing_num) == 255) //Manual block
			{

				if(random() > 0.5)
					gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/sabre/strike1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/sabre/strike2.wav"), 1, ATTN_NORM, 0);
//				gi.dprintf("Manual block\n");
				if(ent->client->duel)
					Saber_Quick_Attack (tr.ent, tr.ent->client->swing_num);
				return;
			}
			else if(tr.ent->client->weaponstate != WEAPON_FIRING && Saber_Block (ent, tr.ent)) //autoblock
			{
				return;
			}
		}
		T_Damage (tr.ent, ent, ent, dir, ent->s.origin, vec3_origin, 40, 40, DAMAGE_RADIUS, MOD_SABER);
	}
	else if(tr.fraction != 1)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPLASH);
		gi.WriteByte (5);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		gi.WriteByte (4);
		gi.multicast (tr.endpos, MULTICAST_PVS);
		if(!ent->client->hit_this_swing)
		{
			ent->client->hit_this_swing = 1;
			if(random() > 0.5)
				gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/sabre/strike1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/sabre/strike2.wav"), 1, ATTN_NORM, 0);
		}
	}
}

void fire_saber (edict_t *ent, vec3_t org, vec3_t point, vec3_t dir, int damage)
{


#ifdef SABER_DEBUG
	spawn_temp(point, 1);
#endif

	check_saber_hit (ent, ent->last_saber_pos, point, dir, damage);
	check_saber_hit (ent, org, point, dir, damage);

#ifdef SABER_DEBUG
	spawn_templaser(ent->last_saber_pos, point);
#endif

	VectorCopy(point, ent->last_saber_pos);

	if(ent->client->swing_frame == 1)
	{
		if(random() > 0.25)
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/sabre/swing1.wav"), 1, ATTN_NORM, 0);
		else if(random() > 0.33)
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/sabre/swing2.wav"), 1, ATTN_NORM, 0);
		else if(random() > 0.5)
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/sabre/swing3.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/sabre/swing4.wav"), 1, ATTN_NORM, 0);
	}
}

void weapon_saber_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		point;
	int			damage = 50;

	ent->client->invisi_time = level.time + 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorMA(ent->s.origin, 8, forward, point);

	if(ent->client->swing_frame == 1)
	{
		VectorCopy (ent->s.origin, ent->last_saber_pos);
	}

//	gi.dprintf("%i\n", ent->client->swing_num);

	if(ent->client->swing_num == SWING_CHOP)
		G_ProjectSource (point, chop[ent->client->swing_frame-1], forward, right, start);
//	else if(ent->client->swing_num == BLOCK_LOW)
//	{
//		if(ent->s.frame >= FRAME_b_lowrightA)
//		{
//			G_ProjectSource (point, b_lowright[ent->client->swing_frame-1], forward, right, start);
//		}
//		else
//		{
//			G_ProjectSource (point, b_lowleft[ent->client->swing_frame-1], forward, right, start);
//		}
//	}
	else if(ent->client->swing_num == SWING_RIGHT)
		G_ProjectSource (point, swingrt[ent->client->swing_frame-1], forward, right, start);
	else if(ent->client->swing_num == SWING_LEFT)
		G_ProjectSource (point, swinglft[ent->client->swing_frame-1], forward, right, start);
	else if(ent->client->swing_num == SWING_SLICERIGHT)
		G_ProjectSource (point, sliceright[ent->client->swing_frame-1], forward, right, start);
	else if(ent->client->swing_num == SWING_SLICELEFT)
		G_ProjectSource (point, sliceleft[ent->client->swing_frame-1], forward, right, start);
	else if(ent->client->swing_num == SWING_THRUST)
		G_ProjectSource (point, thrust[ent->client->swing_frame-1], forward, right, start);
//	else if(ent->client->swing_num == BLOCK_UP)
//	{
//		if(ent->s.frame >= FRAME_b_uprightA)
//		{
//			G_ProjectSource (point, b_upright[ent->client->swing_frame-1], forward, right, start);
//		}
//		else
//		{
//			G_ProjectSource (point, b_upleft[ent->client->swing_frame-1], forward, right, start);
//		}
//	}
	else if(ent->client->swing_num == SWING_UCUTLEFT)
		G_ProjectSource (point, upprcutlft[ent->client->swing_frame-1], forward, right, start);
	else if(ent->client->swing_num == SWING_UCUTRIGHT)
		G_ProjectSource (point, upprcutrt[ent->client->swing_frame-1], forward, right, start);
//	else if(ent->client->swing_num == BLOCK_LEFT)
//		G_ProjectSource (point, b_left[ent->client->swing_frame-1], forward, right, start);
//	else if(ent->client->swing_num == BLOCK_RIGHT)
//		G_ProjectSource (point, b_right[ent->client->swing_frame-1], forward, right, start);
//	else
//		G_ProjectSource (point, b_right[ent->client->swing_frame-1], forward, right, start);
//		return;
	else if(ent->client->swing_num == SWING_CROUCH_A)
		G_ProjectSource (point, crawlslrt[ent->client->swing_frame-1], forward, right, start);
	else if(ent->client->swing_num == SWING_CROUCH_B)
		G_ProjectSource (point, crawlslrt[ent->client->swing_frame-1], forward, right, start);
	else if(ent->client->swing_num == SWING_RUN_A)
		G_ProjectSource (point, runsla[ent->client->swing_frame-1], forward, right, start);
	else if(ent->client->swing_num == SWING_RUN_B)
		G_ProjectSource (point, runslb[ent->client->swing_frame-1], forward, right, start);
	else
		VectorCopy(ent->s.origin, start);

	fire_saber (ent, ent->s.origin, start, forward, damage);

	// send muzzle flash
	ent->client->ps.gunframe++;
	ent->client->swing_frame++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
}

/*
                         Movement
                                                  +Crouch
  (none)
                         Chop
                                                  ?Block Mid
  Forward
                         Thrust
                                                  Block High
  Back
                         Uppercut (Right/Left)
                                                  Block Low
  Sidestep
                         Swing (Right/Left)
                                                  Slice (Right/Left)
*/

void Weapon_Saber (edict_t *ent, int secfunc)
{
	static int	pause_frames[]	= {0};
	int	fire_frames1[] = {121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 0};
	int	block_frames[] = {11, 12, 13, 14, 15, 0};

	if (ent->client->weaponstate == WEAPON_READY)
	{
		ent->client->hit_this_swing = 0;
		if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
		{
			if(advanced->value && !ent->client->duel)
			{
				if(random() < 0.5)
					ent->client->swing_num = SWING_RUN_A;
				else
					ent->client->swing_num = SWING_RUN_B;
			}
			else if(!ent->client->duel)
			{
				if(ent->client->forwardmove < 0)
				{
					if(ent->client->sidemove < 0)
					{
						if(ent->client->upmove >= 0)
						{
							ent->client->swing_num = SWING_UCUTRIGHT;
						}
						else
						{
							if(random() < 0.5)
								ent->client->swing_num = SWING_CROUCH_A;
							else
								ent->client->swing_num = SWING_CROUCH_B;
						}
					}
					else if(ent->client->sidemove > 0)
					{
						if(ent->client->upmove >= 0)
						{
							ent->client->swing_num = SWING_UCUTLEFT;
						}
						else
						{
							if(random() < 0.5)
								ent->client->swing_num = SWING_CROUCH_A;
							else
								ent->client->swing_num = SWING_CROUCH_B;
						}
					}
					else
					{
						if(ent->client->upmove >= 0)
						{
							if(random() > 0.5)
								ent->client->swing_num = SWING_UCUTRIGHT;
							else
								ent->client->swing_num = SWING_UCUTLEFT;
						}
						else
						{
							if(random() < 0.5)
								ent->client->swing_num = SWING_CROUCH_A;
							else
								ent->client->swing_num = SWING_CROUCH_B;
						}
					}
				}
				else if(ent->client->sidemove < 0)
				{
					if(ent->client->upmove >= 0)
					{
						ent->client->swing_num = SWING_RIGHT;
					}
					else
					{
						if(random() < 0.5)
							ent->client->swing_num = SWING_CROUCH_A;
						else
							ent->client->swing_num = SWING_CROUCH_B;
					}
				}
				else if(ent->client->sidemove > 0)
				{
					if(ent->client->upmove >= 0)
					{
						ent->client->swing_num = SWING_LEFT;
					}
					else
					{
						if(random() < 0.5)
							ent->client->swing_num = SWING_CROUCH_A;
						else
							ent->client->swing_num = SWING_CROUCH_B;
					}
				}
				else if(ent->client->forwardmove > 0)
				{
					if((ent->client->forwardmove > 200) && (ent->client->upmove >= 0))
					{
						if(random() < 0.5)
							ent->client->swing_num = SWING_RUN_A;
						else
							ent->client->swing_num = SWING_RUN_B;
					}
					else
					{
						if(ent->client->upmove >= 0)
						{
							ent->client->swing_num = SWING_CHOP;
						}
						else
						{
							if(random() < 0.5)
								ent->client->swing_num = SWING_CROUCH_A;
							else
								ent->client->swing_num = SWING_CROUCH_B;
						}
					}
				}
				else
				{
					if(ent->client->upmove >= 0)
						ent->client->swing_num = SWING_THRUST;
					else
					{
						if(random() < 0.5)
							ent->client->swing_num = SWING_CROUCH_A;
						else
							ent->client->swing_num = SWING_CROUCH_B;
					}
				}
			}
//RipVTide
			else if(ent->client->forwardmove < 0)
			{
				if(ent->client->sidemove < 0)
				{
					if(ent->client->upmove >= 0)
					{
						ent->client->swing_num = SWING_UCUTRIGHT;
					}
					else
					{
						ent->client->swing_num = BLOCK_LOWRIGHT;
					}
				}
				else if(ent->client->sidemove > 0)
				{
					if(ent->client->upmove >= 0)
					{
						ent->client->swing_num = SWING_UCUTLEFT;
					}
					else
					{
						ent->client->swing_num = BLOCK_LOWLEFT;
					}
				}
				else
				{
					if(ent->client->upmove >= 0)
					{
						if(random() > 0.5)
							ent->client->swing_num = SWING_UCUTRIGHT;
						else
							ent->client->swing_num = SWING_UCUTLEFT;
					}
					else
					{
						if(random() > 0.5)
							ent->client->swing_num = BLOCK_LOWLEFT;
						else
							ent->client->swing_num = BLOCK_LOWRIGHT;
					}
				}
			}
			else if(ent->client->sidemove < 0)
			{
				if(ent->client->upmove >= 0)
				{
					ent->client->swing_num = SWING_RIGHT;
				}
				else
				{
					ent->client->swing_num = SWING_SLICERIGHT;
				}
			}
			else if(ent->client->sidemove > 0)
			{
				if(ent->client->upmove >= 0)
				{
					ent->client->swing_num = SWING_LEFT;
				}
				else
				{
					ent->client->swing_num = SWING_SLICELEFT;
				}
			}
			else if(ent->client->forwardmove > 0)
			{
				if(ent->client->upmove >= 0)
				{
					ent->client->swing_num = SWING_THRUST;
				}
				else
				{
					if(random() > 0.5)
						ent->client->swing_num = BLOCK_UPLEFT;
					else
						ent->client->swing_num = BLOCK_UPLEFT;
				}
			}
			else
			{
				if(ent->client->upmove >= 0)
					ent->client->swing_num = SWING_CHOP;
				else
				{
					if(random() > 0.5)
						ent->client->swing_num = BLOCK_LEFT;
					else
						ent->client->swing_num = BLOCK_RIGHT;
				}
			}

//			if(ent->client->upmove < 0)
//				ent->client->swing_num += 8;
			ent->client->swing_frame = 1;
			Weapon_Generic_Saber (ent, 10, 41, 102, 112, pause_frames, fire_frames1, weapon_saber_fire);
			return;
		}
	}
	if(ent->client->weaponstate == WEAPON_FIRING)
	{
		Weapon_Generic_Saber (ent, 10, 41, 102, 112, pause_frames, fire_frames1, weapon_saber_fire);
		return;
	}
	Weapon_Generic_Saber (ent, 10, 41, 102, 112, pause_frames, fire_frames1, weapon_saber_fire);
}

/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/
void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean trooper)
{
	vec3_t	forward, right;
	vec3_t	start, end;
	trace_t	angle_trace;

	ent->client->invisi_time = level.time + 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, g_offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorMA(ent->s.origin, 512, forward, end);
	angle_trace = gi.trace(ent->s.origin, NULL, NULL, end, ent, 0);

	VectorSubtract(angle_trace.endpos, ent->s.origin, forward);
	VectorNormalize(forward);

	fire_blaster (ent, start, forward, damage, 2048, trooper);
	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte(0);
//	if (hyper)
//		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
//	else
//		gi.WriteByte (MZ_BLASTER | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);

	if(trooper)
		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/rifle/fire.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/pistol/fire.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage;
	vec3_t	offset;

	if (deathmatch->value)
		damage = 15;
	else if (skill->value < 3)
		damage = 15;
	else
		damage = 25;

	if(!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if(!ent->client->pers.clipammo[WEAP_PISTOL])
		{
			if(ent->client->pers.inventory[ent->client->ammo_index])
			{
//				ent->client->pers.inventory[ent->client->ammo_index]--;
//				ent->client->pers.clipammo[WEAP_PISTOL] = CLIP_PISTOL;
				if(!(ent->client->a_reload == 1))
					ent->client->reload = 12;
//				else
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			VectorSet(offset, 23, 6, ent->viewheight-5);
			Blaster_Fire (ent, offset, damage, false);
			ent->client->pers.clipammo[WEAP_PISTOL]--;
		}
		ent->client->ps.gunframe++;
	}
}

void Weapon_Blaster_Reload (edict_t *ent)
{
	if(ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->pers.clipammo[WEAP_PISTOL] = CLIP_PISTOL;
		ent->client->ps.gunframe++;
		SpawnEmptyClip(ent, "models/items/ammo/pistclip/tris.md2");
		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/pistol/reload.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	}
}

void Weapon_Blaster (edict_t *ent, int secfunc)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, 67, pause_frames, fire_frames, Weapon_Blaster_Fire, Weapon_Blaster_Reload);
}

