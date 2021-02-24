#include "g_local.h"
#include "m_player.h"

void CmdGotoSecret(edict_t *ent);
/*
==============
ThrowRightHandItem
==============
*/
void ThrowRightHandItem(edict_t *ent, int speed )
{
	vec3_t	velocity, angles, org;
	vec3_t	forward, right, up;
	edict_t *dropped;
        gitem_t *item, *ammoItem;

	ammoItem = NULL;
	
        if (ent->client->pers.cstats[CSTAT_RIGHTHAND] <= II_HANDS)
                return;
                
        item = GetItemByTag(ent->client->pers.cstats[CSTAT_RIGHTHAND]);

        if (!item)
                return;

        // calc angles
	VectorCopy(ent->client->v_angle, angles );
        if (ent->health <= 0)
                angles[PITCH] = 0;      // flat
	AngleVectors( angles, velocity, NULL, NULL );
        VectorScale(velocity, speed, velocity );

        // calc position
	AngleVectors (angles, forward, right, up);
	VectorCopy(ent->s.origin, org);

        VectorMA(org, 16, up, org);
        VectorMA(org, 12, forward, org);
        VectorMA(org, 8, right, org);

        dropped = LaunchItem(ent, item, org, velocity);
        
        if (dropped)
        {
                dropped->nextthink = level.time + 5;
                dropped->s.frame = 0;

		ammoItem = GetItemByTag(item->ammoTag);
		if (ammoItem && (ammoItem->flags & IT_CLIP))
		{
	                dropped->count = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];
        	        if ((item->quantity > 0) && (dropped->count > 0)) // the lower the ammo, the faster it disappears
	                        dropped->nextthink += (item->quantity / dropped->count) * 28;
			//else
	                        //dropped->s.frame += 2;
		}
		else if ((item->flags & IT_ARMOR) || (item->flags & IT_WEAPON))
	                dropped->count = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];
		else
		{
			dropped->nextthink = level.time + 30;
		}
		
		// special handling for chaingun, which always comes with a pack
		if ((dropped->item->tag == II_CHAINGUN) || (dropped->item->tag == II_CHAINGUN_PACK))
		{
			int i;
			
			if (dropped->item->tag == II_CHAINGUN_PACK)
		                dropped->count = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];

			dropped->item = GetItemByTag(II_CHAINGUN);
			
			for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        		{
        			if (ent->client->pers.item_bodyareas[i] == II_CHAINGUN_PACK)
        			{
        				dropped->count = ent->client->pers.item_quantities[i];
					ent->client->pers.item_quantities[i] = 0;
					RemoveItem(ent, i);
				}

        			if (ent->client->pers.item_bodyareas[i] == II_CHAINGUN)
					RemoveItem(ent, i);
			}
		}

                // copy item flags to modelindex2
                //      eg. empty, alt ammo etc
                dropped->viewheight |= ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS];
		dropped->last_fire = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE];

		if ((item->tag == II_FRAG_HANDGRENADE) || (item->tag == II_EMP_HANDGRENADE))
		{
			if (dropped->count > 1)
			{
				dropped->count = 1;
				ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 1;
		        	gi.linkentity (dropped);
				return;
			}
		}
		
        	gi.linkentity (dropped);
        }

	ent->client->weapon_sound = 0;
        ent->client->pers.cstats[CSTAT_RIGHTHAND] = II_HANDS;
        ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
        ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = 0;
}

