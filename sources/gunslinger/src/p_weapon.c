// g_weapon.c

#include "g_local.h"
#include "m_player.h"
#include "z_gq.h"

static qboolean	is_quad;
static byte		is_silenced;
//int				sscnt=0;

void weapon_molotov_cocktail_fire (edict_t *ent, qboolean held);
void weapon_grenade_fire (edict_t *ent, qboolean held);

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
	int			index, foo;
	gitem_t		*ammo;
	qboolean	override=false;

	if (ent->item->weapmodel==WEAP_DOUBLEREVOLVER)
		ent->item=FindItem("Revolver");

	foo = ITEM_INDEX(FindItem("Revolver"));
//	debugmsg("Found weapon: %i   Style: %i  Index: %i  Total: %i\n", ITEM_INDEX(ent->item), other->style, foo, other->style+foo);
	if (((int)playmode->value == PM_LAST_MAN_STANDING) && use_classes->value) {
		if ((ent->item->weapmodel!=WEAP_REVOLVER) && (ITEM_INDEX(ent->item) != other->style+foo))
			return false;
		if ((ent->item->weapmodel==WEAP_REVOLVER) && (other->client->pers.inventory[foo]>0) && (other->style != 1))
			return false;
	}

	index = ITEM_INDEX(ent->item);

//	debugmsg("Found %s\n", ent->item->pickup_name);
	if ((ent->item->weapmodel==WEAP_REVOLVER) && (other->client->pers.inventory[index]>0)) {
		// We've found a revolver, and we already have 2.  Drop the revolver with the fewest rounds in it.
		if (other->client->pers.inventory[index+1]>0) {
			// Swap the ammo amounts so we can fill the first revolver
			// The first revolver ALWAYS has more than (or the same as) the second
			other->client->pers.carried[ITEM_INDEX(ent->item)+1]=other->client->pers.carried[ITEM_INDEX(ent->item)];
//			debugmsg("Okay, switching ammo.\n");
		} else {	// Found a revolver and we only have 1.  Take it.
			index++;
			ent->item=FindItem("Double Revolver");
//			debugmsg("Second gun\n");
		}
/*	} else if (ent->item->weapmodel==WEAP_DOUBLEREVOLVER) {
		if (other->client->pers.inventory[index-1]==0) {
//			debugmsg("No revolvers at all\n");
			index--;
			ent->item=FindItem("Revolver");
		} else if (other->client->pers.inventory[index]>0) {
			other->client->pers.carried[ITEM_INDEX(ent->item)]=other->client->pers.carried[ITEM_INDEX(ent->item)-1];
//			debugmsg("Ah, got to switch ammo\n");
			index--;
			ent->item=FindItem("Revolver");
		} else {
//			debugmsg("This will be the second gun\n");
		}
*/	}

	if (weapons_unique->value && other->client->pers.inventory[index] && !(ent->item->flags & IT_AMMO)) {
//		gi_bprintf(PRINT_HIGH, "Already got one!\n");
		return false;	// Can't carry two unique weapons... Says me!!! -- Stone
	}
	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value) 
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}
	ammo = FindItem (ent->item->ammo);
	if (!other->client->pers.inventory[index] || (ent->item->flags & IT_AMMO)) {
		other->client->pers.inventory[index]++;
//		debugmsg("Picking up.\n");
		override=true;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		else {
			//gi_bprintf(PRINT_HIGH, "Capacity: %i\n", ent->item->capacity);
			if (!Add_Ammo (other, ammo, ammo->quantity)) {
//				debugmsg("Couldn't add any ammo.\n");
				if ((other->client->pers.carried[ITEM_INDEX(ent->item)] >= ent->item->capacity) && !override) {
//					debugmsg("Weapon is full.\n");
					return false;
				}
			}
		}
		// Fill up their current weapon
		other->client->pers.carried[ITEM_INDEX(ent->item)] = ent->item->capacity;

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) && ! (ent->spawnflags & DROPPED_ITEM))
		{
			if (deathmatch->value)
			{
				if (((int)(dmflags->value) & DF_WEAPONS_STAY) && !weapons_unique->value) {
					ent->flags |= FL_RESPAWN;
				} else {
					if (!weapons_unique->value || (ent->item->flags & IT_AMMO)) {	// Weapons do not respawn if unique! -- Stone
						SetRespawn (ent, 30);
					} else {
						ent->flags |= FL_RESPAWN;
						ent->svflags |= SVF_NOCLIENT;
						ent->solid = SOLID_NOT;
//						gi.linkentity (ent);
					}
				}
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	} else {
		if ((other->client->pers.carried[ITEM_INDEX(ent->item)] >= ent->item->capacity) && !override) {
//			debugmsg("Weapon is full.\n");
			return false;
		}
	}

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || (other->client->pers.weapon->weapmodel==WEAP_BOWIE_KNIFE) ) ) {
		other->client->newweapon = ent->item;
//		gi_bprintf(PRINT_HIGH, "Got better weapon.\n");
	}

	CalcWeight(other);

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
	int num, index;

