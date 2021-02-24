#include "g_local.h"
#include "m_player.h"

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

/*
=================
Calc_Arc - This is a support routine for keeping an object velocity-aligned
           I.E for arrows/darts
=================
*/

void Calc_Arc (edict_t *ent)
{
        vec3_t move;

        vectoangles(ent->velocity, move);
        VectorSubtract(move, ent->s.angles, move);

        move[0] = fmod((move[0] + 180), 360) - 180;
        move[1] = fmod((move[1] + 180), 360) - 180;
        move[2] = fmod((move[2] + 180), 360) - 180;
        VectorScale(move, 1/FRAMETIME, ent->avelocity);
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

//======================================================================

qboolean CheckAltAmmo (edict_t *ent, gitem_t *curweapon, int ammoTag)
{
	switch(curweapon->tag)
	{
	case II_SUBMACH:
		if (ammoTag == II_SUBMACH_CLIP)
			return true;
		if (ammoTag == II_HV_SUBMACH_CLIP)
			return true;
		break;

	case II_SHOTGUN:
		if (ammoTag == II_SHOTGUN_CLIP)
			return true;
		if (ammoTag == II_EXP_SHOTGUN_CLIP)
			return true;
		if (ammoTag == II_SOLID_SHOTGUN_CLIP)
			return true;
		break;

	case II_GRENADE_LAUNCHER:
		if (ammoTag == II_FRAG_GRENADES)
			return true;
		if (ammoTag == II_HEP_GRENADES)
			return true;
		if (ammoTag == II_EMP_GRENADES)
			return true;
		break;


	default:
		break;
	}

	return false;
}

char *GetTwoWeaponViewModel(gitem_t *right, gitem_t *left)
{
        if (right->tag == II_PISTOL)
        {
                if (left->tag == II_PISTOL)
                        return "models/v_twin_pistols/tris.md2";
        }

        if (right->tag == II_SUBMACH)
        {
                if (left->tag == II_SUBMACH)
                        return "models/v_twin_submach/tris.md2";
        }

        if (right->tag == II_SHOTGUN)
        {
                if (left->tag == II_SHOTGUN)
                        return "models/v_twin_shotguns/tris.md2";
        }
        
        return NULL;
}

int GetTwoWeaponModelIndex2Tag(gitem_t *right, gitem_t *left)
{
        if (right->tag == II_PISTOL)
        {
                if (left->tag == II_PISTOL)
                        return II_TWIN_PISTOL;
        }

        if (right->tag == II_SUBMACH)
        {
                if (left->tag == II_SUBMACH)
                        return II_TWIN_SUBMACH;
        }

        if (right->tag == II_SHOTGUN)
        {
                if (left->tag == II_SHOTGUN)
                        return II_TWIN_SHOTGUN;
        }
        
        return 0;
}

qboolean TwoWeaponComboOk(edict_t *ent, int rtag, int ltag)
{
        gitem_t *right, *left;
        //qboolean two;
        //char *viewmodel;
        //int i, w2tag;
        
        right = NULL;
        if (rtag > II_HANDS)
                right = GetItemByTag(rtag);

        left = NULL;
        if (ltag  > II_HANDS)
                left = GetItemByTag(ltag);

        if (!right || !left)
        {
        	if (right && !left)
        		return true;
        		
                return false;
	}

	// if this is a grenade, just setup the grenade type
	if ((left->tag == II_FRAG_HANDGRENADE) || (left->tag == II_EMP_HANDGRENADE))
	{
	        ent->client->pers.hgren_type = left->tag;
	        return false;
	}
	
        return (GetTwoWeaponViewModel(right, left) != NULL);
}

void SetupItemModels(edict_t *ent)
{
        gitem_t *right, *left;
        qboolean two;
        char *viewmodel = NULL;
        int i, w2tag;
        
        two = false;
        
        right = NULL;
        if (ent->client->pers.cstats[CSTAT_RIGHTHAND] > II_HANDS)
                right = GetItemByTag(ent->client->pers.cstats[CSTAT_RIGHTHAND]);

        left = NULL;
        if (ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS)
                left = GetItemByTag(ent->client->pers.cstats[CSTAT_LEFTHAND]);
        
        if (right && left)
        {
                viewmodel = GetTwoWeaponViewModel(right, left);
                w2tag = GetTwoWeaponModelIndex2Tag(right, left);
        }
        else if (right)
        {
        	if (right->view_model)
                	viewmodel = right->view_model;
		else if (right->world_model)
                	viewmodel = right->world_model;
                w2tag = right->tag;
        }
        else if (left)
        {
        	if (left->view_model)
                	viewmodel = left->view_model;
		else if (left->world_model)
                	viewmodel = left->world_model;
                w2tag = left->tag;
        }
        else
        {
                //gi.dprintf("SetupItemModels : unknown weapon(s)\n");
        	ent->client->ps.gunframe = 0;
                viewmodel = NULL;
                w2tag = -1;
        }
        
        w2tag -= 1;

	// set weapon sound
	ent->client->weapon_sound = 0;
        
	// set view model
	if (viewmodel)
	{
                //gi.dprintf("SetupItemModels : viewmodel = %s\n", viewmodel);
	        ent->client->ps.gunindex = gi.modelindex(viewmodel);
	}
	else
	{
	        ent->client->ps.gunindex = 0;
        }

	// set visible model
	if (ent->s.modelindex == 255) 
	{
		if (w2tag > 0)
			i = ((w2tag & 0xff) << 8);
		else
			i = 0;

		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}
}

//======================================================================

void ChangeLeftWeapon(edict_t *ent)
{
        gitem_t *item;
        int bodyarea;
        
        //gi.dprintf("ChangeLeftWeapon\n");

	if (ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS)
	{
		ent->client->previous_left_tag = ent->client->pers.cstats[CSTAT_RIGHTHAND];
                item = GetItemByTag(ent->client->pers.cstats[CSTAT_LEFTHAND]);
                bodyarea = GetFreeBodyArea(item, ent);
                if (bodyarea > -1)
                {
			ent->client->previous_left_bodyarea = bodyarea;
                        StashItem (ent, item, bodyarea, ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO], ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS], ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE]);
		}
        }

	ent->client->ps.gunframe = 0;
        ent->client->pers.cstats[CSTAT_LEFTHAND] = II_HANDS;
        ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = 0;
        ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS] = 0;
        ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE] = 0;

        // special handling
        if ((ent->client->newLeftWeapon != -1) && (ent->client->pers.item_bodyareas[ent->client->newLeftWeapon] > II_HANDS))
        {
                ent->client->pers.cstats[CSTAT_LEFTHAND] = ent->client->pers.item_bodyareas[ent->client->newLeftWeapon];
                ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = ent->client->pers.item_quantities[ent->client->newLeftWeapon];
                ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS] = ent->client->pers.item_flags[ent->client->newLeftWeapon];
                ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE] = ent->client->pers.item_ammotypes[ent->client->newLeftWeapon];
		ent->client->weaponstate = W_LEFT_RAISING;
                RemoveItem(ent, ent->client->newLeftWeapon);
        }
        else
		ent->client->weaponstate = W_READY;

        ent->client->newLeftWeapon = -1;
        ent->client->newRightWeapon = -1;
        
        SetupItemModels(ent);
}