/*
==============
ThrowLeftHandItem
==============
*/
void ThrowLeftHandItem(edict_t *ent, int speed )
{
	vec3_t	velocity, angles, org;
	vec3_t	forward, right, up;
	edict_t *dropped;
        gitem_t *item, *ammoItem;

	ammoItem = NULL;

        if (ent->client->pers.cstats[CSTAT_LEFTHAND]<= II_HANDS)
                return;

        item = GetItemByTag(ent->client->pers.cstats[CSTAT_LEFTHAND]);

        if (!item)
                return;

        // calc angles
	VectorCopy(ent->client->v_angle, angles );
        if (ent->health <= 0)
                angles[PITCH] = 0;      // flat
	AngleVectors( angles, velocity, NULL, NULL );
        VectorScale(velocity, speed, velocity );

        // calc position
	AngleVectors (angles, forward, right, up);
	VectorCopy(ent->s.origin, org);

        if (forward[2] < 0)
                VectorMA(org, 2 * forward[2], up, org);
        else if (forward[2] > 0)
                VectorMA(org, -2 * forward[2], up, org);

        VectorMA(org, 16, up, org);
        VectorMA(org, 12, forward, org);
        VectorMA(org, -8, right, org);

        dropped = LaunchItem(ent, item, org, velocity );
        
        if (dropped)
        {
                dropped->nextthink = level.time + 5;
                dropped->s.frame = 0;

		ammoItem = GetItemByTag(item->ammoTag);
		if (ammoItem && (ammoItem->flags & IT_CLIP))
		{
	                dropped->count = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO];
        	        if ((item->quantity > 0) && (dropped->count > 0)) // the lower the ammo, the faster it disappears
	                        dropped->nextthink += (item->quantity / dropped->count) * 28;
			//else
	                        //dropped->s.frame += 2;
		}
		else if ((item->flags & IT_ARMOR) || (item->flags & IT_WEAPON))
	                dropped->count = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO];
		else
		{
			dropped->nextthink = level.time + 30;
		}

		// special handling for chaingun, which always comes with a pack
		if ((dropped->item->tag == II_CHAINGUN) || (dropped->item->tag == II_CHAINGUN_PACK))
		{
			int i;

			if (dropped->item->tag == II_CHAINGUN_PACK)
		                dropped->count = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO];
			
			dropped->item = GetItemByTag(II_CHAINGUN);
			
			for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        		{
        			if (ent->client->pers.item_bodyareas[i] == II_CHAINGUN_PACK)
        			{
        				dropped->count = ent->client->pers.item_quantities[i];
					ent->client->pers.item_quantities[i] = 0;
					RemoveItem(ent, i);
				}

        			if (ent->client->pers.item_bodyareas[i] == II_CHAINGUN)
					RemoveItem(ent, i);
			}
		}

                // copy item flags to modelindex2
                //      eg. empty, alt ammo etc
                dropped->viewheight |= ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS];
		dropped->last_fire = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE];
        }

	ent->client->weapon_sound = 0;
        ent->client->pers.cstats[CSTAT_LEFTHAND]= II_HANDS;
        ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = 0;
        ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS] = 0;
}


