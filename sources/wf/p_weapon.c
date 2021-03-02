// g_weapon.c

#include "g_local.h"
//ERASER START
#include "m_player.h"
#include "bot_procs.h"
void	botPickBestWeapon(edict_t *self);
//ERASER END

void fire_slowgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_plasmabeam (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_tesla (edict_t *self, vec3_t start, vec3_t aimdir, int damage_multiplier, int speed);
void fire_gasgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_blaster_wf (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);
void weapon_knife_fire (edict_t *ent);
int wfWeaponDamage(gitem_t *it);


//static qboolean	is_quad;
qboolean	is_quad;
byte		is_silenced;


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
			//Dont decrease # of silenced shots if the class
			//is assigned it as perminent
			if ((who->client) && (who->client->player_items & ITEM_SILENCER))
				who->client->silencer_shots = 999;
			else
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
	gclient_t	*client;//ERASER
//ERASER START
	if (other->client)
		client = other->client;
	else
		return false;
//ERASER END

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

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
		other->client->newweapon = ent->item;

//ERASER START
	// check for bot change weapon//FIXME ACRID THIS IS THE ONLY WORKING ONE
	if (other->bot_client)
	{
		botPickBestWeapon(other);
	}
	return true;
}
void ShowGun(edict_t *ent)//eraser vwep 3.20 support
{
	int i;
	// set visible model for bots vwep
	if (ent->s.modelindex == 255) 
	{
		if (ent->client->pers.weapon)
		{
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		}
		else
		{
			i = 0;
		}
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}
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
	int	weapon;

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
		{
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		}
		else
		{
			i = 0;
		}
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

	// Update weapon damage icon
	ent->client->weapon_damage = wfWeaponDamage(ent->client->pers.weapon);

	//special case for grenades and grapple
	if (Q_stricmp("ammo_grenades", ent->client->pers.weapon->classname ) == 0)
	{
		ent->client->weapon_damage = wf_game.grenade_damage[ent->client->pers.grenade_type];
	}
	else if (Q_stricmp("ammo_grenades2", ent->client->pers.weapon->classname ) == 0)
	{
		ent->client->weapon_damage = wf_game.grenade_damage[ent->client->pers.grenade_type];
	}
	else if (Q_stricmp("ammo_grenades3", ent->client->pers.weapon->classname ) == 0)
	{
		ent->client->weapon_damage = wf_game.grenade_damage[ent->client->pers.grenade_type];
	}

	
	//Turn on/off laser sight if needed.  Not on if homing is not on
	weapon = ent->client->pers.weapon->tag;
	if ((weapon == WEAPON_NAPALMMISSLE) && ( ent->client->pers.homing_state) )
		lasersight_on (ent);
	else
		lasersight_off (ent);


}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
//ERASER????? FIXME
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;
//ERASER???? END

	lasersight_off (ent);

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

		//WF
//		if (ent->client->player_items & ITEM_SILENCER)
//			is_silenced = MZ_SILENCED;	//Always silenced!
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

	//K2:Camera crash fix  acrid 3/99 botcam
	if (ent->client->bIsCamera)
		return;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		if (ammo_item == 0)
		{
			safe_cprintf (ent, PRINT_HIGH, "Ammo %s does not exist for %s.\n", item->ammo, item->pickup_name);
			return;
		}
		ammo_index = ITEM_INDEX(ammo_item);

			if (!ent->client->pers.inventory[ammo_index])
			{
				safe_cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
				return;
			}


		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			safe_cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}

//WF - Special Use function for grapple
void Use_Grapple (edict_t *ent, gitem_t *item)
{
	//Has server turned off grappling on server?
	if ((int)wfflags->value & WF_NO_GRAPPLE) 
	{
		safe_cprintf(ent, PRINT_HIGH, "Sorry - The grapple is turned off on this server.\n");
		return;
	}

	//Is this class capable of using the grapple?
	if ((ent->client) &&  ((ent->client->player_special & SPECIAL_GRAPPLE) == 0))
	{
		safe_cprintf(ent, PRINT_HIGH, "Sorry - This class cannot use the grapple.\n");
		return;
	}

	Use_Weapon(ent, item);
}


/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	safe_cprintf (ent, PRINT_HIGH, "Sorry, you can't drop a weapon in WF!\n");
	return;

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
	int i;
	vec3_t	oldorg, oldangles;