void ChangeRightWeapon(edict_t *ent)
{
        gitem_t *item;
        int bodyarea;
        
        //gi.dprintf("ChangeRightWeapon\n");

	if (ent->client->pers.cstats[CSTAT_RIGHTHAND] > II_HANDS)
	{
		ent->client->previous_right_tag = ent->client->pers.cstats[CSTAT_RIGHTHAND];
                item = GetItemByTag(ent->client->pers.cstats[CSTAT_RIGHTHAND]);
                bodyarea = GetFreeBodyArea(item, ent);
                if (bodyarea > -1)
                {
			ent->client->previous_right_bodyarea = bodyarea;
                        StashItem (ent, item, bodyarea, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO], ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE]);
		}
        }

	ent->client->ps.gunframe = 0;

        ent->client->pers.cstats[CSTAT_RIGHTHAND] = II_HANDS;
        ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
        ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = 0;
        ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] = 0;

        // special handling
        if ((ent->client->newRightWeapon != -1) && (ent->client->pers.item_bodyareas[ent->client->newRightWeapon] > II_HANDS))
        {
                ent->client->pers.cstats[CSTAT_RIGHTHAND] = ent->client->pers.item_bodyareas[ent->client->newRightWeapon];
                ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = ent->client->pers.item_quantities[ent->client->newRightWeapon];
                ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = ent->client->pers.item_flags[ent->client->newRightWeapon];
                ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] = ent->client->pers.item_ammotypes[ent->client->newRightWeapon];
		ent->client->weaponstate = W_RIGHT_RAISING;
                RemoveItem(ent, ent->client->newRightWeapon);
        }
        else
		ent->client->weaponstate = W_READY;

	// if we have an offhand weapon...
	if (ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS)
	{	// ... but don't have a goodhand weapon, switch hands...
		if (ent->client->pers.cstats[CSTAT_RIGHTHAND] <= II_HANDS)
		{
                	ent->client->pers.cstats[CSTAT_RIGHTHAND] = ent->client->pers.cstats[CSTAT_LEFTHAND];
                	ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO];
        	        ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS];
	                ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE];
                	ent->client->pers.cstats[CSTAT_LEFTHAND] = 0;
                	ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = 0;
        	        ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS] = 0;
	                ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE] = 0;
		}// ...else check to see if we can still use our offhand weapon
	 	else if (!TwoWeaponComboOk(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND], ent->client->pers.cstats[CSTAT_LEFTHAND]))
			ChangeLeftWeapon(ent);
	}

        ent->client->newLeftWeapon = -1;
        ent->client->newRightWeapon = -1;
        
        SetupItemModels(ent);
}