/*
==============
ThrowBodyAreaItem
==============
*/
edict_t *ThrowBodyAreaItem(edict_t *ent, int speed, int bodyarea)
{
	vec3_t	velocity, angles, org;
	vec3_t	forward, right, up;
	edict_t *dropped;
        gitem_t *item, *ammoItem;

	ammoItem = NULL;

        if (ent->client->pers.item_bodyareas[bodyarea] <= II_HANDS)
                return NULL;

        if (ent->client->pers.item_bodyareas[bodyarea] == II_CHAINGUN_PACK)
        {
	        if (ent->client->pers.cstats[CSTAT_LEFTHAND] == II_CHAINGUN)
	        {
			ThrowRightHandItem(ent, 150);
        		return NULL;
		}
	}
        else if (ent->client->pers.item_bodyareas[bodyarea] == II_CHAINGUN)
        {
	        if (ent->client->pers.cstats[CSTAT_LEFTHAND] == II_CHAINGUN_PACK)
	        {
			ThrowRightHandItem(ent, 150);
        		return NULL;
		}
        	return NULL;
	}
                
        item = GetItemByTag(ent->client->pers.item_bodyareas[bodyarea]);

        if (!item)
                return NULL;

        // calc angles
	VectorCopy(ent->client->v_angle, angles );
        if (ent->health <= 0)
                angles[PITCH] = 0;      // flat
	AngleVectors( angles, velocity, NULL, NULL );
        VectorScale(velocity, speed, velocity );

        // calc position
	AngleVectors (angles, forward, right, up);
	VectorCopy(ent->s.origin, org);

        if (forward[2] < 0)
                VectorMA(org, 2 * forward[2], up, org);
        else if (forward[2] > 0)
                VectorMA(org, -2 * forward[2], up, org);

        VectorMA(org, 16, up, org);
        VectorMA(org, 12, forward, org);
        VectorMA(org, 8, right, org);

        dropped = LaunchItem(ent, item, org, velocity);
        
        if (dropped)
        {
                dropped->count = ent->client->pers.item_quantities[bodyarea];
                dropped->nextthink = level.time + 5;
                dropped->s.frame = 0;

		ammoItem = GetItemByTag(item->ammoTag);
		if (ammoItem && (ammoItem->flags & IT_CLIP))
		{
        	        if ((item->quantity > 0) && (dropped->count > 0)) // the lower the ammo, the faster it disappears
	                        dropped->nextthink += (item->quantity / dropped->count) * 28;
			//else
	                        //dropped->s.frame += 2;
		}
		else
		{
			dropped->nextthink = level.time + 30;
		}

		// special handling for chaingun, which always comes with a pack
		if ((dropped->item->tag == II_CHAINGUN) || (dropped->item->tag == II_CHAINGUN_PACK))
		{
			int i;

			if (dropped->item->tag == II_CHAINGUN_PACK)
		                dropped->count = ent->client->pers.item_quantities[bodyarea];
			
			dropped->item = GetItemByTag(II_CHAINGUN);
			
			for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        		{
        			if (ent->client->pers.item_bodyareas[i] == II_CHAINGUN_PACK)
        			{
        				dropped->count = ent->client->pers.item_quantities[i];
					ent->client->pers.item_quantities[i] = 0;
					RemoveItem(ent, i);
				}
        			if (ent->client->pers.item_bodyareas[i] == II_CHAINGUN)
					RemoveItem(ent, i);
			}
		}

                // copy item flags to modelindex2
                //      eg. empty, alt ammo etc
                dropped->viewheight |= ent->client->pers.item_flags[bodyarea];
		dropped->last_fire = ent->client->pers.item_ammotypes[bodyarea];

		if ((item->tag == II_FRAG_HANDGRENADE) || (item->tag == II_EMP_HANDGRENADE))
		{
			if (dropped->count > 1)
			{
				dropped->count = 1;
				ent->client->pers.item_quantities[bodyarea] = 1;
		        	gi.linkentity (dropped);
				return dropped;
			}
		}
        }

        RemoveItem (ent, bodyarea);
        
        return dropped;
}

/*
==============
CmdDropWeapon
==============
*/
void CmdDropWeapon (edict_t *ent)
{
	int last_right, last_left;
	

        if (ent->movetype == MOVETYPE_NOCLIP)
                return; // spectators can't drop weapons, they're spectators!!

	last_right = ent->client->pers.cstats[CSTAT_RIGHTHAND];
	last_left = ent->client->pers.cstats[CSTAT_LEFTHAND];

        if ((ent->client->pers.cstats[CSTAT_RIGHTHAND] > II_HANDS) && (ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS))
        {
                qboolean right = false;
                qboolean left = false;

                if ((ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] <= ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO])
                 || (ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] == 0))
                        left = true;

                if ((ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] <= ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO])
                 || (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] == 0))
                        right = true;

                if (right && left)
                {
                        ThrowRightHandItem(ent, 50);
                        ThrowLeftHandItem(ent, 50);
                }
                else if (left)
                {
                        ThrowLeftHandItem(ent, 50);
                }
                else if (right)
                {
                        ThrowRightHandItem(ent, 50);
                }
        }
        else if (ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS)
        {
                ThrowLeftHandItem(ent, 50);
        }
        else if (ent->client->pers.cstats[CSTAT_RIGHTHAND] > II_HANDS)
        {
                ThrowRightHandItem(ent, 50);
        }

	// switch hands if we stil have a weapon in our other hand
	if (ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS)
	{
		ent->client->pers.cstats[CSTAT_RIGHTHAND] = ent->client->pers.cstats[CSTAT_LEFTHAND];
                ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO];
        	ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS];
	        ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE];
                ent->client->pers.cstats[CSTAT_LEFTHAND] = 0;
                ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = 0;
        	ent->client->pers.cstats[CSTAT_LEFTHAND_FLAGS] = 0;
	        ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE] = 0;
	}

        SetupItemModels(ent);

	if ((ent->client->pers.cstats[CSTAT_RIGHTHAND] <= II_HANDS) && (ent->client->pers.cstats[CSTAT_LEFTHAND] <= II_HANDS))
		AutoSwitchWeapon(ent, last_right, last_left);
}


