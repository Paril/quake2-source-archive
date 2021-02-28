// g_weapon.c

#include "g_local.h"
#include "m_player.h"

qboolean	is_quad;
// RAFAEL
qboolean is_quadfire;
byte		is_silenced;

void lasersight_on (edict_t *ent);
void lasersight_off (edict_t *ent);
void weapon_grenade_fire (edict_t *ent, qboolean held);
// RAFAEL
void weapon_trap_fire (edict_t *ent, qboolean held);

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
	/*edict_t		*noise;

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
	gi.linkentity (noise);*/
}

void ShowGun(edict_t *ent);
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

	//K03 Begin
	if (other->client->pers.inventory[index] == 0)
		other->client->pers.inventory[index]++;
	//K03 End

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
	
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);

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

	if (other->client->pers.weapon != ent->item && // not the same weapon equipped
		(other->client->pers.inventory[index] == 1) && // do not already have one
		( !deathmatch->value || other->client->pers.weapon == Fdi_BLASTER) )
	{
		if ((V_WeaponUpgradeVal(other, WEAPON_BLASTER) < 1) 
			&& (other->myskills.respawn_weapon != 13)) //4.2 don't switch weapons if blaster is set as respawn weapon
			other->client->newweapon = ent->item;
	}

	return true;
}

/*
================
ToggleSecondary

Returns true if this weapon has secondary fire, and
prints client messages confirming mode change
================
*/
qboolean ToggleSecondary (edict_t *ent, gitem_t *item, qboolean printmsg)
{
	if (!ent->mtype)
	{
		if (!item)
			return false; // must have a weapon
		if (ent->client->weaponstate != WEAPON_READY)
			return false; // weapon must be ready
	}

	if (ent->client->weapon_mode)
	{
		// flag carrier in CTF can't use weapons
		if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
			return false;


		if (strcmp(item->pickup_name, "Sword") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Normal firing\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Blaster") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Normal firing\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Railgun") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Normal firing\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Grenade Launcher") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Normal firing\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Machinegun") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Full automatic\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Chaingun") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Normal firing\n");
			return true;
		}
	}
	else
	{
		if (!item) // TODO: Should the check be here, or should togglesecondary not be called?
			return false;

		if (strcmp(item->pickup_name, "Sword") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Lance mode\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Blaster") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Blast mode\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Railgun") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Sniper mode\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Grenade Launcher") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Pipebomb mode\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Machinegun") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Burst fire\n");
			return true;
		}
		if (strcmp(item->pickup_name, "Chaingun") == 0) {
			if (printmsg)
				safe_cprintf(ent, PRINT_HIGH, "Assault cannon\n");
			return true;
		}
	}
	return false;
}

/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
// ### Hentai ### BEGIN
void ShowGun(edict_t *ent)
{
	int i,j;

	if(!ent->client->pers.weapon)
	{
		ent->s.modelindex2 = 0;
		return;
	}
	if(!vwep->value)
	{
		ent->s.modelindex2 = 255;
		return;
	}

	j = Get_KindWeapon(ent->client->pers.weapon);
	if(j == WEAP_GRAPPLE) j = WEAP_BLASTER;

	ent->s.modelindex2 = 255;
	if (ent->client->pers.weapon)
		i = ((j & 0xff) << 8);
	else
		i = 0;

	ent->s.skinnum = (ent - g_edicts - 1) | i;

}
// ### Hentai ### END

void ChangeWeapon (edict_t *ent)
{
	char *mdl;

	ent->client->refire_frames = 0;
	lasersight_off(ent);
	
	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;
	ent->client->burst_count = 0;

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
//lm ctf
	mdl = ent->client->pers.weapon->view_model;
	ent->client->ps.gunindex = gi.modelindex(mdl/*ent->client->pers.weapon->view_model*/);
//lm ctf

	//K03 Begin
	if (ent->client->pers.weapon == FindItem("Sword"))
		ent->client->ps.gunindex = 0;
	//K03 End

	// ### Hentai ### BEGIN
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
	
	ShowGun(ent);	

	// ### Hentai ### END
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	gitem_t*	item = NULL;

	//GHz START
	ent->client->refire_frames = 0;
	lasersight_off(ent);
	//GHz END

	if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_SLUGS)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_RAILGUN)] )
	{
		item = Fdi_RAILGUN;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_CELLS)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_HYPERBLASTER)] )
	{
		item = Fdi_HYPERBLASTER;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_BULLETS)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_CHAINGUN)] )
	{
		item = Fdi_CHAINGUN;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_BULLETS)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_MACHINEGUN)] )
	{
		item = Fdi_MACHINEGUN;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_SHELLS)] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_SUPERSHOTGUN)] )
	{
		item = Fdi_SUPERSHOTGUN;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_SHELLS)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_SHOTGUN)] )
	{
		item = Fdi_SHOTGUN;
	}
	if(item == NULL) item = Fdi_BLASTER;

	if(ent->svflags & SVF_MONSTER) item->use(ent,item);
	else ent->client->newweapon = item;

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
		// RAFAEL
		is_quadfire = (ent->client->quadfire_framenum > level.framenum);
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

	if(ent->svflags & SVF_MONSTER) 
	{
		if(ent->client->newweapon != NULL) return;
		if(!Q_stricmp (item->pickup_name, "Blaster"))
		{
			ent->client->newweapon = item;
			return;
		}
	}

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			if(!(ent->svflags & SVF_MONSTER)) safe_cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			if(!(ent->svflags & SVF_MONSTER)) safe_cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}

