// g_weapon.c

#include "g_local.h"
#include "m_player.h"
#include "g_brrr.h"


static qboolean is_quad;
static byte     is_silenced;

void weapon_grenade_fire (edict_t *ent, qboolean held);
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
    vec3_t  _distance;

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
    edict_t     *noise;

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
    int         index;
	gitem_t     *ammo;

	index = ITEM_INDEX(ent->item);

    if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value)
        && other->client->pers.inventory[index])
    {
        if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
            return false;   // leave the weapon for others to pickup
    }

    other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		/* *** FAKE DEATH *** */
		else if (ent->tomvar)
        	Add_Ammo (other, ammo, ent->count);
		/* *** */
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

    return true;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
/* *** VWEP *** */
void ShowGun( edict_t *ent)
{
	int nIndex;
	char *pszIcon;

	// No weapon?
	if ( !ent->client->pers.weapon)
	{
		ent->s.modelindex2 = 0;
		return;
	}

	// Determine the weapon's precache index.

	nIndex = 0;
	pszIcon = ent->client->pers.weapon->icon;

	if ( strcmp( pszIcon, "w_blaster") == 0)
		nIndex = 1;
	else if ( strcmp( pszIcon, "w_shotgun") == 0)
		nIndex = 2;
	else if ( strcmp( pszIcon, "w_sshotgun") == 0)
		nIndex = 3;
	else if ( strcmp( pszIcon, "w_machinegun") == 0)
		nIndex = 4;
	else if ( strcmp( pszIcon, "w_chaingun") == 0)
		nIndex = 5;
	else if ( strcmp( pszIcon, "a_grenades") == 0)
	{
		if (ent->client->resp.pipes)
			nIndex = 13;
		else if (ent->client->resp.proximity)
			nIndex = 14;
		else
			nIndex = 6;
	}
	else if ( strcmp( pszIcon, "w_glauncher") == 0)
		nIndex = 7;
	else if ( strcmp( pszIcon, "w_rlauncher") == 0)
		nIndex = 8;
	else if ( strcmp( pszIcon, "w_hyperblaster") == 0)
	{
		if (ent->client->resp.disruptor)
			nIndex = 16;
        else if (ent->client->resp.freezer)
            nIndex = 19;
		else
			nIndex = 9;
	}
	else if ( strcmp( pszIcon, "w_railgun") == 0)
	{
		if (ent->client->resp.sniper)
			nIndex = 17;
		else
			nIndex = 10;
	}
	else if ( strcmp( pszIcon, "w_bfg") == 0)
	{
		if (ent->client->resp.phaser)
			nIndex = 18;
		else
			nIndex = 11;
	}
	else if ( strcmp( pszIcon, "w_grapple") == 0)
		nIndex = 12;
	else if ( strcmp( pszIcon, "w_saber") == 0)
		nIndex = 15;

	// Clear previous weapon model.
	ent->s.skinnum &= 255;

	// Set new weapon model.
	ent->s.skinnum |= (nIndex << 8);
	ent->s.modelindex2 = 255;
}
/* OLD ... void ShowGun(edict_t *ent)
{
	char heldmodel[128];
	int len;

	if(!ent->client->pers.weapon)
	{
		ent->s.modelindex2 = 0;
		return;
	}

	strcpy(heldmodel, "players/");
	strcat(heldmodel, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	for(len = 8; heldmodel[len]; len++)
	{
		if(heldmodel[len] == '/')
			heldmodel[++len] = '\0';
	}
	strcat(heldmodel, ent->client->pers.weapon->icon);
	strcat(heldmodel, ".md2");
	//gi.dprintf ("%s\n", heldmodel);
	ent->s.modelindex2 = gi.modelindex(heldmodel);  // Hentai's custom gun models

}*/
/* *** */
void ChangeWeapon (edict_t *ent)
{
	char *message=NULL;

/*    if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	} *** Eh? Just a load of pants! */

	/* *** ALLOW checks for weapons *** */
	if (ent->client->newweapon==FindItem("light saber") && !((unsigned long)ann_allow2->value&ANN_ALLOW2_LIGHTSABER))
		message="Light Sabers";
	if (ent->client->newweapon==FindItem("shotgun") && !ent->client->resp.autoshot && !((unsigned long)ann_allow->value&ANN_ALLOW_SHOTGUN))
		message="Shotguns";
	else if (ent->client->newweapon==FindItem("shotgun") && ent->client->resp.autoshot && !((unsigned long)ann_allow->value&ANN_ALLOW_AUTOSHOTGUN))
		message="Auto-shotguns";
	else if (ent->client->newweapon==FindItem("super shotgun") && !((unsigned long)ann_allow->value&ANN_ALLOW_SUPERSHOTGUN))
		message="Super shotguns";
	else if (ent->client->newweapon==FindItem("machinegun") && !((unsigned long)ann_allow->value&ANN_ALLOW_MACHINEGUN))
		message="Machineguns";
	else if (ent->client->newweapon==FindItem("chaingun") && !((unsigned long)ann_allow->value&ANN_ALLOW_CHAINGUN))
		message="Chainguns";
	else if (ent->client->newweapon==FindItem("grenade launcher") && !ent->client->resp.flare_state && !ent->client->resp.cluster_state && !((unsigned long)ann_allow->value&ANN_ALLOW_GRENADELAUNCHER))
		message="Grenade launchers";
	else if (ent->client->newweapon==FindItem("grenade launcher") && !ent->client->resp.flare_state && ent->client->resp.cluster_state && !((unsigned long)ann_allow->value&ANN_ALLOW_CLUSTERGRENADE))
		message="Cluster grenades";
	else if (ent->client->newweapon==FindItem("grenade launcher") && ent->client->resp.flare_state && !ent->client->resp.cluster_state && !((unsigned long)ann_allow->value&ANN_ALLOW_FLARE))
		message="Flares";
	else if (ent->client->newweapon==FindItem("rocket launcher") && !ent->client->resp.bounce && !ent->client->resp.homing_state && !ent->client->resp.guided && !ent->client->resp.nuke_state && !((unsigned long)ann_allow->value&ANN_ALLOW_ROCKETLAUNCHER))
		message="Rocket launchers";
	else if (ent->client->newweapon==FindItem("rocket launcher") && ent->client->resp.bounce && !ent->client->resp.homing_state && !ent->client->resp.guided && !ent->client->resp.nuke_state && !((unsigned long)ann_allow->value&ANN_ALLOW_BOUNCEMISSILE))
		message="Bounce missiles";
	else if (ent->client->newweapon==FindItem("rocket launcher") && !ent->client->resp.bounce && ent->client->resp.homing_state && !ent->client->resp.guided && !ent->client->resp.nuke_state && !((unsigned long)ann_allow->value&ANN_ALLOW_HOMINGMISSILE))
		message="Homing missiles";
	else if (ent->client->newweapon==FindItem("rocket launcher") && !ent->client->resp.bounce && !ent->client->resp.homing_state && ent->client->resp.guided && !ent->client->resp.nuke_state && !((unsigned long)ann_allow->value&ANN_ALLOW_HOMINGMISSILE))
		message="Guided missiles";
	else if (ent->client->newweapon==FindItem("rocket launcher") && !ent->client->resp.bounce && !ent->client->resp.homing_state && !ent->client->resp.guided && ent->client->resp.nuke_state && !((unsigned long)ann_allow->value&ANN_ALLOW_NUKE))
		message="Nukes";
	else if (ent->client->newweapon==FindItem("hyperblaster") && !ent->client->resp.hlaser && !ent->client->resp.freezer && !ent->client->resp.disruptor && !((unsigned long)ann_allow->value&ANN_ALLOW_HYPERBLASTER))
		message="Hyperblasters";
	else if (ent->client->newweapon==FindItem("hyperblaster") && ent->client->resp.hlaser && !ent->client->resp.freezer && !ent->client->resp.disruptor && !((unsigned long)ann_allow->value&ANN_ALLOW_LASERBLASTER))
		message="Laserblasters";
	else if (ent->client->newweapon==FindItem("hyperblaster") && !ent->client->resp.hlaser && ent->client->resp.freezer && !ent->client->resp.disruptor && !((unsigned long)ann_allow->value&ANN_ALLOW_FREEZER))
		message="Freezers";
	else if (ent->client->newweapon==FindItem("hyperblaster") && !ent->client->resp.hlaser && !ent->client->resp.freezer && ent->client->resp.disruptor && !((unsigned long)ann_allow2->value&ANN_ALLOW2_DISRUPTOR))
		message="Disruptors";
	else if (ent->client->newweapon==FindItem("railgun") && !ent->client->resp.sniper && !((unsigned long)ann_allow->value&ANN_ALLOW_RAILGUN))
		message="Railguns";
	else if (ent->client->newweapon==FindItem("railgun") && ent->client->resp.sniper && !((unsigned long)ann_allow->value&ANN_ALLOW_SNIPERRIFLE))
		message="HV sniper rifles";
	else if (ent->client->newweapon==FindItem("bfg10k") && !ent->client->resp.phaser && !((unsigned long)ann_allow->value&ANN_ALLOW_BFG10K))
		message="BFG10Ks";
	else if (ent->client->newweapon==FindItem("bfg10k") && ent->client->resp.phaser && !((unsigned long)ann_allow->value&ANN_ALLOW_PHASER))
		message="Phasers";
	else if (ent->client->newweapon==FindItem("grenades") && !ent->client->resp.pipes && !ent->client->resp.proximity && !((unsigned long)ann_allow->value&ANN_ALLOW_HANDGRENADE))
		message="Hand grenades";
	else if (ent->client->newweapon==FindItem("grenades") && ent->client->resp.pipes && !ent->client->resp.proximity && !((unsigned long)ann_allow->value&ANN_ALLOW_PIPEBOMB))
		message="Pipebombs";
	else if (ent->client->newweapon==FindItem("grenades") && !ent->client->resp.pipes && ent->client->resp.proximity && !((unsigned long)ann_allow->value&ANN_ALLOW_PROXIMITYMINE))
		message="Proximity mines";

	if (message)
	{
		ent->client->newweapon=FindItem("blaster");
	}
	/* *** */

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	/* *** BRRR STATUSBAR *** */
	calcbs(ent);
	/* *** */

    /* *** LIGHT SABER *** */
    if (ent->client->pers.weapon==FindItem("light saber"))
        ent->s.effects |= EF_BLASTER;
    else
        ent->s.effects &= ~EF_BLASTER;

	/* *** TOM *** */
	if (message)
		gi.cprintf(ent,PRINT_HIGH,"%s are disallowed on this server!\n",message);
	/* *** */

	if (!ent->client->pers.weapon || ent->s.modelindex != 255) /* *** VWEP *** */
	{   // dead, or not on client, so VWep animations could do wacky things
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;

	/* *** GUIDED MISSILES *** */
	if (ent->client->rocketview)
	{
        ent->client->missile->takedamage=DAMAGE_NO;
		rocket_touch(ent->client->missile,ent->client->missile,NULL,NULL);
		ent->client->rocketview = false;
		ent->client->missile = NULL;
	}
	/* *** */

	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	/* *** NEW WEAPON MODELS *** */
	if (ent->client->pers.weapon==FindItem("grenades"))
	{
		if (ent->client->resp.pipes)
			ent->client->ps.gunindex = gi.modelindex("models/weapons/v_pipebomb/tris.md2");
		else if (ent->client->resp.proximity)
			ent->client->ps.gunindex = gi.modelindex("models/weapons/v_proximity/tris.md2");
	}
	else if (ent->client->pers.weapon==FindItem("hyperblaster"))
	{
		if (ent->client->resp.disruptor)
			ent->client->ps.gunindex = gi.modelindex("models/weapons/v_disruptor/tris.md2");
   		else if (ent->client->resp.freezer)
			ent->client->ps.gunindex = gi.modelindex("models/weapons/v_freezethrower/tris.md2");
   		else if (ent->client->resp.hlaser)
			ent->client->ps.gunindex = gi.modelindex("models/weapons/v_laserblaster/tris.md2");
	}
	else if (ent->client->pers.weapon==FindItem("railgun"))
	{
		if (ent->client->resp.sniper)
			ent->client->ps.gunindex = gi.modelindex("models/weapons/v_sniper/tris.md2");
	}
	else if (ent->client->pers.weapon==FindItem("bfg10k"))
	{
		if (ent->client->resp.phaser)
			ent->client->ps.gunindex = gi.modelindex("models/weapons/v_phaser/tris.md2");
	}

	/* *** */

	/* *** VWEP *** */
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

	/* *** */
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
    int         ammo_index;
    gitem_t     *ammo_item;

    // see if we're already using it
    if (item == ent->client->pers.weapon)
		return;

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
    int     index;

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
#define FRAME_FIRE_FIRST        (FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST        (FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST  (FRAME_IDLE_LAST + 1)

static void Weapon_Generic2 (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int     n;

	/* *** VWEP *** */
	if(ent->s.modelindex != 255)
		return; // not on client, so VWep animations could do wacky things
	/* *** */

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		/* *** VWEP *** */
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
		/* *** */

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
		/* *** VWEP *** */
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
		/* *** */

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
//ZOID
				if (!CTFApplyStrengthSound(ent))
//ZOID
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
//ZOID
				CTFApplyHasteSound(ent);
//ZOID

				/* *** GUIDED MISSILES *** */
				if (ent->client->rocketview)
				{
                    ent->client->missile->takedamage=DAMAGE_NO;
					rocket_touch(ent->client->missile,ent->client->missile,NULL,NULL);
					ent->client->rocketview = false;
					ent->client->missile = NULL;
				}
				else
				{
				/* *** */
					fire (ent);
					break;
				}
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

	/* *** VWEP *** */
	if (ent->s.modelindex != 255)
		return; // not on client, so VWep animations could do wacky things
	/* *** */

	Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,
		FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames,
		fire_frames, fire);

	// run the weapon frame again if hasted
	if (stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate == WEAPON_FIRING)
		return;

	/* *** PHASER *** */
	if (stricmp(ent->client->pers.weapon->pickup_name, "bfg10k") == 0 &&
    	ent->client->resp.phaser &&
		ent->client->weaponstate == WEAPON_FIRING)
		return;
	/* *** */

	if ((CTFApplyHaste(ent) ||
		((Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate != WEAPON_FIRING)) ||
        ((Q_stricmp(ent->client->pers.weapon->pickup_name, "hyperblaster") == 0 &&
        ent->client->resp.freezer))

        )
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

#define GRENADE_TIMER       3.0
#define GRENADE_MINSPEED    400
#define GRENADE_MAXSPEED    800

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t  offset;
	vec3_t  forward, right;
	vec3_t  start;
	int     damage = 125;
	float   timer;
	int     speed;
	float   radius;

	if (ent->client && ent->client->resp.proximity)
	{
		if (((int)prox_limit->value>=0) && (numprox(ent)>=(int)prox_limit->value))
		{
			gi.cprintf(ent, PRINT_HIGH, "Only %d proximity mines per player!\n",(int)prox_limit->value);
			ent->client->weaponstate = WEAPON_READY;
			return;
		}
		if (!((int)dmflags->value&DF_INFINITE_AMMO))
		{
			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] < PROXIMITY_GRENADES)
			{
				gi.cprintf(ent, PRINT_HIGH, "You need %d grenades for a proximity mine!\n",PROXIMITY_GRENADES);
				ent->client->weaponstate = WEAPON_READY;
				ent->client->resp.proximity=0;
				NoAmmoWeaponChange(ent);
				return;
			}
			else
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))]-=PROXIMITY_GRENADES-1;
			}
		}
	}
	else if (ent->client && ent->client->resp.pipes)
	{
		if (((int)pipe_limit->value>=0) && (numpipes(ent)>=(int)pipe_limit->value))
		{
			gi.cprintf(ent, PRINT_HIGH, "Only %d pipebombs per player!\n",(int)pipe_limit->value);
			ent->client->weaponstate = WEAPON_READY;
			return;
		}
		if (!((int)dmflags->value&DF_INFINITE_AMMO))
		{
			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] < PIPEBOMB_GRENADES)
			{
				gi.cprintf(ent, PRINT_HIGH, "You need %d grenades for a pipebomb!\n",PIPEBOMB_GRENADES);
				ent->client->weaponstate = WEAPON_READY;
				ent->client->resp.pipes=0;
				NoAmmoWeaponChange(ent);
				return;
			}
			else
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))]-=PIPEBOMB_GRENADES-1;
			}
		}
	}

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	/* *** VWEP *** */
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else if(ent->s.modelindex != 255)
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
	/* *** */

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;
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

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			/* *** PIPEBOMB / PROXIMITY MINE *** */
//			&& !ent->client->resp.pipes && !ent->client->resp.proximity
			/* *** */
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			/* *** PIPEBOMBS / PROXIMITY MINES *** */
//			&& !ent->client->resp.pipes && !ent->client->resp.proximity
			/* *** */
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
	vec3_t  offset;
	vec3_t  forward, right;
	vec3_t  start;
	int     damage = 120;
	float   radius;

	if (!((int)dmflags->value&DF_INFINITE_AMMO))
	{
		if (ent->client && ent->client->resp.cluster_state)
		{
			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] < CLUSTER_GRENADES)
			{
				gi.cprintf(ent, PRINT_HIGH, "You need %d grenades for a cluster grenade!\n",CLUSTER_GRENADES);
				ent->client->weaponstate = WEAPON_READY;
				ent->client->resp.cluster_state=0;
				NoAmmoWeaponChange(ent);
				return;
			}
			else
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))]-=CLUSTER_GRENADES-1;
			}
		}
	}

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

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int  pause_frames[]  = {34, 51, 59, 0};
	static int  fire_frames[]   = {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t  offset, start;
	vec3_t  forward, right;
	int     damage;
	float   damage_radius;
	int     radius_damage;

	/* *** HOMING MISSILES and NUKES and BOUNCE ROCKETS *** */
	if (!((int)dmflags->value&DF_INFINITE_AMMO))
	{
		if ((ent->client && ent->client->resp.homing_state) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < HOMING_ROCKETS))
		{
			gi.cprintf(ent, PRINT_HIGH, "You need %d rockets for a homing missile!\n",HOMING_ROCKETS);
			ent->client->weaponstate = WEAPON_READY;
			ent->client->resp.homing_state=0;
			NoAmmoWeaponChange(ent);
			return;
		}
		else if ((ent->client && ent->client->resp.nuke_state) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < NUKE_ROCKETS))
		{
			gi.cprintf(ent, PRINT_HIGH, "You need %d rockets for a nuke!\n",NUKE_ROCKETS);
			ent->client->weaponstate = WEAPON_READY;
			ent->client->resp.nuke_state=0;
			NoAmmoWeaponChange(ent);
			return;
		}
		else if ((ent->client && ent->client->resp.bounce) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < BOUNCE_ROCKETS))
		{
			gi.cprintf(ent, PRINT_HIGH, "You need %d rockets for a bounce missile!\n",BOUNCE_ROCKETS);
			ent->client->weaponstate = WEAPON_READY;
			ent->client->resp.bounce=0;
			NoAmmoWeaponChange(ent);
			return;
		}
		else if ((ent->client && ent->client->resp.guided) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < GUIDED_ROCKETS))
		{
			gi.cprintf(ent, PRINT_HIGH, "You need %d rockets for a guided missile!\n",GUIDED_ROCKETS);
			ent->client->resp.guided=0;
			NoAmmoWeaponChange(ent);
			ent->client->weaponstate = WEAPON_READY;
			return;
		}
	}

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

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int  pause_frames[]  = {25, 33, 42, 50, 0};
	static int  fire_frames[]   = {5, 0};

	ent->classname="rlfire";
	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
	ent->classname="";
}