/*
===============
DrawItemSelect
===============
*/
void DrawItemSelect(edict_t *ent)
{
        int     i, count;
        int     j, oj, c, stat;
        int     br, fr;
        gitem_t *item;

        // not selecting a weapon or respawned, forget about it...
        if (ent->client->itemSelect < 0)
                return;

	// don't display if dead or time is up
        if ((level.time - ent->client->cycleItems > CYCLE_ITEMS_TIME) || (ent->health < 1))
        {
                if (ent->client->ps.stats[STAT_HIGHLIGHT] != gi.imageindex ("i_null"))
                {
                        for (i = STAT_INV1; i <= STAT_INV7; i++)
                                ent->client->ps.stats[i] = gi.imageindex ("i_null");

                        ent->client->ps.stats[STAT_HIGHLIGHT] = gi.imageindex ("i_null");
                }
                ent->client->cycleItems = -10;
                return;
        }

	// showing weapon select clears pickup item display, but not the blend blob
        ent->client->pickup_msg_time = 0;

        count = 0;
        j = ent->client->itemSelect;
        for (i = 0; i < BA_MAX; i++)
        {
                if (ent->client->sortedItems[j] > 0)
                        count++;

                j++;
                if (j >= BA_MAX)
                        j = 0;
        }
        
        if (count < 1)
                return; // bah?
        
        if (count > 7)
                count = 7;

        ent->client->ps.stats[STAT_HIGHLIGHT] = gi.imageindex ("i_highlight");
	item = GetItemByTag(ent->client->sortedItems[ent->client->itemSelect]);
	if (item)
		ent->client->ps.stats[STAT_INV4] = gi.imageindex(item->icon);

        j = ent->client->itemSelect;

	if (count > 1)
	{
		for (c = 0; c < BA_MAX; c++)
		{
			j++;
			if (j >= BA_MAX)
				j = 0;
			
			item = GetItemByTag(ent->client->sortedItems[j]);
			if (item)
			{
				ent->client->ps.stats[STAT_INV5] = gi.imageindex(item->icon);
				break;
			}
		}
	}

	if (count > 3)
	{
		for (c = 0; c < BA_MAX; c++)
		{
			j++;
			if (j >= BA_MAX)
				j = 0;
			
			item = GetItemByTag(ent->client->sortedItems[j]);
			if (item)
			{
				ent->client->ps.stats[STAT_INV6] = gi.imageindex(item->icon);
				break;
			}
		}
	}

	if (count > 5)
	{
		for (c = 0; c < BA_MAX; c++)
		{
			j++;
			if (j >= BA_MAX)
				j = 0;
			
			item = GetItemByTag(ent->client->sortedItems[j]);
			if (item)
			{
				ent->client->ps.stats[STAT_INV7] = gi.imageindex(item->icon);
				break;
			}
		}
	}

        j = ent->client->itemSelect;

	if (count > 2)
	{
		for (c = 0; c < BA_MAX; c++)
		{
			j--;
			if (j < 0)
				j = BA_MAX - 1;
			
			item = GetItemByTag(ent->client->sortedItems[j]);
			if (item)
			{
				ent->client->ps.stats[STAT_INV3] = gi.imageindex(item->icon);
				break;
			}
		}
	}
	
	if (count > 4)
	{
		for (c = 0; c < BA_MAX; c++)
		{
			j--;
			if (j < 0)
				j = BA_MAX - 1;
			
			item = GetItemByTag(ent->client->sortedItems[j]);
			if (item)
			{
				ent->client->ps.stats[STAT_INV2] = gi.imageindex(item->icon);
				break;
			}
		}
	}

	if (count > 6)
	{
		for (c = 0; c < BA_MAX; c++)
		{
			j--;
			if (j < 0)
				j = BA_MAX - 1;
			
			item = GetItemByTag(ent->client->sortedItems[j]);
			if (item)
			{
				ent->client->ps.stats[STAT_INV1] = gi.imageindex(item->icon);
				break;
			}
		}
	}
}

/*
===============
ItemSelectable
===============
*/
static qboolean ItemSelectable(edict_t *ent) 
{
        if (ent->client->itemSelect >= BA_MAX)
        {
                return false;
	}

        if (ent->client->sortedItems[ent->client->itemSelect] < II_HANDS)
        {
                return false;
        }
                
        return true;
}