void Use_Weapon2 (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	if(ent->svflags & SVF_MONSTER) 
	{
		Use_Weapon(ent,item);
		return;	
	}

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;
	
	if (item->ammo)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (!ent->client->pers.inventory[ammo_index] && !g_select_empty->value)
		{
			safe_cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
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
		if(!(ent->svflags & SVF_MONSTER)) safe_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
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

void Weapon_Generic2 (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;
	
	//K03 Begin
	// can't use gun if we are just spawning, or we
	// are a morphed player using anything other
	// than a player model
	if(ent->deadflag || (ent->s.modelindex != 255) || (isMorphingPolt(ent))
		|| (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent)) // special rules, fc can't attack
		|| ((ent->client->respawn_time - (0.1*FRAME_ACTIVATE_LAST)) > level.time) // VWep animations screw up corpses
		|| (ent->flags & FL_WORMHOLE)//4.2 can't use weapons in wormhole
		|| ent->shield) // can't use weapons while shield is deployed
		return;
	if (ent->client->ps.gunframe > FRAME_DEACTIVATE_LAST)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = FRAME_IDLE_FIRST;
	}
	//K03 End

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}// ### Hentai ### BEGIN
		else if((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
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
		// ### Hentai ### END

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		// fast weapon switch
		//3.0 weapon masters get free fast weapon switch
		if ((ent->myskills.level >= 10 || ent->myskills.class_num == CLASS_WEAPONMASTER) && (ent->client->newweapon != ent->client->pers.weapon))
			ent->client->ps.gunframe = FRAME_ACTIVATE_LAST;
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
		// fast weapon switch
		//3.0 weapon masters get free fast weapon switch
		ent->client->weaponstate = WEAPON_DROPPING;
		if ((ent->myskills.level >= 10 || ent->myskills.class_num == CLASS_WEAPONMASTER) && (ent->client->newweapon != ent->client->pers.weapon))
		{
			ChangeWeapon(ent);
			return;
		}
		else
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
	
		// ### Hentai ### BEGIN
		if((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
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
		// ### Hentai ### END

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
				//GHz START
				ent->client->idle_frames = 0;
				//4.5 the first shot merely resets the chatprotect state, but doesn't actually fire
				if (ent->flags & FL_CHATPROTECT)
				{
					// reset chat protect and cloaking flags
					ent->flags ^= FL_CHATPROTECT;
					ent->svflags &= ~SVF_NOCLIENT;

					// re-draw weapon
					ent->client->newweapon = ent->client->pers.weapon;
					ent->client->weaponstate = WEAPON_DROPPING;
					ChangeWeapon(ent);

					Teleport_them(ent);
					return;
				}
				//GHz END
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
				if (level.time > ent->client->ctf_techsndtime)
				{
					qboolean quad = false;

					if (ent->client->quad_framenum > level.framenum)
						quad = true;

					if (ent->client->pers.inventory[strength_index])
					{
						if (quad)
							gi.sound(ent, CHAN_ITEM, gi.soundindex("ctf/tech2x.wav"), 1, ATTN_NORM, 0);
						else
							gi.sound(ent, CHAN_ITEM, gi.soundindex("ctf/tech2.wav"), 1, ATTN_NORM, 0);
					}
					else if (ent->client->pers.inventory[haste_index])
						gi.sound(ent, CHAN_ITEM, gi.soundindex("ctf/tech3.wav"), 1, ATTN_NORM, 0);
					else if (quad)
						gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

					ent->client->ctf_techsndtime = level.time + 0.9;
				}

				//K03 Begin
				ent->shots++;
				ent->myskills.shots++;
				if (ent->movetype != MOVETYPE_NOCLIP || (ent->myskills.abilities[CLOAK].current_level == 10 && getTalentLevel(ent, TALENT_IMP_CLOAK) == 4)) // don't uncloak if they are in noclip, or permacloaked due to upgrade levels
					ent->svflags &= ~SVF_NOCLIENT;
				ent->client->cloaking = false;
				ent->client->cloakable = 0;
				//K03 End
				
				fire (ent);
				//gi.dprintf("fired at %d\n",level.framenum);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}

//K03 Begin
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int freezeLevel=0;
	que_t *curse;

	// pick the highest level freeze
	if ((curse = que_findtype(ent->curses, NULL, AURA_HOLYFREEZE)) != NULL)
		freezeLevel = curse->ent->owner->myskills.abilities[HOLY_FREEZE].current_level;
	if (ent->chill_time > level.time && ent->chill_level > freezeLevel)
		freezeLevel = ent->chill_level;

	//3.0 New hfa code (more balanced?)
	if (freezeLevel)
	{
		int Continue;

		//Figure out what frame should be skipped (every x frames)
		switch(freezeLevel)
		{
		case 0:
		case 1:
		case 2:
		case 3:	Continue = 7;	break;
		case 4:
		case 5:	Continue = 6;	break;
		case 6:
		case 7:	Continue = 5;	break;
		case 8:
		case 9:
		default: Continue = 4;	break; // 25% reduced firing rate
		}

		//Examples:
		//If Continue == 2, every second frame is skipped (50% slower firing rate)
		//If Continue == 6, every sixth frame is skipped (17% slower firing rate)

		ent->FrameShot++;
		if (ent->FrameShot >= Continue)
		{
            ent->FrameShot = 0;
			return;
		}
	}

	Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
		FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, fire_frames, fire);
	//ent->FrameShot = 0;

	if (!ent->myskills.abilities[HASTE].disable || ent->client->pers.inventory[haste_index])
	{
		// number of frames to fire an extra shot
		int haste_wait;

		if (ent->myskills.abilities[HASTE].current_level <= 1)
			haste_wait = 5; // 17% improvement (1/1.2)
		else if (ent->myskills.abilities[HASTE].current_level <= 2)
			haste_wait = 4; // 20% improvement (1/1.25)
		else if (ent->myskills.abilities[HASTE].current_level <= 3)
			haste_wait = 3; // 25% improvement (1/1.333)
		else if (ent->myskills.abilities[HASTE].current_level <= 4)
			haste_wait = 2; // 33% improvement (1/1.5)
		else
			haste_wait = 1; // 50% improvement (1/2)

		// haste tech
		if (ent->client->pers.inventory[haste_index])
		{
			if ((haste_wait > 0) && (ent->myskills.level <= 5))
				haste_wait = 0; // 100% improvement (2x firing rate)
			else if ((haste_wait > 1) && (ent->myskills.level <= 10))
				haste_wait = 1;
			else if ((haste_wait > 3))
				haste_wait = 3;
		}

		// if enough frames have passed by, then call the weapon func
		// an additional time
		if (ent->haste_num >= haste_wait)
		{
			Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
				FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, fire_frames, fire);
			ent->haste_num = 0;
		}
		ent->haste_num++;
	}

	//gi.dprintf("gunframe=%d\n", ent->client->ps.gunframe);
}
//K03 End
/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER			3.0
#define GRENADE_MINSPEED		400
#define GRENADE_MAXSPEED		800
#define GRENADE_INITIAL_SPEED	800
#define GRENADE_ADDON_SPEED		40

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = GRENADE_INITIAL_DAMAGE + GRENADE_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_HANDGRENADE].mods[0].current_level;//K03
	int		radius_damage = GRENADE_INITIAL_RADIUS_DAMAGE + GRENADE_ADDON_RADIUS_DAMAGE * ent->myskills.weapons[WEAPON_HANDGRENADE].mods[0].current_level;
	float	timer;
	int		speed, min_speed;
	int		max_speed = GRENADE_INITIAL_SPEED + GRENADE_ADDON_SPEED * ent->myskills.weapons[WEAPON_HANDGRENADE].mods[1].current_level;
	float	radius;

	//3.0 disable chat protect (somehow throwing a hg does not reset a client's idle frames)
	//4.5 the first shot merely resets the chatprotect state, but doesn't actually fire
	if (ent->flags & FL_CHATPROTECT)
	{
		// reset chat protect and cloaking flags
		ent->flags ^= FL_CHATPROTECT;
		ent->svflags &= ~SVF_NOCLIENT;

		// re-draw weapon
		ent->client->newweapon = ent->client->pers.weapon;
		ent->client->weaponstate = WEAPON_DROPPING;
		ChangeWeapon(ent);

		Teleport_them(ent);
		return;
	}

	radius = GRENADE_INITIAL_RADIUS + GRENADE_ADDON_RADIUS * ent->myskills.weapons[WEAPON_HANDGRENADE].mods[2].current_level;//K03
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	min_speed = 0.5 * max_speed;
	speed = min_speed + (GRENADE_TIMER - timer) * ((max_speed - min_speed) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, radius_damage,held);
	//gi.dprintf("fired grenade at %.1f\n", level.time);
	//gi.dprintf("fired grenade at %.1f\n", timer);

	//K03 Begin
	ent->shots++;
	ent->myskills.shots++;
	ent->svflags &= ~SVF_NOCLIENT;
	if (ent->myskills.abilities[CLOAK].current_level < 10 || getTalentLevel(ent, TALENT_IMP_CLOAK) < 4)
	{
		ent->client->cloaking = false;
		ent->client->cloakable = 0;
	}
	//K03 End

	// ### Hentai ### BEGIN