//	gi_bprintf(PRINT_HIGH, "Changing weapons.\n");
	// Exchange ammo amounts if carrying two revolvers -- Stone
	index=ITEM_INDEX(FindItem("Double Revolver"));
	if ((ent->client->newweapon==ent->client->pers.weapon) && ent->client->pers.inventory[index]) {
		num=ent->client->pers.carried[index];
		ent->client->pers.carried[index]=ent->client->pers.carried[index-1];
		ent->client->pers.carried[index-1]=num;
	} else {
		if (ent->client->pers.carried[index] > ent->client->pers.carried[index-1]) {
			num=ent->client->pers.carried[index];
			ent->client->pers.carried[index]=ent->client->pers.carried[index-1];
			ent->client->pers.carried[index-1]=num;
		}
	}
	GQ_UnzoomPlayer(ent);
	if (ent->client->grenade_time)
	{
//		gi_bprintf(PRINT_HIGH, "Whoops, throwing!\n");
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		if (ent->client->pers.weapon->weapmodel == WEAP_COCKTAIL)
			weapon_molotov_cocktail_fire (ent, false);
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
NoAmmoWeaponChange -- Also, "best weapon"
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("12 Gauge Shells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("pump shotgun"))] )
	{
		ent->client->newweapon = FindItem ("pump shotgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem(".30-30 Bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("winchester rifle"))] )
	{
		ent->client->newweapon = FindItem ("winchester rifle");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("12 Gauge Shells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("coach gun"))] )
	{
		ent->client->newweapon = FindItem ("coach gun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem(".45-70 Bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("sharps rifle"))] )
	{
		ent->client->newweapon = FindItem ("sharps rifle");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem(".45 Colt Bullets"))] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Double Revolver"))] )
	{
		ent->client->newweapon = FindItem ("Double Revolver");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem(".45 Colt Bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Revolver"))] )
	{
//		gi_bprintf(PRINT_HIGH, "Changing to revolver\n");
		ent->client->newweapon = FindItem ("Revolver");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Hatchet"))]>0)
	{
//		gi_bprintf(PRINT_HIGH, "Changing to hatchet\n");
		ent->client->newweapon = FindItem ("Hatchet");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Molotov Cocktail"))]>0)
	{
//		gi_bprintf(PRINT_HIGH, "Changing to molotov cocktail\n");
		ent->client->newweapon = FindItem ("Molotov Cocktail");
		return;
	}
//	gi_bprintf(PRINT_HIGH, "Changing to bowie knife\n");
	ent->client->newweapon = FindItem ("bowie knife");
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
//		gi_bprintf(PRINT_HIGH, "Think weapon.\n");
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
//	gi_bprintf(PRINT_HIGH, "Using weapon!\n");
	if (item == ent->client->pers.weapon) {
		// If we're carrying two revolvers, we can switch
		if (GQ_MatchItem(item, "Revolver") && ent->client->pers.inventory[ITEM_INDEX(item)+1]) {
//			item=item++;
		} else {
			return;
		}
	}

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon->capacity && !ent->client->pers.carried[ITEM_INDEX(item)])
		{
			gi_cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.carried[ITEM_INDEX(item)] + ent->client->pers.inventory[ammo_index] < 1)
		{
			gi_cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
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
	edict_t	*dropped;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	if (((item == ent->client->pers.weapon) || (item == ent->client->newweapon)) && (ent->client->weaponstate != WEAPON_READY)) {
//		debugmsg("Not ready!\n");
		return;
	}
	
	if (!Q_strcasecmp(item->pickup_name,"Revolver") && (ent->client->pers.inventory[index+1]>0)) {
//		gi_bprintf(PRINT_HIGH,"Dropping Double Revolver instead.\n");
		item=FindItem("Double Revolver");
		index++;
	}
	// see if we're already using it
/*	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		gi_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}
*/
	if (ent->client->zoomed && (item == ent->client->pers.weapon)) {
		GQ_UnzoomPlayer(ent);
	}
	dropped=Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
	if (item->weapmodel==WEAP_CANNON) {
		dropped->velocity[0]=0;
		dropped->velocity[1]=0;
		dropped->nextthink = level.time + 5;
	}

	// You can now drop your current weapon!
	if ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))
	{
		gi_bprintf(PRINT_HIGH,"Dropped current weapon!\n");
		NoAmmoWeaponChange(ent);
		ent->client->ps.gunframe = 0;
		ent->client->weaponstate = WEAPON_THROWING;
	}
	CalcWeight(ent);
	ValidateSelectedItem (ent);
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

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int FRAME_FIRE2_FIRST, int FRAME_FIRE2_LAST, int *pause_frames, int *fire_frames, int *fire_frames2, void (*fire)(edict_t *ent), void (*fire2)(edict_t *ent))
{
	int		n;
	int		index;
	gitem_t *weap;

//	debugmsg("Gun: %s  Frame: %i\n",ent->client->pers.weapon->pickup_name,ent->client->ps.gunframe);
	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_THROWING) {
//		debugmsg("Weapon throwing!\n");
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 0;
		ChangeWeapon (ent);
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
		if (GQ_MatchItem(ent->client->artifact,"Hands of Lightning")) {
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
		}
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if ((ent->client->pers.weapon->weapmodel==WEAP_PUMPACTIONSHOTGUN) && (ent->client->ps.gunframe==6))
			gi.sound(ent, CHAN_VOICE, gi.soundindex("gunslinger/pumpReload.wav"), 1, ATTN_NORM, 0);
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			if (ent->client->pers.weapon->weapmodel == WEAP_DOUBLEREVOLVER)
				ent->client->ps.gunframe = FRAME_IDLE_FIRST+4;
			return;
		}

		ent->client->ps.gunframe++;
		if (GQ_MatchItem(ent->client->artifact,"Hands of Lightning")) {
			if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
			{
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}
			
			ent->client->ps.gunframe++;
		}
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		ent->client->reloadcount=0;

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

	if (ent->client->weaponstate == WEAPON_RELOADING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)	// Gun has dropped fully away... now reload
		{
//			gi_bprintf(PRINT_HIGH, "Reloadcount: %i\n",ent->client->reloadcount);
			index=ITEM_INDEX(ent->client->pers.weapon);
			// Can't reload the cannon underwater
			if ((ent->client->pers.weapon->weapmodel == WEAP_CANNON) && (ent->waterlevel>2)) {
				ent->client->reloading = false;
				ent->client->weaponstate = WEAPON_ACTIVATING;
				ent->client->ps.gunframe = 0;
			}
			// End reloading if there's not enough bullets left to do a proper reload
			if (ent->client->pers.inventory[ent->client->ammo_index] < ent->client->pers.weapon->quantity) {
				ent->client->reloading = false;
				ent->client->weaponstate = WEAPON_ACTIVATING;
				ent->client->ps.gunframe = 0;
				// If the gun is empty too, change weapons
				if (ent->client->pers.carried[index]<1) {
					NoAmmoWeaponChange(ent);
					ChangeWeapon(ent);
				}
				return;
			}
			// Reloading sound
			if (ent->client->reloadcount == 0) {
				if ((ent->client->pers.weapon->weapmodel==WEAP_DOUBLEREVOLVER) && (ent->client->pers.carried[index-1] >= 6))
					gi.sound(ent, CHAN_VOICE, gi.soundindex("gunslinger/loadrevolver.wav"), 1, ATTN_NORM, 0);
				else 
					gi.sound(ent, CHAN_VOICE, gi.soundindex(ent->client->pers.weapon->reload_sound), 1, ATTN_NORM, 0);
			}

			if (ent->client->reloadcount < ent->client->pers.weapon->reloadtime) {
//				gi_bprintf(PRINT_HIGH, "Reload pause %i\n",ent->client->reloadcount);
				ent->client->reloadcount++;
				if ((ent->client->pers.weapon->weapmodel==WEAP_DOUBLEREVOLVER) && (ent->client->pers.carried[index-1] >= 6)) {
					ent->client->reloadcount++;
					// Loading only one of two revolvers.  Speed up reload?
				}
				return;
			}
			if (!ent->client->reloading) {
//				gi_bprintf(PRINT_HIGH, "End reloading\n");
				ent->client->weaponstate = WEAPON_ACTIVATING;
				ent->client->ps.gunframe = 0;
			}
			ent->client->reloadcount=0;
			if (GQ_MatchItem(ent->client->artifact,"Hands of Lightning")) {
				ent->client->reloadcount=(int)((ent->client->pers.weapon->reloadtime+1)/2);
				gi.sound(ent, CHAN_VOICE, gi.soundindex(ent->client->pers.weapon->reload_sound), 1, ATTN_NORM, 0);
			}

//			gi_bprintf(PRINT_HIGH, "Do reload...\n");
			// Okay, actually do the reloading now
			ent->client->pers.carried[index]++;
			ent->client->pers.inventory[ent->client->ammo_index]--;
			if (index==ITEM_INDEX(FindItem("Coach Gun")) && (ent->client->pers.inventory[ent->client->ammo_index]>0) && (ent->client->pers.carried[index] < ent->client->pers.weapon->capacity)) {
				ent->client->pers.carried[index]=2;
				ent->client->pers.inventory[ent->client->ammo_index]--;
			}
			if (ent->client->pers.carried[index] > ent->client->pers.weapon->capacity)
				ent->client->pers.carried[index] = ent->client->pers.weapon->capacity;
			if (ent->client->pers.inventory[ent->client->ammo_index]<0)
				ent->client->pers.inventory[ent->client->ammo_index]=0;
			if (index==ITEM_INDEX(FindItem("Double Revolver"))) {
				weap=ent->client->pers.weapon-1;
				if (ent->client->pers.carried[index-1] < weap->capacity) {
//					gi_bprintf(PRINT_HIGH, "Loading both guns\n");
					ent->client->pers.carried[index-1]++;
					ent->client->pers.inventory[ent->client->ammo_index]--;
				}
			}
			if (ent->client->pers.carried[index] >= ent->client->pers.weapon->capacity) {
//				gi_bprintf(PRINT_HIGH, "Weapon is full\n");
				ent->client->reloading = false;
				ent->client->weaponstate = WEAPON_ACTIVATING;
				ent->client->ps.gunframe = 0;
			}
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
		//gi_bprintf(PRINT_HIGH, "Reload drop frame increment\n");
		ent->client->ps.gunframe++;
		if (GQ_MatchItem(ent->client->artifact,"Hands of Lightning")) {
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)	// Gun has dropped fully away... now reload
			{
				ent->client->ps.gunframe = FRAME_DEACTIVATE_LAST-1;
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
		}
		return;
	}

	// Weapon Reloading
	if ((ent->client->weaponstate != WEAPON_FIRING) && (ent->client->weaponstate != WEAPON_ALT_FIRING)
			&& (ent->client->reloading)
			&& (ent->client->pers.carried[ITEM_INDEX(ent->client->pers.weapon)] < ent->client->pers.weapon->capacity)
			&& (ent->client->pers.inventory[ent->client->ammo_index]))
	{
		if ((ent->client->pers.weapon->weapmodel==WEAP_CANNON) && (ent->waterlevel>2)) {
			return;
		}
		ent->client->weaponstate = WEAPON_RELOADING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		ent->client->reloadcount=0;
		if (GQ_MatchItem(ent->client->artifact,"Hands of Lightning"))
			ent->client->reloadcount=(int)((ent->client->pers.weapon->reloadtime+1)/2);
		GQ_UnzoomPlayer(ent);
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

	// Weapon Firing!
	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			index=ITEM_INDEX(ent->client->pers.weapon);
			if ((!ent->client->ammo_index) || 
				((ent->client->pers.carried[index] >= ent->client->pers.weapon->quantity) || 
				(!ent->client->pers.weapon->capacity)))
			{
				if ((ent->waterlevel >2) && ((ent->client->pers.weapon->weapmodel==WEAP_CANNON) || (ent->client->pers.weapon->weapmodel==WEAP_COCKTAIL)) ) {
//					debugmsg("Can't fire under water!\n");
				} else {
//					sscnt++;
//					if (sscnt>31)
//						sscnt=0;
					ent->client->ps.gunframe = FRAME_FIRE_FIRST;
					ent->client->weaponstate = WEAPON_FIRING;
					ent->client->respawn_framenum=0;

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
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					// Out of ammo sound
					gi.sound(ent, CHAN_VOICE, gi.soundindex("gunslinger/empty.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				// Out of ammo completely?  Time for a new weapon
				if ((ent->client->pers.inventory[ent->client->ammo_index] < ent->client->pers.weapon->quantity2)
					&& (ent->client->pers.weapon->capacity)) {
					NoAmmoWeaponChange (ent);
				}
//				gi_bprintf(PRINT_HIGH, "Got to reload!\n");
			}
		}
		else    // Alternate firing mode!
		{
			if (ent->client->alt_fire_on) {
				//				gi_cprintf(ent, PRINT_HIGH, "Weapon_Generic 2");
				index=ITEM_INDEX(ent->client->pers.weapon);
				if ((!ent->client->ammo_index) || 
					((ent->client->pers.carried[index] >= ent->client->pers.weapon->quantity2) || 
					(!ent->client->pers.weapon->capacity)))
				{
					if ((ent->waterlevel >2) && ((ent->client->pers.weapon->weapmodel==WEAP_CANNON) || (ent->client->pers.weapon->weapmodel==WEAP_COCKTAIL)) ) {
//						debugmsg("Can't fire under water!\n");
					} else {
//						sscnt++;
						ent->client->ps.gunframe = FRAME_FIRE2_FIRST;
						ent->client->weaponstate = WEAPON_ALT_FIRING;
						ent->client->respawn_framenum=0;
						
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
				}
				else
				{
					if (level.time >= ent->pain_debounce_time)
					{
						gi.sound(ent, CHAN_VOICE, gi.soundindex("gunslinger/empty.wav"), 1, ATTN_NORM, 0);
						ent->pain_debounce_time = level.time + 1;
					}
					if (ent->client->ps.gunframe > FRAME_FIRE2_LAST)
						ent->client->ps.gunframe = FRAME_IDLE_FIRST+1;
				}
				
			} else {
				
				if (ent->client->ps.gunframe >= FRAME_IDLE_LAST)
				{
					ent->client->ps.gunframe = FRAME_IDLE_FIRST;
					if (ent->client->pers.weapon->weapmodel == WEAP_DOUBLEREVOLVER)
						ent->client->ps.gunframe = FRAME_IDLE_FIRST+4;
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
	}

	// Weapon firing
	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				if (deathmatch->value && ((int)playmode->value == PM_BADGE_WARS) && (ent->client->team == 2))
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
	// Weapon alt firing
	if (ent->client->weaponstate == WEAPON_ALT_FIRING)
	{
		for (n = 0; fire_frames2[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames2[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				fire2 (ent);
				break;
			}
		}

		if (!fire_frames2[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe >= FRAME_FIRE2_LAST+2)
//			ent->client->ps.gunframe = FRAME_IDLE_FIRST+2;
			ent->client->weaponstate = WEAPON_READY;
	}
}


/*
======================================================================

MOLOTOV COCKTAIL

======================================================================
*/

#define COCKTAIL_TIMER		5.0
#define COCKTAIL_MINSPEED	400
#define COCKTAIL_MAXSPEED	1000

void weapon_molotov_cocktail_fire (edict_t *ent, qboolean held)		// Molotov cocktail
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 80;
	float	radius = 120;
	float	timer;
	int		speed;

	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = COCKTAIL_MINSPEED + (COCKTAIL_TIMER - timer) * ((COCKTAIL_MAXSPEED - COCKTAIL_MINSPEED) / COCKTAIL_TIMER);
	if (GQ_MatchItem(ent->client->artifact, "Strength of the Bear")) {
		speed*=2;
	}

	fire_cocktail (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		ent->client->pers.inventory[ent->client->ammo_index]--;
		CalcWeight(ent);
	}

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

void weapon_molotov_cocktail_fire2 (edict_t *ent)		// Molotov cocktail
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 80;
	float	radius = 120;

	radius = damage;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	drop_cocktail (ent, start, damage, radius);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		ent->client->pers.inventory[ent->client->ammo_index]--;
		CalcWeight(ent);
	}

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


void Weapon_Molotov_Cocktail (edict_t *ent)		// Molotov cocktail
{
//	debugmsg("Frame: %i\n", ent->client->ps.gunframe);

	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
//		debugmsg("Changing\n");
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_THROWING) {
//		debugmsg("Weapon throwing!\n");
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 0;
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 12;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if (!ent->client->pers.inventory[ent->client->ammo_index])
			NoAmmoWeaponChange (ent);
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{	
				if (ent->waterlevel<3) {
					ent->client->ps.gunframe = 1;
					ent->client->weaponstate = WEAPON_FIRING;
					ent->client->grenade_time = 0;
				} else {
					ent->client->ps.gunframe++;
					if (++ent->client->ps.gunframe > 44)
						ent->client->ps.gunframe = 12;
					return;
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
			return;
		} else {
			if (ent->client->alt_fire_on)
			{
				if (ent->client->pers.inventory[ent->client->ammo_index])
				{
					if (ent->waterlevel<3) {
						ent->client->ps.gunframe = 1;
						ent->client->weaponstate = WEAPON_ALT_FIRING;
						ent->client->grenade_time = 0;
					} else {
						ent->client->ps.gunframe++;
						if (++ent->client->ps.gunframe > 44)
							ent->client->ps.gunframe = 12;
						return;
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
				return;
			}
		}
/*
		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}
*/
		if (++ent->client->ps.gunframe > 44)
			ent->client->ps.gunframe = 12;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->waterlevel>2) {
			ent->client->ps.gunframe=12;
			ent->client->weaponstate=WEAPON_READY;
			return;
		}
		if (ent->client->ps.gunframe==1) {
//			debugmsg("Playing.\n");
			gi.sound(ent, CHAN_ITEM, gi.soundindex("gunslinger/cocktail.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
		}
		if ((ent->client->ps.gunframe >= 3) && (ent->client->ps.gunframe < 8))
		{
			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BLASTER | MZ_SILENCED);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

//		if (ent->client->ps.gunframe == 5)
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 8)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + COCKTAIL_TIMER + 0.2;
//				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/blastf1a.wav"), 1, ATTN_NORM, 0);
				weapon_molotov_cocktail_fire (ent, true);
				ent->client->grenade_blew_up = true;
				if (!ent->client->pers.inventory[ent->client->ammo_index]) {
					ent->client->grenade_time=0;
					NoAmmoWeaponChange (ent);
					ChangeWeapon (ent);
					return;
				}
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 11;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 8)
		{
			ent->client->weapon_sound = 0;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/blastf1a.wav"), 1, ATTN_NORM, 0);
			weapon_molotov_cocktail_fire (ent, false);
			if (!ent->client->pers.inventory[ent->client->ammo_index]) {
				ent->client->grenade_time=0;
				NoAmmoWeaponChange (ent);
				ChangeWeapon (ent);
				return;
			}
		}

		if ((ent->client->ps.gunframe == 11) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
	if (ent->client->weaponstate == WEAPON_ALT_FIRING)
	{
		if (ent->waterlevel>2) {
			ent->client->ps.gunframe=12;
			ent->client->weaponstate=WEAPON_READY;
			return;
		}
		if (ent->client->ps.gunframe==1) {
//			debugmsg("Playing.\n");
			gi.sound(ent, CHAN_ITEM, gi.soundindex("gunslinger/cocktail.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
		}
		if ((ent->client->ps.gunframe >= 3) && (ent->client->ps.gunframe < 8))
		{
			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BLASTER | MZ_SILENCED);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

		if (ent->client->ps.gunframe == 8)
		{
			ent->client->grenade_time = level.time + COCKTAIL_TIMER + 0.2;
//			ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			ent->client->weapon_sound = 0;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/blastf1a.wav"), 1, ATTN_NORM, 0);
			weapon_molotov_cocktail_fire2 (ent);
			if (!ent->client->pers.inventory[ent->client->ammo_index]) {
				ent->client->grenade_time=0;
				NoAmmoWeaponChange (ent);
				ChangeWeapon (ent);
				return;
			}
			ent->client->ps.gunframe=11;
		}

		if ((ent->client->ps.gunframe == 11) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}


/*
=================
fire_grenade
=================
*/
static void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;
	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	//antibot code
	if (other->flags & FL_ANTIBOT)
		ent->owner->client->pers.hit_antibot ++;
	//end

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode (ent);
}

void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}



/*
======================================================================

HATCHET

======================================================================
*/

#define HATCHET_MINSPEED	400
#define HATCHET_MAXSPEED	800

void weapon_hatchet_fire (edict_t *ent)		// Hatchet
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 45;
	float	timer;
	int		speed;

	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = HATCHET_MINSPEED + (COCKTAIL_TIMER - timer) * ((HATCHET_MAXSPEED - HATCHET_MINSPEED) / COCKTAIL_TIMER);
	if (GQ_MatchItem(ent->client->artifact, "Strength of the Bear")) {
		speed*=2;
		damage*=2;
	}
	fire_hatchet (ent, start, forward, damage, speed, timer);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		ent->client->pers.inventory[ent->client->ammo_index]--;
		CalcWeight(ent);
	}

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

void Weapon_Hatchet_Fire2 (edict_t *ent)		// Hatchet slash
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = HATCHET_SLASH_DAMAGE;
	int			kick = damage;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("gladiator/melee3.wav"), 1, ATTN_NORM, 0);
	ent->client->ps.gunframe++;
	if (GQ_MatchItem(ent->client->artifact, "Strength of the Bear")) {
		damage*=2;
	}
	fire_melee (ent, start, forward, HATCHET_MELEE_RANGE, damage, kick);
}

void Weapon_Hatchet (edict_t *ent)		// Hatchet
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_THROWING) {
//		debugmsg("Weapon throwing!\n");
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 0;
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 9;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if (!ent->client->pers.inventory[ent->client->ammo_index])
			NoAmmoWeaponChange (ent);
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
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		} else {
			if (ent->client->alt_fire_on)
			{
				if (ent->client->pers.inventory[ent->client->ammo_index])
				{
					ent->client->ps.gunframe = 35;
					ent->client->weaponstate = WEAPON_ALT_FIRING;

				}
				else
				{
					NoAmmoWeaponChange (ent);
				}
				return;
			}
		}

		if ((ent->client->ps.gunframe == 9) || (ent->client->ps.gunframe == 15) || (ent->client->ps.gunframe == 25))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 34)
			ent->client->ps.gunframe = 9;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 2)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + COCKTAIL_TIMER + 0.2;
			}
			if (ent->client->buttons & BUTTON_ATTACK)
				return;

		}

		if (ent->client->ps.gunframe == 2)
		{
			ent->client->weapon_sound = 0;
			weapon_hatchet_fire (ent);
			if (!ent->client->pers.inventory[ent->client->ammo_index]) {
				ent->client->grenade_time=0;
				NoAmmoWeaponChange (ent);
				ChangeWeapon (ent);
				return;
			}
		}

		if ((ent->client->ps.gunframe == 1) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 9)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
	if (ent->client->weaponstate == WEAPON_ALT_FIRING)
	{
		if (ent->client->ps.gunframe == 37) {
			Weapon_Hatchet_Fire2 (ent);
			if (!ent->client->pers.inventory[ent->client->ammo_index]) {
				ent->client->grenade_time=0;
				NoAmmoWeaponChange (ent);
				ChangeWeapon (ent);
				return;
			}
		}

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe > 38) {
			ent->client->ps.gunframe = 9;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
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

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		ent->client->pers.inventory[ent->client->ammo_index]--;
	}
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};
	static int	fire_frames2[]	= {4, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, 4, 11, pause_frames, fire_frames, fire_frames2, Weapon_RocketLauncher_Fire, Weapon_RocketLauncher_Fire);
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

	fire_blaster (ent, start, forward, damage, 1000, effect, hyper);

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

/*
void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage;

	if (deathmatch->value)
		damage = 15;
	else
		damage = 10;
	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}
*/

void Weapon_Winchester_Rifle_Fire (edict_t *ent)	// Winchester '94 rifle
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 65;
	int			kick = 25;
	int			index;
	char		soundstr[30];

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -10;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_bullet (ent, start, forward, damage, kick, DEFAULT_WIN94_SPREAD, DEFAULT_WIN94_SPREAD, MOD_WINCHESTER_RIFLE);
	sprintf(soundstr, "gunslinger/winch%i.wav", (rand()%2)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(soundstr), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}

void Weapon_Winchester_Rifle_Fire2 (edict_t *ent)		// Winchester '94 rifle
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 65;
	int			kick = 25;
	int			index;
	char		soundstr[30];

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -10;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_bullet (ent, start, forward, damage, kick, DEFAULT_WIN94_SPREAD*5, DEFAULT_WIN94_SPREAD*5, MOD_WINCHESTER_RIFLE);
	sprintf(soundstr, "gunslinger/winch%i.wav", (rand()%2)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(soundstr), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}


void Weapon_Winchester_Rifle (edict_t *ent)		// Winchester '94 rifle
{
	static int	pause_frames[]	= {18, 38, 0};
	static int	fire_frames[]	= {7, 0};
	static int	fire_frames2[]	= {62, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, 62, 64, pause_frames, fire_frames, fire_frames2, Weapon_Winchester_Rifle_Fire, Weapon_Winchester_Rifle_Fire2);
}

/*
======================================================================

COACH GUN

======================================================================
*/
void Weapon_Coachgun_Fire2 (edict_t *ent)		// Coach shotgun
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = DEFAULT_SHOTGUN_DAMAGE;
	int			kick = 2*DEFAULT_SHOTGUN_DAMAGE;
	int			index;
//	char		soundstr[30];

	index=ITEM_INDEX(ent->client->pers.weapon);
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -8;

	if (ent->client->pers.carried[index]==1)
		VectorSet(offset, 0, 12,  ent->viewheight-8);
	else
		VectorSet(offset, 0, 4,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[ROLL]  = ent->client->v_angle[ROLL];
	if (ent->client->pers.carried[index]==1) {
//		gi_bprintf(PRINT_HIGH,"Right barrel\n");
		v[YAW]   = ent->client->v_angle[YAW] + 1;
		AngleVectors (v, forward, NULL, NULL);
		fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
	} else {
//		gi_bprintf(PRINT_HIGH,"Left barrel\n");
		v[YAW]   = ent->client->v_angle[YAW] - 1;
		AngleVectors (v, forward, NULL, NULL);
		fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
	}

	// send muzzle flash
//	sprintf(soundstr, "gunslinger/shotgun%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("gunslinger/shotgun1a.wav"), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe+=3;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}

void Weapon_Coachgun_Fire (edict_t *ent)		// Coach shotgun
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = DEFAULT_SHOTGUN_DAMAGE;
	int			kick = 2*DEFAULT_SHOTGUN_DAMAGE;
	int			index;
//	char		soundstr[30];

	index=ITEM_INDEX(ent->client->pers.weapon);
	if (ent->client->pers.carried[index]<2) {
		Weapon_Coachgun_Fire2(ent);
		return;
	}
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -16;


	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 1;
	v[ROLL]  = ent->client->v_angle[ROLL];
	VectorSet(offset, 0, 4,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 1;
	VectorSet(offset, 0, 12,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, MOD_SSHOTGUN);

	// send muzzle flash
//	sprintf(soundstr, "gunslinger/shotgun%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("gunslinger/shotgun1b.wav"), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		ent->client->pers.carried[index]-=2;
	}
}

void Weapon_Coachgun (edict_t *ent)		// Coach shotgun
{
	static int	pause_frames[]	= {18, 40, 0};
	static int	fire_frames[]	= {7, 0};
	static int	fire_frames2[]	= {61, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, 61, 68, pause_frames, fire_frames, fire_frames2, Weapon_Coachgun_Fire, Weapon_Coachgun_Fire2);
}

void Weapon_Pump_Action_Shotgun_Fire (edict_t *ent)		// Winchester '97 shotgun
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = DEFAULT_SHOTGUN_DAMAGE;
	int			kick = 2*DEFAULT_SHOTGUN_DAMAGE;
	int			index;
//	char		soundstr[30];

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -8;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

	// send muzzle flash
//	sprintf(soundstr, "gunslinger/shotgun%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("gunslinger/shotgu2c.wav"), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}

void Weapon_Pump_Action_Shotgun_Fire2 (edict_t *ent)		// Winchester '97 shotgun
{
	vec3_t		start, dir;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		up, end;
	int			damage = DEFAULT_SHOTGUN_DAMAGE;
	int			kick = 2*DEFAULT_SHOTGUN_DAMAGE;
	int			index;
	float		r, u;
//	char		soundstr[30];

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);
	vectoangles (forward, dir);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -8;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
	r = (crandom()*800);
	u = (crandom()*800);
	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);
	
	fire_shotgun (ent, start, end, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
	
	// send muzzle flash
//	sprintf(soundstr, "gunslinger/shotgun%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("gunslinger/shotgu2d.wav"), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}


void Weapon_Pump_action_shotgun (edict_t *ent)		// Winchester Model 97 shotgun
{
	static int	pause_frames[]	= {26, 32, 38, 0};
	static int	fire_frames[]	= {11, 0};
	static int	fire_frames2[]	= {63, 0};

	Weapon_Generic (ent, 10, 19, 59, 63, 63, 67, pause_frames, fire_frames, fire_frames2, Weapon_Pump_Action_Shotgun_Fire, Weapon_Pump_Action_Shotgun_Fire2);
}


/*
======================================================================

REVOLVERS

======================================================================
*/

void weapon_revolver_fire (edict_t *ent)		// Peacemaker
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 35;
	int			kick = 5;
	int			index;
	char		soundstr[30];

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -4;

	VectorSet(offset, 0, 9,  ent->viewheight-2);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_REVOLVER);

	// send muzzle flash
	sprintf(soundstr, "gunslinger/rev1-%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(soundstr), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}

void weapon_revolver_fire2 (edict_t *ent)		// Peacemaker
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 35;
	int			kick = 5;
	int			index;
	char		soundstr[30];

	// Apparently, id decided to skip a frame to speed up shooting -- Stone
	//if (ent->client->ps.gunframe == 9)
	//{
	//	ent->client->ps.gunframe++;
	//	return;
	//}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -4;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD*2, DEFAULT_BULLET_VSPREAD*2, MOD_REVOLVER);

	// send muzzle flash
	sprintf(soundstr, "gunslinger/revolver%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(soundstr), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}


void Weapon_Revolver (edict_t *ent)		// Peacemaker
{
	static int	pause_frames[]	= {23, 36, 0};
	static int	fire_frames[]	= {4, 0};
	static int	fire_frames2[]	= {49, 0};

	Weapon_Generic (ent, 3, 9, 45, 49, 49, 50, pause_frames, fire_frames, fire_frames2, weapon_revolver_fire, weapon_revolver_fire2);
//	static int	pause_frames[]	= {22, 28, 34, 0};
//	static int	fire_frames[]	= {8, 9, 0};

//	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_doublerevolver_fire (edict_t *ent)		// Double peacemakers
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
//	vec3_t		v;
	int			damage = 35;
	int			kick = 5;
	int			index;
	char		soundstr[30];

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -4;

	if (ent->client->ps.gunframe == 7) {
		VectorSet(offset, 0, -8,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	} else {
		VectorSet(offset, 0, 8,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_DREVOLVER);

	// send muzzle flash
	sprintf(soundstr, "gunslinger/rev1-%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(soundstr), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		if (ent->client->ps.gunframe != 7) {
//			gi_bprintf(PRINT_HIGH, "Right gun\n");
			ent->client->pers.carried[index]--;
			CalcWeight(ent);
		} else {
//			gi_bprintf(PRINT_HIGH, "Left gun\n");
			ent->client->pers.carried[index-1]--;
			CalcWeight(ent);
		}
	}
	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void weapon_doublerevolver_fire2 (edict_t *ent)		// Double Peacemakers
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
//	vec3_t		v;
	int			damage = 35;
	int			kick = 5;
	int			index;
	char		soundstr[30];

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -4;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	VectorSet(offset, 0, -8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_DREVOLVER);

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_DREVOLVER);

	/*
	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	*/

	// send muzzle flash
	sprintf(soundstr, "gunslinger/rev1-%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex(soundstr), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		ent->client->pers.carried[index-1]--;
		CalcWeight(ent);
	}
	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_DoubleRevolver (edict_t *ent)		// Double Peacemakers
{
	static int	pause_frames[]	= {23, 43, 0};
	static int	fire_frames[]	= {4, 7, 0};
	static int	fire_frames2[]	= {49, 0};

	Weapon_Generic (ent, 3, 9, 45, 49, 49, 55, pause_frames, fire_frames, fire_frames2, weapon_doublerevolver_fire, weapon_doublerevolver_fire2);
//	static int	pause_frames[]	= {29, 42, 57, 0};
//	static int	fire_frames[]	= {7, 13, 0};

//	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

SHARPS RIFLE

======================================================================
*/

void weapon_sharps_rifle_fire (edict_t *ent)		// Sharps rifle
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 120;
	int			kick = 100;
	int			index;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -1, ent->client->kick_origin);
	if (ent->client->zoomed)
		ent->client->kick_angles[0] = -5;
	else
		ent->client->kick_angles[0] = -20;

	VectorSet(offset, 0, 8,  ent->viewheight);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	if (ent->client->zoomed)
		fire_bullet (ent, start, forward, damage, kick, DEFAULT_SHARPS_SPREAD, DEFAULT_SHARPS_SPREAD, MOD_SHARPS_RIFLE);
	else
		fire_bullet (ent, start, forward, damage, kick, DEFAULT_WIN94_SPREAD, DEFAULT_WIN94_SPREAD, MOD_SHARPS_RIFLE);
		
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("gunslinger/sharps.wav"), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}

void weapon_sharps_rifle_fire2 (edict_t *ent)		// Sharps rifle
{
	char	hand[128];
	if (ent->client->zoomed) {
//		gi_bprintf(PRINT_HIGH, "Unzooming!");
		if (ent->client->ps.gunframe==58) {
			ent->client->ps.gunframe=60;
			return;
		}
		GQ_UnzoomPlayer(ent);
	} else {
//		gi_bprintf(PRINT_HIGH, "Zooming!");
		if (ent->client->ps.gunframe==58) {
			ent->client->ps.gunframe++;
			return;
		}
		ent->client->oldfov=ent->client->ps.fov;
		ent->client->oldhand=ent->client->pers.hand;
		if (ent->client->oldhand<0)
			ent->client->oldhand=0;
		ent->client->pers.hand=2;
		ent->client->ps.fov=15;
		ent->client->zoomed=true;
		sprintf(hand, "set hand 2\nset fov 15\nset sensitivity $zoomsensitivity\nset cl_pitchspeed $zoompitch\nset cl_yawspeed $zoomyaw\n");
//		debugmsg(hand);
		ent->message=RPS_AddToStringPtr(ent->message, hand, true);
	}
	ent->client->alt_fire_on=false;
	ent->client->ps.gunframe++;
}


void Weapon_Sharps_Rifle (edict_t *ent)		// Sharps rifle
{
	static int	pause_frames[]	= {15, 30, 0};
	static int	fire_frames[]	= {7, 0};
	static int	fire_frames2[]	= {58, 60, 0};

	Weapon_Generic (ent, 6, 14, 54, 58, 58, 61, pause_frames, fire_frames, fire_frames2, weapon_sharps_rifle_fire, weapon_sharps_rifle_fire2);
}


/*
======================================================================

CANNON

======================================================================
*/

void weapon_cannon_fire (edict_t *ent)		// Cannon
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 200;
	float	radius;
	int		index;

	if (ent->client->ps.gunframe==9) {
//		debugmsg("Playing.\n");
		gi.sound(ent, CHAN_ITEM, gi.soundindex("gunslinger/light3.wav"), 1, ATTN_NORM, 0);
		ent->client->ps.gunframe++;
		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}
	if (ent->client->ps.gunframe < 18)
	{
// Cannon fuse is waterproof!!!
//		if (ent->waterlevel>2) {
//			ent->client->ps.gunframe=31;
//			return;
//		}
		// send muzzle flash
		ent->s.frame = FRAME_attack1-2;
		ent->client->anim_end = FRAME_attack1;
		
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BLASTER);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}
	if (ent->client->ps.gunframe==18) {
		gi.sound(ent, CHAN_ITEM, gi.soundindex("gunslinger/cannon3.wav"), 1, ATTN_NORM, 0);
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BLASTER);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		PlayerNoise(ent, start, PNOISE_WEAPON);
	}
	if (ent->client->ps.gunframe<20) {
		ent->s.frame = FRAME_attack1-2;
		ent->client->anim_end = FRAME_attack1;
		ent->client->ps.gunframe++;
		return;
	}

	radius = damage;
	if (is_quad) {
		damage *= 4;
		radius *= 2;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -30;

	fire_cannon (ent, start, forward, damage, 2000, radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}

void weapon_cannon_fire2 (edict_t *ent)		// Cannon
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 75;
	float	radius;
	int		index;

	if (ent->client->ps.gunframe==9) {
//		debugmsg("Playing.\n");
		gi.sound(ent, CHAN_ITEM, gi.soundindex("gunslinger/light3.wav"), 1, ATTN_NORM, 0);
		ent->client->ps.gunframe++;
		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}
	if (ent->client->ps.gunframe < 18)
	{
// Cannon fuse is waterproof!!!
//		if (ent->waterlevel>2) {
//			ent->client->ps.gunframe=31;
//			return;
//		}
		// send muzzle flash
		ent->s.frame = FRAME_attack1-2;
		ent->client->anim_end = FRAME_attack1;
		
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BLASTER);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}
	if (ent->client->ps.gunframe==18) {
		gi.sound(ent, CHAN_ITEM, gi.soundindex("gunslinger/cannon3.wav"), 1, ATTN_NORM, 0);
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BLASTER);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		PlayerNoise(ent, start, PNOISE_WEAPON);
	}
	if (ent->client->ps.gunframe<20) {
		ent->s.frame = FRAME_attack1-2;
		ent->client->anim_end = FRAME_attack1;
		ent->client->ps.gunframe++;
		return;
	}

	radius = damage*2;
	if (is_quad) {
		damage *= 4;
		radius *= 2;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -1, ent->client->kick_origin);
	ent->client->kick_angles[0] = -30;

	fire_cannon2 (ent, start, forward, damage, 2000, radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ) {
		index=ITEM_INDEX(ent->client->pers.weapon);
		ent->client->pers.carried[index]--;
		CalcWeight(ent);
	}
}