/*
===============
CmdNext
===============
*/
void CmdNext(edict_t *ent)
{
	int i, original;
	int curItem, x, count;

        if (ent->health < 1)
                return;

        if (ent->movetype == MOVETYPE_NOCLIP)
                return; // spectators can't drop weapons, they're spectators!!

        for (i = STAT_INV1; i <= STAT_INV7; i++)
                ent->client->ps.stats[i] = gi.imageindex ("i_null");

        ent->client->ps.stats[STAT_HIGHLIGHT] = gi.imageindex ("i_null");

	// sort the weapons into similar groups
	// start from primary hands weapon type, if there is one
	// if not, try the other hand. If all else fails, start from the II_HANDS + 1
        memset(ent->client->sortedItems, 0, sizeof(ent->client->sortedItems));
        memset(ent->client->sortedItemBodyAreas, 0, sizeof(ent->client->sortedItemBodyAreas));
        curItem = II_HANDS + 1;
        ent->client->sortedItems[0] = II_HANDS;
        ent->client->sortedItemBodyAreas[0] = BA_MAX;
        count = 0;

	if ((ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS) && (ent->client->pers.cstats[CSTAT_LEFTHAND] < II_MAX_WEAPONS))
		curItem = ent->client->pers.cstats[CSTAT_LEFTHAND];

	if ((ent->client->pers.cstats[CSTAT_RIGHTHAND] > II_HANDS) && (ent->client->pers.cstats[CSTAT_RIGHTHAND] < II_MAX_WEAPONS))
		curItem = ent->client->pers.cstats[CSTAT_RIGHTHAND];

        // weapons first
        for (x = II_HANDS + 1; x < II_MAX_WEAPONS; x++)
        {
                for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
                {
                        if (ent->client->pers.item_bodyareas[i] == curItem)
                        {
                                count++;
                                ent->client->sortedItems[count] = curItem;
                                ent->client->sortedItemBodyAreas[count] = i;
                        }
                }

                if (count >= BA_MAX)
                        break;

                curItem++;
                
                if (curItem >= II_MAX_WEAPONS)
                        curItem = II_HANDS + 1;
        }

        // rest of items
        curItem = ent->client->sortedItems[count] + 1;
        if (curItem <= II_MAX_WEAPONS)
                curItem = II_MAX_WEAPONS + 1;

        for (x = II_MAX_WEAPONS + 1; x < II_MAX; x++)
        {
                if (count >= BA_MAX)
                        break;

                for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
                {
                        if (ent->client->pers.item_bodyareas[i] == curItem)
                        {
                                count++;
                                if (count < BA_MAX)
                                {
                                        ent->client->sortedItems[count] = curItem;
                                        ent->client->sortedItemBodyAreas[count] = i;
                                }
                        }
                }

                if (count >= BA_MAX)
                        break;

                curItem++;
                
                if (curItem >= II_MAX)
                        curItem = II_MAX_WEAPONS + 1;
        }

        if (!ItemSelectable(ent))
                ent->client->itemSelect = 0;

        if (CYCLE_ITEMS_TIME < level.time - ent->client->cycleItems)
        {
                ent->client->cycleItems = level.time;
                return;
        }

        original = ent->client->itemSelect;

        for (i = 0; i < BA_MAX + 1; i++)
        {
                ent->client->itemSelect++;

                if (ent->client->itemSelect >= BA_MAX)
                {
                        ent->client->itemSelect = 0;
		}
		
                if (ItemSelectable(ent))
			break;
	}

        if (i == BA_MAX + 1)
        {
                ent->client->itemSelect = original;
	}
	
        if (!ItemSelectable(ent))
        {
                ent->client->itemSelect = 0;
	}
	
        ent->client->cycleItems = level.time;
}