/*	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
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
	}*/
	// ### Hentai ### END

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	//K03 Begin
	if (ent->myskills.weapons[WEAPON_HANDGRENADE].mods[4].current_level < 1)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BLASTER | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	//K03 ENd

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

void Weapon_Grenade2 (edict_t *ent)
{
	if (ent->shield)
		return;
	//gi.dprintf("gunframe = %d\n", ent->client->ps.gunframe);
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
				ent->client->grenade_delay = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					if (ent->myskills.weapons[WEAPON_HANDGRENADE].mods[4].current_level < 1)//K03
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
		if (ent->client->ps.gunframe == 5 && ent->myskills.weapons[WEAPON_HANDGRENADE].mods[4].current_level < 1)//K03
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				if (ent->myskills.weapons[WEAPON_HANDGRENADE].mods[4].current_level < 1)//K03
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

		//ent->client->ps.gunframe++;
		
		// (apple)
		// This will skip half the priming animation, thus shortening
		// the firing delay by about half. To keep the same refire rate
		// a delay is applied after the priming phase. This will keep the
		// HG from going into its next weaponstate until delay is done.
		if(ent->client->ps.gunframe < 11)
		{
			ent->client->ps.gunframe += 2;
			ent->client->grenade_delay++;
		}
		else if(ent->client->grenade_delay > 0)
		{
			ent->client->grenade_delay--;
			ent->client->ps.gunframe++;
		}
		else
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe >= 16 && ent->client->grenade_delay == 0)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