//	char	*s;
//WF34 START
	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}
//WF34 END
	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
//WF34 START
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
//WF34 END
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
//WF24 S
//		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST || fastswitch)
//WF24 E
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
//WF24 S
//		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		if (fastswitch)
			ent->client->ps.gunframe = FRAME_DEACTIVATE_LAST;
		else
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
//WF24 E
//WF34 START 3.20
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
//WF34 END 3.20

		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
//WF24 S & K2:Begin
			//If respawn protected, remove it because player pressed fire/attack
			if (K2_IsProtected(ent))
				ent->client->protecttime = 0;
//WF24 E & K2:End

			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
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
//ERASER START
				if (ent->client->latency > 0)
				{	// simulate lag

					// find the best lag_trail location to use
					i = (int) (ent->client->latency/100);
					if (i > 9)
						i = 9;

					VectorCopy(ent->s.origin, oldorg);
					VectorCopy(ent->client->v_angle, oldangles);

					VectorCopy((*ent->client->lag_trail)[i], ent->s.origin);
					VectorCopy((*ent->client->lag_angles)[i], ent->client->v_angle);
					gi.linkentity(ent);
				}
//ERASER END
//ZOID
				if (!CTFApplyStrengthSound(ent))
//ZOID
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
//ZOID
				CTFApplyHasteSound(ent);
//ZOID
				if (ent->disguiseshots && fire != weapon_knife_fire)//WF34
				{
					ent->disguiseshots--;//WF34
				}

				if (ent->client->latency > 0)
				{	// simulate lag
					fire (ent);//ERASER START

					VectorCopy(oldorg, ent->s.origin);
					VectorCopy(oldangles, ent->client->v_angle);

					gi.linkentity(ent);

					break;
				}
				else
				{

					fire (ent);
					break;
				}//ERASER END
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

	if(ent->Slower<level.time)//WF34 LINE
		Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,	FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, fire_frames, fire);
	else
		ent->FrameShot++;//WF34 2 LINES

	// run the weapon frame again if hasted
	if (stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate == WEAPON_FIRING)
		return;

	if ((CTFApplyHaste(ent) ||
		(Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 
		&& ent->client->weaponstate != WEAPON_FIRING)) 
		&& oldstate == ent->client->weaponstate)
	{
		if(ent->Slower<level.time)//WF34 LINE
			Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,	FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, fire_frames, fire);
		else
			ent->FrameShot++;//WF34 2 LINES
	}//WF34 START
	if (ent->Slower>level.time)
	{
		if(ent->FrameShot>1)
		{
			ent->FrameShot=0;
			Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,	FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, fire_frames, fire);
		}//WF34 END
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

int fireButtonOn(edict_t *ent)
{
	if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
		return 1;
	else
		return 0;
}

char *getWeaponState(edict_t *ent)
{
	if (ent->client->weaponstate == WEAPON_READY) return "WEAPON_READY";
	else if (ent->client->weaponstate == WEAPON_ACTIVATING ) return "WEAPON_ACTIVATING";
	else if (ent->client->weaponstate == WEAPON_DROPPING ) return "WEAPON_DROPPING";
	else if (ent->client->weaponstate == WEAPON_FIRING ) return "WEAPON_FIRING";
	else return "(unknown)";
}

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
//	char	*s;
	int		damage = wf_game.grenade_damage[GRENADE_TYPE_NORMAL];
//WF
	int		lbdamage = 15; //reduce damage
//WF
	float	timer;
	int		speed;
	float	radius;

//if (wfdebug)
//{
//gi.dprintf("Grenade fire: held=%d, frame = %d, %s, fire=%d, grenadetype=%d\n",
//		   held, ent->client->ps.gunframe, getWeaponState(ent),
//		   fireButtonOn(ent), ent->client->pers.grenade_type);
//}
	//Remove respawn protection and disguise
	if (ent->disguised)
	{
		WFRemoveDisguise(ent);
	}

	if (K2_IsProtected(ent))
		ent->client->protecttime = 0;


	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
//	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((wf_game.grenade_speed[ent->client->pers.grenade_type] - GRENADE_MINSPEED) / GRENADE_TIMER);

	if (speed < 0) speed = 200;