/*
==============
CmdPrev
==============
*/
void CmdPrev(edict_t *ent)
{
	int i, original;
	int curItem, x, count;

	// sort the weapons into similar groups
	// start from primary hands weapon type, if there is one
	// if not, try the other hand. If all else fails, start from the II_HANDS + 1
        memset(ent->client->sortedItems, 0, sizeof(ent->client->sortedItems));
        memset(ent->client->sortedItemBodyAreas, 0, sizeof(ent->client->sortedItemBodyAreas));
        curItem = II_HANDS + 1;
        ent->client->sortedItems[0] = II_HANDS;
        ent->client->sortedItemBodyAreas[0] = BA_MAX;
        count = 0;

	if ((ent->client->pers.cstats[CSTAT_LEFTHAND] > II_HANDS) && (ent->client->pers.cstats[CSTAT_LEFTHAND] < II_MAX_WEAPONS))
		curItem = ent->client->pers.cstats[CSTAT_LEFTHAND];

	if ((ent->client->pers.cstats[CSTAT_RIGHTHAND] > II_HANDS) && (ent->client->pers.cstats[CSTAT_RIGHTHAND] < II_MAX_WEAPONS))
		curItem = ent->client->pers.cstats[CSTAT_RIGHTHAND];

        // weapons first
        for (x = II_HANDS + 1; x < II_MAX_WEAPONS; x++)
        {
                for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
                {
                        if (ent->client->pers.item_bodyareas[i] == curItem)
                        {
                                count++;
                                ent->client->sortedItems[count] = curItem;
                                ent->client->sortedItemBodyAreas[count] = i;
                        }
                }

                if (count >= BA_MAX)
                        break;

                curItem++;
                
                if (curItem >= II_MAX_WEAPONS)
                        curItem = II_HANDS + 1;
        }

        // rest of items
        curItem = ent->client->sortedItems[count] + 1;
        if (curItem <= II_MAX_WEAPONS)
                curItem = II_MAX_WEAPONS + 1;

        for (x = II_MAX_WEAPONS + 1; x < II_MAX; x++)
        {
                if (count >= BA_MAX)
                        break;

                for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
                {
                        if (ent->client->pers.item_bodyareas[i] == curItem)
                        {
                                count++;
                                if (count < BA_MAX)
                                {
                                        ent->client->sortedItems[count] = curItem;
                                        ent->client->sortedItemBodyAreas[count] = i;
                                }
                        }
                }

                if (count >= BA_MAX)
                        break;

                curItem++;
                
                if (curItem >= II_MAX)
                        curItem = II_MAX_WEAPONS + 1;
        }

        if (!ItemSelectable(ent))
                ent->client->itemSelect = 0;

        if (CYCLE_ITEMS_TIME < level.time - ent->client->cycleItems)
        {
                ent->client->cycleItems = level.time;
                return;
        }

        original = ent->client->itemSelect;

        for (i = 0; i < BA_MAX + 1; i++)
        {
                ent->client->itemSelect--;

                if (ent->client->itemSelect < 0)
                {
                        ent->client->itemSelect = BA_MAX - 1;
		}
		
                if (ItemSelectable(ent))
			break;
	}

        if (i == BA_MAX + 1)
        {
                ent->client->itemSelect = original;
	}

        if (!ItemSelectable(ent))
        {
                ent->client->itemSelect = 0;
	}

        ent->client->cycleItems = level.time;
}

qboolean HasSpawnFunc(char *classname);