//======================================================================

void AutoSwitchOld(edict_t *ent)
{
	int rba, lba;
	
	rba = ent->client->previous_right_bodyarea;
	lba = ent->client->previous_left_bodyarea;

	if (ent->client->pers.item_bodyareas[rba] == ent->client->previous_right_tag)
	{
        	ent->client->newRightWeapon = rba;
		ChangeRightWeapon(ent);
	}
	
	if (ent->client->pers.item_bodyareas[lba] == ent->client->previous_left_tag)
	{
        	ent->client->newLeftWeapon = lba;
		ChangeLeftWeapon(ent);
	}

	ent->client->previous_right_bodyarea = 0;
	ent->client->previous_left_bodyarea = 0;
	ent->client->previous_right_tag = -1;
	ent->client->previous_left_tag = -1;
}

void AutoSwitchAnything(edict_t *ent)
{
	int i;
	
	AutoSwitchOld(ent);

	if (ent->client->pers.cstats[CSTAT_RIGHTHAND] != II_HANDS)
		return;

	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
	{
		if ((ent->client->pers.item_bodyareas[i] < II_MAX_WEAPONS)
		 && (ent->client->pers.item_bodyareas[i] > II_HANDS))
		{
        		ent->client->newRightWeapon = i;
			ChangeRightWeapon(ent);
			break;
		}
	}
}

void AutoSwitchWeapon(edict_t *ent, int last_right, int last_left)
{
	int i, right, left;
	right = -1;
	left = -1;

	if ((last_right == 0) && (last_left == 0))
	{
		AutoSwitchAnything(ent);
		return;
	}

	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
	{
		if ((last_right > II_HANDS) && (right == -1) && (ent->client->pers.item_bodyareas[i] == last_right) && (i != left))
        		right = i;
		if ((last_left > II_HANDS) && (left == -1) && (ent->client->pers.item_bodyareas[i] == last_left) && (i != right))
        		left = i;
		if ((right != -1) && (left != -1))
			break;
	}

	if ((right == -1) && (left == -1))
	{
		AutoSwitchAnything(ent);
		return;
	}
	
	if (right != -1)
	{
        	ent->client->newRightWeapon = right;
		ChangeRightWeapon(ent);
	}

	if (left != -1)
	{
        	ent->client->newLeftWeapon = left;
		ChangeLeftWeapon(ent);
	}
}
	