//WF - throw the right kind of grenade
	switch( ent->client->pers.grenade_type ) 
	{
		case GRENADE_TYPE_LASERBALL:
			fire_laserball (ent, start, forward, lbdamage, speed, timer, radius);
			break;
		case GRENADE_TYPE_GOODYEAR:
			fire_goodyear (ent, start, forward, damage, speed, timer, radius);
			break;
	    case GRENADE_TYPE_NORMAL:
			fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);
			break;
	    case GRENADE_TYPE_FLASH:
			fire_flash (ent, start, forward, damage, speed, timer, radius);
			break;
	    case GRENADE_TYPE_PROXIMITY:
			fire_proximity (ent, start, forward, damage, speed, timer, radius, PROXIMITY_TYPE_GRENADE);
			break;
	    case GRENADE_TYPE_CLUSTER:
            fire_cluster  (ent, start, forward, damage, speed, timer, radius);
			break;
	    case GRENADE_TYPE_PIPEBOMB:
			fire_pipebomb (ent, start, forward, damage, speed, timer, radius);
			break;
	    case GRENADE_TYPE_EARTHQUAKE:
			fire_earthquake_grenade (ent, start, forward, damage, speed, timer, radius);
			break;
	    case GRENADE_TYPE_TURRET:
			fire_turret_grenade (ent, start, forward, damage, speed, timer, radius, held);
			break;
	    case GRENADE_TYPE_NAPALM:
			fire_napalm  (ent, start, forward, damage, speed, timer, radius);
			break;
	    case GRENADE_TYPE_CONCUSSION:
			fire_concussiongrenade (ent, start, forward, damage, speed, 2.5, radius);
			break;
//	    case GRENADE_TYPE_NARCOTIC:
//			fire_napalm  (ent, start, forward, damage, speed, timer, radius);
//			break;
	    case GRENADE_TYPE_PLAGUE:
			fire_diseasegrenade (ent, start, forward, damage, speed, 2.5, radius);
			break;
	    case GRENADE_TYPE_MAGNOTRON:
			fire_magnogrenade (ent, start, forward, damage, speed, 2.5, radius);
			break;
	    case GRENADE_TYPE_SHOCK:
			fire_shockgrenade (ent, start, forward, damage, speed, 2.5, radius);
			break;
	    case GRENADE_TYPE_SHRAPNEL:
			fire_bulletgrenade (ent, start, forward, damage, speed, 2.5, radius);
			break;
	    case GRENADE_TYPE_FLARE:
			fire_flare (ent, start, forward, damage, speed, 2.5, radius);
			break;
	    case GRENADE_TYPE_SLOW:
			fire_slowgrenade  (ent, start, forward, damage, speed, timer, radius);
			break;
	    case GRENADE_TYPE_LASERCUTTER:
			fire_laser_grenade (ent, start, forward, damage, speed, 2.5, radius, held);
			break;
	    case GRENADE_TYPE_GAS:
			fire_gasgrenade (ent, start, forward, damage, speed, 2.5, radius);
			break;
	    case GRENADE_TYPE_TESLA:
			fire_tesla (ent, start, forward, damage, speed);
			break;
	    default :
			fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);
			safe_cprintf (ent, PRINT_HIGH, "Unknown Type, using Normal Grenade\n");
			break;
	}

//	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);
//WF	

	
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		if (ent->client->pers.grenade_type == GRENADE_TYPE_EARTHQUAKE)
			ent->client->pers.inventory[ent->client->ammo_index] = 
				ent->client->pers.inventory[ent->client->ammo_index] - EARTHQUAKE_GRENADES;

		// turret grenades are taken out somewhere else
		else if (ent->client->pers.grenade_type != GRENADE_TYPE_TURRET)
			ent->client->pers.inventory[ent->client->ammo_index]--;

	}

	//Don't allow negative grenades
	else if (ent->client->pers.inventory[ent->client->ammo_index] < 0)
		ent->client->pers.inventory[ent->client->ammo_index] = 0;

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
	qboolean ok_to_throw;
	int max;
	char *str;
	int count;

	str = NULL;

	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
//WF
//		wf_show_grenade_type(ent);
//WF
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
		ent->client->latched_buttons &= ~BUTTON_ATTACK;