/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	 vec3_t  forward, right;
	 vec3_t  start;
	 vec3_t  offset;

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


void Weapon_Blaster_Fire (edict_t *ent)
{
	int     damage;

	if (deathmatch->value)
		damage = 15;
	else
		damage = 10;
	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int  pause_frames[]  = {19, 32, 0};
	static int  fire_frames[]   = {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	 float   rotation;
	 vec3_t  offset;
	 int     effect;
	 int     damage;

	 /* *** DISRUPTOR RIFLE *** */
	 if (!((int)dmflags->value&DF_INFINITE_AMMO))
	 {
		 if ((ent->client && ent->client->resp.disruptor) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < DISRUPTOR_CELLS))
		 {
			  gi.cprintf(ent, PRINT_HIGH, "You need %d cells to fire the disruptor!\n",DISRUPTOR_CELLS);
			  ent->client->weaponstate = WEAPON_READY;
			  ent->client->resp.disruptor=0;
			  NoAmmoWeaponChange(ent);
			  ent->client->weapon_sound = 0;
			  return;
		 }
	 }

	 if (!ent->client->resp.disruptor)
	 /* *** */
		 ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	 if (!(ent->client->buttons & BUTTON_ATTACK))
	 {
		/* *** DISRUPTOR *** */
		if (ent->client->resp.disruptor && ent->client->ps.gunframe>=9 && ent->client->ps.gunframe<=19)
			ent->client->ps.gunframe=20;
		else
        /* *** */
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
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;
			Blaster_Fire (ent, offset, damage, true, effect);

			/* *** VWEP *** */
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
			/* *** */

			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;
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
	static int  pause_frames[]  = {0};
	static int  fire_frames[10];/*[]   = {6, 7, 8, 9, 10, 11, 0};*/

	if (ent->client->resp.disruptor)
	{
		fire_frames[0]=10;
		fire_frames[1]=14;
		fire_frames[2]=18;
		fire_frames[3]=0;

		Weapon_Generic (ent, 5, 31, 33, 37, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
	}
	else
	{
		fire_frames[0]=6;
		fire_frames[1]=7;
		fire_frames[2]=8;
		fire_frames[3]=9;
		fire_frames[4]=10;
		fire_frames[5]=11;
		fire_frames[6]=0;

		Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
	}
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire (edict_t *ent)
{
    int i;
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      angles;
    int         damage = 8;
	int         kick = 2;
    vec3_t      offset;

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
    fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

    gi.WriteByte (svc_muzzleflash);
    gi.WriteShort (ent-g_edicts);
    gi.WriteByte (MZ_MACHINEGUN | is_silenced);
    gi.multicast (ent->s.origin, MULTICAST_PVS);

    PlayerNoise(ent, start, PNOISE_WEAPON);

    /* *** VWEP *** */
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
    /* *** */

    if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
        ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Machinegun (edict_t *ent)
{
    static int  pause_frames[]  = {23, 45, 0};
    static int  fire_frames[]   = {4, 5, 0};

    Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire (edict_t *ent)
{
    int         i;
    int         shots;
    vec3_t      start;
    vec3_t      forward, right, up;
    float       r, u;
	vec3_t      offset;
    int         damage;
    int         kick = 2;

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

        fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
    }

    // send muzzle flash
    gi.WriteByte (svc_muzzleflash);
    gi.WriteShort (ent-g_edicts);
    gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
    gi.multicast (ent->s.origin, MULTICAST_PVS);

    PlayerNoise(ent, start, PNOISE_WEAPON);

	/* *** VWEP *** */
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
    /* *** */


    if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
        ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun (edict_t *ent)
{
    static int  pause_frames[]  = {38, 43, 51, 61, 0};
    static int  fire_frames[]   = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

    Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire (edict_t *ent)
{
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      offset;
    int         damage = 4;
    int         kick = 8;
    int         mod;

    /* *** AUTO SHOTGUN *** */
    if (ent->client->resp.autoshot)
		mod=MOD_AUTOSHOT;
    else
        mod=MOD_SHOTGUN;

    if (ent->client->resp.autoshot && ent->client->resp.autocount==0)
    {
        ent->client->resp.autocount=AUTO_SHOTS-1;
    }
    /* *** */

    if (ent->client->ps.gunframe == 9)
    {
        /* *** AUTO SHOTGUN *** */
        if (ent->client->resp.autoshot)
        {
            if (ent->client->pers.inventory[ent->client->ammo_index]==0 || ent->client->resp.autocount==0)
            {
                ent->client->ps.gunframe++;
                return;
            }
            else
            {
                ent->client->ps.gunframe=8;
                ent->client->resp.autocount--;
			}
        }
        else
        /* *** */
        {
            ent->client->ps.gunframe++;
            return;
        }
    }

    /* *** AUTO SHOTGUN *** */
    if (ent->client->resp.autoshot && ent->client->resp.autocount==0)
    {
        ent->client->resp.autocount=AUTO_SHOTS-1;
        ent->client->ps.gunframe=10;
        return;
    }
    /* *** */

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

    /* *** AUTO SHOTGUN *** */
    if (ent->client->resp.autoshot)
    {
        damage*=0.75;
        kick*=4;
    }
    /* *** */

    if (deathmatch->value)
        fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, mod);
    else
        fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, mod);

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
    static int  pause_frames[]  = {22, 28, 34, 0};
    static int  fire_frames[]   = {8, 9, 0};

    Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
    vec3_t      start;
    vec3_t      forward, right;
	vec3_t      offset;
    vec3_t      v;
    int         damage = 6;
    int         kick = 12;

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
	static int  pause_frames[]  = {29, 42, 57, 0};
	static int  fire_frames[]   = {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire (edict_t *ent)
{
	vec3_t      start;
	vec3_t      forward, right;
	vec3_t      offset;
	int         damage;
	int         kick;

	if (!((int)dmflags->value&DF_INFINITE_AMMO))
	{
		if ((ent->client && ent->client->resp.sniper) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] < SNIPER_SLUGS))
		{
			gi.cprintf(ent, PRINT_HIGH, "You need %d slugs to fire the sniper rifle!\n",SNIPER_SLUGS);
			ent->client->resp.sniper=0;
			NoAmmoWeaponChange(ent);
			ent->client->weaponstate = WEAPON_READY;
			ChangeWeapon(ent);
			return;
		}
	}

	if (deathmatch->value)
	{   // normal damage is too extreme in dm
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
	/* *** SNIPER RIFLE *** */
	if (!ent->client->resp.sniper)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	/* *** */

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int  pause_frames[2];/*[]  = {56, 0};*/
	static int  fire_frames[2];/*[]   = {4, 0};*/

	if (ent->client->resp.sniper)
	{
		pause_frames[0]=57;
		pause_frames[1]=0;

		fire_frames[0]=5;
		fire_frames[1]=0;

		Weapon_Generic (ent, 3, 23, 57, 61, pause_frames, fire_frames, weapon_railgun_fire);
	}
	else
	{
		pause_frames[0]=56;
		pause_frames[1]=0;

		fire_frames[0]=4;
		fire_frames[1]=0;

		Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
	}
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent)
{
	vec3_t  offset, start;
	vec3_t  forward, right;
	int     damage;
	float   damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	/* *** PHASER *** */
	if (ent->client && ent->client->resp.phaser)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < PHASER_CELLS)
		{
			gi.cprintf(ent, PRINT_HIGH, "You need %d cells to fire the phaser!\n",PHASER_CELLS);
			ent->client->resp.phaser=0;
			NoAmmoWeaponChange(ent);
			ent->client->weaponstate = WEAPON_READY;
			return;
		}
	}
	/* *** */

	/* *** PHASER *** */
	if (ent->client->ps.gunframe == 4)
	{
		if (ent->client->resp.phaser)
		{
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/phaser.wav"), 1, ATTN_NORM, 0);

			ent->client->ps.gunframe++;
			return;
		}
	}
	/* *** */

	if (ent->client->ps.gunframe == 9)
	{
		/* *** PHASER *** */
		if (!ent->client->resp.phaser)
		/* *** */
		{
			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BFG | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			PlayerNoise(ent, start, PNOISE_WEAPON);
		}

		ent->client->ps.gunframe++;
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

	/* *** PHASER *** */
	if (!ent->client->resp.phaser)
	{
		// make a big pitch kick with an inverse fall
		ent->client->v_dmg_pitch = -40;
		ent->client->v_dmg_roll = crandom()*8;
		ent->client->v_dmg_time = level.time + DAMAGE_TIME;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	/* *** PHASER *** */
	if (ent->client->resp.phaser)
		fire_phaser (ent, start, forward, damage, 400, damage_radius);
	else
	/* *** */
		fire_bfg (ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	/* *** PHASER *** */
	if (!ent->client->resp.phaser)
		PlayerNoise(ent, start, PNOISE_WEAPON);
	/* *** */

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		/* *** PHASER *** */
		if (ent->client->resp.phaser)
			ent->client->pers.inventory[ent->client->ammo_index] -= PHASER_CELLS;
		else
			ent->client->pers.inventory[ent->client->ammo_index] -= 50;
	}
}

void Weapon_BFG (edict_t *ent)
{
	static int  pause_frames[10];	/* {39, 45, 50, 55, 0} */
	static int  fire_frames[10];	/* {9, 17, 0} */

	if (ent->client->resp.phaser)
	{
		pause_frames[0]=56;
		pause_frames[1]=57;
		pause_frames[2]=58;
		pause_frames[3]=58;
		pause_frames[4]=1;
		pause_frames[5]=2;
		pause_frames[6]=3;
		pause_frames[7]=0;

		fire_frames[0]=4;
		fire_frames[1]=18;
		fire_frames[2]=0;

		Weapon_Generic (ent, 3, 50, 53, 58, pause_frames, fire_frames, weapon_bfg_fire);
	}
	else
	{
		pause_frames[0]=39;
		pause_frames[1]=45;
		pause_frames[2]=50;
		pause_frames[3]=55;
		pause_frames[4]=0;

		fire_frames[0]=9;
		fire_frames[1]=17;
		fire_frames[2]=0;

		Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
	}
}

/* *** LIGHT SABER *** */
#include "g_saber.h"
/* *** */

//======================================================================