//K03 Begin
void Weapon_Grenade(edict_t *ent)
{
	// can't use gun if we are just spawning, or we
	// are a morphed player using anything other
	// than a player model
	if(ent->deadflag || (ent->s.modelindex != 255) || (isMorphingPolt(ent))
		|| (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
		|| (ent->client->respawn_time > level.time)
		|| (ent->flags & FL_WORMHOLE))
		return;
	Weapon_Grenade2(ent);
	
//	if (ent->myskills.weapons[WEAPON_HANDGRENADE].mods[1].current_level > 9)
//		Weapon_Grenade2(ent);
//	if (ent->myskills.weapons[WEAPON_HANDGRENADE].mods[1].current_level > 4)
//		Weapon_Grenade2(ent);
	
}
//K03 End

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

	int damage = GRENADELAUNCHER_INITIAL_DAMAGE + GRENADELAUNCHER_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[0].current_level;
	float radius = GRENADELAUNCHER_INITIAL_RADIUS + GRENADELAUNCHER_ADDON_RADIUS * ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[1].current_level;
	int speed = GRENADELAUNCHER_INITIAL_SPEED + (GRENADELAUNCHER_ADDON_SPEED * ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[2].current_level);
	int	radius_damage = GRENADELAUNCHER_INITIAL_RADIUS_DAMAGE + GRENADELAUNCHER_ADDON_RADIUS_DAMAGE * ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[0].current_level;

	if (is_quad)
		damage *= 4;

	//GHz: We dont have enough ammo to fire, so change weapon and abort
	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe++;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		NoAmmoWeaponChange(ent);
		return;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	//K03 Begin
	if ((ent->max_pipes < MAX_PIPES) || !ent->client->weapon_mode)
	{
		fire_grenade (ent, start, forward, damage, speed, 2.5, radius, radius_damage);
		if (! ((int)dmflags->value & DF_INFINITE_AMMO) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
		if (ent->client->weapon_mode)
			ent->max_pipes++;
	}

	if (ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[4].current_level < 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_GRENADE | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
	//K03 End

	ent->client->ps.gunframe++;

	//K03 Begin
	if (ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	//K03 End
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	int fire_last = 13;//16;
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, fire_last, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);

	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/
void fire_lockon_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);


void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	//K03 Begin
	int speed = ROCKETLAUNCHER_INITIAL_SPEED + ROCKETLAUNCHER_ADDON_SPEED * ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[2].current_level;
	damage = ROCKETLAUNCHER_INITIAL_DAMAGE + ROCKETLAUNCHER_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[0].current_level;
	radius_damage = ROCKETLAUNCHER_INITIAL_RADIUS_DAMAGE + ROCKETLAUNCHER_ADDON_RADIUS_DAMAGE * ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[0].current_level;
	damage_radius = ROCKETLAUNCHER_INITIAL_DAMAGE_RADIUS + ROCKETLAUNCHER_ADDON_DAMAGE_RADIUS * ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[1].current_level;
	//K03 End
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
	//K03 Begin
	//gi.dprintf("called rocketlauncher_fire at %d (%d)\n", level.framenum, ent->client->ps.gunframe);
	fire_rocket (ent, start, forward, damage, speed, damage_radius, radius_damage);
	
	if (ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[4].current_level < 1)
		{
			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_ROCKET | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
	//K03 End

	ent->client->ps.gunframe++;

	if (ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[4].current_level < 1)//K03
		PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	//K03 Begin
	int fire_last=12;

	Weapon_Generic (ent, 4, fire_last, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
	//K03 End
	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
	
}


/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyperblaster, int effect, int speed)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	//K03 Begin
	//trace_t tr;
	//vec3_t end;
	//K03 End

	//gi.dprintf("blaster_fire()\n");
	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);

	if(!(ent->svflags & SVF_MONSTER))
	{
		VectorAdd (offset, g_offset, offset);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
	}
	else
	{
		VectorSet(offset, 0, 0, ent->viewheight-8);
		VectorAdd (offset, ent->s.origin, start);
	}

	// blast mode?
	if (!hyperblaster && ent->client && ent->client->weapon_mode)
		fire_blaster(ent, start, forward, damage, speed, effect, BLASTER_PROJ_BLAST, MOD_BLASTER, 2.0, true);
	// hyperblaster shot?
	else if (hyperblaster)
		fire_blaster(ent, start, forward, damage, speed, effect, BLASTER_PROJ_BOLT, MOD_HYPERBLASTER, 2.0, false);
	// normal blaster shot
	else
		fire_blaster(ent, start, forward, damage, speed, effect, BLASTER_PROJ_BOLT, MOD_BLASTER, 2.0, true);

	if (hyperblaster && ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[4].current_level < 1)
	{
		if (ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[4].current_level)
			is_silenced = MZ_SILENCED;
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	else if (!hyperblaster)
	{
		// send muzzle flash
		if (ent->client && ent->client->weapon_mode)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_IONRIPPER | MZ_SILENCED);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
			if (ent->myskills.weapons[WEAPON_BLASTER].mods[4].current_level < 1)
				gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/photon.wav"), 1, ATTN_NORM, 0);
			//else
				//gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/photon.wav"), 0.3, ATTN_NORM, 0);
		}
		else
		{
			if (ent->myskills.weapons[WEAPON_BLASTER].mods[4].current_level > 0)
				is_silenced = MZ_SILENCED;

			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BLASTER | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
	}
	
	if (hyperblaster && ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	else if (!hyperblaster && ent->myskills.weapons[WEAPON_BLASTER].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	//K03 End
}


void Weapon_Blaster_Fire (edict_t *ent)
{
	int	min, max, damage, effect, ammo;
	int	speed = BLASTER_INITIAL_SPEED + BLASTER_ADDON_SPEED * ent->myskills.weapons[WEAPON_BLASTER].mods[2].current_level;
	float	temp;

	if (ent->myskills.weapons[WEAPON_BLASTER].mods[3].current_level < 1)
		effect = EF_BLASTER;
	else
		effect = EF_HYPERBLASTER;

	min = BLASTER_INITIAL_DAMAGE_MIN + (BLASTER_ADDON_DAMAGE_MIN * ent->myskills.weapons[WEAPON_BLASTER].mods[0].current_level);
	max = BLASTER_INITIAL_DAMAGE_MAX + (BLASTER_ADDON_DAMAGE_MAX * ent->myskills.weapons[WEAPON_BLASTER].mods[0].current_level);
	damage = GetRandom(min, max);

	
	if (ent->client->weapon_mode)
	{
		temp = (float) ent->client->refire_frames / 10;
		if (temp > 5)
			temp = 5;
		damage *= temp;

		// ammo
		ammo = floattoint(temp);
		if (ammo < 1)
			ammo = 1;
	}
	else
		ammo = 1;

	// insufficient ammo
	if (ent->monsterinfo.lefty < ammo)
	{
		// play no ammo sound
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}

		// keep weapon ready
		ent->client->ps.gunframe = 10;
		return;
	}

	ent->monsterinfo.lefty -= ammo; // decrement ammo counter

	if (ent->client->weapon_mode)
		safe_cprintf(ent, PRINT_HIGH, "%d damage blaster bolt fired (%.1fx).\n", damage, temp);

	Blaster_Fire (ent, vec3_origin, damage, false, effect, speed);
	ent->client->ps.gunframe++;
	ent->client->refire_frames = 0;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};
//GHz START
	if (ent->mtype)
		return; // morph does not use weapons

	// are we in secondary mode?
	if (ent->client->weapon_mode)
	{
		// fire when button is released
		if (!(ent->client->buttons & BUTTON_ATTACK))
		{
			if ((ent->client->ps.gunframe > 9) && (ent->client->refire_frames >= 10))
				ent->client->buttons |= BUTTON_ATTACK;
			else
				ent->client->refire_frames = 0;
		}
		else
		{
			// charge up weapon
			if (ent->client->refire_frames == 50)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				gi.centerprintf(ent, "Blaster fully charged.\n(5.0x damage)\n");
			}
			
			if (ent->client->refire_frames == 25)
				gi.centerprintf(ent, "Blaster 50%c charged.\n(2.5x damage)\n", '%');
			// dont fire yet
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->refire_frames++;
		}
	}