//WF
		//make sure we don't have too many active grenades of each grenade type 
		ok_to_throw = true;
		count = ent->client->pers.active_grenades[ent->client->pers.grenade_type];
		switch( ent->client->pers.grenade_type ) 
		{
			case GRENADE_TYPE_LASERBALL:
				max = MAX_TYPE_LASERBALL;
				str = "Laserball";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_GOODYEAR:
				max = MAX_TYPE_GOODYEAR;
				str = "Goodyear";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_CONCUSSION:
				max = MAX_TYPE_CONCUSSION;
				str = "Concussion";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_MAGNOTRON:
				max = MAX_TYPE_MAGNOTRON;
				str = "Magnotron";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_PIPEBOMB:
				max = MAX_TYPE_PIPEBOMB;
				str = "Pipebomb";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_SLOW:
				max = MAX_TYPE_SLOW;
				str = "Slow";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_PLAGUE:
				max = MAX_TYPE_PLAGUE;
				str = "Plague";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_NORMAL:
				max = MAX_TYPE_NORMAL;
				str = "Normal";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_FLASH:
				max = MAX_TYPE_FLASH;
				str = "Flash";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_PROXIMITY:
				max = MAX_TYPE_PROXIMITY;
				str = "Proximity";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_CLUSTER:
				max = MAX_TYPE_CLUSTER;
				str = "Cluster";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_EARTHQUAKE:
				max = MAX_TYPE_EARTHQUAKE;
				str = "Earthquake";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_TURRET:
				max = MAX_TYPE_TURRET;
				str = "Turret";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_SHOCK:
				max = MAX_TYPE_SHOCK;
				str = "Shock";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_NAPALM:
				max = MAX_TYPE_NAPALM;
				str = "Napalm";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_LASERCUTTER:
				max = MAX_TYPE_LASERCUTTER;
				str = "Laser cutter";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_TESLA:
				max = MAX_TYPE_TESLA;
				str = "Tesla";
				if (count >= max) ok_to_throw = false;
				break;
			case GRENADE_TYPE_GAS:
				max = MAX_TYPE_GAS;
				str = "Gas";
				if (count >= max) ok_to_throw = false;
				break;
			}
			if (ok_to_throw == false)
			{
				safe_cprintf(ent, PRINT_HIGH, "You can only have %d active %s type grenades. \n",max,str);
				return;
			}

			//Is this grenade banned?
/*
			if ((bangrenade->string) && (str) && (strcmp(str, bangrenade->string) == 0))
			{
			safe_cprintf(ent, PRINT_HIGH, "Sorry, the %s grenade has been disabled.\n", str);
				return;
			}
*/


			//for laserball, make sure they have cells
			if ( ent->client->pers.grenade_type == GRENADE_TYPE_LASERBALL) 
			{
				if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < LASERBALL_CELLS)
				{
					safe_cprintf(ent, PRINT_HIGH, "Not enough cells for laserball.\n");
					return;
				}
			}
			//for earthquake grenade, make sure they have enough grenades
			if ( ent->client->pers.grenade_type == GRENADE_TYPE_EARTHQUAKE) 
			{
				if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] < EARTHQUAKE_GRENADES)
				{
					safe_cprintf(ent, PRINT_HIGH, "Not enough grenades. You need %d for an earthquake.\n",EARTHQUAKE_GRENADES);
					return;
				}
			}
//WF
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
//WF - Speed up grenade throw
		if (ent->client->ps.gunframe < 11)
				ent->client->ps.gunframe = 11;
//WF
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
	int		damage = wf_game.weapon_damage[WEAPON_GRENADELAUNCHER];
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

//	if (ent->client->player_class == CLASS_CYBORG)
//        fire_cluster (ent, start, forward, damage, 600, 2.5, radius);
//	else
		fire_grenade (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_GRENADELAUNCHER], 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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

//	damage = 100 + (int)(random() * 20.0);
	damage = wf_game.weapon_damage[WEAPON_ROCKETLAUNCHER];
	radius_damage = 105;
	damage_radius = 105;
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

	//WF - increase speed of rocket