void Weapon_Cannon (edict_t *ent)		// Cannon
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};
	static int	fire_frames2[]	= {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};

	Weapon_Generic (ent, 8, 35, 40, 44, 9, 35, pause_frames, fire_frames, fire_frames2, weapon_cannon_fire, weapon_cannon_fire2);
}


//======================================================================

/*
======================================================================

BOWIE KNIFE - Added by Stone

======================================================================
*/

void Weapon_Bowie_Knife_Fire (edict_t *ent)		// Bowie knife
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = KNIFE_STAB_DAMAGE;
	int			kick = damage;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	//start[2]+=4;

	//VectorScale (forward, -2, ent->client->kick_origin);
	//ent->client->kick_angles[0] = -2;

	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("gladiator/melee3.wav"), 1, ATTN_NORM, 0);
	ent->client->ps.gunframe++;
	fire_melee (ent, start, forward, KNIFE_STAB_RANGE, damage, kick);
	
	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_SHOTGUN | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	//The knife is a "silent" attack!  Comment out the next line
	//PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Bowie_Knife_Fire2 (edict_t *ent)		// Bowie knife
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = KNIFE_SLASH_DAMAGE;
	int			kick = damage;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("gunslinger/knifeSlash.wav"), 1, ATTN_NORM, 0);
	ent->client->ps.gunframe++;
	fire_melee (ent, start, forward, KNIFE_SLASH_RANGE, damage, kick);
}


void Weapon_Bowie_Knife (edict_t *ent)		// Bowie knife
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {4, 0};
	static int	fire_frames2[]	= {52, 0};

	Weapon_Generic (ent, 3, 7, 47, 51, 51, 53, pause_frames, fire_frames, fire_frames2, Weapon_Bowie_Knife_Fire, Weapon_Bowie_Knife_Fire2);
}