//GHz END
	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}

void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;
//GHz START
	int			i;
	int			speed, shots = 0;
	qboolean	fire_this_frame = false;

	// only fire every other frame
	//if (ent->client->ps.gunframe == 6 || ent->client->ps.gunframe == 9
	//	|| ent->client->ps.gunframe == 15)
	//{
		fire_this_frame = true;
		shots++;
	//}
	// get weapon properties
	damage = HYPERBLASTER_INITIAL_DAMAGE + HYPERBLASTER_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[0].current_level;
	speed = HYPERBLASTER_INITIAL_SPEED + HYPERBLASTER_ADDON_SPEED * ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[2].current_level;

	if (ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[4].current_level)
		is_silenced = MZ_SILENCED;
//GHz END

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		//GHz START
		if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
			shots = ent->client->pers.inventory[ent->client->ammo_index];

		if (!shots && !ent->client->pers.inventory[ent->client->ammo_index])
		//GHz END
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
			//GHz START
			for (i=0; i<shots; i++)
			{
				//gi.dprintf("Fired HB for %d damage at %.1f\n", damage, level.time);

				rotation = (ent->client->ps.gunframe + i - 5) * 2*M_PI/6;
				offset[0] = -4 * sin(rotation);
				offset[1] = 0;
				offset[2] = 4 * cos(rotation);

				if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
					effect = EF_HYPERBLASTER;
				else
					effect = 0;
				
				Blaster_Fire (ent, offset, damage, true, effect, speed);

				if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
					ent->client->pers.inventory[ent->client->ammo_index]--;
			}
			//GHz END

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