/*
==================
CmdGive

Give items to a client
==================
*/
void CmdGive (edict_t *ent, char *all)
{
	vec3_t	velocity, angles, org;
	vec3_t	forward, right, up;
        edict_t         *given;
        gitem_t         *item;
	char		*name;
        int             i;
	void	(*spawn)(edict_t *ent);

        if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

        if (all)
        {
        	name = all;
	}
        else if (Q_stricmp(gi.argv(1), "all") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			item = &itemlist[i];
			if (item && item->classname)
				CmdGive (ent, item->classname);
		}
		return;
	}

        if (Q_stricmp(gi.argv(1), "health") == 0)
	{
                if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;

                return;
	}

	if (Q_stricmp(name, "weapon_supershotgun") == 0)
		name = "weapon_shotgun";
	if (Q_stricmp(name, "super shotgun") == 0)
		name = "weapon_shotgun";

	VectorCopy(ent->client->v_angle, angles );
	angles[PITCH] = 0;      // flat
	if (all)
	{
		angles[YAW] += crandom() * 180;
		AngleVectors(angles, velocity, NULL, NULL );
        	VectorScale(velocity, 150 + (random() * 300), velocity );
        	velocity[2] += 100 + random() * 100;
	}
	else
	{			
		AngleVectors(angles, velocity, NULL, NULL );
        	VectorScale(velocity, 150, velocity );
	}
	
        // calc position
	AngleVectors (angles, forward, right, up);
	VectorCopy(ent->s.origin, org);
        VectorMA(org, 32, forward, org);
        VectorMA(org, 16, up, org);

        item = FindItemByClassname (name);
        if (!item)
	{
		name = gi.argv(1);
                item = FindItem (name);
	}

	if (item)
	{
        	given = LaunchItem(ent, item, org, velocity);
	
        	if (given)
        	{
                	given->nextthink = level.time + 5;
                	given->s.frame = 0;
			given->count = item->quantity;
			given->nextthink = level.time + 30;

			// special handling for assault rifle (which has alt-ammo)
			if (given->item->tag == II_ASSAULT_RIFLE)
			{
				given->viewheight |= (SHF_EXTRA_AMMO1 | SHF_EXTRA_AMMO2 | SHF_EXTRA_AMMO3 | SHF_EXTRA_AMMO4);
			}

			given->last_fire = given->item->ammoTag;

        		gi.linkentity (given);
	        }
	        return;
	}

	if (HasSpawnFunc(name))
	{
		given = G_Spawn ();
		given->classname = name;
	        VectorCopy(angles, given->s.angles);
	        VectorMA(ent->s.origin, 128, forward, given->s.origin);
	        given->s.origin[2] += 16;

		if (Q_stricmp(name, "misc_actor") == 0)
			given->item = GetItemByTag(II_ASSAULT_RIFLE);
			
		if (given)
		{
			ED_CallSpawn (given);
			return;
		}
	}

	gi.cprintf (ent, PRINT_HIGH, "unknown entity\n");

}

/*
===============
UpdateInv
===============
*/
void UpdateInv(edict_t *ent)
{
	int i;

	/*
 	for (i = 0; i < MAX_WEAPONS + 1; i++)
	{
        	ent->client->itemSelect++;

                if (ent->client->itemSelect >= MAX_WEAPONS)
                {
                	ent->client->itemSelect = 0;
		}
		
		if (ItemSelectable(ent))
			break;
	}
	*/

	ent->client->cycleItems = -10;
	CmdPrev(ent);
	ent->client->cycleItems = -10;
	CmdNext(ent);
}

/*
===============
CmdDrop
===============
*/
void CmdDrop(edict_t *ent)
{
	if (level.time - ent->client->cycleItems <= CYCLE_ITEMS_TIME)
        {
                ent->client->cycleItems = level.time;
		ThrowBodyAreaItem(ent, 150, ent->client->sortedItemBodyAreas[ent->client->itemSelect]);
		UpdateInv(ent);
	}
	else
	{
		CmdDropWeapon(ent);
	}
}

/*
===============
CmdReload
===============
*/
void CmdReload(edict_t *ent)
{
	if (CanRightReload(ent))
		ent->client->weaponstate = W_START_RIGHT_RELOAD;
}

edict_t *zGetChaseTarget(edict_t *ent)
{
	int i;
	edict_t *other;

	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		
		if (other == ent) // chase target even if you're not spectator
			continue;

		if (other->client->deadflag)
			continue;

		if (other->movetype == MOVETYPE_NOCLIP)
			continue;
	
		if (other->inuse && !other->client->resp.spectator) {
			return other;
		}
	}
	gi.centerprintf(ent, "No other players to chase.");

	return NULL;
}

static void DecoyThink(edict_t *ent)
{
	ent->s.frame = FRAME_stand01 + (((ent->s.frame - FRAME_stand01) + 1) % (FRAME_stand40 - FRAME_stand01));
	ent->nextthink = level.time + FRAMETIME;
}