//	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage, MOD_ROCKET);
	fire_rocket (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_ROCKETLAUNCHER], damage_radius, radius_damage, MOD_ROCKET);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	//If this is a homing rocket, take an extra one away. (Homing cost 2 rockets)
	//If that was their last rocket, give them a freebee
	if ((ent->client) && (ent->client->pers.homing_state) &&
		(ent->client->pers.inventory[ent->client->ammo_index] > 0))
	{
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
	}
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

	fire_blaster_wf (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_BLASTER], effect, hyper);

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

	damage = wf_game.weapon_damage[WEAPON_BLASTER];
	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}
/*
===================================
New HyperBlaster code - PlasmaBeam
by Fireball
===================================
*/
void Weapon_PlasmaBeam_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;
	vec3_t		start;
	vec3_t		forward, right;//, up;
//	vec3_t		angles;
	//vec3_t		end;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

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
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

/*
			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
*/
			effect = EF_HYPERBLASTER;
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;

			if (is_quad)
				damage *= 4;
			AngleVectors (ent->client->v_angle, forward, right, NULL);

			VectorSet(offset, 0, 7,  ent->viewheight-8);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
			fire_plasmabeam (ent, start, forward, damage, 200);

			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_RAILGUN | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			PlayerNoise(ent, start, PNOISE_WEAPON);


			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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

void Weapon_HyperBlaster_Fire_WF (edict_t *ent)
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
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
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
			damage = wf_game.weapon_damage[WEAPON_HYPERBLASTER];
			
			//WF JR LAG fixing in case lag gets high
			//Hopefully this will help during a fierce battle and the 
			//ping times rocket up
			ent->ShotNumber++;
			if (ent->client->ping >400)
			{
				if (ent->ShotNumber>1)
				{
					ent->ShotNumber = 0;
					Blaster_Fire (ent, offset, damage*2, true, effect);
				}
			}
			else
			{
				ent->ShotNumber = 0;
				Blaster_Fire (ent, offset, damage, true, effect);
			}
			//end lag code
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
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
//				effect = EF_HYPERBLASTER;
				effect = EF_IONRIPPER;
			else
				effect = 0;
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;
			Blaster_Fire (ent, offset, damage, true, effect);
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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



void Weapon_HyperBlaster (edict_t *ent)
{
/*	3.4 beta 2 version of hyperblaster
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_PlasmaBeam_Fire);	// Changed to Weapon_PlasmaBeam_Fire by Fireball
*/
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	//Orig id type 
	//Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);

	//3.3 version of hyperblaster
	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire_WF);

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
	int			damage = wf_game.weapon_damage[WEAPON_MACHINEGUN];
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
	if (!deathmatch->value)
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
	//WF JR LAG fixing in case lag gets high
		//Hopefully this will help during a fierce battle and the 
		//ping times rocket up
		ent->ShotNumber++;
		if (ent->client->ping >400)
		{
			if (ent->ShotNumber>1)
			{
				ent->ShotNumber = 0;
				fire_bullet (ent, start, forward, damage*2, kick*2, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
			}
		}
		else
		{
			ent->ShotNumber = 0;
			fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
		}
//end lag code
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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
		damage = wf_game.weapon_damage[WEAPON_CHAINGUN];
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
		//WF JR LAG fixing in case lag gets high
		//Hopefully this will help during a fierce battle and the 
		//ping times rocket up
		ent->ShotNumber++;
		if (ent->client->ping >400)
		{
			if (ent->ShotNumber>1)
			{
				ent->ShotNumber=0;
				fire_bullet (ent, start, forward, damage*2, kick*2, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
			}
		}
		else
		{
			fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
			ent->ShotNumber=0;
		}
		//end lag
	}
//WF
	//Send a tracer bullet
	if ((shots == 3) && (random() < .16)) 
	{
		fire_blaster (ent, start, forward, 0, wf_game.weapon_speed[WEAPON_BLASTER], EF_HYPERBLASTER, true);
	}

//WF
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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
	int			damage = wf_game.weapon_damage[WEAPON_SHOTGUN];
	int			kick = 8;

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

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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
	int			damage = wf_game.weapon_damage[WEAPON_SUPERSHOTGUN];
	int			kick = 12;

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

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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

	damage = wf_game.weapon_damage[WEAPON_RAILGUN];
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
	fire_rail (ent, start, forward, damage, kick, MOD_RAILGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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

	damage = wf_game.weapon_damage[WEAPON_BFG];

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
	fire_bfg (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_BFG], damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}


//======================================================================