void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);

	// RAFAEL
	if (is_quadfire)
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
	int			kick = 2;
	vec3_t		offset;
	float		damage =	MACHINEGUN_INITIAL_DAMAGE + MACHINEGUN_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_MACHINEGUN].mods[0].current_level;
	int			vspread =	DEFAULT_BULLET_VSPREAD;
	int			hspread =	DEFAULT_BULLET_HSPREAD;
	int			shots=1;

	if (ent->myskills.weapons[WEAPON_MACHINEGUN].mods[4].current_level)
		is_silenced = MZ_SILENCED;

	// bullet spread is reduced while in burst mode
	if (ent->client->weapon_mode)
	{
		vspread *= 0.5;
		hspread *= 0.5;
	}
	// bullet spread is reduced when mg is upgraded
	if (ent->myskills.weapons[WEAPON_MACHINEGUN].mods[3].current_level >= 1)
	{
		vspread *= 0.75;
		hspread *= 0.75;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}
	// keep track of burst bullets
	if (ent->client->weapon_mode)
		ent->client->burst_count++;

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];
	if (!shots && !ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		ent->client->burst_count = 0;
		NoAmmoWeaponChange(ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1;i<3;i++)
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
	//K03 Begin
	if (ent->client->weapon_mode)
	{
	//	shots *= 2;
		damage *= 2;
	}
	for (i=0;i<shots;i++)
	{
		fire_bullet (ent, start, forward, damage, kick, hspread, vspread, MOD_MACHINEGUN);
	}
	// fire tracers
	if (ent->lasthbshot <= level.time)
	{
		if (ent->myskills.weapons[WEAPON_MACHINEGUN].mods[2].current_level >= 1)
		{
			damage = MACHINEGUN_ADDON_TRACERDAMAGE * ent->myskills.weapons[WEAPON_MACHINEGUN].mods[2].current_level;
			fire_blaster(ent, start, forward, damage, 2000, EF_BLUEHYPERBLASTER, BLASTER_PROJ_BOLT, MOD_HYPERBLASTER, 2.0, false);
		}
		ent->lasthbshot = level.time + 0.5;
	}

	if (is_silenced)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/mg_silenced.wav"), 0.5, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/mg_unsilenced.wav"), 1, ATTN_NORM, 0);

	if (ent->myskills.weapons[WEAPON_MACHINEGUN].mods[4].current_level < 1)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_IONRIPPER|MZ_SILENCED);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	if (ent->myskills.weapons[WEAPON_MACHINEGUN].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	//K03 End

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;

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
ent->client->weaponstate = WEAPON_READY;
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	// burst fire mode fires 5 shots and then waits 0.5 seconds to fire another burst
	if (ent->client->weapon_mode)
	{

		if (!(ent->client->buttons & BUTTON_ATTACK))
		{
			// reset counters if we have completed a burst or are idle
			if (ent->client->burst_count > 4 || ent->client->burst_count == 0)
			{
				ent->client->machinegun_shots = 0;
				ent->client->burst_count = 0;
			}
			else
			{
				// continue burst if it wasn't completed
				if (ent->client->burst_count < 5)
				{
					ent->client->buttons |= BUTTON_ATTACK;
					ent->client->trap_time = level.time + 1.0;
				}
			}
		}
		else
		{
			// delay between each burst
			if (ent->client->trap_time > level.time)
			{
				ent->client->burst_count = 0;
				ent->client->buttons &= ~BUTTON_ATTACK;
				ent->client->latched_buttons &= ~BUTTON_ATTACK;
			}
			// burst completed
			if (ent->client->burst_count >= 4)
				ent->client->trap_time = level.time + 1.0;
		}
	}
	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);

	// RAFAEL
	if (is_quadfire)
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
	//int			kick = 2;

	//K03 Begin
	float		damage = CHAINGUN_INITIAL_DAMAGE + CHAINGUN_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_CHAINGUN].mods[0].current_level;

	int			vspread=DEFAULT_BULLET_VSPREAD;
	int			hspread=DEFAULT_BULLET_HSPREAD;

	if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[3].current_level >= 1)
	{
		vspread *= 0.75;
		hspread *= 0.75;
	}
	//K03 End

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		ent->client->weaponstate = WEAPON_READY;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		if (ent->client->ps.gunframe < 64)//K03
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

	if (ent->client->ps.gunframe <= 9)
		shots = 2;
	else 
		if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 3;
		else
			shots = 2;
	}
	else
		shots = 4;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
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
		damage *= 4;

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

		fire_bullet (ent, start, forward, damage, 5, hspread, vspread, MOD_CHAINGUN);
	}

	//K03 begin
	if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[2].current_level >= 1)
		if (ent->lasthbshot <= level.time)
		{
			damage = CHAINGUN_ADDON_TRACERDAMAGE * ent->myskills.weapons[WEAPON_CHAINGUN].mods[2].current_level;
			fire_blaster(ent, start, forward, damage, 2000, EF_BLUEHYPERBLASTER, BLASTER_PROJ_BOLT, MOD_HYPERBLASTER, 2.0, false);
			ent->lasthbshot = level.time + 0.5;
		}
	if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[4].current_level < 1)
	{
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	//K03 End

	// ### Hentai ### BEGIN

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
		ent->client->anim_end = FRAME_attack8;
	}


	// ### Hentai ### END

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}

void AssaultCannon_Fire (edict_t *ent)
{
	int			shots, i;
	int			damage, kick, vspread, hspread;
	float		f, r, u;
	qboolean	canfire=false;
	vec3_t		forward, right, start, up, offset;

	damage = kick = 2*(CHAINGUN_INITIAL_DAMAGE+CHAINGUN_ADDON_DAMAGE*ent->myskills.weapons[WEAPON_CHAINGUN].mods[0].current_level);
	vspread = DEFAULT_BULLET_VSPREAD;
	hspread = DEFAULT_BULLET_HSPREAD;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[3].current_level > 0)
	{
		vspread *= 0.75;
		hspread *= 0.75;
	}

	if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[4].current_level > 0)
		f = 0.3;
	else
		f = 1;

	if (ent->client->ps.gunframe <= 14)
	{
		// beginning animation, so play spin-up sound
		if (ent->client->ps.gunframe == 5)
			gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/asscan_spinup.wav"), f, ATTN_NORM, 0);
		ent->client->ps.gunframe++; // we're done, so advance to next frame
		return;
	}
	else if ((ent->client->ps.gunframe == 15) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		ent->client->weaponstate = WEAPON_READY;
		return;
	}
	// attack frames loop
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15; // go to beginning of fire frames
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->groundentity && (VectorLength(ent->velocity) < 1))
		canfire = true;

	if (ent->client->ps.gunframe == 22)
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/asscan_endfire.wav"), f, ATTN_NORM, 0);
	else if (!(level.framenum % 2))
	{
		if (canfire)
			gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/asscan_fire.wav"), f, ATTN_NORM, 0);
		else
			gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/asscan_pause.wav"), f, ATTN_NORM, 0);
	}
	
	if (!canfire)
		return;

	shots = 4;

	// if we don't have enough ammo, use whatever we have
	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	// out of ammo
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

	// change player view
	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 1.5;
		ent->client->kick_angles[i] = crandom() * 3;
	}

	if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[4].current_level < 1)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_IONRIPPER|MZ_SILENCED);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
	
	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, hspread, vspread, MOD_CHAINGUN);
	}

	if ((ent->myskills.weapons[WEAPON_CHAINGUN].mods[2].current_level > 0) && (level.time >= ent->lasthbshot))
	{
		damage = CHAINGUN_ADDON_TRACERDAMAGE*ent->myskills.weapons[WEAPON_CHAINGUN].mods[2].current_level;
		fire_blaster(ent, start, forward, damage, 2000, 0, BLASTER_PROJ_BOLT, MOD_HYPERBLASTER, 2.0, false);
		ent->lasthbshot = level.time + 0.3;
	}

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}