/*
===============
EndCoopView
===============
*/
void EndCoopView(edict_t *ent)
{
	edict_t *decoy = NULL;
	int i;

	ent->client->chase_target = NULL;
	ent->client->update_chase = false;
	ent->client->ps.stats[STAT_CHASE] = 0;
	ent->client->ps.pmove.pm_type = PM_SPECTATOR;
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

	if (ent->target_ent && !ent->client->deadflag)
	{
		decoy = ent->target_ent;
		
		VectorCopy(decoy->s.origin, ent->s.origin);
		VectorCopy(decoy->spawnpoint, ent->s.angles);
		VectorCopy (decoy->spawnpoint, ent->client->ps.viewangles);
		VectorCopy (decoy->spawnpoint, ent->client->v_angle);
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
		decoy->decoy = false;

		ent->movetype = MOVETYPE_WALK;
		ent->solid = SOLID_BBOX;
		ent->svflags &= ~SVF_NOCLIENT;
		SetupItemModels(ent);


		decoy->svflags |= SVF_NOCLIENT;
		decoy->solid = SOLID_NOT;
		decoy->think = G_FreeEdict;
		decoy->nextthink = level.time + 0.1;
		decoy->target_ent = NULL;
		decoy = NULL;
		ent->target_ent = NULL;
	}

	gi.linkentity (ent);
}

/*
===============
CmdCoopView
===============
*/
void CmdCoopView(edict_t *ent)
{
	edict_t *decoy = NULL;

	if (deathmatch->value)
		return;

	if (ent->client->chase_target)
	{
		edict_t *old = ent->client->chase_target;
		ChaseNext(ent);
		if ((ent->client->chase_target == old) || (ent->client->chase_target == ent))
			EndCoopView(ent);
	}
	else
	{
		ent->client->chase_target = zGetChaseTarget(ent);
		if (ent->client->chase_target)
		{
			if (!ent->client->deadflag)
			{
				decoy = G_Spawn();
				if (!decoy)
					return;
				decoy->s = ent->s;
				decoy->s.number = decoy - g_edicts;
				VectorCopy(ent->client->ps.viewangles, decoy->spawnpoint);
				decoy->svflags = ent->svflags;
				VectorCopy (ent->mins, decoy->mins);
				VectorCopy (ent->maxs, decoy->maxs);
				VectorCopy (ent->absmin, decoy->absmin);
				VectorCopy (ent->absmax, decoy->absmax);
				VectorCopy (ent->size, decoy->size);
				decoy->viewheight = ent->viewheight;
				decoy->solid = ent->solid;
				decoy->clipmask = ent->clipmask;
				decoy->owner = ent->owner;
				decoy->movetype = MOVETYPE_TOSS;
				decoy->takedamage = ent->takedamage;
				decoy->health = ent->health;
				decoy->nextthink = level.time + FRAMETIME;
				decoy->think = DecoyThink;
				decoy->target_ent = ent;
				decoy->decoy = true;
				gi.linkentity (decoy);
	
				ent->target_ent = decoy;
			}
			ent->client->ps.pmove.pm_type = PM_SPECTATOR;
			ent->client->update_chase = true;

			ent->movetype = MOVETYPE_NOCLIP;
			ent->solid = SOLID_NOT;
			ent->svflags |= SVF_NOCLIENT;
			ent->client->ps.gunindex = 0;
			gi.linkentity (ent);

			UpdateChaseCam(ent);
		}
	}
}

/*
=================
z_ClientCommand
=================
*/
qboolean z_ClientCommand (edict_t *ent)
{
	char	*cmd;

	cmd = gi.argv(0);

	if (Q_stricmp(cmd, "pact") == 0)
	{
		ent->client->action = true;
		return true;
        }
	else if (Q_stricmp(cmd, "mact") == 0)
	{
		ent->client->action = false;
		return true;
        }
	else if (Q_stricmp(cmd, "drop") == 0)
	{
                CmdDrop(ent);
		return true;
        }
	else if (Q_stricmp (cmd, "weapprev") == 0)
	{
                CmdPrev(ent);
		return true;
        }
	else if (Q_stricmp (cmd, "weapnext") == 0)
	{
                CmdNext(ent);
		return true;
        }
	else if (Q_stricmp (cmd, "give") == 0)
	{
		CmdGive(ent, NULL);
		return true;
	}
	else if (Q_stricmp (cmd, "reload") == 0)
	{
		CmdReload(ent);
		return true;
	}
	else if (Q_stricmp (cmd, "coopview") == 0)
	{
		CmdCoopView(ent);
		return true;
	}
	else if (Q_stricmp (cmd, "gotosecret") == 0)
	{
		if (sv_edit->value)
		{
			CmdGotoSecret(ent);
			return true;
		}
		else
			return false;
	}
        else
                return false;
}