void Think_Pistol(edict_t *ent);
void Think_TwinPistol(edict_t *ent);
void Think_SubMach(edict_t *ent);
void Think_TwinSubMach(edict_t *ent);
void Think_Hands(edict_t *ent);
void Think_TwinShotgun(edict_t *ent);
void Think_Shotgun(edict_t *ent);
void Think_HandGrenade(edict_t *ent);
void Think_Chaingun(edict_t *ent);
void Think_GrenadeLauncher(edict_t *ent);
void Think_AssaultRifle(edict_t *ent);
void Think_Railgun(edict_t *ent);
void Think_Health(edict_t *ent);
void Think_HealthLarge(edict_t *ent);
void Think_WEdit(edict_t *ent);
void Think_Armor(edict_t *ent);
void Think_StroggSoldierWeapon(edict_t *ent);
void Think_BitchWeapon(edict_t *ent);
void Think_GunnerWeapon(edict_t *ent);
void Think_InfWeapon(edict_t *ent);
void Think_TankWeapon(edict_t *ent);
void Think_MedicWeapon(edict_t *ent);

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{
        gclient_t *client;
        gitem_t *item;
        int bodyarea;

        ////gi.dprintf("Think_Weapon : stat_righthand = %i\n", ent->client->pers.cstats[CSTAT_RIGHTHAND]);

	// if just died, put the weapon away
	if (ent->health < 1)
	        return;
		
        client = ent->client;
        
        if (!client)
                return; // not a client, wtf you doing here?

	if (client->chase_target)
		return;

        if (level.time - ent->client->cycleItems <= CYCLE_ITEMS_TIME)
        {
                bodyarea = -1;
                if (ent->client->latched_buttons & BUTTON_ATTACK)
                {
        	        ent->client->latched_buttons &= ~(BUTTON_ATTACK | BUTTON_ALT_ATTACK);
	                ent->client->buttons &= ~(BUTTON_ATTACK | BUTTON_ALT_ATTACK);

                        if (ent->client->pers.cstats[CSTAT_RIGHTHAND] > II_HANDS)
                        {
                                item = GetItemByTag(ent->client->pers.cstats[CSTAT_RIGHTHAND]);
                                if (item)
                                {
					if (CheckAltAmmo(ent, item, ent->client->sortedItems[ent->client->itemSelect]))
					{
						ent->client->weaponstate = W_START_RIGHT_RELOAD;
                		        	ent->client->cycleItems = -10;
		                	        ent->client->newRightAmmoType = ent->client->sortedItems[ent->client->itemSelect];
		                	        return;
					}
					else
					{
                                        	bodyarea = GetFreeBodyArea(item, ent);
                                        	if (bodyarea == -1)
                                                	ThrowRightHandItem(ent, 50);
					}
                                }
                        }
                        ent->client->cycleItems = -10;
                        ent->client->newRightWeapon = ent->client->sortedItemBodyAreas[ent->client->itemSelect];
	                return;
                }
                else if ((ent->client->latched_buttons & BUTTON_ALT_ATTACK) && TwoWeaponComboOk(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND], ent->client->pers.item_bodyareas[ent->client->sortedItemBodyAreas[ent->client->itemSelect]]))
                {
        	        ent->client->latched_buttons &= ~(BUTTON_ATTACK | BUTTON_ALT_ATTACK);
	                ent->client->buttons &= ~(BUTTON_ATTACK | BUTTON_ALT_ATTACK);

                        if (ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS)
                        {
                                item = GetItemByTag(ent->client->pers.cstats[CSTAT_LEFTHAND]);
                                if (item)
                                {
					if (CheckAltAmmo(ent, item, ent->client->sortedItems[ent->client->itemSelect]))
					{
						ent->client->weaponstate = W_START_RIGHT_RELOAD;
                		        	ent->client->cycleItems = -10;
		                	        ent->client->newLeftAmmoType = ent->client->sortedItems[ent->client->itemSelect];
		                	        return;
					}
					else
					{
        	                                bodyarea = GetFreeBodyArea(item, ent);
	                                        if (bodyarea == -1)
                	                                ThrowLeftHandItem(ent, 50);
                                	}
                        	}
			}
                        ent->client->cycleItems = -10;
                        ent->client->newLeftWeapon = ent->client->sortedItemBodyAreas[ent->client->itemSelect];
	                return;
                }
        	else if (ent->client->action)
                        ent->client->cycleItems = -10;
        }
                
        switch (client->pers.cstats[CSTAT_RIGHTHAND])
        {
        case II_PISTOL:
                switch (client->pers.cstats[CSTAT_LEFTHAND])
                {
                case II_PISTOL:
                        Think_TwinPistol(ent);
                        break;
		
                case II_HANDS:
                default:
                        Think_Pistol(ent);
                        break;
                }
                break;

        case II_SUBMACH:
                switch (client->pers.cstats[CSTAT_LEFTHAND])
                {
                case II_SUBMACH:
                        Think_TwinSubMach(ent);
                        break;
		
                case II_HANDS:
                default:
                        Think_SubMach(ent);
                        break;
                }
                break;

        case II_SHOTGUN:
                switch (client->pers.cstats[CSTAT_LEFTHAND])
                {
                case II_SHOTGUN:
                        Think_TwinShotgun(ent);
                        break;
		
                case II_HANDS:
                default:
                        Think_Shotgun(ent);
                        break;
                }
                break;

        case II_FRAG_HANDGRENADE:
        case II_EMP_HANDGRENADE:
                switch (client->pers.cstats[CSTAT_LEFTHAND])
                {
                case II_HANDS:
                default:
                        Think_HandGrenade(ent);
                        break;
                }
                break;

        case II_CHAINGUN:
                switch (client->pers.cstats[CSTAT_LEFTHAND])
                {
                case II_HANDS:
                default:
                        Think_Chaingun(ent);
                        break;
                }
                break;

        case II_GRENADE_LAUNCHER:
                switch (client->pers.cstats[CSTAT_LEFTHAND])
                {
                case II_HANDS:
                default:
                        Think_GrenadeLauncher(ent);
                        break;
                }
                break;

        case II_ASSAULT_RIFLE:
                switch (client->pers.cstats[CSTAT_LEFTHAND])
                {
                case II_HANDS:
                default:
			Think_AssaultRifle(ent);
                        break;
                }
                break;

        case II_RAILGUN:
                switch (client->pers.cstats[CSTAT_LEFTHAND])
                {
                case II_HANDS:
                default:
			Think_Railgun(ent);
                        break;
                }
                break;

        case II_HEALTH:
                Think_Health(ent);
                break;

        case II_HEALTH_LARGE:
                Think_HealthLarge(ent);
                break;

	case II_WEAPON_EDIT:
		Think_WEdit(ent);
		break;

	case II_JACKET_ARMOUR:
	case II_COMBAT_ARMOUR:
	case II_BODY_ARMOUR:
	case II_BANDOLIER:
	case II_BACK_PACK:
		Think_Armor(ent);
		break;

	case II_STROGG_BLASTER:
	case II_STROGG_SHOTGUN:
	case II_STROGG_SUBMACH:
		Think_StroggSoldierWeapon(ent);
		break;

	case II_BITCH_ROCKET_LAUNCHER:
		Think_BitchWeapon(ent);
		break;

	case II_GUN_CHAINGUN:
		Think_GunnerWeapon(ent);
		break;

	case II_INF_CHAINGUN:
		Think_InfWeapon(ent);
		break;

	case II_TANK_ROCKET_LAUNCHER:
		Think_TankWeapon(ent);
		break;

	case II_MEDIC_HYPER_BLASTER:
		Think_MedicWeapon(ent);
		break;
	
        case II_HANDS:
        default:
                Think_Hands(ent);
                break;
        }
}