void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	
	//K03 Begin
	int fire_last=31;
	
	// spin-up delay
	if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[1].current_level > 0)
	{
		if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[1].current_level > 9)
			fire_last = 21;
		else if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[1].current_level > 7)
			fire_last = 23;
		else if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[1].current_level > 5)
			fire_last = 25;
		else if (ent->myskills.weapons[WEAPON_CHAINGUN].mods[1].current_level > 2)
			fire_last = 27;
		else fire_last = 29;
	}
	
	if (ent->client->weapon_mode)
		Weapon_Generic (ent, 4, fire_last, 61, 64, pause_frames, fire_frames, AssaultCannon_Fire);
	else
		Weapon_Generic (ent, 4, fire_last, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
	//K03 End

	// RAFAEL
	if (is_quadfire)
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
	int			kick = 8;
	float		temp;

	//K03 Begin
	float		damage = SHOTGUN_INITIAL_DAMAGE + SHOTGUN_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_SHOTGUN].mods[0].current_level;
	int			vspread = 500;
	int			hspread = 500;
	int			bullets = SHOTGUN_INITIAL_BULLETS + SHOTGUN_ADDON_BULLETS * ent->myskills.weapons[WEAPON_SHOTGUN].mods[2].current_level;
	if (ent->myskills.weapons[WEAPON_SHOTGUN].mods[3].current_level >= 1)
	{
		vspread *= 0.75;
		hspread *= 0.75;
	}
	
	//K03 End
	
	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}
	
	//gi.dprintf("fired shotgun at %d\n", level.framenum);

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

	// shotgun strike upgrade
	// 10% chance to deal double damage at level 10
	temp = 1.0 / (1.0 + 0.0222*ent->myskills.weapons[WEAPON_SHOTGUN].mods[1].current_level);

	if (random() > temp)
	{
		damage *= 2;
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("ctf/tech2.wav"), 1, ATTN_NORM, 0);
	}

	fire_shotgun (ent, start, forward, damage, kick, vspread, hspread, bullets, MOD_SHOTGUN);
	// send muzzle flash
	if (ent->myskills.weapons[WEAPON_SHOTGUN].mods[4].current_level < 1)
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SHOTGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	//K03 End

	ent->client->ps.gunframe++;
	//K03 Begin
	if (ent->myskills.weapons[WEAPON_SHOTGUN].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	//K03 End

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	//K03 Begin
	int fire_last=16;
	/*
	if (ent->myskills.weapons[WEAPON_SHOTGUN].mods[1].current_level > 0)
	{
		if (ent->myskills.weapons[WEAPON_SHOTGUN].mods[1].current_level > 9)
			fire_last = 14;
		else if (ent->myskills.weapons[WEAPON_SHOTGUN].mods[1].current_level > 6)
			fire_last = 15;
		else if (ent->myskills.weapons[WEAPON_SHOTGUN].mods[1].current_level > 3)
			fire_last = 16;
		else fire_last = 17;
	}*/

	Weapon_Generic (ent, 7, fire_last, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
	//K03 End

	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			kick = 12;
	//K03 Begin
	float		damage = SUPERSHOTGUN_INITIAL_DAMAGE + SUPERSHOTGUN_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[0].current_level;
	int			vspread = DEFAULT_SHOTGUN_VSPREAD;
	int			hspread = DEFAULT_SHOTGUN_HSPREAD;
	int			bullets = SUPERSHOTGUN_INITIAL_BULLETS + SUPERSHOTGUN_ADDON_BULLETS * ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[2].current_level;
//	float		temp;
	
	if (ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[3].current_level >= 1)
	{
		vspread *= 0.75;
		hspread *= 0.75;
	}
	
	//K03 End

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

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	/*
	// special handling of pellet weapons with ghost
	temp = 0.033 * ent->myskills.ghost_level;
	if (temp >= random())
		damage = 0;
	*/

	bullets = ceil((float)bullets/2);
//	gi.dprintf("%d dmg %d bullets\n", damage, bullets);
	fire_shotgun (ent, start, forward, damage, kick, hspread, vspread, bullets, MOD_SSHOTGUN);//K03
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, hspread, vspread, bullets, MOD_SSHOTGUN);//K03

	//K03 Begin
	if (ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[4].current_level < 1)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SSHOTGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	//K03 End

	ent->client->ps.gunframe++;
	//K03 Begin
	if (ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	//K03 End

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
	//K03 End

	// RAFAEL
	if (is_quadfire)
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
	int			kick=200;

	//K03 Begin
	int			damage = RAILGUN_INITIAL_DAMAGE + RAILGUN_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_RAILGUN].mods[0].current_level;
	//K03 End

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// sniper shots deal massive damage
	if (ent->client->weapon_mode)
	{
		damage *= 3;
		is_silenced = MZ_SILENCED;
	}
		
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_rail (ent, start, forward, damage, kick);
	
	if (ent->myskills.weapons[WEAPON_RAILGUN].mods[4].current_level < 1)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_RAILGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	//K03 End

	ent->client->ps.gunframe++;
	//K03 Begin
	if (ent->myskills.weapons[WEAPON_RAILGUN].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	//K03 End

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->refire_frames = 0;
}

void fire_20mm (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void weapon_20mm_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;

	int damage;//25 + (int) floor(2.5 * ent->myskills.weapons[WEAPON_20MM].mods[0].current_level);
	int kick;//= 150 - (100 * ent->myskills.weapons[WEAPON_20MM].mods[3].current_level);

	//min = WEAPON_20MM_INITIAL_DMG_MIN + WEAPON_20MM_ADDON_DMG_MIN*ent->myskills.weapons[WEAPON_20MM].mods[0].current_level;
	//max = WEAPON_20MM_INITIAL_DMG_MAX + WEAPON_20MM_ADDON_DMG_MAX*ent->myskills.weapons[WEAPON_20MM].mods[0].current_level;
	//damage = GetRandom(min, max);

	//4.57
	damage = WEAPON_20MM_INITIAL_DMG + WEAPON_20MM_ADDON_DMG * ent->myskills.weapons[WEAPON_20MM].mods[0].current_level;
	kick = damage;
	if (ent->myskills.weapons[WEAPON_20MM].mods[3].current_level)
		kick *= 0.5;
	//if (kick < 0)
	//	kick = 0;

	if (!ent->groundentity && !ent->waterlevel){
		ent->client->ps.gunframe = 5;
		if (ent->client && !(ent->svflags & SVF_MONSTER))
			safe_cprintf(ent, PRINT_HIGH, "You must be stepping on the ground or in water to fire the 20mm cannon.\n");
		return;
	}

	if (is_quad)
	{
		damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	//gi.dprintf("called fire_20mm() at %f for %d damage\n", level.time, damage);
	fire_20mm (ent, start, forward, damage, kick);
	
	if (ent->myskills.weapons[WEAPON_20MM].mods[4].current_level < 1)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_IONRIPPER|MZ_SILENCED);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}


	ent->client->ps.gunframe++;

	if (ent->myskills.weapons[WEAPON_20MM].mods[4].current_level < 1)
	{
		PlayerNoise(ent, start, PNOISE_WEAPON);
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/sgun1.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/sgun1.wav"), 0.3, ATTN_NORM, 0);
	}

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};	
	int			fire_last = 18;
	
	if (ent->mtype)
		return;

	// are we in sniper mode?
	if (ent->client->weapon_mode)
	{
		// fire when button is released
		if (!(ent->client->buttons & BUTTON_ATTACK))
		{
			if (ent->client->ps.gunframe > (fire_last+1) && ent->client->refire_frames >= 2*(fire_last-2))
				ent->client->buttons |= BUTTON_ATTACK;
			else
				ent->client->refire_frames = 0;
			ent->client->snipertime = 0;//4.5
			lasersight_off(ent);
		}
		else
		{
			// charge up weapon
			if (ent->client->refire_frames == 2*(fire_last-2))
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			// make sure laser sight is on to alert enemies
			if ((ent->client->refire_frames >= 2*(fire_last-2)) && !ent->lasersight)
				lasersight_on(ent);
			// dont fire yet
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->refire_frames++;
			ent->client->snipertime = level.time + FRAMETIME;
		}
	}
	Weapon_Generic (ent, 3, fire_last, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}

void Weapon_20mm (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};
		
	//K03 Begin
	int fire_last = 18;

	Weapon_Generic (ent, 3, 4, 56, 61, pause_frames, fire_frames, weapon_20mm_fire);
	//K03 End

	// RAFAEL
	if (is_quadfire)
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
	int		dmg, speed;
	float	range;

	speed = BFG10K_INITIAL_SPEED + BFG10K_ADDON_SPEED * ent->myskills.weapons[WEAPON_BFG10K].mods[2].current_level;
	range = BFG10K_RADIUS;
	dmg = BFG10K_INITIAL_DAMAGE + BFG10K_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_BFG10K].mods[0].current_level;

	if (is_quad)
		dmg *= 4;

	if (ent->client->ps.gunframe == 9)
	{
	//	gi.dprintf("fired bfg at %.1f\n", level.time);
		// send muzzle flash
		if (ent->myskills.weapons[WEAPON_BFG10K].mods[4].current_level < 1) {
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BFG | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

		ent->client->ps.gunframe++;

		if (ent->client->pers.inventory[ent->client->ammo_index] >= 50) {
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorScale (forward, -2, ent->client->kick_origin);

			// make a big pitch kick with an inverse fall
			ent->client->v_dmg_pitch = -20;
			ent->client->v_dmg_roll = crandom()*4;
			ent->client->v_dmg_time = level.time + DAMAGE_TIME;

			VectorSet(offset, 8, 8, ent->viewheight-8);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

			if (ent->myskills.weapons[WEAPON_BFG10K].mods[4].current_level < 1)
				PlayerNoise(ent, start, PNOISE_WEAPON);
			fire_bfg (ent, start, forward, dmg, speed, range);

			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index] -= 50;
		}
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
/*
	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, dmg, speed, range); */

	ent->client->ps.gunframe++;

	/*if (ent->myskills.weapons[WEAPON_BFG10K].mods[4].current_level < 1)
		PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 25;*/
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};
	
	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
	//K03 Endda
	// RAFAEL
//	if (is_quadfire)
//		Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}
